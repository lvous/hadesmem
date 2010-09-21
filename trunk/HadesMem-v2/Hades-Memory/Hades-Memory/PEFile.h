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

// HadesMem
#include "Error.h"
#include "MemoryMgr.h"

namespace Hades
{
  namespace Memory
  {
    // PE file format wrapper
    class PeFile
    {
    public:
      // PeFile exception type
      class Error : public virtual HadesMemError 
      { };

      // Constructor
      inline PeFile(MemoryMgr& MyMemory, PVOID Address);

      // Get memory manager
      inline MemoryMgr& GetMemoryMgr() const;

      // Get base address
      inline PBYTE GetBase() const;

      // Convert RVA to VA
      inline virtual PVOID RvaToVa(DWORD Rva);

    private:
      // Memory instance
      MemoryMgr* m_pMemory;

      // Base address
      PBYTE m_pBase;
    };

    // PE file format wrapper
    // For PE files mapped as data rather than images
    class PeFileAsData : public PeFile
    {
    public:
      // Constructor
      inline PeFileAsData(MemoryMgr& MyMemory, PVOID Address);

      // Convert RVA to VA
      inline virtual PVOID RvaToVa(DWORD Rva);
    };

    // Constructor
    PeFileAsData::PeFileAsData(MemoryMgr& MyMemory, PVOID Address) 
      : PeFile(MyMemory, Address)
    { }

    // Convert RVA to VA
    PVOID PeFileAsData::RvaToVa(DWORD Rva)
    {
      // Ensure RVA is valid
      if (!Rva)
      {
        return nullptr;
      }

      // Get memory manager
      MemoryMgr* MyMemory(&GetMemoryMgr());

      // Get PE file base
      PBYTE pBase(GetBase());

      // Get DOS header
      auto DosHeader(MyMemory->Read<IMAGE_DOS_HEADER>(pBase));
      if (DosHeader.e_magic != IMAGE_DOS_SIGNATURE)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("PeFileAsData::RvaToVa") << 
          ErrorString("Invalid DOS header."));
      }

      // Get NT headers
      auto pNtHeaders(pBase + DosHeader.e_lfanew);
      auto NtHeadersRaw(MyMemory->Read<IMAGE_NT_HEADERS>(pNtHeaders));
      if (NtHeadersRaw.Signature != IMAGE_NT_SIGNATURE)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("PeFileAsData::RvaToVa") << 
          ErrorString("Invalid NT headers."));
      }

      // Get first section header
      auto pSectionHeader(reinterpret_cast<PIMAGE_SECTION_HEADER>(pNtHeaders + 
        sizeof(NtHeadersRaw.FileHeader) + sizeof(NtHeadersRaw.Signature) + 
        NtHeadersRaw.FileHeader.SizeOfOptionalHeader));
      auto SectionHeader(MyMemory->Read<IMAGE_SECTION_HEADER>(pSectionHeader));

      // Get number of sections
      WORD NumSections = NtHeadersRaw.FileHeader.NumberOfSections;

      // Loop over all sections
      for (WORD i = 0; i < NumSections; ++i)
      {
        // If RVA is in target file/raw data region perform adjustments to 
        // turn it into a VA.
        if (SectionHeader.PointerToRawData <= Rva && (SectionHeader.
          PointerToRawData + SectionHeader.SizeOfRawData) > Rva)
        {
          Rva -= SectionHeader.PointerToRawData;
          Rva += SectionHeader.VirtualAddress;

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
    PVOID PeFile::RvaToVa(DWORD Rva)
    {
      return Rva ? (m_pBase + Rva) : nullptr;
    }
  }
}
