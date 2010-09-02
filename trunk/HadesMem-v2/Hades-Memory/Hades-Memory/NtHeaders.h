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

      bool IsSignatureValid() const;

      DWORD GetSignature() const;

      void SetSignature(DWORD Signature);

    private:
      NtHeaders& operator= (NtHeaders const&);

      PeFile const& m_PeFile;
      MemoryMgr const& m_Memory;

      DosHeader m_DosHeader;

      PVOID m_pNtHeadersBase;

      IMAGE_NT_HEADERS m_NtHeadersRaw;
    };

    NtHeaders::NtHeaders(PeFile const& MyPeFile)
      : m_PeFile(MyPeFile), 
      m_Memory(m_PeFile.GetMemoryMgr()), 
      m_DosHeader(m_PeFile), 
      m_pNtHeadersBase(nullptr), 
      m_NtHeadersRaw()
    {
      auto pBase = static_cast<PBYTE>(m_PeFile.GetBase());
      m_pNtHeadersBase = pBase + m_DosHeader.GetNewHeaderOffset();
      m_NtHeadersRaw = m_Memory.Read<IMAGE_NT_HEADERS>(m_pNtHeadersBase);
    }

    bool NtHeaders::IsSignatureValid() const
    {
      return IMAGE_NT_SIGNATURE == GetSignature();
    }

    DWORD NtHeaders::GetSignature() const
    {
      return m_NtHeadersRaw.Signature;
    }

    void NtHeaders::SetSignature(DWORD Signature)
    {
      m_NtHeadersRaw.Signature = Signature;
      m_Memory.Write(m_PeFile.GetBase(), m_NtHeadersRaw);

      m_NtHeadersRaw = m_Memory.Read<IMAGE_NT_HEADERS>(m_pNtHeadersBase);
      if (m_NtHeadersRaw.Signature != Signature)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetSignature") << 
          ErrorString("Could not set signature. Verification mismatch."));
      }
    }
  }
}
