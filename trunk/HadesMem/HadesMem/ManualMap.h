#pragma once

// Windows API
#include <Windows.h>
#include <atlbase.h>
#include <atlfile.h>

// C++ Standard Library
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <iterator>
#include <iostream>

// HadesMem
#include "I18n.h"
#include "Memory.h"
#include "Module.h"

namespace Hades
{
  namespace Memory
  {
    // ManualMap exception type
    class ManualMapError : public virtual HadesMemError 
    { };

    // Manual mapping class
    // Credits to Darawk for the original implementation this is based off
    class ManualMap
    {
    public:
      // Constructor
      inline ManualMap(MemoryMgr const& MyMemory);

      // Manually map a DLL
      inline PVOID Map(std::wstring const& Path);

    private:
      // Convert RVA to file offset
      inline DWORD RvaToFileOffset(PIMAGE_NT_HEADERS pNtHeaders, DWORD Rva);

      // Map sections
      inline void MapSections(PIMAGE_NT_HEADERS pNtHeaders, PVOID RemoteAddr, 
        std::vector<BYTE> const& ModBuffer);

      // Fix imports
      inline void FixImports(std::vector<BYTE>& ModBuffer, 
        PIMAGE_NT_HEADERS pNtHeaders, PIMAGE_IMPORT_DESCRIPTOR pImpDesc);

      // Get address of export in remote process
      inline FARPROC GetRemoteProcAddress(HMODULE RemoteMod, 
        std::wstring const& Module, std::string const& Function);

      // Disable assignment
      ManualMap& operator= (ManualMap const&);

      // MemoryMgr instance
      MemoryMgr const& m_Memory;
    };

    // Constructor
    ManualMap::ManualMap(MemoryMgr const& MyMemory) 
      : m_Memory(MyMemory)
    { }

    // Manually map a DLL
    PVOID ManualMap::Map(std::wstring const& Path)
    {
      // Open file for reading
      std::wcout << "Opening module file for reading." << std::endl;
      std::ifstream ModuleFile(Path.c_str(), std::ios::binary);
      if (!ModuleFile)
      {
        BOOST_THROW_EXCEPTION(ManualMapError() << 
          ErrorFunction("ManualMap::Map") << 
          ErrorString("Could not open module file."));
      }

      // Read file into buffer
      std::wcout << "Reading module file into buffer." << std::endl;
      std::vector<BYTE> ModuleFileBuf((std::istreambuf_iterator<char>(
        ModuleFile)), std::istreambuf_iterator<char>());

      // Ensure file is a valid PE file
      std::wcout << "Performing PE file format validation." << std::endl;
      auto pBase = &ModuleFileBuf[0];
      auto pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(pBase);
      if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
      {
        BOOST_THROW_EXCEPTION(ManualMapError() << 
          ErrorFunction("ManualMap::Map") << 
          ErrorString("Target file is not a valid PE file (DOS)."));
      }
      auto pNtHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(pBase + 
        pDosHeader->e_lfanew);
      if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE)
      {
        BOOST_THROW_EXCEPTION(ManualMapError() << 
          ErrorFunction("ManualMap::Map") << 
          ErrorString("Target file is not a valid PE file (NT)."));
      }

