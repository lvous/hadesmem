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

namespace Hades
{
  namespace Memory
  {
    class DosHeader
    {
    public:
      class Error : public virtual HadesMemError
      { };

      DosHeader(PeFile const& MyPeFile);

      bool IsMagicValid() const;

      void EnsureMagicValid() const;

      WORD GetMagic() const;
      WORD GetChecksum() const;
      LONG GetNewHeaderOffset() const;

      void SetMagic(WORD Magic);
      void SetChecksum(WORD Checksum);
      void SetNewHeaderOffset(LONG Offset);

    private:
      DosHeader& operator= (DosHeader const&);

      PeFile const& m_PeFile;
      MemoryMgr const& m_Memory;

      PVOID m_pBase;
    };

    DosHeader::DosHeader(PeFile const& MyPeFile)
      : m_PeFile(MyPeFile), 
      m_Memory(m_PeFile.GetMemoryMgr()), 
      m_pBase(m_PeFile.GetBase())
    {
      EnsureMagicValid();
    }

    bool DosHeader::IsMagicValid() const
    {
      return IMAGE_DOS_SIGNATURE == GetMagic();
    }

    void DosHeader::EnsureMagicValid() const
    {
      if (!IsMagicValid())
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("DosHeader::EnsureSignatureValid") << 
          ErrorString("DOS header magic invalid."));
      }
    }

    WORD DosHeader::GetMagic() const
    {
      auto DosHeaderRaw = m_Memory.Read<IMAGE_DOS_HEADER>(m_pBase);
      return DosHeaderRaw.e_magic;
    }

    WORD DosHeader::GetChecksum() const
    {
      auto DosHeaderRaw = m_Memory.Read<IMAGE_DOS_HEADER>(m_pBase);
      return DosHeaderRaw.e_csum;
    }

    LONG DosHeader::GetNewHeaderOffset() const
    {
      auto DosHeaderRaw = m_Memory.Read<IMAGE_DOS_HEADER>(m_pBase);
      return DosHeaderRaw.e_lfanew;
    }

    void DosHeader::SetMagic(WORD Magic) 
    {
      auto DosHeaderRaw = m_Memory.Read<IMAGE_DOS_HEADER>(m_pBase);
      DosHeaderRaw.e_magic = Magic;
      m_Memory.Write(m_PeFile.GetBase(), DosHeaderRaw);

      DosHeaderRaw = m_Memory.Read<IMAGE_DOS_HEADER>(m_pBase);
      if (DosHeaderRaw.e_magic != Magic)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("DosHeader::SetMagic") << 
          ErrorString("Could not set magic. Verification mismatch."));
      }
    }

    void DosHeader::SetChecksum(WORD Checksum) 
    {
      auto DosHeaderRaw = m_Memory.Read<IMAGE_DOS_HEADER>(m_pBase);
      DosHeaderRaw.e_csum = Checksum;
      m_Memory.Write(m_PeFile.GetBase(), DosHeaderRaw);

      DosHeaderRaw = m_Memory.Read<IMAGE_DOS_HEADER>(m_pBase);
      if (DosHeaderRaw.e_csum != Checksum)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("DosHeader::SetChecksum") << 
          ErrorString("Could not set checksum. Verification mismatch."));
      }
    }

    void DosHeader::SetNewHeaderOffset(LONG Offset) 
    {
      auto DosHeaderRaw = m_Memory.Read<IMAGE_DOS_HEADER>(m_pBase);
      DosHeaderRaw.e_lfanew = Offset;
      m_Memory.Write(m_PeFile.GetBase(), DosHeaderRaw);

      DosHeaderRaw = m_Memory.Read<IMAGE_DOS_HEADER>(m_pBase);
      if (DosHeaderRaw.e_lfanew != Offset)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("DosHeader::SetNewHeaderOffset") << 
          ErrorString("Could not set new header offset. Verification "
            "mismatch."));
      }
    }
  }
}
