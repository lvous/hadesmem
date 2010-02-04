#pragma once

// Windows API
#include <Windows.h>
#include <TlHelp32.h>

// C++ Standard Library
#include <memory>
#include <string>

// Boost
#pragma warning(push, 1)
#pragma warning(disable: 4706)
#include <boost/exception.hpp>
#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>
#pragma warning(pop)

// HadesMem
#include "I18n.h"
#include "Error.h"
#include "EnsureCleanup.h"

namespace Hades
{
  namespace Memory
  {
    // Process exception type
    class ProcessError : public virtual HadesMemError 
    { };

    // Process managing class
    class Process : private boost::noncopyable
    {
    public:
      // Open process from process ID
      inline explicit Process(DWORD ProcID);

      // Open process from process name
      inline explicit Process(std::wstring const& ProcName);

      // Open process from window name (and optionally class)
      inline explicit Process(std::wstring const& WindowName, 
        std::wstring const* const ClassName);

      // Get process handle
      inline HANDLE GetHandle() const;
      
      // Get process ID
      inline DWORD GetID() const;

    private:
      // Open process given process id
      inline void Open(DWORD ProcID);

      // Gets the SeDebugPrivilege
      inline void GetSeDebugPrivilege();

      // Process handle
      EnsureCloseHandle m_Handle;

      // Process ID
      DWORD m_ID;
    };

    // Open process from process id
    Process::Process(DWORD ProcID) 
      : m_Handle(nullptr), 
      m_ID(0) 
    {
      // Get SeDebugPrivilege
      GetSeDebugPrivilege();

      // Open process
      Open(ProcID);
    }

    // Open process from process name
    Process::Process(std::wstring const& ProcName) 
      : m_Handle(nullptr), 
      m_ID(0) 
    {
      // Get SeDebugPrivilege
      GetSeDebugPrivilege();

      // Grab a new snapshot of the process
      EnsureCloseHandle const Snap(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 
        0));
      if (Snap == INVALID_HANDLE_VALUE)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(ProcessError() << 
          ErrorFunction("Process:Process") << 
          ErrorString("Could not get process snapshot.") << 
          ErrorCodeWin(LastError));
      }

      // Search for process
      PROCESSENTRY32 ProcEntry = { sizeof(ProcEntry) };
      bool Found = false;
      for (BOOL MoreMods = Process32First(Snap, &ProcEntry); MoreMods; 
        MoreMods = Process32Next(Snap, &ProcEntry)) 
      {
        Found = (I18n::ToLower<wchar_t>(ProcEntry.szExeFile) == 
          I18n::ToLower<wchar_t>(ProcName));
        if (Found)
        {
          break;
        }
      }

      // Check process was found
      if (!Found)
      {
        BOOST_THROW_EXCEPTION(ProcessError() << 
          ErrorFunction("Process:Process") << 
          ErrorString("Could not find process."));
      }

      // Open process
      m_ID = ProcEntry.th32ProcessID;
      Open(m_ID);
    }

    // Open process from window name and class
    Process::Process(std::wstring const& WindowName, 
      std::wstring const* const ClassName) 
      : m_Handle(nullptr), 
      m_ID(0) 
    {
      // Get SeDebugPrivilege
      GetSeDebugPrivilege();

      // Find window
      HWND const MyWnd = FindWindow(ClassName ? ClassName->c_str() : nullptr, 
        WindowName.c_str());
      if (!MyWnd)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(ProcessError() << 
          ErrorFunction("Process:Process") << 
          ErrorString("Could not find window.") << 
          ErrorCodeWin(LastError));
      }

      // Get process ID from window
      GetWindowThreadProcessId(MyWnd, &m_ID);
      if (!m_ID)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(ProcessError() << 
          ErrorFunction("Process:Process") << 
          ErrorString("Could not get process id from window.") << 
          ErrorCodeWin(LastError));
      }

      // Open process
      Open(m_ID);
    }

    // Open process given process id
    void Process::Open(DWORD ProcID)
    {
      // Open process
      m_Handle = OpenProcess(PROCESS_CREATE_THREAD | 
        PROCESS_QUERY_INFORMATION | 
        PROCESS_VM_OPERATION | 
        PROCESS_VM_READ | 
        PROCESS_VM_WRITE, 
        FALSE, 
        ProcID);
      if (!m_Handle)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(ProcessError() << 
          ErrorFunction("Process:Open") << 
          ErrorString("Could not open process.") << 
          ErrorCodeWin(LastError));
      }
    }

    // Gets the SeDebugPrivilege
    void Process::GetSeDebugPrivilege()
    {
      // Open current process token with adjust rights
      HANDLE TempToken = 0;
      BOOL const RetVal = OpenProcessToken(GetCurrentProcess(), 
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &TempToken);
      if (!RetVal) 
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(ProcessError() << 
          ErrorFunction("Process:GetSeDebugPrivilege") << 
          ErrorString("Could not open process token.") << 
          ErrorCodeWin(LastError));
      }
      EnsureCloseHandle const Token(TempToken);

      // Get the LUID for SE_DEBUG_NAME 
      LUID Luid = { NULL }; // Locally unique identifier
      if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &Luid)) 
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(ProcessError() << 
          ErrorFunction("Process:GetSeDebugPrivilege") << 
          ErrorString("Could not look up privilege value for SeDebugName.") << 
          ErrorCodeWin(LastError));
      }
      if (Luid.LowPart == NULL && Luid.HighPart == NULL) 
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(ProcessError() << 
          ErrorFunction("Process:GetSeDebugPrivilege") << 
          ErrorString("Could not get LUID for SeDebugName.") << 
          ErrorCodeWin(LastError));
      }

      // Process privileges
      TOKEN_PRIVILEGES Privileges = { NULL };
      // Set the privileges we need
      Privileges.PrivilegeCount = 1;
      Privileges.Privileges[0].Luid = Luid;
      Privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

      // Apply the adjusted privileges
      if (!AdjustTokenPrivileges(Token, FALSE, &Privileges, sizeof(Privileges), 
        NULL, NULL)) 
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(ProcessError() << 
          ErrorFunction("Process:GetSeDebugPrivilege") << 
          ErrorString("Could not adjust token privileges.") << 
          ErrorCodeWin(LastError));
      }
    }

    // Get process handle
    HANDLE Process::GetHandle() const
    {
      return m_Handle;
    }

    // Get process ID
    DWORD Process::GetID() const
    {
      return m_ID;
    }
  }
}
