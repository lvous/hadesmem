/*
This file is part of HadesMem.
Copyright (C) 2011 Joshua Boyce (a.k.a. RaptorFactor).
<http://www.raptorfactor.com/> <raptorfactor@raptorfactor.com>

HadesMem is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

HadesMem is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with HadesMem.  If not, see <http://www.gnu.org/licenses/>.
*/

// Hades
#include <HadesMemory/Process.hpp>
#include <HadesMemory/Detail/StringBuffer.hpp>
#include <HadesMemory/Detail/EnsureCleanup.hpp>

// Windows API
#include <Windows.h>
#include <psapi.h>
#include <Shellapi.h>

namespace HadesMem
{
  // Process implementation
  class Process::Impl
  {
  public:
    // Constructor
    explicit Impl(DWORD ProcID) 
      : m_Handle(), 
      m_ID(0), 
      m_IsWoW64(false)
    {
      // Open process
      if (GetCurrentProcessId() == ProcID)
      {
        m_Handle.reset(new EnsureCloseHandle(GetCurrentProcess()));
        m_ID = GetCurrentProcessId();
      }
      else
      {
        Open(m_ID);
      }
      
      // Set WoW64 member
      SetWoW64();
    }
    
    // Swap
    void swap(Impl& Rhs) /*noexcept*/
    {
      m_Handle.swap(Rhs.m_Handle);
      std::swap(m_ID, Rhs.m_ID);
      std::swap(m_IsWoW64, Rhs.m_IsWoW64);
    }
      
    // Copy constructor
    Impl(Impl const& Rhs) /*noexcept*/
      : m_Handle(Rhs.m_Handle), 
      m_ID(Rhs.m_ID), 
      m_IsWoW64(Rhs.m_IsWoW64)
    { }
    
    // Copy-assignment operator
    Impl& operator=(Impl Rhs) /*noexcept*/
    {
      this->swap(Rhs);
      return *this;
    }

    // Get process handle
    HANDLE Impl::GetHandle() const /*noexcept*/
    {
      return *m_Handle;
    }

    // Get process ID
    DWORD Impl::GetID() const /*noexcept*/
    {
      return m_ID;
    }
      
    // Get process path
    std::wstring Impl::GetPath() const
    {
      // Note: The QueryFullProcessImageName API is more efficient and 
      // reliable but is only available on Vista+.
      DWORD const PathSize = 32767;
      std::wstring Path;
      if (!GetModuleFileNameEx(*m_Handle, nullptr, MakeStringBuffer(
        Path, PathSize), PathSize))
      {
        DWORD const LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("Process::GetPath") << 
          ErrorString("Could not get path.") << 
          ErrorCodeWinLast(LastError));
      }
      
      return Path;
    }
    
    // Is WoW64 process
    bool Impl::IsWoW64() const /*noexcept*/
    {
      return m_IsWoW64;
    }
    
  private:
    // Get WoW64 status of process and set member var
    void Impl::SetWoW64()
    {
      // Get WoW64 status of self
      BOOL IsWoW64Me = FALSE;
      if (!IsWow64Process(GetCurrentProcess(), &IsWoW64Me))
      {
        DWORD const LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("Process::SetWoW64") << 
          ErrorString("Could not detect WoW64 status of current process.") << 
          ErrorCodeWinLast(LastError));
      }

