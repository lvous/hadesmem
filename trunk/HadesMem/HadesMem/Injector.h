#pragma once

// Windows API
#include <Windows.h>

// C++ Standard Library
#include <memory>
#include <string>
#include <vector>

// Boost
#include <boost/filesystem.hpp>

// HadesMem
#include "Module.h"
#include "Memory.h"

// Image base linker 'trick'
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace Hades
{
  namespace Memory
  {
    // Injector exception type
    class InjectorError : public virtual HadesMemError 
    { };
    
    // DLL injection class
    class Injector
    {
    public:
      // Constructor
      inline Injector(MemoryMgr const& MyMemory);

      // Inject DLL
      inline HMODULE InjectDll(std::wstring const& Path, 
        std::string const& Export, DWORD& ReturnValue, 
        bool PathResolution = true);

    private:
      // Disable assignment
      Injector& operator= (Injector const&);

      // MemoryMgr instance
      MemoryMgr const& m_Memory;
    };

    // Constructor
    Injector::Injector(MemoryMgr const& MyMemory) 
      : m_Memory(MyMemory)
    { }

    // Inject DLL
    HMODULE Injector::InjectDll(std::wstring const& Path, 
      std::string const& Export, DWORD& ReturnValue, bool PathResolution)
    {
      // String to hold 'real' path to module
      std::wstring PathReal(Path);

      // Check whether we need to convert the path from a relative to 
      // an absolute
      if (PathResolution && !boost::filesystem::wpath(Path).
        has_root_directory())
      {
        // Get handle to self
        HMODULE const Self = reinterpret_cast<HMODULE>(&__ImageBase);

        // Get path to self
        std::vector<wchar_t> SelfPath(MAX_PATH);
        if (!GetModuleFileName(Self, &SelfPath[0], MAX_PATH) || 
          GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
          DWORD LastError = GetLastError();
          BOOST_THROW_EXCEPTION(InjectorError() << 
            ErrorFunction("Injector::InjectDll") << 
            ErrorString("Could not get path to self.") << 
            ErrorCodeWin(LastError));
        }

        // Convert relative path to absolute path
        PathReal = boost::filesystem::complete(Path, boost::filesystem::wpath(
          &SelfPath[0]).parent_path()).file_string();
      }

      // Convert path to lower case
      PathReal = I18n::ToLower<wchar_t>(PathReal);

      // Ensure target file exists
      if (PathResolution && !boost::filesystem::exists(PathReal))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(InjectorError() << 
          ErrorFunction("Injector::InjectDll") << 
          ErrorString("Could not find module file.") << 
          ErrorCodeWin(LastError));
      }

      // Get process handle
      HANDLE const MyProcess = m_Memory.GetProcessHandle();

      // Calculate the number of bytes needed for the DLL's pathname
      size_t const PathBufSize  = (Path.length() + 1) * sizeof(wchar_t);

      // Allocate space in the remote process for the pathname
      AllocAndFree const LibFileRemote(m_Memory, PathBufSize);
      if (!LibFileRemote.GetAddress())
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(InjectorError() << 
          ErrorFunction("Injector::InjectDll") << 
          ErrorString("Could not allocate memory.") << 
          ErrorCodeWin(LastError));
      }

      // Copy the DLL's pathname to the remote process' address space
      m_Memory.Write(LibFileRemote.GetAddress(), PathReal);

      // Get the real address of LoadLibraryW in Kernel32.dll
      HMODULE const hKernel32 = GetModuleHandleW(L"Kernel32");
      if (!hKernel32)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(InjectorError() << 
          ErrorFunction("Injector::InjectDll") << 
          ErrorString("Could not get handle to Kernel32.") << 
          ErrorCodeWin(LastError));
      }
      PTHREAD_START_ROUTINE const pLoadLibraryW = 
        reinterpret_cast<PTHREAD_START_ROUTINE>(GetProcAddress(hKernel32, 
        "LoadLibraryW"));
      if (!pLoadLibraryW)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(InjectorError() << 
          ErrorFunction("Injector::InjectDll") << 
          ErrorString("Could not get pointer to LoadLibraryW.") << 
          ErrorCodeWin(LastError));
      }

      // Create a remote thread that calls LoadLibraryW
      EnsureCloseHandle const ThreadInject(CreateRemoteThread(MyProcess, 
        nullptr, 0, pLoadLibraryW, LibFileRemote.GetAddress(), 0, NULL));
      if (!ThreadInject)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(InjectorError() << 
          ErrorFunction("Injector::InjectDll") << 
          ErrorString("Could not create injection remote thread.") << 
          ErrorCodeWin(LastError));
      }

      // Wait for the remote thread to terminate
      if (WaitForSingleObject(ThreadInject, INFINITE) != WAIT_OBJECT_0)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(InjectorError() << 
          ErrorFunction("Injector::InjectDll") << 
          ErrorString("Could not wait for injection remote thread.") << 
          ErrorCodeWin(LastError));
      }

      // Get thread exit code
      DWORD ExitCodeInject = 0;
      if (!GetExitCodeThread(ThreadInject, &ExitCodeInject))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(InjectorError() << 
          ErrorFunction("Injector::InjectDll") << 
          ErrorString("Could not get export remote thread exit code.") << 
          ErrorCodeWin(LastError));
      }
      if (!ExitCodeInject)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(InjectorError() << 
          ErrorFunction("Injector::InjectDll") << 
          ErrorString("Call to LoadLibraryW in remote process failed.") << 
          ErrorCodeWin(LastError));
      }
      
      // Get module list for remote process
      auto const ModuleList(GetModuleList(m_Memory));
      // Look for target module
      auto const Iter = std::find_if(ModuleList.begin(), ModuleList.end(), 
        [&PathReal] (std::shared_ptr<Module> const& MyModule) 
      {
        return I18n::ToLower<wchar_t>(MyModule->GetName()) == PathReal || 
          I18n::ToLower<wchar_t>(MyModule->GetPath()) == PathReal;
      });
      // Ensure target module was found
      if (Iter == ModuleList.end())
      {
        BOOST_THROW_EXCEPTION(InjectorError() << 
          ErrorFunction("Injector::InjectDll") << 
          ErrorString("Could not find module in remote process."));
      }

      // Base of remote module
      PBYTE const ModRemote = reinterpret_cast<PBYTE>((*Iter)->GetBase());

      // If no export has been specified then there's nothing left to do
      if (Export.empty() || Export == " ")
      {
        return reinterpret_cast<HMODULE>(ModRemote);
      }

      // Load module as data so we can read the EAT locally
      EnsureFreeLibrary const MyModule(LoadLibraryExW(PathReal.c_str(), NULL, 
        DONT_RESOLVE_DLL_REFERENCES));
      if (!MyModule)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(InjectorError() << 
          ErrorFunction("Injector::InjectDll") << 
          ErrorString("Could not load module locally.") << 
          ErrorCodeWin(LastError));
      }

      // Get module pointer
      PBYTE const Module = static_cast<PBYTE>(static_cast<PVOID>(MyModule));

      // Find export
      FARPROC pExportAddr = GetProcAddress(MyModule, Export.c_str());

      // Nothing found, throw exception
      if (!pExportAddr)
      {
        BOOST_THROW_EXCEPTION(InjectorError() << 
          ErrorFunction("Injector::InjectDll") << 
          ErrorString("Could not find export."));
      }

      // If image is relocated we need to recalculate the address
      if (Module != ModRemote)
      {
        pExportAddr = reinterpret_cast<FARPROC>(ModRemote + 
          (reinterpret_cast<PBYTE>(pExportAddr) - Module));
      }

      // Convert local address to remote address
      PTHREAD_START_ROUTINE const pfnThreadRtn = 
        reinterpret_cast<PTHREAD_START_ROUTINE>(pExportAddr);

      // Create a remote thread that calls the desired export
      EnsureCloseHandle const ThreadExport(CreateRemoteThread(MyProcess, NULL, 
        0, pfnThreadRtn, ModRemote, 0, NULL));
      if (!ThreadExport) 
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(InjectorError() << 
          ErrorFunction("Injector::InjectDll") << 
          ErrorString("Could not create export remote thread.") << 
          ErrorCodeWin(LastError));
      }

      // Wait for the remote ThreadExport to terminate
      if (WaitForSingleObject(ThreadExport, INFINITE) != WAIT_OBJECT_0)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(InjectorError() << 
          ErrorFunction("Injector::InjectDll") << 
          ErrorString("Could not wait for export remote thread.") << 
          ErrorCodeWin(LastError));
      }

      // Get thread exit code
      DWORD ExitCodeExport = 0;
      if (!GetExitCodeThread(ThreadExport, &ExitCodeExport))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(InjectorError() << 
          ErrorFunction("Injector::InjectDll") << 
          ErrorString("Could not get export remote thread exit code.") << 
          ErrorCodeWin(LastError));
      }
      
      // Set return value
      ReturnValue = ExitCodeExport;

      // Return base address of remote module
      return reinterpret_cast<HMODULE>(ModRemote);
    }
  }
}
