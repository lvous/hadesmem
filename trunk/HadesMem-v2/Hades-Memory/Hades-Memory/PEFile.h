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

// Windows
#include <Windows.h>

// Boost
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/noncopyable.hpp>
#pragma warning(pop)

// Hades
#include "Fwd.h"
#include "Error.h"

namespace Hades
{
  namespace Memory
  {
    // PE file format wrapper
    class PeFile : private boost::noncopyable
    {
    public:
      // PeFile exception type
      class Error : public virtual HadesMemError 
      { };

      // Constructor
      PeFile(MemoryMgr& MyMemory, PVOID Address);

      // Get memory manager
      MemoryMgr& GetMemoryMgr() const;

      // Get base address
      PBYTE GetBase() const;

      // Convert RVA to VA
      virtual PVOID RvaToVa(DWORD Rva) const;

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
      PeFileAsData(MemoryMgr& MyMemory, PVOID Address);

      // Convert RVA to VA
      virtual PVOID RvaToVa(DWORD Rva) const;
    };
  }
}
