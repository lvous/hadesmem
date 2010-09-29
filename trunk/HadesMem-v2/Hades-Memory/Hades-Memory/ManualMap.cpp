/*
This file is part of HadesMem.
Copyright � 2010 RaptorFactor (aka Cypherjb, Cypher, Chazwazza). 
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

// Note: This file contains some code from the ReactOS project.
// Todo: Find and tag all such code.

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
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#pragma warning(pop)

// Hades
#include "PeLib.h"
#include "Module.h"
#include "Injector.h"
#include "MemoryMgr.h"
#include "ManualMap.h"
#include "Hades-Common/I18n.h"
#include "Hades-Common/EnsureCleanup.h"

namespace 
{
  std::array<ULONG, 16> const SectionCharacteristicsToProtect =
  {
    PAGE_NOACCESS,          /* 0 = NONE */
    PAGE_NOACCESS,          /* 1 = SHARED */
    PAGE_EXECUTE,           /* 2 = EXECUTABLE */
    PAGE_EXECUTE,           /* 3 = EXECUTABLE, SHARED */
    PAGE_READONLY,          /* 4 = READABLE */
    PAGE_READONLY,          /* 5 = READABLE, SHARED */
    PAGE_EXECUTE_READ,      /* 6 = READABLE, EXECUTABLE */
    PAGE_EXECUTE_READ,      /* 7 = READABLE, EXECUTABLE, SHARED */
    /*
    * FIXME? do we really need the WriteCopy field in segments? can't we use
    * PAGE_WRITECOPY here?
    */
    PAGE_READWRITE,         /* 8 = WRITABLE */
    PAGE_READWRITE,         /* 9 = WRITABLE, SHARED */
    PAGE_EXECUTE_READWRITE, /* 10 = WRITABLE, EXECUTABLE */
    PAGE_EXECUTE_READWRITE, /* 11 = WRITABLE, EXECUTABLE, SHARED */
    PAGE_READWRITE,         /* 12 = WRITABLE, READABLE */
    PAGE_READWRITE,         /* 13 = WRITABLE, READABLE, SHARED */
    PAGE_EXECUTE_READWRITE, /* 14 = WRITABLE, READABLE, EXECUTABLE */
    PAGE_EXECUTE_READWRITE, /* 15 = WRITABLE, READABLE, EXECUTABLE, SHARED */
  };
}

namespace Hades
{
  namespace Memory
  {
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
      TlsDir const MyTlsDir(MyPeFile);
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
        DWORD const TlsRet = m_pMemory->Call(reinterpret_cast<PBYTE>(
          RemoteBase) + reinterpret_cast<DWORD_PTR>(pCallback), TlsCallArgs);
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
        DWORD const ExpRet = m_pMemory->Call(ExportAddr, ExpArgs);
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
        std::vector<BYTE> const SectionData(DataStart, DataEnd);

        // Write section data to process
        m_pMemory->Write(TargetAddr, SectionData);

        // Get section characteristics
        DWORD SecCharacteristics = Current.GetCharacteristics();

        // Handle case where no explicit protection is provided. Infer 
        // protection flags from section type.
        if((SecCharacteristics & (IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | 
          IMAGE_SCN_MEM_WRITE)) == 0)
        {
          if(SecCharacteristics & IMAGE_SCN_CNT_CODE)
          {
            SecCharacteristics |= IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ;
          }

          if(SecCharacteristics & IMAGE_SCN_CNT_INITIALIZED_DATA)
          {
            SecCharacteristics |= IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;
          }

          if(SecCharacteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA)
          {
            SecCharacteristics |= IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;
          }
        }

        // Look up protection flags for section
        DWORD SecProtect = SectionCharacteristicsToProtect[
          SecCharacteristics >> 28];

        // Set the proper page protections for this section
        DWORD OldProtect;
        if (!VirtualProtectEx(m_pMemory->GetProcessHandle(), TargetAddr, 
          SizeOfRawData, SecProtect, &OldProtect))
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
      for (; pImpDesc->Characteristics; ++pImpDesc) 
      {
        // Check for forwarded imports
        if (pImpDesc->ForwarderChain != static_cast<DWORD>(-1) && 
          pImpDesc->ForwarderChain != 0)
        {
          BOOST_THROW_EXCEPTION(Error() << 
            ErrorFunction("ManualMap::FixImports") << 
            ErrorString("Image has unhandled forwarded imports."));
        }

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
        while (pThunkData->u1.AddressOfData != 0) 
        {
          // Get import data
          PIMAGE_IMPORT_BY_NAME const pNameImport = 
            static_cast<PIMAGE_IMPORT_BY_NAME>(MyPeFile.RvaToVa(
            static_cast<DWORD>(pThunkData->u1.AddressOfData)));

          // Get name of function
          std::string const ImpName(reinterpret_cast<char*>(pNameImport->
            Name));
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

      // Get end of reloc dir
      PVOID pRelocDirEnd = reinterpret_cast<PBYTE>(pRelocDir) + RelocDirSize;

      // Debug output
      std::wcout << "Fixing relocations." << std::endl;

      // Get image base
      ULONG_PTR const ImageBase = MyNtHeaders.GetImageBase();

      // Calcuate module delta
      LONG_PTR const Delta = reinterpret_cast<ULONG_PTR>(pRemoteBase) - 
        ImageBase;

      // Ensure we don't read into invalid data
      while (pRelocDir < pRelocDirEnd && pRelocDir->SizeOfBlock > 0)
      {
        // Get base of reloc dir
        PBYTE const RelocBase = static_cast<PBYTE>(MyPeFile.RvaToVa(
          pRelocDir->VirtualAddress));

        // Get number of relocs
        DWORD const NumRelocs = (pRelocDir->SizeOfBlock - sizeof(
          IMAGE_BASE_RELOCATION)) / sizeof(WORD); 

        // Get pointer to reloc data
        PWORD pRelocData = reinterpret_cast<PWORD>(pRelocDir + 1);

        // Loop over all relocation entries
        for(DWORD i = 0; i < NumRelocs; ++i, ++pRelocData) 
        {
          // Get reloc data
          BYTE RelocType = *pRelocData >> 12;
          WORD Offset = *pRelocData & 0xFFF;

          // Process reloc
          switch (RelocType)
          {
          case IMAGE_REL_BASED_ABSOLUTE:
            break;

          case IMAGE_REL_BASED_HIGHLOW:
            *reinterpret_cast<DWORD32*>(RelocBase + Offset) += 
              static_cast<DWORD32>(Delta);
            break;

          case IMAGE_REL_BASED_DIR64:
            *reinterpret_cast<DWORD64*>(RelocBase + Offset) += Delta;
            break;

          default:
            std::wcout << "Unsupported relocation type: " << RelocType << 
              std::endl;

            BOOST_THROW_EXCEPTION(Error() << 
              ErrorFunction("ManualMap::FixRelocations") << 
              ErrorString("Unsuppported relocation type."));
          }
        }

        // Advance to next reloc info block
        pRelocDir = reinterpret_cast<PIMAGE_BASE_RELOCATION>(pRelocData); 
      }
    }
  }
}
