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

// Hades
#include "PeFile.h"
#include "MemoryMgr.h"

namespace Hades
{
  namespace Memory
  {
    // Constructor
    PeFileAsData::PeFileAsData(MemoryMgr& MyMemory, PVOID Address) 
      : PeFile(MyMemory, Address)
    { }

    // Convert RVA to VA
    PVOID PeFileAsData::RvaToVa(DWORD Rva) const
    {
      // Ensure RVA is valid
      if (!Rva)
      {
        return nullptr;
      }

      // Get memory manager
      MemoryMgr* MyMemory = &GetMemoryMgr();

      // Get PE file base
      PBYTE const pBase = GetBase();

      // Get DOS header
      IMAGE_DOS_HEADER const DosHeader = MyMemory->Read<IMAGE_DOS_HEADER>(
        pBase);
      if (DosHeader.e_magic != IMAGE_DOS_SIGNATURE)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("PeFileAsData::RvaToVa") << 
          ErrorString("Invalid DOS header."));
      }

      // Get NT headers
      PBYTE const pNtHeaders = pBase + DosHeader.e_lfanew;
      IMAGE_NT_HEADERS const NtHeadersRaw = MyMemory->Read<IMAGE_NT_HEADERS>(
        pNtHeaders);
      if (NtHeadersRaw.Signature != IMAGE_NT_SIGNATURE)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("PeFileAsData::RvaToVa") << 
          ErrorString("Invalid NT headers."));
      }

      // Get first section header
      PIMAGE_SECTION_HEADER pSectionHeader = 
        reinterpret_cast<PIMAGE_SECTION_HEADER>(pNtHeaders + FIELD_OFFSET(
        IMAGE_NT_HEADERS, OptionalHeader) + NtHeadersRaw.FileHeader.
        SizeOfOptionalHeader);
      IMAGE_SECTION_HEADER SectionHeader = MyMemory->
        Read<IMAGE_SECTION_HEADER>(pSectionHeader);

      // Get number of sections
      WORD NumSections = NtHeadersRaw.FileHeader.NumberOfSections;

      // Loop over all sections
      for (WORD i = 0; i < NumSections; ++i)
      {
        // If RVA is in target file/raw data region perform adjustments to 
        // turn it into a VA.
        if (SectionHeader.VirtualAddress <= Rva && (SectionHeader.
          VirtualAddress + SectionHeader.Misc.VirtualSize) > Rva)
        {
          Rva -= SectionHeader.VirtualAddress;
          Rva += SectionHeader.PointerToRawData;

          return GetBase() + Rva;
        }

        // Get next section
        SectionHeader = MyMemory->Read<IMAGE_SECTION_HEADER>(++pSectionHeader);
      }

      // Conversion failed
      return nullptr;
    }

    // Constructor
    PeFile::PeFile(MemoryMgr& MyMemory, PVOID Address)
      : m_pMemory(&MyMemory), 
      m_pBase(static_cast<PBYTE>(Address))
    { }

    // Get memory manager
    MemoryMgr& PeFile::GetMemoryMgr() const
    {
      return *m_pMemory;
    }

    // Get base address
    PBYTE PeFile::GetBase() const
    {
      return m_pBase;
    }

    // Convert RVA to VA
    PVOID PeFile::RvaToVa(DWORD Rva) const
    {
      return Rva ? (m_pBase + Rva) : nullptr;
    }
  }
}
