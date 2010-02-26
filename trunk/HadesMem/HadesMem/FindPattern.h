#pragma once

// Windows API
#include <Windows.h>

// C++ Standard Library
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

// RapidXML
#include <RapidXML/rapidxml.hpp>

// HadesMem
#include "I18n.h"
#include "Module.h"
#include "Memory.h"

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
      inline explicit FindPattern(MemoryMgr const& MyMemory);
      inline explicit FindPattern(MemoryMgr const& MyMemory, HMODULE Module);
      inline explicit FindPattern(MemoryMgr const& MyMemory, PVOID Start, 
        PVOID End);
      
      // Find pattern
      inline PVOID Find(std::wstring const& Mask, 
        std::vector<BYTE> const& Data);

      // Load patterns from XML file
      inline void LoadFromXML(std::wstring const& Path);

      // Get address map
      inline std::map<std::wstring, PVOID> GetAddresses() const;

      // Operator[] overload to allow retrieving addresses by name
      inline PVOID operator[](std::wstring const& Name) const;

    private:
      // Disable assignment
      FindPattern& operator= (FindPattern const&);

      // Initialize using PE header
      void Initialize();

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
        // Initialize using PE header
        Initialize();
      }
    }

    // Constructor
    FindPattern::FindPattern(MemoryMgr const& MyMemory, HMODULE Module) 
      : m_Memory(MyMemory), 
      m_Start(nullptr), 
      m_End(nullptr), 
      m_Addresses()
    {
      // Ensure file is a valid PE file
      auto pBase = reinterpret_cast<PBYTE>(Module);
      auto DosHeader = MyMemory.Read<IMAGE_DOS_HEADER>(pBase);
      if (DosHeader.e_magic != IMAGE_DOS_SIGNATURE)
      {
        BOOST_THROW_EXCEPTION(FindPatternError() << 
          ErrorFunction("ManualMap::Map") << 
          ErrorString("Target file is not a valid PE file (DOS)."));
      }
      auto NtHeader = MyMemory.Read<IMAGE_NT_HEADERS>(pBase + DosHeader.
        e_lfanew);
      if (NtHeader.Signature != IMAGE_NT_SIGNATURE)
      {
        BOOST_THROW_EXCEPTION(FindPatternError() << 
          ErrorFunction("ManualMap::Map") << 
          ErrorString("Target file is not a valid PE file (NT)."));
      }

      // Get base of code section
      m_Start = pBase + NtHeader.OptionalHeader.BaseOfCode;

      // Calculate end of code section
      m_End = m_Start + NtHeader.OptionalHeader.SizeOfCode;
    }

    // Constructor
    FindPattern::FindPattern(MemoryMgr const& MyMemory) 
      : m_Memory(MyMemory), 
      m_Start(nullptr), 
      m_End(nullptr), 
      m_Addresses()
    {
      // Initialize using PE header
      Initialize();
    }

    // Find pattern
    PVOID FindPattern::Find(std::wstring const& Mask, 
      std::vector<BYTE> const& Data)
    {
      return m_Memory.Find(Data, m_Start, m_End, Mask);
    }

    // Load patterns from XML file
    void FindPattern::LoadFromXML(std::wstring const& Path)
    {
      // Open current file
      std::wifstream PatternFile(Path.c_str());
      if (!PatternFile)
      {
        BOOST_THROW_EXCEPTION(FindPatternError() << 
          ErrorFunction("FindPattern::LoadFromXML") << 
          ErrorString("Could not open pattern file."));
      }

      // Copy file to buffer
      std::istreambuf_iterator<wchar_t> PatFileBeg(PatternFile);
      std::istreambuf_iterator<wchar_t> PatFileEnd;
      std::vector<wchar_t> PatFileBuf(PatFileBeg, PatFileEnd);
      PatFileBuf.push_back(L'\0');

      // Open XML document
      rapidxml::xml_document<wchar_t> AccountsDoc;
      AccountsDoc.parse<0>(&PatFileBuf[0]);

      // Ensure pattern tag is found
      auto PatternsTag = AccountsDoc.first_node(L"Patterns");
      if (!PatternsTag)
      {
        BOOST_THROW_EXCEPTION(FindPatternError() << 
          ErrorFunction("FindPattern::LoadFromXML") << 
          ErrorString("Invalid pattern file format."));
      }

      // Loop over all patterns
      for (auto Pattern = PatternsTag->first_node(L"Pattern"); Pattern; 
        Pattern = Pattern->next_sibling(L"Pattern"))
      {
        // Get pattern attributes
        auto NameNode = Pattern->first_attribute(L"Name");
        auto MaskNode = Pattern->first_attribute(L"Mask");
        auto DataNode = Pattern->first_attribute(L"Data");
        std::wstring Name(NameNode ? NameNode->value() : L"");
        std::wstring Mask(MaskNode ? MaskNode->value() : L"");
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
        PBYTE Address = static_cast<PBYTE>(Find(Mask, DataBuf));

        // Only apply options if pattern was found
        if (Address != 0)
        {
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
              // Get the modification value
              auto ModVal = PatOpts->first_attribute(L"Value");
              if (!ModVal)
              {
                BOOST_THROW_EXCEPTION(FindPatternError() << 
                  ErrorFunction("FindPattern::LoadFromXML") << 
                  ErrorString("No value specified for 'Add' option."));
              }

              // Convert value to usable form
              DWORD_PTR AddValReal = 0;
              std::wstringstream Converter(ModVal->value());
              if (!(Converter >> std::hex >> AddValReal >> std::dec))
              {
                BOOST_THROW_EXCEPTION(FindPatternError() << 
                  ErrorFunction("FindPattern::LoadFromXML") << 
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
                BOOST_THROW_EXCEPTION(FindPatternError() << 
                  ErrorFunction("FindPattern::LoadFromXML") << 
                  ErrorString("Unsupported pattern option."));
              }
            }
            // Handle 'Lea' option (abs deref)
            else if (OptionName == L"Lea")
            {
              // Perform absolute 'dereference'
              Address = m_Memory.Read<PBYTE>(Address);
            }
            // Handle 'Rel' option (rel deref)
            else if (OptionName == L"Rel")
            {
              // Get instruction size
              auto SizeAttr = PatOpts->first_attribute(L"Size");
              if (!SizeAttr)
              {
                BOOST_THROW_EXCEPTION(FindPatternError() << 
                  ErrorFunction("FindPattern::LoadFromXML") << 
                  ErrorString("No size specified for 'Size' in 'Rel' "
                  "option."));
              }

              // Convert instruction size to usable format
              DWORD_PTR Size = 0;
              std::wstringstream SizeConverter(SizeAttr->value());
              if (!(SizeConverter >> std::hex >> Size >> std::dec))
              {
                BOOST_THROW_EXCEPTION(FindPatternError() << 
                  ErrorFunction("FindPattern::LoadFromXML") << 
                  ErrorString("Invalid conversion for 'Size' in 'Rel' "
                  "option."));
              }

              // Get instruction offset
              auto OffsetAttr = PatOpts->first_attribute(L"Offset");
              if (!OffsetAttr)
              {
                BOOST_THROW_EXCEPTION(FindPatternError() << 
                  ErrorFunction("FindPattern::LoadFromXML") << 
                  ErrorString("No value specified for 'Offset' in 'Rel' "
                  "option."));
              }

              // Convert instruction onffset to usable format
              DWORD_PTR Offset = 0;
              std::wstringstream OffsetConverter(OffsetAttr->value());
              if (!(OffsetConverter >> std::hex >> Offset >> std::dec))
              {
                BOOST_THROW_EXCEPTION(FindPatternError() << 
                  ErrorFunction("FindPattern::LoadFromXML") << 
                  ErrorString("Invalid conversion for 'Offset' in 'Rel' "
                  "option."));
              }

              // Perform relative 'dereference'
              Address = m_Memory.Read<PBYTE>(Address) + 
                reinterpret_cast<DWORD_PTR>(Address) + Size - Offset;
            }
            else
            {
              // Unknown pattern option
              BOOST_THROW_EXCEPTION(FindPatternError() << 
                ErrorFunction("FindPattern::LoadFromXML") << 
                ErrorString("Unknown pattern option."));
            }
          }
        }

        // Add address to map
        m_Addresses[Name] = Address;
      }
    }

    // Get address map
    std::map<std::wstring, PVOID> FindPattern::GetAddresses() const
    {
      return m_Addresses;
    }

    // Initialize using PE header
    void FindPattern::Initialize()
    {
      // Get module list
      auto ModuleList = GetModuleList(m_Memory);

      // Ensure module list is valid
      if (ModuleList.empty())
      {
        BOOST_THROW_EXCEPTION(FindPatternError() << 
          ErrorFunction("FindPattern::FindPattern") << 
          ErrorString("Could not get module list."));
      }

      // Get pointer to image headers
      auto pBase = reinterpret_cast<PBYTE>(ModuleList[0]->GetBase());
      auto DosHeader = m_Memory.Read<IMAGE_DOS_HEADER>(pBase);
      auto NtHeader = m_Memory.Read<IMAGE_NT_HEADERS>(pBase + DosHeader.
        e_lfanew);

      // Get base of code section
      m_Start = pBase + NtHeader.OptionalHeader.BaseOfCode;

      // Calculate end of code section
      m_End = m_Start + NtHeader.OptionalHeader.SizeOfCode;
    }

    // Operator[] overload to allow retrieving addresses by name
    PVOID FindPattern::operator[](std::wstring const& Name) const
    {
      auto Iter = m_Addresses.find(Name);
      return Iter != m_Addresses.end() ? Iter->second : nullptr;
    }
  }
}
