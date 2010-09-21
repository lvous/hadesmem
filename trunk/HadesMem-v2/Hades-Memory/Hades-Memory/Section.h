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
      inline Section(PeFile& MyPeFile, WORD Number);

      // Get name
      inline std::string GetName() const;

      // Get section header base
      inline PBYTE GetBase() const;

      // Get raw section header
      inline IMAGE_SECTION_HEADER GetSectionHeaderRaw() const;

    private:
      // PE file
      PeFile* m_pPeFile;

      // Memory instance
      MemoryMgr* m_pMemory;

      // Section number
      WORD m_SectionNum;
    };

    // Constructor
    Section::Section(PeFile& MyPeFile, WORD Number)
      : m_pPeFile(&MyPeFile), 
      m_pMemory(&m_pPeFile->GetMemoryMgr()), 
      m_SectionNum(Number)
    { }

    // Get name
    std::string Section::GetName() const
    {
      // Get base of section header
      PBYTE pSecHdr(GetBase());

      // Read RVA of module name
      auto NameData(m_pMemory->Read<std::array<char, 8>>(pSecHdr + 
        FIELD_OFFSET(IMAGE_SECTION_HEADER, Name)));

      // Convert section name to string
      std::string Name;
      for (std::size_t i = 0; i < 8 && NameData[i]; ++i)
      {
        Name += NameData[i];
      }

      // Return section name
      return Name;
    }

    // Get section header base
    PBYTE Section::GetBase() const
    {
      // Get NT headers
      NtHeaders MyNtHeaders(*m_pPeFile);

      // Ensure section number is valid
      if (m_SectionNum >= MyNtHeaders.GetNumberOfSections())
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("Section::GetBase") << 
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

      // Return base of section header
      return reinterpret_cast<PBYTE>(pSectionHeader);
    }

    // Get raw section header
    IMAGE_SECTION_HEADER Section::GetSectionHeaderRaw() const
    {
      // Get target raw section header
      return m_pMemory->Read<IMAGE_SECTION_HEADER>(GetBase());
    }
  }
}
