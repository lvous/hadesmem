#pragma once

// Windows API
#include <Windows.h>

// C++ Standard Library
#include <map>
#include <array>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

// RapidXML
#include <RapidXML/rapidxml.hpp>

// HadesMem
#include "I18n.h"
#include "Error.h"
#include "Module.h"
#include "Memory.h"
#include "Process.h"

namespace Hades
{
  namespace Memory
  {
    // FindPattern exception type
    class FindPatternError : public virtual HadesMemError 
    { };

    // Pattern finding class
    class FindPattern
    {
    public:
      // Constructor
      inline explicit FindPattern(MemoryMgr const& MyMemory, 
        PVOID Start = nullptr, PVOID End = nullptr);
      
      // Find pattern
      inline PVOID Find(std::string const& Mask, 
        std::vector<BYTE> const& Data);

      // Load patterns from XML file
      inline void LoadFromXML(std::wstring const& Path);

      // Get address map
      inline std::map<std::wstring, PVOID> GetAddresses() const;

    private:
      // Check whether an address matches a given pattern
      inline bool DataCompare(DWORD_PTR Offset, std::string const& Mask, 
        std::vector<BYTE> const& Data, std::shared_ptr<std::vector<BYTE>>);

      // Memory manager instance
      MemoryMgr const& m_Memory;

      // Start and end addresses of search region
      PBYTE m_Start;
      PBYTE m_End;

      // Map to hold addresses
      std::map<std::wstring, PVOID> m_Addresses;
    };

    // Constructor
    FindPattern::FindPattern(MemoryMgr const& MyMemory, PVOID Start, PVOID End) 
      : m_Memory(MyMemory), 
      m_Start(static_cast<PBYTE>(Start)), 
      m_End(static_cast<PBYTE>(End)), 
      m_Addresses()
    {
      // If start or end are not specified by the user then calculate them
      if (!m_Start || !m_End)
      {
        // Get module list
        auto ModuleList = GetModuleList(MyMemory);

        // Ensure module list is valid
        if (ModuleList.empty())
        {
          BOOST_THROW_EXCEPTION(FindPatternError() << 
            ErrorFunction("FindPattern::FindPattern") << 
            ErrorString("Could not get module list."));
        }

        // Get pointer to image headers
        auto pBase = reinterpret_cast<PBYTE>(ModuleList[0]->GetBase());
        auto DosHeader = MyMemory.Read<IMAGE_DOS_HEADER>(pBase);
        auto NtHeader = MyMemory.Read<IMAGE_NT_HEADERS>(pBase + DosHeader.
          e_lfanew);

        // Get base of code section
        m_Start = pBase + NtHeader.OptionalHeader.BaseOfCode;

        // Calculate end of code section
        m_End = m_Start + NtHeader.OptionalHeader.SizeOfCode;
      }
    }

    // Find pattern
    PVOID FindPattern::Find(std::string const& Mask, 
      std::vector<BYTE> const& Data)
    {
      // Rather than performing a read for each address we instead perform 
      // caching.
      std::shared_ptr<std::vector<BYTE>> MyBuffer;
      // Loop over entire memory region
      for (auto Address = m_Start; Address != m_End; ++Address)
      {
        // Read 0x5000 addresses at a time
        DWORD_PTR ChunkSize = 0x5000;
        // Calculate current cache offset
        DWORD_PTR Offset = reinterpret_cast<DWORD_PTR>(Address) % ChunkSize;
        // Whenever we reach the chunk size we need to re-cache
        if (Offset == 0)
        {
          MyBuffer.reset(new std::vector<BYTE>(m_Memory.
            Read<std::vector<BYTE>>(Address, ChunkSize + Mask.size())));
        }
        // Check if current address matches pattern
        if (DataCompare(Offset, Mask, Data, MyBuffer))
        {
          // Return found address
          return Address;
        }
      }
      // Nothing found, return null
      return nullptr; 
    }

    // Check whether an address matches a given pattern
    bool FindPattern::DataCompare(DWORD_PTR Offset, std::string const& Mask, 
      std::vector<BYTE> const& Data, 
      std::shared_ptr<std::vector<BYTE>> MyBuffer)
    {
      // Loop over all characters in mask
      for (std::string::size_type i = 0; i != Mask.size(); ++i)
      {
        // Assume anything other than 'x' is a wildcard, and return 
        // false if the pattern doesn't match
        if (Mask[i] == L'x' && Data[i] != (*MyBuffer)[Offset + i])
        {
          return false;
        }
      }
      // Mask matched
      return true;
    }

