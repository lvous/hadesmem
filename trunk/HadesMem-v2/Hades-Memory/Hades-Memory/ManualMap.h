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
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <iterator>
#include <iostream>

// Boost
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/filesystem.hpp>
#pragma warning(pop)

// Hades
#include "PeLib.h"
#include "Module.h"
#include "Injector.h"
#include "MemoryMgr.h"
#include "Hades-Common/EnsureCleanup.h"

namespace Hades
{
  namespace Memory
  {
    // Manual mapping class
    class ManualMap : private boost::noncopyable
    {
    public:
      // ManualMap exception type
      class Error : public virtual HadesMemError 
      { };

      // Constructor
      inline ManualMap(MemoryMgr& MyMemory);

      // Manually map DLL
      inline PVOID Map(boost::filesystem::path const& Path, 
        std::string const& Export = "", bool InjectHelper = true) const;

    private:
      // Map sections
      inline void MapSections(PeFile& MyPeFile, PVOID RemoteAddr) const;

      // Fix imports
      inline void FixImports(PeFile& MyPeFile) const;

      // Fix relocations
      inline void FixRelocations(PeFile& MyPeFile, PVOID RemoteAddr) const;

      // MemoryMgr instance
      MemoryMgr* m_pMemory;
    };

    // Constructor
    ManualMap::ManualMap(MemoryMgr& MyMemory) 
      : m_pMemory(&MyMemory)
    { }

    // Manually map DLL
    PVOID ManualMap::Map(boost::filesystem::path const& Path, 
      std::string const& Export, bool InjectHelper) const
    {
      // Open file for reading
      std::basic_ifstream<BYTE> ModuleFile(Path.c_str(), std::ios::binary | 
        std::ios::ate);
      if (!ModuleFile)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("ManualMap::Map") << 
          ErrorString("Could not open image file."));
      }

      // Get file size
      std::streamsize const FileSize = ModuleFile.tellg();

