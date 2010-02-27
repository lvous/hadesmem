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
#include "Memory.h"

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
      std::ifstream ModuleFile(Path.c_str(), std::ios::binary);
      if (!ModuleFile)
      {
        BOOST_THROW_EXCEPTION(ManualMapError() << 
          ErrorFunction("ManualMap::Map") << 
          ErrorString("Could not open module file."));
      }

      // Read file into buffer
      std::vector<BYTE> ModuleFileBuf((std::istreambuf_iterator<char>(
        ModuleFile)), std::istreambuf_iterator<char>());

      // Ensure file is a valid PE file
      auto pBase = &ModuleFileBuf[0];
      auto pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(pBase);
      if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
      {
        BOOST_THROW_EXCEPTION(ManualMapError() << 
          ErrorFunction("ManualMap::Map") << 
          ErrorString("Target file is not a valid PE file (DOS)."));
      }
      auto pNtHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(pBase + 
        pDosHeader->e_lfanew);
      if (pNtHeader->Signature != IMAGE_NT_SIGNATURE)
      {
        BOOST_THROW_EXCEPTION(ManualMapError() << 
          ErrorFunction("ManualMap::Map") << 
          ErrorString("Target file is not a valid PE file (NT)."));
      }

      // Fix import table if it exists
      auto ImpDircSize = pNtHeader->OptionalHeader.DataDirectory[
        IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
      if (ImpDircSize)
      {
        auto pImpDir = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(pBase + 
          RvaToFileOffset(pNtHeader, pNtHeader->OptionalHeader.
          DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress));

        // Todo: Fix imports here
        pImpDir;
      }

      // Fix relocations if applicable
      auto RelocDirSize = pNtHeader->OptionalHeader.DataDirectory[
        IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
      if (RelocDirSize)
      {
        auto pRelocDir = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(pBase + 
          RvaToFileOffset(pNtHeader, pNtHeader->OptionalHeader.
          DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress));

        // Todo: Fix relocs here
        pRelocDir;
      }

      // Todo: TLS support

      // Allocate memory for image
      PVOID RemoteBase = m_Memory.Alloc(pNtHeader->OptionalHeader.SizeOfImage);
      std::wcout << "RemoteBase: " << RemoteBase << std::endl;
      std::wcout << "SizeOfImage: " << pNtHeader->OptionalHeader.SizeOfImage << std::endl;

      // Write DOS header to process
      PBYTE DosHeaderStart = pBase;
      PBYTE DosHeaderEnd = DosHeaderStart + sizeof(IMAGE_DOS_HEADER);
      std::vector<BYTE> DosHeaderBuf(DosHeaderStart, DosHeaderEnd);
      m_Memory.Write(RemoteBase, DosHeaderBuf);

      // Write PE header to process
      PBYTE PeHeaderStart = reinterpret_cast<PBYTE>(pNtHeader);
      PBYTE PeHeaderEnd = PeHeaderStart + sizeof(pNtHeader->Signature) + 
        sizeof(pNtHeader->FileHeader) + pNtHeader->FileHeader.
        SizeOfOptionalHeader;
      std::vector<BYTE> PeHeaderBuf(PeHeaderStart, PeHeaderEnd);
      PBYTE TargetAddr = reinterpret_cast<PBYTE>(RemoteBase) + 
        pDosHeader->e_lfanew;
      std::wcout << "NT Header: " << TargetAddr << std::endl;
      m_Memory.Write(TargetAddr, PeHeaderBuf);
      
      // Write sections to process
      MapSections(pNtHeader, RemoteBase, ModuleFileBuf);

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
      // Loop over all sections 
      DWORD BytesWritten = 0; 
      PIMAGE_SECTION_HEADER pCurrent = IMAGE_FIRST_SECTION(pNtHeaders);
      for(WORD i = 0; i != pNtHeaders->FileHeader.NumberOfSections; ++i, 
        ++pCurrent) 
      { 
        std::wcout << "Current Section: " << (char*)pCurrent->Name << std::endl;

        // Once we've reached the SizeOfImage, the rest of the sections 
        // don't need to be mapped, if there are any. 
        if(BytesWritten >= pNtHeaders->OptionalHeader.SizeOfImage) 
        {
          break;
        }

        // Calculate target address for section in remote process
        PVOID TargetAddr = reinterpret_cast<PBYTE>(RemoteAddr) + 
          pCurrent->VirtualAddress;
        std::wcout << "Target Address: " << TargetAddr << std::endl;
        
        // Calculate start and end of section data in buffer
        PBYTE DataStart = const_cast<PBYTE>(&ModBuffer[0]) + 
          pCurrent->PointerToRawData;
        PBYTE DataEnd = DataStart + pCurrent->SizeOfRawData;

        // Get section data
        std::vector<BYTE> SectionData(DataStart, DataEnd);
        
        // Write section data to process
        m_Memory.Write(TargetAddr, SectionData);

        // Calculate virtual size of section
        DWORD VirtualSize = pCurrent->Misc.VirtualSize; 
        std::wcout << "VirtualSize: " << VirtualSize << std::endl;
        BytesWritten += VirtualSize;
        std::wcout << "BytesWritten: " << BytesWritten << std::endl;

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
  }
}
