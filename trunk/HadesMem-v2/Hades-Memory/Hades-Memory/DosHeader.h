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
    // PE file DOS header
    class DosHeader
    {
    public:
      // DOS header error class
      class Error : public virtual HadesMemError
      { };

      // Constructor
      DosHeader(PeFile const& MyPeFile);

      // Whether magic is valid
      bool IsMagicValid() const;

      // Ensure magic is valid
      void EnsureMagicValid() const;

      // Get magic
      WORD GetMagic() const;

      // Get checksum
      WORD GetChecksum() const;

      // Get new header offset
      LONG GetNewHeaderOffset() const;

      // Set magic
      void SetMagic(WORD Magic);

      // Set checksum
      void SetChecksum(WORD Checksum);

      // Set new header offset
      void SetNewHeaderOffset(LONG Offset);

    private:
      // Disable assignment
      DosHeader& operator= (DosHeader const&);

      // PE file
      PeFile const& m_PeFile;

      // Memory instance
      MemoryMgr const& m_Memory;

      // DOS header base
      PBYTE m_pBase;
    };

    // Constructor
    DosHeader::DosHeader(PeFile const& MyPeFile)
      : m_PeFile(MyPeFile), 
      m_Memory(m_PeFile.GetMemoryMgr()), 
      m_pBase(static_cast<PBYTE>(m_PeFile.GetBase()))
    {
      // Ensure magic is valid
      EnsureMagicValid();
    }

    // Whether magic is valid
    bool DosHeader::IsMagicValid() const
    {
      return IMAGE_DOS_SIGNATURE == GetMagic();
    }

    // Ensure magic is valid
    void DosHeader::EnsureMagicValid() const
    {
      if (!IsMagicValid())
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("DosHeader::EnsureSignatureValid") << 
          ErrorString("DOS header magic invalid."));
      }
    }

    // Get magic
    WORD DosHeader::GetMagic() const
    {
      return m_Memory.Read<WORD>(m_pBase + FIELD_OFFSET(IMAGE_DOS_HEADER, 
        e_magic));
    }

    // Get checksum
    WORD DosHeader::GetChecksum() const
    {
      return m_Memory.Read<WORD>(m_pBase + FIELD_OFFSET(IMAGE_DOS_HEADER, 
        e_csum));
    }

    // Get new header offset
    LONG DosHeader::GetNewHeaderOffset() const
    {
      return m_Memory.Read<LONG>(m_pBase + FIELD_OFFSET(IMAGE_DOS_HEADER, 
        e_lfanew));
    }

    // Set magic
    void DosHeader::SetMagic(WORD Magic) 
    {
      m_Memory.Write(m_pBase + FIELD_OFFSET(IMAGE_DOS_HEADER, e_magic), 
        Magic);

      if (GetMagic() != Magic)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("DosHeader::SetMagic") << 
          ErrorString("Could not set magic. Verification mismatch."));
      }
    }

    // Set checksum
    void DosHeader::SetChecksum(WORD Checksum) 
    {
      m_Memory.Write(m_pBase + FIELD_OFFSET(IMAGE_DOS_HEADER, e_csum), 
        Checksum);

      if (GetChecksum() != Checksum)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("DosHeader::SetChecksum") << 
          ErrorString("Could not set checksum. Verification mismatch."));
      }
    }

    // Set new header offset
    void DosHeader::SetNewHeaderOffset(LONG Offset) 
    {
      m_Memory.Write(m_pBase + FIELD_OFFSET(IMAGE_DOS_HEADER, e_lfanew), 
        Offset);

      if (GetNewHeaderOffset() != Offset)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("DosHeader::SetNewHeaderOffset") << 
          ErrorString("Could not set new header offset. Verification "
          "mismatch."));
      }
    }
  }
}
