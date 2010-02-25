#pragma once

// Windows API
#include <Windows.h>

// C++ Standard Library
#include <map>
#include <array>
#include <string>
#include <vector>
#include <fstream>

// RapidXML
#include <RapidXML/rapidxml.hpp>

// HadesMem
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
      inline PVOID Find(std::wstring const& Name, std::string const& Mask, 
        std::vector<BYTE> const& Data);

      // Load patterns from XML file
      inline void LoadFromXML(std::wstring const& Path);

    private:
      // Check whether an address matches a given pattern
      inline bool DataCompare(DWORD_PTR Offset, std::string const& Mask, 
        std::vector<BYTE> const& Data, std::shared_ptr<std::vector<BYTE>>);

      // Convert RVA to file offset
      inline DWORD RvaToFileOffset(PIMAGE_NT_HEADERS pNtHeaders, DWORD Rva);

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
    PVOID FindPattern::Find(std::wstring const& Name, std::string const& Mask, 
      std::vector<BYTE> const& Data)
    {
      // Rather than performing a read for each address we instead perform 
      // caching.
      std::shared_ptr<std::vector<BYTE>> MyBuffer;
      // Loop over entire memory region
      for (auto i = m_Start; i != m_End; ++i)
      {
        // Read 0x5000 addresses at a time
        DWORD_PTR ChunkSize = 0x5000;
        // Calculate current cache offset
        DWORD_PTR Offset = reinterpret_cast<DWORD_PTR>(i) % ChunkSize;
        // Whenever we reach the chunk size we need to re-cache
        if (Offset == 0)
        {
          MyBuffer.reset(new std::vector<BYTE>(m_Memory.
            Read<std::vector<BYTE>>(i, ChunkSize + 1)));
        }
        // Check if current address matches pattern
        if (DataCompare(Offset, Mask, Data, MyBuffer))
        {
          // If name is specified then enter into map
          if (!Name.empty())
          {
            m_Addresses[Name] = i;
          }
          // Return found address
          return i;
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

      // Loop over all pattern groups
      auto PatternsTag = AccountsDoc.first_node(L"Patterns");
      for (auto PatternGroups = PatternsTag->first_node(); PatternGroups; 
        PatternGroups = PatternGroups->next_sibling())
      {
        // Loop over all patterns
        for (auto Pattern = PatternGroups->first_node(L"Pattern"); Pattern; 
          Pattern = Pattern->next_sibling(L"Pattern"))
        {
          // Get pattern attributes
          auto NameNode = Pattern->first_attribute(L"Name");
          auto MaskNode = Pattern->first_attribute(L"Mask");
          auto DataNode = Pattern->first_attribute(L"Data");
          std::wstring Name(NameNode ? NameNode->value() : L"");
          std::wstring Mask(MaskNode ? MaskNode->value() : L"");
          std::wstring Data(DataNode ? DataNode->value() : L"");
          std::string DataReal(I18n::ConvertStr(Data));

          // Ensure pattern attributes are valid
          if (Name.empty() || Mask.empty() || Data.empty())
          {
            BOOST_THROW_EXCEPTION(FindPatternError() << 
              ErrorFunction("FindPattern::LoadFromXML") << 
              ErrorString("Invalid pattern attributes."));
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
                ErrorString("Invalid data."));
            }
            DataBuf.push_back(static_cast<BYTE>(Current));
          }

          // Find pattern
          Find(Name, I18n::ConvertStr(Mask), DataBuf);
        }
      }
    }
  }
}
