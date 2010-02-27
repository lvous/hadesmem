#pragma once

// Windows API
#include <Windows.h>

// C++ Standard Library
#include <memory>
#include <string>
#include <vector>

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
      inline DWORD InjectDll(std::wstring const& Path, 
        std::string const& Export);

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
    DWORD Injector::InjectDll(std::wstring const& Path, 
      std::string const& Export)
    {
      // String to hold 'real' path to module
      std::wstring PathReal(Path);

      // Check whether we need to convert the path from a relative to 
      // an absolute
      if (PathReal[1] != ':')
      {
        // Get handle to self
        HMODULE const Self = reinterpret_cast<HMODULE>(&__ImageBase);

        // Get path to loader
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

        // ConvertStr path to loader to path to module
        std::wstring ModulePath(&SelfPath[0]);
        ModulePath = ModulePath.substr(0, ModulePath.rfind(L"\\") + 1);
        ModulePath.append(Path);

        // Set new path
        PathReal = ModulePath;
      }

      // Check path/file is valid
      if (GetFileAttributes(PathReal.c_str()) == INVALID_FILE_ATTRIBUTES)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(InjectorError() << 
          ErrorFunction("Injector::InjectDll") << 
          ErrorString("Could not find module file.") << 
          ErrorCodeWin(LastError));
      }

      // Get process handle
      HANDLE MyProcess = m_Memory.GetProcessHandle();

      // Convert path to lower case
      std::wstring const PathLower(I18n::ToLower<wchar_t>(PathReal));

      // Calculate the number of bytes needed for the DLL's pathname
      size_t const PathBufSize  = (Path.length() + 1) * sizeof(wchar_t);

      // Allocate space in the remote process for the pathname
      AllocAndFree LibFileRemote(m_Memory, PathBufSize);
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
      auto ModuleList(GetModuleList(m_Memory));
      // Look for target module
      auto Iter = std::find_if(ModuleList.begin(), ModuleList.end(), 
        [&PathLower] (std::shared_ptr<Module> const& MyModule) 
      {
        return I18n::ToLower<wchar_t>(MyModule->GetName()) == PathLower || 
          I18n::ToLower<wchar_t>(MyModule->GetPath()) == PathLower;
      });
      // Ensure target module was found
      if (Iter == ModuleList.end())
      {
        BOOST_THROW_EXCEPTION(InjectorError() << 
          ErrorFunction("Injector::InjectDll") << 
          ErrorString("Could not find module in remote process."));
      }

      // If no export has been specified then there's nothing left to do
      if (Export.empty() || Export == " ")
        return 0;

      // Load module as data so we can read the EAT locally
      EnsureFreeLibrary const MyModule(LoadLibraryExW(PathLower.c_str(), NULL, 
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

      // Base of remote module
      PBYTE ModRemote = reinterpret_cast<PBYTE>((*Iter)->GetBase());

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

      return ExitCodeExport;
    }
  }
}
