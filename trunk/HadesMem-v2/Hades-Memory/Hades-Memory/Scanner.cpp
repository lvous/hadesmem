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

// C++ Standard Library
#include <fstream>
#include <sstream>

// Boost
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/lexical_cast.hpp>
#pragma warning(pop)

// RapidXML
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <RapidXML/rapidxml.hpp>
#pragma warning(pop)

// Hades
#include "PeFile.h"
#include "Module.h"
#include "Scanner.h"
#include "DosHeader.h"
#include "NtHeaders.h"
#include "Hades-Common/I18n.h"

namespace Hades
{
  namespace Memory
  {
    // Constructor
    Scanner::Scanner(MemoryMgr& MyMemory) 
      : m_pMemory(&MyMemory), 
      m_Start(nullptr), 
      m_End(nullptr), 
      m_Addresses()
    {
      // Get pointer to image headers
      ModuleEnum MyModuleEnum(*m_pMemory);
      PBYTE const pBase = reinterpret_cast<PBYTE>(MyModuleEnum.First()->
        GetBase());
      PeFile MyPeFile(*m_pMemory, pBase);
      DosHeader const MyDosHeader(MyPeFile);
      NtHeaders const MyNtHeaders(MyPeFile);

      // Get base of code section
      m_Start = pBase + MyNtHeaders.GetBaseOfCode();

      // Calculate end of code section
      m_End = m_Start + MyNtHeaders.GetSizeOfImage();
    }

    // Constructor
    Scanner::Scanner(MemoryMgr& MyMemory, HMODULE Module) 
      : m_pMemory(&MyMemory), 
      m_Start(nullptr), 
      m_End(nullptr), 
      m_Addresses()
    {
      // Ensure file is a valid PE file
      PBYTE const pBase = reinterpret_cast<PBYTE>(Module);
      PeFile MyPeFile(*m_pMemory, pBase);
      DosHeader const MyDosHeader(MyPeFile);
      NtHeaders const MyNtHeaders(MyPeFile);

      // Get base of code section
      m_Start = pBase + MyNtHeaders.GetBaseOfCode();

      // Calculate end of code section
      m_End = m_Start + MyNtHeaders.GetSizeOfImage();
    }