    // Load patterns from XML file
    void FindPattern::LoadFromXML(std::wstring const& Path)
    {
      // Open current file
      std::wifstream PatternFile(Path.c_str());

      // Copy file to buffer
      auto PatFileBeg = std::istreambuf_iterator<wchar_t>(PatternFile);
      auto PatFileEnd = std::istreambuf_iterator<wchar_t>();
      std::vector<wchar_t> PatFileBuf(PatFileBeg, PatFileEnd);
      PatFileBuf.push_back(L'\0');

      // Open XML document
      rapidxml::xml_document<wchar_t> AccountsDoc;
      AccountsDoc.parse<0>(&PatFileBuf[0]);

      // Loop over all patterns
      auto PatternsTag = AccountsDoc.first_node(L"Patterns");
      for (auto Pattern = PatternsTag->first_node(L"Pattern"); Pattern; 
        Pattern = Pattern->next_sibling(L"Pattern"))
      {
        // Get pattern attributes
        auto NameNode = Pattern->first_attribute(L"Name");
        auto MaskNode = Pattern->first_attribute(L"Mask");
        auto DataNode = Pattern->first_attribute(L"Data");
        std::wstring Name(NameNode ? NameNode->value() : L"");
        std::wstring Mask(MaskNode ? MaskNode->value() : L"");
        std::string MaskReal(boost::lexical_cast<std::string>(Mask));
        std::wstring Data(DataNode ? DataNode->value() : L"");
        std::string DataReal(boost::lexical_cast<std::string>(Data));

        // Ensure pattern attributes are valid
        if (Name.empty() || Mask.empty() || Data.empty())
        {
          BOOST_THROW_EXCEPTION(FindPatternError() << 
            ErrorFunction("FindPattern::LoadFromXML") << 
            ErrorString("Invalid pattern attributes."));
        }

        // Ensure data is valid
        if (Data.size() % 2)
        {
          BOOST_THROW_EXCEPTION(FindPatternError() << 
            ErrorFunction("FindPattern::LoadFromXML") << 
            ErrorString("Data size invalid."));
        }

        // Ensure mask is valid
        if (Mask.size() * 2 != Data.size())
        {
          BOOST_THROW_EXCEPTION(FindPatternError() << 
            ErrorFunction("FindPattern::LoadFromXML") << 
            ErrorString("Mask size invalid invalid."));
        }

        // Convert data to byte buffer
        std::vector<BYTE> DataBuf;
        for (auto i = DataReal.begin(); i != DataReal.end(); i += 2)
        {
          std::string CurrentStr(i, i + 2);
          std::stringstream Converter(CurrentStr);
          int Current = 0;
          if (!(Converter >> std::hex >> Current >> std::dec))
          {
            BOOST_THROW_EXCEPTION(FindPatternError() << 
              ErrorFunction("FindPattern::LoadFromXML") << 
              ErrorString("Invalid data conversion."));
          }
          DataBuf.push_back(static_cast<BYTE>(Current));
        }

        // Find pattern
        PBYTE Address = static_cast<PBYTE>(Find(MaskReal, DataBuf));

        // Loop over all pattern options
        for (auto PatOpts = Pattern->first_node(); PatOpts; 
          PatOpts = PatOpts->next_sibling())
        {
          // Get option name
          std::wstring OptionName(PatOpts->name());

          // Handle 'Add' and 'Sub' options
          bool IsAdd = std::wstring(OptionName) == L"Add";
          bool IsSub = std::wstring(OptionName) == L"Sub";
          if (IsAdd || IsSub)
          {
            auto AddVal = PatOpts->first_attribute(L"Value");
            if (!AddVal)
            {
              BOOST_THROW_EXCEPTION(FindPatternError() << 
                ErrorFunction("FindPattern::LoadFromXML") << 
                ErrorString("No value specified for 'Add' option."));
            }

            DWORD_PTR AddValReal = 0;
            std::wstringstream Converter(AddVal->value());
            if (!(Converter >> std::hex >> AddValReal >> std::dec))
            {
              BOOST_THROW_EXCEPTION(FindPatternError() << 
                ErrorFunction("FindPattern::LoadFromXML") << 
                ErrorString("Invalid conversion for 'Add' option."));
            }

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
              BOOST_THROW_EXCEPTION(FindPatternError() << 
                ErrorFunction("FindPattern::LoadFromXML") << 
                ErrorString("Unsupported pattern option."));
            }
          }
          else if (OptionName == L"Lea")
          {
            Address = m_Memory.Read<PBYTE>(Address);
          }
          else if (OptionName == L"Rel")
          {
            auto SizeAttr = PatOpts->first_attribute(L"Size");
            if (!SizeAttr)
            {
              BOOST_THROW_EXCEPTION(FindPatternError() << 
                ErrorFunction("FindPattern::LoadFromXML") << 
                ErrorString("No size specified for 'Size' in 'Rel' option."));
            }

            DWORD_PTR Size = 0;
            std::wstringstream SizeConverter(SizeAttr->value());
            if (!(SizeConverter >> std::hex >> Size >> std::dec))
            {
              BOOST_THROW_EXCEPTION(FindPatternError() << 
                ErrorFunction("FindPattern::LoadFromXML") << 
                ErrorString("Invalid conversion for 'Size' in 'Rel' option."));
            }

            auto OffsetAttr = PatOpts->first_attribute(L"Offset");
            if (!OffsetAttr)
            {
              BOOST_THROW_EXCEPTION(FindPatternError() << 
                ErrorFunction("FindPattern::LoadFromXML") << 
                ErrorString("No value specified for 'Offset' in 'Rel' "
                "option."));
            }

            DWORD_PTR Offset = 0;
            std::wstringstream OffsetConverter(OffsetAttr->value());
            if (!(OffsetConverter >> std::hex >> Offset >> std::dec))
            {
              BOOST_THROW_EXCEPTION(FindPatternError() << 
                ErrorFunction("FindPattern::LoadFromXML") << 
                ErrorString("Invalid conversion for 'Offset' in 'Rel' "
                "option."));
            }

            Address = m_Memory.Read<PBYTE>(Address) + 
              reinterpret_cast<DWORD_PTR>(Address) + Size - Offset;
          }
          else
          {
            BOOST_THROW_EXCEPTION(FindPatternError() << 
              ErrorFunction("FindPattern::LoadFromXML") << 
              ErrorString("Unknown pattern option."));
          }
        }

        m_Addresses[Name] = Address;
      }
    }

    // Get address map
    std::map<std::wstring, PVOID> FindPattern::GetAddresses() const
    {
      return m_Addresses;
    }
  }
}