      // Allocate memory to hold file data
      // Doing this rather than copying data into a vector to avoid having to 
      // play with the page protection flags on the heap.
      PBYTE const pBase = static_cast<PBYTE>(VirtualAlloc(NULL, 
        static_cast<SIZE_T>(FileSize), MEM_COMMIT | MEM_RESERVE, 
        PAGE_READWRITE));
      if (!pBase)
      {
        DWORD const LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("ManualMap::Map") << 
          ErrorString("Could not allocate memory for image data.") << 
          ErrorCodeWin(LastError));
      }
      Windows::EnsureReleaseRegion const EnsureFreeLocalMod(pBase);

      // Seek to beginning of file
      ModuleFile.seekg(0, std::ios::beg);

      // Read file into memory
      ModuleFile.read(pBase, FileSize);

      // Create memory manager for local proc
      MemoryMgr MyMemoryLocal(GetCurrentProcessId());

      // Ensure file is a valid PE file
      std::wcout << "Performing PE file format validation." << std::endl;
      PeFileAsData MyPeFile(MyMemoryLocal, pBase);
      DosHeader const MyDosHeader(MyPeFile);
      NtHeaders const MyNtHeaders(MyPeFile);

      // Allocate memory for image
      std::wcout << "Allocating remote memory for image." << std::endl;
      PVOID const RemoteBase = m_pMemory->Alloc(MyNtHeaders.GetSizeOfImage());
      std::wcout << "Image base address: " << RemoteBase << "." << std::endl;
      std::wcout << "Image size: " << std::hex << MyNtHeaders.GetSizeOfImage() 
        << std::dec << "." << std::endl;

      // Get all TLS callbacks
      std::vector<PIMAGE_TLS_CALLBACK> TlsCallbacks;
      TlsDir MyTlsDir(MyPeFile);
      if (MyTlsDir.IsValid())
      {
        TlsCallbacks = MyTlsDir.GetCallbacks();
        std::for_each(TlsCallbacks.cbegin(), TlsCallbacks.cend(), 
          [&] (PIMAGE_TLS_CALLBACK pCurrent)
        {
          std::wcout << "TLS Callback: " << pCurrent << std::endl;
        });
      }

      // Process import table
      FixImports(MyPeFile);

      // Process relocations
      FixRelocations(MyPeFile, RemoteBase);

      // Write DOS header to process
      std::wcout << "Writing DOS header." << std::endl;
      std::wcout << "DOS Header: " << RemoteBase << std::endl;
      m_pMemory->Write(RemoteBase, *reinterpret_cast<PIMAGE_DOS_HEADER>(
        pBase));

      // Write NT headers to process
      PBYTE const NtHeadersStart = reinterpret_cast<PBYTE>(MyNtHeaders.
        GetBase());
      PBYTE const NtHeadersEnd = Section(MyPeFile, 0).GetBase();
      std::vector<BYTE> const PeHeaderBuf(NtHeadersStart, NtHeadersEnd);
      PBYTE const TargetAddr = static_cast<PBYTE>(RemoteBase) + MyDosHeader.
        GetNewHeaderOffset();
      std::wcout << "Writing NT header." << std::endl;
      std::wcout << "NT Header: " << TargetAddr << std::endl;
      m_pMemory->Write(TargetAddr, PeHeaderBuf);

      // Write sections to process
      MapSections(MyPeFile, RemoteBase);

      // Calculate module entry point
      PVOID const EntryPoint = static_cast<PBYTE>(RemoteBase) + 
        MyNtHeaders.GetAddressOfEntryPoint();
      std::wcout << "Entry Point: " << EntryPoint << "." << std::endl;

      // Get address of export in remote process
      PVOID const ExportAddr = m_pMemory->GetRemoteProcAddress(
        reinterpret_cast<HMODULE>(RemoteBase), Path, Export.c_str());
      std::wcout << "Export Address: " << ExportAddr << "." << std::endl;

      // Inject helper module
      if (InjectHelper)
      {
#if defined(_M_AMD64) 
        Map(L"Hades-MMHelper_AMD64.dll", "Initialize", false);
#elif defined(_M_IX86) 
        Map(L"Hades-MMHelper_IA32.dll", "_Initialize@4", false);
#else 
#error "Unsupported architecture."
#endif
      }

      // Call all TLS callbacks
      std::for_each(TlsCallbacks.cbegin(), TlsCallbacks.cend(), 
        [&] (PIMAGE_TLS_CALLBACK pCallback) 
      {
        std::vector<PVOID> TlsCallArgs;
        TlsCallArgs.push_back(0);
        TlsCallArgs.push_back(reinterpret_cast<PVOID>(DLL_PROCESS_ATTACH));
        TlsCallArgs.push_back(RemoteBase);
        DWORD TlsRet = m_pMemory->Call(reinterpret_cast<PBYTE>(RemoteBase) + 
          reinterpret_cast<DWORD_PTR>(pCallback), TlsCallArgs);
        std::wcout << "TLS Callback Returned: " << TlsRet << "." << std::endl;
      });

      // Call entry point
      std::vector<PVOID> EpArgs;
      EpArgs.push_back(0);
      EpArgs.push_back(reinterpret_cast<PVOID>(DLL_PROCESS_ATTACH));
      EpArgs.push_back(RemoteBase);
      DWORD const EpRet = m_pMemory->Call(EntryPoint, EpArgs);
      std::wcout << "Entry Point Returned: " << EpRet << "." << std::endl;

      // Call remote export (if specified)
      if (ExportAddr)
      {
        std::vector<PVOID> ExpArgs;
        ExpArgs.push_back(RemoteBase);
        DWORD ExpRet = m_pMemory->Call(ExportAddr, ExpArgs);
        std::wcout << "Export Returned: " << ExpRet << "." << std::endl;
      }

      // Return pointer to module in remote process
      return RemoteBase;
    }

    // Map sections
    void ManualMap::MapSections(PeFile& MyPeFile, PVOID RemoteAddr) const
    {
      // Debug output
      std::wcout << "Mapping sections." << std::endl;

      // Enumerate all sections
      SectionEnum MySectionEnum(MyPeFile);
      for (SectionEnum::SectionIter i(MySectionEnum); *i; ++i)
      {
        // Get section
        Section& Current = **i;

        // Get section name
        std::string const Name(Current.GetName());
        std::wcout << "Section Name: " << Name.c_str() << std::endl;

        // Calculate target address for section in remote process
        PVOID const TargetAddr = reinterpret_cast<PBYTE>(RemoteAddr) + 
          Current.GetVirtualAddress();
        std::wcout << "Target Address: " << TargetAddr << std::endl;

        // Calculate virtual size of section
        DWORD const VirtualSize = Current.GetVirtualSize(); 
        std::wcout << "Virtual Size: " << std::hex << VirtualSize << std::dec 
          << std::endl;

        // Calculate start and end of section data in buffer
        DWORD const SizeOfRawData = Current.GetSizeOfRawData();
        PBYTE const DataStart = MyPeFile.GetBase() + Current.
          GetPointerToRawData();
        PBYTE const DataEnd = DataStart + SizeOfRawData;

        // Get section data
        std::vector<BYTE> SectionData(DataStart, DataEnd);

        // Write section data to process
        m_pMemory->Write(TargetAddr, SectionData);

        // Set the proper page protections for this section
        DWORD OldProtect;
        if (!VirtualProtectEx(m_pMemory->GetProcessHandle(), TargetAddr, 
          SizeOfRawData, Current.GetCharacteristics() & 0x00FFFFFF, 
          &OldProtect))
        {
          DWORD const LastError = GetLastError();
          BOOST_THROW_EXCEPTION(Error() << 
            ErrorFunction("ManualMap::MapSections") << 
            ErrorString("Could not change page protections for section.") << 
            ErrorCodeWin(LastError));
        }
      }
    }

    // Fix imports
    void ManualMap::FixImports(PeFile& MyPeFile) const
    {
      // Get NT headers
      NtHeaders const MyNtHeaders(MyPeFile);

      // Get import data dir size and address
      DWORD const ImpDirSize = MyNtHeaders.GetDataDirectorySize(NtHeaders::
        DataDir_Import);
      PIMAGE_IMPORT_DESCRIPTOR pImpDesc = 
        static_cast<PIMAGE_IMPORT_DESCRIPTOR>(MyPeFile.RvaToVa(MyNtHeaders.
        GetDataDirectoryVirtualAddress(NtHeaders::DataDir_Import)));
      if (!ImpDirSize || !pImpDesc)
      {
        // Debug output
        std::wcout << "Image has no imports." << std::endl;

        // Nothing more to do
        return;
      }

      // Debug output
      std::wcout << "Fixing imports." << std::endl;

      // Loop through all the required modules
      for (; pImpDesc && pImpDesc->Name; ++pImpDesc) 
      {
        // Get module name
        char const* pModuleName = static_cast<char const*>(MyPeFile.RvaToVa(
          pImpDesc->Name));
        std::wstring const ModuleNameW(boost::lexical_cast<std::wstring, 
          std::string>(pModuleName));
        std::wstring const ModuleNameLowerW(boost::to_lower_copy(ModuleNameW));
        std::wcout << "Module Name: " << ModuleNameW << "." << std::endl;

        // Check whether dependent module is already loaded
        ModuleEnum MyModuleList(*m_pMemory);
        std::unique_ptr<Module> MyModule;
        for (ModuleEnum::ModuleListIter i(MyModuleList); *i; ++i)
        {
          Module& Current = **i;
          if (boost::to_lower_copy(Current.GetName()) == ModuleNameLowerW || 
            boost::to_lower_copy(Current.GetPath()) == ModuleNameLowerW)
          {
            MyModule = std::move(*i);
          }
        }

        // Module base address and name
        HMODULE CurModBase = 0;
        std::wstring CurModName;

        // If dependent module is not yet loaded then inject it
        if (!MyModule)
        {
          // Inject dependent DLL
          std::wcout << "Injecting dependent DLL." << std::endl;
          Injector MyInjector(*m_pMemory);
          CurModBase = MyInjector.InjectDll(ModuleNameW, false);
          CurModName = ModuleNameW;
        }
        else
        {
          CurModBase = MyModule->GetBase();
          CurModName = MyModule->GetName();
        }

        // Lookup the first import thunk for this module
        // Todo: Forwarded import support
        PIMAGE_THUNK_DATA pThunkData = static_cast<PIMAGE_THUNK_DATA>(
          MyPeFile.RvaToVa(pImpDesc->FirstThunk));
        while(pThunkData->u1.AddressOfData) 
        {
          // Get import data
          PIMAGE_IMPORT_BY_NAME const pNameImport = 
            static_cast<PIMAGE_IMPORT_BY_NAME>(MyPeFile.RvaToVa(
            pThunkData->u1.AddressOfData));

          // Get name of function
          std::string const ImpName(reinterpret_cast<char*>(pNameImport->Name));
          std::cout << "Function Name: " << ImpName << "." << std::endl;

          // Get function address in remote process
          bool const ByOrdinal = IMAGE_SNAP_BY_ORDINAL(pNameImport->Hint);
          LPCSTR Function = ByOrdinal ? reinterpret_cast<LPCSTR>(IMAGE_ORDINAL(
            pNameImport->Hint)) : reinterpret_cast<LPCSTR>(pNameImport->Name);
          FARPROC FuncAddr = m_pMemory->GetRemoteProcAddress(CurModBase, 
            CurModName, Function);

          // Set function address
          pThunkData->u1.Function = reinterpret_cast<DWORD_PTR>(FuncAddr);

          // Advance to next function
          ++pThunkData;
        }
      } 
    }

    // Fix relocations
    void ManualMap::FixRelocations(PeFile& MyPeFile, PVOID pRemoteBase) const
    {
      // Get NT headers
      NtHeaders const MyNtHeaders(MyPeFile);

      // Get import data dir size and address
      DWORD const RelocDirSize = MyNtHeaders.GetDataDirectorySize(NtHeaders::
        DataDir_BaseReloc);
      PIMAGE_BASE_RELOCATION pRelocDir = 
        static_cast<PIMAGE_BASE_RELOCATION>(MyPeFile.RvaToVa(MyNtHeaders.
        GetDataDirectoryVirtualAddress(NtHeaders::DataDir_BaseReloc)));
      if (!RelocDirSize || !pRelocDir)
      {
        // Debug output
        std::wcout << "Image has no relocations." << std::endl;

        // Nothing more to do
        return;
      }

      // Debug output
      std::wcout << "Fixing relocations." << std::endl;

      // Get image base
      ULONG_PTR const ImageBase = MyNtHeaders.GetImageBase();

      // Calcuate module delta
      LONG_PTR const Delta = reinterpret_cast<ULONG_PTR>(pRemoteBase) - 
        ImageBase;

      // Number of bytes processed
      DWORD BytesProcessed = 0; 

      // Ensure we don't read into invalid data
      while (BytesProcessed < RelocDirSize)
      {
        // Get base of reloc dir
        PVOID RelocBase = MyPeFile.RvaToVa(pRelocDir->VirtualAddress);

        // Get number of relocs
        DWORD const NumRelocs = (pRelocDir->SizeOfBlock - sizeof(
          IMAGE_BASE_RELOCATION)) / sizeof(WORD); 

        // Get pointer to reloc data
        WORD* pRelocData = reinterpret_cast<WORD*>(reinterpret_cast<DWORD_PTR>(
          pRelocDir) + sizeof(IMAGE_BASE_RELOCATION));

        // Loop over all relocation entries
        for(DWORD i = 0; i < NumRelocs; ++i, ++pRelocData) 
        {
          // Perform relocation if necessary
          if ((*pRelocData >> 12) & IMAGE_REL_BASED_HIGHLOW)
          {
            *reinterpret_cast<DWORD_PTR*>(static_cast<PBYTE>(RelocBase) + 
              (*pRelocData & 0x0FFF)) += Delta;
          }
        }

        // Add to number of bytes processed
        BytesProcessed += pRelocDir->SizeOfBlock;

        // Advance to next reloc info block
        pRelocDir = reinterpret_cast<PIMAGE_BASE_RELOCATION>(pRelocData); 
      }
    }
  }
}
