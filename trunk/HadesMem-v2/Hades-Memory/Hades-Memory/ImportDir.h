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

// C++ Standard Library
#include <string>

// Boost
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/noncopyable.hpp>
#pragma warning(pop)

// Hades
#include "Fwd.h"
#include "Error.h"
#include "PeFile.h"

namespace Hades
{
  namespace Memory
  {
    // Import directory wrapper
    class ImportDir : private boost::noncopyable
    {
    public:
      // ImportDir error type
      class Error : public virtual HadesMemError
      { };

      // Constructor
      ImportDir(PeFile& MyPeFile);

      // Whether import directory is valid
      bool IsValid() const;

      // Ensure import directory is valid
      void EnsureValid() const;

      // Get import directory base
      PBYTE GetBase() const;

      // Advance to next descriptor
      void Advance() const;

      // Get characteristics
      DWORD GetCharacteristics() const;

      // Get time and date stamp
      DWORD GetTimeDateStamp() const;

      // Get forwarder chain
      DWORD GetForwarderChain() const;

      // Get name (raw)
      DWORD GetNameRaw() const;

      // Get name
      std::string GetName() const;

      // Get first think
      DWORD GetFirstThunk() const;

    private:
      PeFile* m_pPeFile;

      MemoryMgr* m_pMemory;

      mutable PIMAGE_IMPORT_DESCRIPTOR m_pImpDesc;
    };

    // Import thunk wrapper
    class ImportThunk : private boost::noncopyable
    {
    public:
      // ImportDir error type
      class Error : public virtual HadesMemError
      { };

      // Constructor
      ImportThunk(PeFile& MyPeFile, PVOID pThunk);

      // Whether thunk is valid
      bool IsValid() const;

      // Advance to next thunk
      void Advance() const;

      // Get address of data
      DWORD_PTR GetAddressOfData() const;

      // Get ordinal (raw)
      DWORD_PTR GetOrdinalRaw() const;
      
      // Whether import is by ordinal
      bool ByOrdinal() const;

      // Get ordinal
      WORD GetOrdinal() const;

      // Get function
      DWORD_PTR GetFunction() const;

      // Get hint
      WORD GetHint() const;

      // Get name
      std::string GetName() const;

      // Set function
      void SetFunction(DWORD_PTR Function);

    private:
      PeFile* m_pPeFile;

      MemoryMgr* m_pMemory;

      mutable PIMAGE_THUNK_DATA m_pThunk;

      mutable PBYTE m_pBase;
    };
  }
}