      // Fix import table if it exists
      auto ImpDirSize = pNtHeaders->OptionalHeader.DataDirectory[
        IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
      if (ImpDirSize)
      {
        auto pImpDir = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(pBase + 
          RvaToFileOffset(pNtHeaders, pNtHeaders->OptionalHeader.
          DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress));

        FixImports(ModuleFileBuf, pNtHeaders, pImpDir);
      }

      // Fix relocations if applicable
      auto RelocDirSize = pNtHeaders->OptionalHeader.DataDirectory[
        IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
      if (RelocDirSize)
      {
        auto pRelocDir = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(pBase + 
          RvaToFileOffset(pNtHeaders, pNtHeaders->OptionalHeader.
          DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress));

        // Todo: Fix relocs here
        pRelocDir;
      }

      // Todo: TLS support

      // Allocate memory for image
      std::wcout << "Allocating memory for module." << std::endl;
      PVOID RemoteBase = m_Memory.Alloc(pNtHeaders->OptionalHeader.
        SizeOfImage);
      std::wcout << "Module base address: " << RemoteBase << "." << std::endl;
      std::wcout << "Module size: " << pNtHeaders->OptionalHeader.SizeOfImage 
        << "." << std::endl;

      // Write DOS header to process
      PBYTE DosHeaderStart = pBase;
      PBYTE DosHeaderEnd = DosHeaderStart + sizeof(IMAGE_DOS_HEADER);
      std::vector<BYTE> DosHeaderBuf(DosHeaderStart, DosHeaderEnd);
      std::wcout << "Writing DOS header." << std::endl;
      std::wcout << "DOS Header: " << DosHeaderStart << std::endl;
      m_Memory.Write(RemoteBase, DosHeaderBuf);

      // Write PE header to process
      PBYTE PeHeaderStart = reinterpret_cast<PBYTE>(pNtHeaders);
      PBYTE PeHeaderEnd = PeHeaderStart + sizeof(pNtHeaders->Signature) + 
        sizeof(pNtHeaders->FileHeader) + pNtHeaders->FileHeader.
        SizeOfOptionalHeader;
      std::vector<BYTE> PeHeaderBuf(PeHeaderStart, PeHeaderEnd);
      PBYTE TargetAddr = reinterpret_cast<PBYTE>(RemoteBase) + 
        pDosHeader->e_lfanew;
      std::wcout << "Writing NT header." << std::endl;
      std::wcout << "NT Header: " << TargetAddr << std::endl;
      m_Memory.Write(TargetAddr, PeHeaderBuf);
      
      // Write sections to process
      MapSections(pNtHeaders, RemoteBase, ModuleFileBuf);

      // Todo: Write EP calling stub to process here

      // Todo: Call EIP calling stub here

      // Return pointer to module in remote process
      return RemoteBase;
    }

    // Convert RVA to file offset
    DWORD ManualMap::RvaToFileOffset(PIMAGE_NT_HEADERS pNtHeaders, 
      DWORD Rva)
    {
      // Get number of sections
      WORD NumSections = pNtHeaders->FileHeader.NumberOfSections;
      // Get pointer to first section
      PIMAGE_SECTION_HEADER pCurrentSection = IMAGE_FIRST_SECTION(pNtHeaders);

      // Loop over all sections
      for (WORD i = 0; i < NumSections; ++i)
      {
        // Check if the RVA may be inside the current section
        if (pCurrentSection->VirtualAddress <= Rva)
        {
          // Check if the RVA is inside the current section
          if ((pCurrentSection->VirtualAddress + pCurrentSection->
            Misc.VirtualSize) > Rva)
          {
            // Convert RVA to file (raw) offset
            Rva -= pCurrentSection->VirtualAddress;
            Rva += pCurrentSection->PointerToRawData;
            return Rva;
          }
        }

        // Advance to next section
        ++pCurrentSection;
      }

      // Could not perform conversion. Return number signifying an error.
      return static_cast<DWORD>(-1);
    }

    // Map sections
    void ManualMap::MapSections(PIMAGE_NT_HEADERS pNtHeaders, PVOID RemoteAddr, 
      std::vector<BYTE> const& ModBuffer)
    {
      // Debug output
      std::wcout << "Mapping sections." << std::endl;

      // Loop over all sections 
      PIMAGE_SECTION_HEADER pCurrent = IMAGE_FIRST_SECTION(pNtHeaders);
      for(WORD i = 0; i != pNtHeaders->FileHeader.NumberOfSections; ++i, 
        ++pCurrent) 
      {
        // Debug output
        std::string Name(pCurrent->Name, pCurrent->Name + 8);
        std::wcout << "Section Name: " << Name.c_str() << std::endl;

        // Calculate target address for section in remote process
        PVOID TargetAddr = reinterpret_cast<PBYTE>(RemoteAddr) + 
          pCurrent->VirtualAddress;
        std::wcout << "Target Address: " << TargetAddr << std::endl;

        // Calculate virtual size of section
        DWORD VirtualSize = pCurrent->Misc.VirtualSize; 
        std::wcout << "Virtual Size: " << TargetAddr << std::endl;

        // Calculate start and end of section data in buffer
        PBYTE DataStart = const_cast<PBYTE>(&ModBuffer[0]) + 
          pCurrent->PointerToRawData;
        PBYTE DataEnd = DataStart + pCurrent->SizeOfRawData;

        // Get section data
        std::vector<BYTE> SectionData(DataStart, DataEnd);
        
        // Write section data to process
        m_Memory.Write(TargetAddr, SectionData);

        // Set the proper page protections for this section
        DWORD OldProtect;
        if (!VirtualProtectEx(m_Memory.GetProcessHandle(), TargetAddr, 
          VirtualSize, pCurrent->Characteristics & 0x00FFFFFF, &OldProtect))
        {
          DWORD LastError = GetLastError();
          BOOST_THROW_EXCEPTION(ManualMapError() << 
            ErrorFunction("ManualMap::MapSections") << 
            ErrorString("Could not change page protections for section.") << 
            ErrorCodeWin(LastError));
        }
      }
    }

    // Fix imports
    void ManualMap::FixImports(std::vector<BYTE>& ModBuffer, 
      PIMAGE_NT_HEADERS pNtHeaders, PIMAGE_IMPORT_DESCRIPTOR pImpDesc)
    {
      // Debug output
      std::wcout << "Fixing imports." << std::endl;

      // Loop through all the required modules
      char* ModuleName = nullptr;
      PBYTE ModBase = &ModBuffer[0];
      for (; pImpDesc && pImpDesc->Name; ++pImpDesc) 
      {
        // Get module name
        ModuleName = reinterpret_cast<char*>(ModBase + RvaToFileOffset(
          pNtHeaders, pImpDesc->Name));
        std::string ModuleNameA(ModuleName);
        std::wstring ModuleNameW(boost::lexical_cast<std::wstring>(ModuleName));
        std::wcout << "Module Name: " << ModuleNameW << "." << std::endl;

        // Ensure dependent module is already loaded
        // Todo: Recursive loading.
        auto ModuleList(GetModuleList(m_Memory));
        auto ModIter = std::find_if(ModuleList.begin(), ModuleList.end(), 
          [&ModuleNameW] (std::shared_ptr<Module> Current)
        {
          return I18n::ToLower<wchar_t>(Current->GetName()) == 
            I18n::ToLower<wchar_t>(ModuleNameW) || 
            I18n::ToLower<wchar_t>(Current->GetPath()) == 
            I18n::ToLower<wchar_t>(ModuleNameW);
        });
        if (ModIter == ModuleList.end())
        {
          BOOST_THROW_EXCEPTION(ManualMapError() << 
            ErrorFunction("ManualMap::FixImports") << 
            ErrorString("Dependent module not loaded in remote process."));
        }
        HMODULE CurModBase = (*ModIter)->GetBase();
        std::wstring const CurModName((*ModIter)->GetName());

        // Lookup the first import thunk for this module
        // Todo: Support for forwarded functions
        // Todo: Support functions imported by ordinal
        auto pThunkData = reinterpret_cast<PIMAGE_THUNK_DATA>(ModBase + 
          RvaToFileOffset(pNtHeaders, pImpDesc->FirstThunk));
        while(pThunkData->u1.AddressOfData) 
        {
          // Get import data
          auto pNameImport = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(ModBase + 
            RvaToFileOffset(pNtHeaders, pThunkData->u1.AddressOfData));

          // Skip imports by ordinal for now
          if (IMAGE_SNAP_BY_ORDINAL(pNameImport->Hint))
          {
            std::wcout << "Skipping import by ordinal." << std::endl;
            continue;
          }

          // Get name of function
          std::string const ImpName(reinterpret_cast<char*>(pNameImport->Name));
          std::cout << "Function Name: " << ImpName << "." << std::endl;

          // Get function address in remote process
          FARPROC FuncAddr = GetRemoteProcAddress(CurModBase, CurModName, 
            reinterpret_cast<char*>(pNameImport->
            Name));

          // Calculate function delta
          DWORD_PTR FuncDelta = reinterpret_cast<DWORD_PTR>(FuncAddr) - 
            reinterpret_cast<DWORD_PTR>(CurModBase);

          // Set function delta
          pThunkData->u1.Function = FuncDelta;

          // Advance to next function
          pThunkData++;
        }
      } 
    }

    // Get address of export in remote process
    FARPROC ManualMap::GetRemoteProcAddress(HMODULE RemoteMod, 
      std::wstring const& ModulePath, std::string const& Function)
    {
      // Load module as data so we can read the EAT locally
      EnsureFreeLibrary LocalMod(LoadLibraryExW(ModulePath.c_str(), NULL, 
        DONT_RESOLVE_DLL_REFERENCES));
      if (!LocalMod)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(ManualMapError() << 
          ErrorFunction("ManualMap::GetRemoteProcAddress") << 
          ErrorString("Could not load module locally.") << 
          ErrorCodeWin(LastError));
      }

      // Find target function in module
      FARPROC LocalFunc = GetProcAddress(LocalMod, Function.c_str());
      if (!LocalFunc)
      {
        return nullptr;
      }
      
      // Calculate function delta
      DWORD_PTR FuncDelta = reinterpret_cast<DWORD_PTR>(LocalFunc) - 
        reinterpret_cast<DWORD_PTR>(static_cast<HMODULE>(LocalMod));

      // Calculate function location in remote process
      auto RemoteFunc = reinterpret_cast<FARPROC>(reinterpret_cast<DWORD_PTR>(
        RemoteMod) + FuncDelta);

      // Return remote function location
      return RemoteFunc;
    }
  }
}
