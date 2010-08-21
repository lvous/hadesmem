/*
This file is part of HadesMem.
Copyright © 2010 Cypherjb (aka Chazwazza, aka Cypher). 
<http://www.cypherjb.com/> <cypher.jb@gmail.com>

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
      DosHeader(PeFile const& MyPeFile);

      bool IsValid() const;

      WORD GetMagic() const;

      void SetMagic(WORD Magic) const;

    private:
      DosHeader& operator= (DosHeader const&);

      PeFile const& m_PeFile;
      MemoryMgr const& m_Memory;
    };

    DosHeader::DosHeader(PeFile const& MyPeFile)
      : m_PeFile(MyPeFile), 
      m_Memory(m_PeFile.GetMemoryMgr()) 
    { }

    bool DosHeader::IsValid() const
    {
      return IMAGE_DOS_SIGNATURE == GetMagic();
    }

    WORD DosHeader::GetMagic() const
    {
      IMAGE_DOS_HEADER const MyDosHeader = m_Memory.Read<IMAGE_DOS_HEADER>(
        m_PeFile.GetBase());
      return MyDosHeader.e_magic;
    }

    void DosHeader::SetMagic(WORD Magic) const
    {
      IMAGE_DOS_HEADER MyDosHeader = m_Memory.Read<IMAGE_DOS_HEADER>(
        m_PeFile.GetBase());
      MyDosHeader.e_magic = Magic;
      m_Memory.Write(m_PeFile.GetBase(), MyDosHeader);
    }
  }
}
