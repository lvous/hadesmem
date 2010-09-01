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

// Hades
#include "PeFile.h"
#include "DosHeader.h"

namespace Hades
{
  namespace Memory
  {
    class NtHeaders
    {
    public:
      class Error : public virtual HadesMemError
      { };

      NtHeaders(PeFile const& MyPeFile);

    private:
      NtHeaders& operator= (NtHeaders const&);

      PeFile const& m_PeFile;
      MemoryMgr const& m_Memory;

      DosHeader m_DosHeader;

      IMAGE_NT_HEADERS m_NtHeadersRaw;
    };

    NtHeaders::NtHeaders(PeFile const& MyPeFile)
      : m_PeFile(MyPeFile), 
      m_Memory(m_PeFile.GetMemoryMgr()), 
      m_DosHeader(m_PeFile), 
      m_NtHeadersRaw()
    {
      auto pBase = static_cast<PBYTE>(m_PeFile.GetBase());
      m_NtHeadersRaw = m_Memory.Read<IMAGE_NT_HEADERS>(pBase + m_DosHeader.
        GetNewHeaderOffset());
    }
  }
}
