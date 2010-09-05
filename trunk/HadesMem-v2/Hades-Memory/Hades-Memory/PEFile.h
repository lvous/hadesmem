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
    class PeFile
    {
    public:
      // PeFile exception type
      class Error : public virtual HadesMemError 
      { };

      inline PeFile(MemoryMgr const& MyMemory, PVOID Address);

      MemoryMgr const& GetMemoryMgr() const;

      PVOID GetBase() const;

      virtual PVOID RvaToVa(DWORD Rva);

    private:
      PeFile& operator= (PeFile const&);

      MemoryMgr const& m_Memory;

      PVOID m_pBase;
    };

    class PeFileAsData : public PeFile
    {
    public:
      inline PeFileAsData(MemoryMgr const& MyMemory, PVOID Address);

      virtual PVOID RvaToVa(DWORD Rva);
    };

    PeFileAsData::PeFileAsData(MemoryMgr const& MyMemory, PVOID Address) 
      : PeFile(MyMemory, Address)
    { }

    PVOID PeFileAsData::RvaToVa(DWORD Rva)
    {
      if (!Rva)
      {
        return nullptr;
      }

      MemoryMgr const& MyMemory(GetMemoryMgr());

      PBYTE pBase = static_cast<PBYTE>(GetBase());

      auto DosHeader(MyMemory.Read<IMAGE_DOS_HEADER>(pBase));
      if (DosHeader.e_magic != IMAGE_DOS_SIGNATURE)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("PeFileAsData::RvaToVa") << 
          ErrorString("Invalid DOS header."));
      }

      auto pNtHeaders(pBase + DosHeader.e_lfanew);

      auto NtHeadersRaw(MyMemory.Read<IMAGE_NT_HEADERS>(pNtHeaders));
      if (NtHeadersRaw.Signature != IMAGE_NT_SIGNATURE)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("PeFileAsData::RvaToVa") << 
          ErrorString("Invalid NT headers."));
      }

      auto pSectionHeader(reinterpret_cast<PIMAGE_SECTION_HEADER>(pNtHeaders + 
        sizeof(NtHeadersRaw.FileHeader) + sizeof(NtHeadersRaw.Signature) + 
        NtHeadersRaw.FileHeader.SizeOfOptionalHeader));

      auto SectionHeader(MyMemory.Read<IMAGE_SECTION_HEADER>(pSectionHeader));

      WORD NumSections = NtHeadersRaw.FileHeader.NumberOfSections;

      for (WORD i = 0; i < NumSections; ++i)
      {
        if (SectionHeader.PointerToRawData <= Rva && (SectionHeader.
          PointerToRawData + SectionHeader.SizeOfRawData) > Rva)
        {
          Rva -= SectionHeader.PointerToRawData;
          Rva += SectionHeader.VirtualAddress;

          return reinterpret_cast<PBYTE>(GetBase()) + Rva;
        }

        SectionHeader = MyMemory.Read<IMAGE_SECTION_HEADER>(++pSectionHeader);
      }

      return nullptr;
    }

    PeFile::PeFile(MemoryMgr const& MyMemory, PVOID Address)
      : m_Memory(MyMemory), 
      m_pBase(Address)
    { }

    MemoryMgr const& PeFile::GetMemoryMgr() const
    {
      return m_Memory;
    }

    PVOID PeFile::GetBase() const
    {
      return m_pBase;
    }

    PVOID PeFile::RvaToVa(DWORD Rva)
    {
      return Rva ? static_cast<PBYTE>(m_pBase) + Rva : nullptr;
    }
  }
}