    // Constructor
    Scanner::Scanner(MemoryMgr& MyMemory, PVOID Start, PVOID End) 
      : m_pMemory(&MyMemory), 
      m_Start(static_cast<PBYTE>(Start)), 
      m_End(static_cast<PBYTE>(End)), 
      m_Addresses()
    {
      // Ensure range is valid
      if (m_End < m_Start)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("Scanner::Scanner") << 
          ErrorString("Start or end address is invalid."));
      }
    }

    // Load patterns from XML file
    void Scanner::LoadFromXML(boost::filesystem::path const& Path)
    {
      // Open current file
      std::basic_ifstream<TCHAR> PatternFile(Path.string<std::
        basic_string<TCHAR>>().c_str());
      if (!PatternFile)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("Scanner::LoadFromXML") << 
          ErrorString("Could not open pattern file."));
      }

      // Copy file to buffer
      std::istreambuf_iterator<TCHAR> const PatFileBeg(PatternFile);
      std::istreambuf_iterator<TCHAR> const PatFileEnd;
      std::vector<TCHAR> PatFileBuf(PatFileBeg, PatFileEnd);
      PatFileBuf.push_back(L'\0');

      // Open XML document
      std::shared_ptr<rapidxml::xml_document<TCHAR>> const AccountsDoc(
        std::make_shared<rapidxml::xml_document<TCHAR>>());
      AccountsDoc->parse<0>(&PatFileBuf[0]);

      // Ensure pattern tag is found
      rapidxml::xml_node<TCHAR>* PatternsTag = AccountsDoc->first_node(_T(
        "Patterns"));
      if (!PatternsTag)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("Scanner::LoadFromXML") << 
          ErrorString("Invalid pattern file format."));
      }

      // Loop over all patterns
      for (rapidxml::xml_node<TCHAR>* Pattern(PatternsTag->first_node(
        _T("Pattern"))); Pattern; Pattern = Pattern->next_sibling(_T(
        "Pattern")))
      {
        // Get pattern attributes
        rapidxml::xml_attribute<TCHAR> const* NameNode = Pattern->
          first_attribute(_T("Name"));
        rapidxml::xml_attribute<TCHAR> const* MaskNode = Pattern->
          first_attribute(_T("Mask"));
        rapidxml::xml_attribute<TCHAR> const* DataNode = Pattern->
          first_attribute(_T("Data"));
        std::basic_string<TCHAR> const Name(NameNode ? NameNode->value() : 
          _T(""));
        std::basic_string<TCHAR> const Mask(MaskNode ? MaskNode->value() : 
          _T(""));
        std::basic_string<TCHAR> const Data(DataNode ? DataNode->value() : 
          _T(""));
        std::string const DataReal(boost::lexical_cast<std::string>(Data));

        // Ensure pattern attributes are valid
        if (Name.empty() || Mask.empty() || Data.empty())
        {
          BOOST_THROW_EXCEPTION(Error() << 
            ErrorFunction("Scanner::LoadFromXML") << 
            ErrorString("Invalid pattern attributes."));
        }

        // Ensure data is valid
        if (Data.size() % 2)
        {
          BOOST_THROW_EXCEPTION(Error() << 
            ErrorFunction("Scanner::LoadFromXML") << 
            ErrorString("Data size invalid."));
        }

        // Ensure mask is valid
        if (Mask.size() * 2 != Data.size())
        {
          BOOST_THROW_EXCEPTION(Error() << 
            ErrorFunction("Scanner::LoadFromXML") << 
            ErrorString("Mask size invalid."));
        }

        // Convert data to byte buffer
        std::vector<BYTE> DataBuf;
        for (auto i = DataReal.cbegin(); i != DataReal.cend(); i += 2)
        {
          std::string const CurrentStr(i, i + 2);
          std::stringstream Converter(CurrentStr);
          int Current(0);
          if (!(Converter >> std::hex >> Current >> std::dec))
          {
            BOOST_THROW_EXCEPTION(Error() << 
              ErrorFunction("Scanner::LoadFromXML") << 
              ErrorString("Invalid data conversion."));
          }
          DataBuf.push_back(static_cast<BYTE>(Current));
        }

        // Find pattern
        PBYTE Address = static_cast<PBYTE>(Find(DataBuf, Mask));

        // Only apply options if pattern was found
        if (Address != 0)
        {
          // Loop over all pattern options
          for (rapidxml::xml_node<TCHAR> const* PatOpts = Pattern->
            first_node(); PatOpts; PatOpts = PatOpts->next_sibling())
          {
            // Get option name
            std::basic_string<TCHAR> const OptionName(PatOpts->name());

            // Handle 'Add' and 'Sub' options
            bool const IsAdd = (OptionName == _T("Add"));
            bool const IsSub = (OptionName == _T("Sub"));
            if (IsAdd || IsSub)
            {
              // Get the modification value
              rapidxml::xml_attribute<TCHAR> const* ModVal = PatOpts->
                first_attribute(_T("Value"));
              if (!ModVal)
              {
                BOOST_THROW_EXCEPTION(Error() << 
                  ErrorFunction("Scanner::LoadFromXML") << 
                  ErrorString("No value specified for 'Add' option."));
              }

              // Convert value to usable form
              std::basic_stringstream<TCHAR> Converter(ModVal->value());
              DWORD_PTR AddValReal = 0;
              if (!(Converter >> std::hex >> AddValReal >> std::dec))
              {
                BOOST_THROW_EXCEPTION(Error() << 
                  ErrorFunction("Scanner::LoadFromXML") << 
                  ErrorString("Invalid conversion for 'Add' option."));
              }

              // Perform modification
              if (IsAdd)
              {
                Address += AddValReal;
              }
              else if (IsSub)
              {
                Address -= AddValReal;
              }
              else
              {
                BOOST_THROW_EXCEPTION(Error() << 
                  ErrorFunction("Scanner::LoadFromXML") << 
                  ErrorString("Unsupported pattern option."));
              }
            }
            // Handle 'Lea' option (abs deref)
            else if (OptionName == _T("Lea"))
            {
              // Perform absolute 'dereference'
              Address = m_pMemory->Read<PBYTE>(Address);
            }
            // Handle 'Rel' option (rel deref)
            else if (OptionName == _T("Rel"))
            {
              // Get instruction size
              rapidxml::xml_attribute<TCHAR> const* SizeAttr = PatOpts->
                first_attribute(_T("Size"));
              if (!SizeAttr)
              {
                BOOST_THROW_EXCEPTION(Error() << 
                  ErrorFunction("Scanner::LoadFromXML") << 
                  ErrorString("No size specified for 'Size' in 'Rel' "
                  "option."));
              }

              // Convert instruction size to usable format
              std::basic_stringstream<TCHAR> SizeConverter(SizeAttr->value());
              DWORD_PTR Size(0);
              if (!(SizeConverter >> std::hex >> Size >> std::dec))
              {
                BOOST_THROW_EXCEPTION(Error() << 
                  ErrorFunction("Scanner::LoadFromXML") << 
                  ErrorString("Invalid conversion for 'Size' in 'Rel' "
                  "option."));
              }

              // Get instruction offset
              rapidxml::xml_attribute<TCHAR> const* OffsetAttr = PatOpts->
                first_attribute(_T("Offset"));
              if (!OffsetAttr)
              {
                BOOST_THROW_EXCEPTION(Error() << 
                  ErrorFunction("Scanner::LoadFromXML") << 
                  ErrorString("No value specified for 'Offset' in 'Rel' "
                  "option."));
              }

              // Convert instruction offset to usable format
              std::basic_stringstream<TCHAR> OffsetConverter(OffsetAttr->
                value());
              DWORD_PTR Offset(0);
              if (!(OffsetConverter >> std::hex >> Offset >> std::dec))
              {
                BOOST_THROW_EXCEPTION(Error() << 
                  ErrorFunction("Scanner::LoadFromXML") << 
                  ErrorString("Invalid conversion for 'Offset' in 'Rel' "
                  "option."));
              }

              // Perform relative 'dereference'
              Address = m_pMemory->Read<PBYTE>(Address) + reinterpret_cast<
                DWORD_PTR>(Address) + Size - Offset;
            }
            else
            {
              // Unknown pattern option
              BOOST_THROW_EXCEPTION(Error() << 
                ErrorFunction("Scanner::LoadFromXML") << 
                ErrorString("Unknown pattern option."));
            }
          }
        }

        // Add address to map
        m_Addresses[Name] = Address;
      }
    }

    // Get address map
    std::map<std::basic_string<TCHAR>, PVOID> Scanner::GetAddresses() const
    {
      return m_Addresses;
    }

    // Operator[] overload to allow retrieving addresses by name
    PVOID Scanner::operator[](std::basic_string<TCHAR> const& Name) const
    {
      auto const Iter = m_Addresses.find(Name);
      return Iter != m_Addresses.end() ? Iter->second : nullptr;
    }
  }
}
