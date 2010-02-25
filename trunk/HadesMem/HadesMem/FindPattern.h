#pragma once

// Windows API
#include <Windows.h>

// C++ Standard Library
#include <map>
#include <array>
#include <string>
#include <vector>

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
  }
}