      // Get WoW64 status of target process
      BOOL IsWoW64 = FALSE;
      if (!IsWow64Process(*m_Handle, &IsWoW64))
      {
        DWORD const LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("Process::SetWoW64") << 
          ErrorString("Could not detect WoW64 status of target process.") << 
          ErrorCodeWinLast(LastError));
      }
      
      // Set WoW64 status
      m_IsWoW64 = (IsWoW64 != FALSE);

      // Disable x86 -> x64 process manipulation
      if (IsWoW64Me && !IsWoW64)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("Process::SetWoW64") << 
          ErrorString("x86 -> x64 process manipulation is currently "
          "unsupported."));
      }
    }

    // Open process given process id
    void Impl::Open(DWORD ProcID)
    {
      // Open process
      m_Handle.reset(new EnsureCloseHandle(
        OpenProcess(PROCESS_CREATE_THREAD | 
        PROCESS_QUERY_INFORMATION | 
        PROCESS_VM_OPERATION | 
        PROCESS_VM_READ | 
        PROCESS_VM_WRITE, 
        FALSE, 
        ProcID)));
      if (!m_Handle)
      {
        DWORD const LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("Process::Open") << 
          ErrorString("Could not open process.") << 
          ErrorCodeWinLast(LastError));
      }
    }
    
    // Process handle
    // Note: Using shared pointer because handle does not need to be unique, 
    // and copying it may throw, so sharing it makes exception safe code 
    // far easier to write.
    std::shared_ptr<EnsureCloseHandle> m_Handle;
  
    // Process ID
    DWORD m_ID;
    
    // Is WoW64 process
    bool m_IsWoW64;
  };
  
  // Open process from process id
  Process::Process(DWORD ProcID) 
    : m_pImpl(new Impl(ProcID))
  { }

  // Copy constructor
  Process::Process(Process const& Other) /*noexcept*/
    : m_pImpl(new Impl(*Other.m_pImpl))
  { }

  // Copy assignment
  Process& Process::operator=(Process Other) /*noexcept*/
  {
    this->swap(Other); 
    return *this;
  }
  
  // Destructor
  // Note: An empty destructor is required so the compiler can see Impl's 
  // destructor.
  Process::~Process() /*noexcept*/
  { }
  
  // Swap
  void Process::swap(Process& Other) /*noexcept*/
  {
    m_pImpl.swap(Other.m_pImpl);
  }

  // Get process handle
  HANDLE Process::GetHandle() const /*noexcept*/
  {
    return m_pImpl->GetHandle();
  }

  // Get process ID
  DWORD Process::GetID() const /*noexcept*/
  {
    return m_pImpl->GetID();
  }
    
  // Get process path
  std::wstring Process::GetPath() const
  {
    return m_pImpl->GetPath();
  }
  
  // Is WoW64 process
  bool Process::IsWoW64() const /*noexcept*/
  {
    return m_pImpl->IsWoW64();
  }
  
  // Create process
  Process CreateProcess(std::wstring const& Path, 
    std::wstring const& Params, 
    std::wstring const& WorkingDir)
  {
    // Start process
    SHELLEXECUTEINFO ExecInfo;
    ZeroMemory(&ExecInfo, sizeof(ExecInfo));
    ExecInfo.cbSize = sizeof(ExecInfo);
#ifndef SEE_MASK_NOASYNC 
#define SEE_MASK_NOASYNC 0x00000100
#endif
    ExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC;
    ExecInfo.lpFile = Path.empty() ? nullptr : Path.c_str();
    ExecInfo.lpParameters = Params.empty() ? nullptr : Params.c_str();
    ExecInfo.lpDirectory = WorkingDir.empty() ? nullptr : WorkingDir.c_str();
    ExecInfo.nShow = SW_SHOWNORMAL;
    if (!ShellExecuteEx(&ExecInfo))
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Process::Error() << 
        ErrorFunction("CreateProcess") << 
        ErrorString("Could not create process.") << 
        ErrorCodeWinLast(LastError));
    }
    
    // Ensure handle is closed
    EnsureCloseHandle const MyProc(ExecInfo.hProcess);
    
    // Return process object
    return Process(GetProcessId(MyProc));
  }

  // Gets the SeDebugPrivilege
  void GetSeDebugPrivilege()
  {
    // Open current process token with adjust rights
    HANDLE TempToken = 0;
    BOOL const RetVal = OpenProcessToken(GetCurrentProcess(), 
      TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &TempToken);
    if (!RetVal) 
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Process::Error() << 
        ErrorFunction("GetSeDebugPrivilege") << 
        ErrorString("Could not open process token.") << 
        ErrorCodeWinLast(LastError));
    }
    EnsureCloseHandle const Token(TempToken);

    // Get the LUID for SE_DEBUG_NAME 
    LUID Luid = { 0, 0 }; // Locally unique identifier
    if (!LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &Luid)) 
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Process::Error() << 
        ErrorFunction("GetSeDebugPrivilege") << 
        ErrorString("Could not look up privilege value for "
        "SeDebugName.") << 
        ErrorCodeWinLast(LastError));
    }
    if (Luid.LowPart == 0 && Luid.HighPart == 0) 
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Process::Error() << 
        ErrorFunction("GetSeDebugPrivilege") << 
        ErrorString("Could not get LUID for SeDebugName.") << 
        ErrorCodeWinLast(LastError));
    }

    // Process privileges
    TOKEN_PRIVILEGES Privileges;
    ZeroMemory(&Privileges, sizeof(Privileges));
    // Set the privileges we need
    Privileges.PrivilegeCount = 1;
    Privileges.Privileges[0].Luid = Luid;
    Privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // Apply the adjusted privileges
    if (!AdjustTokenPrivileges(Token, FALSE, &Privileges, 
      sizeof(Privileges), nullptr, nullptr)) 
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Process::Error() << 
        ErrorFunction("GetSeDebugPrivilege") << 
        ErrorString("Could not adjust token privileges.") << 
        ErrorCodeWinLast(LastError));
    }
    
    // Ensure privileges were adjusted
    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Process::Error() << 
        ErrorFunction("GetSeDebugPrivilege") << 
        ErrorString("Could not assign all privileges.") << 
        ErrorCodeWinLast(LastError));
    }
  }
}
