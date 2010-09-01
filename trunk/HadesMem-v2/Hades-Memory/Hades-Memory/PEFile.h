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
#include "MemoryMgr.h"

namespace Hades
{
  namespace Memory
  {
    class PeFile
    {
    public:
      enum PeFileType
      {
        PEFileMem, 
        PEFileDisk
      };

      inline PeFile(MemoryMgr const& MyMemory, PeFileType Type, PVOID Address);
      inline PeFile(MemoryMgr const& MyMemory, PeFileType Type, 
        DWORD_PTR Address);

      MemoryMgr const& GetMemoryMgr() const;

      PVOID GetBase() const;

    private:
      PeFile& operator= (PeFile const&);

      MemoryMgr const& m_Memory;

      PeFileType m_Type;

      PVOID m_pBase;
    };

    PeFile::PeFile(MemoryMgr const& MyMemory, PeFileType Type, PVOID Address)
      : m_Memory(MyMemory), 
      m_Type(Type), 
      m_pBase(Address)
    { }

    PeFile::PeFile(MemoryMgr const& MyMemory, PeFileType Type, 
      DWORD_PTR Address)
      : m_Memory(MyMemory), 
      m_Type(Type), 
      m_pBase(reinterpret_cast<PVOID>(Address))
    { }

    MemoryMgr const& PeFile::GetMemoryMgr() const
    {
      return m_Memory;
    }

    PVOID PeFile::GetBase() const
    {
      return m_pBase;
    }
  }
}
