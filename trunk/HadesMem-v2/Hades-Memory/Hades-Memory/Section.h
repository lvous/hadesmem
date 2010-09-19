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
#include "NtHeaders.h"

namespace Hades
{
  namespace Memory
  {
    // PE file section
    class Section
    {
    public:
      // Section error class
      class Error : public virtual HadesMemError
      { };

      // Constructor
      Section(PeFile const& MyPeFile, WORD Number);

      // Get name
      std::string GetName() const;

      // Get raw section header
      IMAGE_SECTION_HEADER GetSectionHeaderRaw() const;

    private:
      // Disable assignment
      Section& operator= (Section const&);

      // PE file
      PeFile const& m_PeFile;

      // Memory instance
      MemoryMgr* m_pMemory;

      // Section number
      WORD m_SectionNum;
    };

    // Constructor
    Section::Section(PeFile const& MyPeFile, WORD Number)
      : m_PeFile(MyPeFile), 
      m_pMemory(m_PeFile.GetMemoryMgr()), 
      m_SectionNum(Number)
    { }

    // Get name
    std::string Section::GetName() const
    {
      // Get section header
      auto const SectionHeaderRaw(GetSectionHeaderRaw());

      // Convert section name to string
      std::string Name;
      for (std::size_t i = 0; i < 8 && SectionHeaderRaw.Name[i]; ++i)
      {
        Name += static_cast<char const>(SectionHeaderRaw.Name[i]);
      }

      // Return section name
      return Name;
    }

    // Get raw section header
    IMAGE_SECTION_HEADER Section::GetSectionHeaderRaw() const
    {
      // Get NT headers
      NtHeaders MyNtHeaders(m_PeFile);

      // Ensure section number is valid
      if (m_SectionNum >= MyNtHeaders.GetNumberOfSections())
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("Section::GetSectionHeaderRaw") << 
          ErrorString("Invalid section number."));
      }

      // Get raw NT headers
      auto NtHeadersRaw(MyNtHeaders.GetHeadersRaw());

      // Get pointer to raw NT headers
      auto pNtHeaders(static_cast<PBYTE>(MyNtHeaders.GetBase()));

      // Get pointer to first section
      auto pSectionHeader(reinterpret_cast<PIMAGE_SECTION_HEADER>(pNtHeaders + 
        sizeof(NtHeadersRaw.FileHeader) + sizeof(NtHeadersRaw.Signature) + 
        NtHeadersRaw.FileHeader.SizeOfOptionalHeader));

      // Adjust pointer to target section
      pSectionHeader += m_SectionNum;

      // Get target raw section header
      return m_pMemory->Read<IMAGE_SECTION_HEADER>(pSectionHeader);
    }
  }
}
