/*
This file is part of HadesMem.
Copyright © 2010 RaptorFactor (aka Cypherjb, Cypher, Chazwazza). 
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

#pragma once

// Windows API
#include <Windows.h>

// C++ Standard Library
#include <array>
#include <memory>
#include <string>
#include <vector>

// Boost
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#pragma warning(pop)

// Hades
#include "Module.h"
#include "MemoryMgr.h"

// Image base linker 'trick'
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace Hades
{
  namespace Memory
  {
    // DLL injection class
    class Injector
    {
    public:
      // Injector exception type
      class Error : public virtual HadesMemError 
      { };

      // Constructor
      inline Injector(MemoryMgr const& MyMemory);

      // Inject DLL
      inline HMODULE InjectDll(boost::filesystem::path const& Path, 
        bool PathResolution = true) const;

      // Call export
      inline DWORD CallExport(boost::filesystem::path const& ModulePath, 
        HMODULE ModuleRemote, std::string const& Export) const;

    private:
      // Disable assignment
      Injector& operator= (Injector const&);

      // MemoryMgr instance
      MemoryMgr const& m_Memory;
    };

    // Create process (as suspended) and inject DLL
    inline boost::shared_ptr<MemoryMgr> CreateAndInject(
      std::wstring const& Path, 
      std::wstring const& Args, 
      std::wstring const& Module, 
      std::string const& Export, 
      HMODULE* ModBaseOut = nullptr, 
      DWORD* ExportRetOut = nullptr)
    {
      // Set up args for CreateProcess
      STARTUPINFO StartInfo = { sizeof(StartInfo) };
      PROCESS_INFORMATION ProcInfo = { 0 };

      // Construct command line.
      std::wstring const CommandLine(L"\"" + Path + L"\" " + Args);
      // Copy command line to buffer
      std::vector<wchar_t> ProcArgs(CommandLine.begin(), CommandLine.end());
      ProcArgs.push_back(L'\0');

      // Attempt process creation
      if (!CreateProcess(Path.c_str(), &ProcArgs[0], NULL, NULL, FALSE, 
        CREATE_SUSPENDED, NULL, NULL, &StartInfo, &ProcInfo))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Injector::Error() << 
          ErrorFunction("CreateAndInject") << 
          ErrorString("Could not create process.") << 
          ErrorCodeWin(LastError));
      }

      // Ensure cleanup
      Windows::EnsureCloseHandle const ProcHandle(ProcInfo.hProcess);
      Windows::EnsureCloseHandle const ThreadHandle(ProcInfo.hThread);

      // Memory manager instance
      boost::shared_ptr<MemoryMgr> MyMemory;

      try
      {
        // Create memory manager
        MyMemory.reset(new Hades::Memory::MemoryMgr(ProcInfo.dwProcessId));

        // Create DLL injector
        Hades::Memory::Injector const MyInjector(*MyMemory);

        // Inject DLL
        HMODULE const ModBase = MyInjector.InjectDll(Module);
        if (ModBaseOut)
        {
          *ModBaseOut = ModBase;
        }

        // If export has been specified
        if (!Export.empty())
        {
          // Call remote export
          DWORD ExportRet = MyInjector.CallExport(Module, ModBase, Export);
          if (ExportRetOut)
          {
            *ExportRetOut = ExportRet;
          }
        }
      }
      // Catch exceptions
      catch (std::exception const& /*e*/)
      {
        // Terminate process if injection failed
        TerminateProcess(ProcInfo.hProcess, 0);
        // Rethrow exception
        throw;
      }

      // Success! Let the process continue execution.
      ResumeThread(ProcInfo.hThread);

      // Return memory manager instance for use in further calls
      return MyMemory;
    }

    // Constructor
    Injector::Injector(MemoryMgr const& MyMemory) 
      : m_Memory(MyMemory)
    { }

    // Inject DLL
    HMODULE Injector::InjectDll(boost::filesystem::path const& Path, 
      bool PathResolution) const
    {
      // String to hold 'real' path to module
      boost::filesystem::path PathReal(Path);

      // Check whether we need to convert the path from a relative to 
      // an absolute
      if (PathResolution && Path.is_relative())
      {
        // Get handle to self
        HMODULE const Self = reinterpret_cast<HMODULE>(&__ImageBase);

        // Get path to self
        std::array<wchar_t, MAX_PATH> SelfPathTemp;
        if (!GetModuleFileName(Self, &SelfPathTemp[0], MAX_PATH) || 
          GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
          DWORD LastError = GetLastError();
          BOOST_THROW_EXCEPTION(Error() << 
            ErrorFunction("Injector::InjectDll") << 
            ErrorString("Could not get path to self.") << 
            ErrorCodeWin(LastError));
        }

        // Path to self
        boost::filesystem::path const SelfPath(&SelfPathTemp[0]);

        // Convert relative path to absolute path
        PathReal = boost::filesystem::absolute(Path, SelfPath.parent_path());
      }

      // Ensure target file exists
      if (PathResolution && !boost::filesystem::exists(PathReal))
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("Injector::InjectDll") << 
          ErrorString("Could not find module file."));
      }

      // Calculate the number of bytes needed for the DLL's pathname
      size_t const PathBufSize  = (PathReal.wstring().length() + 1) * 
        sizeof(wchar_t);

      // Allocate space in the remote process for the pathname
      AllocAndFree const LibFileRemote(m_Memory, PathBufSize);
      if (!LibFileRemote.GetAddress())
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("Injector::InjectDll") << 
          ErrorString("Could not allocate memory.") << 
          ErrorCodeWin(LastError));
      }

      // Copy the DLL's pathname to the remote process' address space
      m_Memory.Write(LibFileRemote.GetAddress(), PathReal.wstring());

      // Get the real address of LoadLibraryW in Kernel32.dll
      HMODULE const hKernel32 = GetModuleHandleW(L"Kernel32");
      if (!hKernel32)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("Injector::InjectDll") << 
          ErrorString("Could not get handle to Kernel32.") << 
          ErrorCodeWin(LastError));
      }
      FARPROC const pLoadLibraryW = GetProcAddress(hKernel32, "LoadLibraryW");
      if (!pLoadLibraryW)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("Injector::InjectDll") << 
          ErrorString("Could not get pointer to LoadLibraryW.") << 
          ErrorCodeWin(LastError));
      }

      // Load module in remote process using LoadLibraryW
      std::vector<PVOID> Args;
      Args.push_back(LibFileRemote.GetAddress());
      if (!m_Memory.Call(pLoadLibraryW, Args))
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("Injector::InjectDll") << 
          ErrorString("Call to LoadLibraryW in remote process failed."));
      }

      // Get path as lowercase string
      std::wstring const PathRealLower(boost::to_lower_copy(PathReal.
        wstring()));

      // Look for target module
      ModuleEnum MyModuleList(m_Memory);
      boost::shared_ptr<Module> MyModule;
      for (ModuleEnum::ModuleListIter MyIter(MyModuleList); *MyIter; ++MyIter)
      {
        if (PathResolution)
        {
          if (boost::filesystem::equivalent((*MyIter)->GetPath(), PathReal))
          {
            MyModule = *MyIter;
          }
        }
        else
        {
          if (boost::to_lower_copy((*MyIter)->GetName()) == PathRealLower || 
            boost::to_lower_copy((*MyIter)->GetPath()) == PathRealLower)
          {
            MyModule = *MyIter;
          }
        }
      }

      // Ensure target module was found
      if (!MyModule)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("Injector::InjectDll") << 
          ErrorString("Could not find module in remote process."));
      }

      // Return module base
      return MyModule->GetBase();
    }

    // Call export
    DWORD Injector::CallExport(boost::filesystem::path const& ModulePath, 
      HMODULE ModuleRemote, std::string const& Export) const
    {
      // Get export address
      FARPROC const pExportAddr(m_Memory.GetRemoteProcAddress(ModuleRemote, 
        ModulePath, Export));

      // Create a remote thread that calls the desired export
      std::vector<PVOID> ExportArgs;
      ExportArgs.push_back(ModuleRemote);
      return m_Memory.Call(pExportAddr, ExportArgs);
    }
  }
}
