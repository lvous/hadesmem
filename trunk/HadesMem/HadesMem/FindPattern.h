#pragma once

// Windows API
#include <Psapi.h>
#include <Windows.h>
#include <atlbase.h>
#include <atlfile.h>
#pragma comment(lib, "psapi")

// C++ Standard Library
#include <map>
#include <array>
#include <string>
#include <vector>

// HadesMem
#include "Error.h"
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
      PVOID Find(std::wstring const& Name, std::string const& Mask, 
        std::vector<char> const& Pattern);

      // Load patterns from XML file
      void LoadFromXML(std::wstring const& Path);

    private:
      // Convert RVA to file offset
      inline DWORD RvaToFileOffset(PIMAGE_NT_HEADERS pNtHeaders, DWORD Rva);

      // Memory manager instance
      MemoryMgr const& m_Memory;

      // Handle to target file
      CAtlFile m_TargetFile;
      // Handle to target file mapping
      CAtlFileMapping<BYTE> m_TargetFileMapping;

      // Start and end addresses of search region
      PVOID m_Start;
      PVOID m_End;

      // Map to hold addresses
      std::map<std::wstring, PVOID> m_Addresses;
    };

    // Constructor
    FindPattern::FindPattern(MemoryMgr const& MyMemory, PVOID Start, PVOID End) 
      : m_Memory(MyMemory), 
      m_TargetFile(), 
      m_TargetFileMapping(), 
      m_Start(Start), 
      m_End(End), 
      m_Addresses()
    {
      if (!m_Start || !m_End)
      {
        // Get path to target
        std::array<wchar_t, MAX_PATH> PathToTargetBuf = { 0 };
        if (!GetModuleFileNameEx(m_Memory.GetProcessHandle(), nullptr, 
          &PathToTargetBuf[0], MAX_PATH))
        {
          DWORD LastError = GetLastError();
          BOOST_THROW_EXCEPTION(FindPatternError() << 
            ErrorFunction("FindPattern::FindPattern") << 
            ErrorString("Could not get path to target.") << 
            ErrorCodeWin(LastError));
        }

        // Open target file
        if (FAILED(m_TargetFile.Create(&PathToTargetBuf[0], GENERIC_READ, 
          FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 
          OPEN_EXISTING)))
        {
          DWORD LastError = GetLastError();
          BOOST_THROW_EXCEPTION(FindPatternError() << 
            ErrorFunction("FindPattern::FindPattern") << 
            ErrorString("Could not open target file.") << 
            ErrorCodeWin(LastError));
        }

        // Map target file into memory
        if (FAILED(m_TargetFileMapping.MapFile(m_TargetFile, 0, 0, 
          PAGE_READONLY, FILE_MAP_READ)))
        {
          DWORD LastError = GetLastError();
          BOOST_THROW_EXCEPTION(FindPatternError() << 
            ErrorFunction("FindPattern::FindPattern") << 
            ErrorString("Could not map view of target.") << 
            ErrorCodeWin(LastError));
        }

        // Get pointer to image headers
        auto pBase = static_cast<PBYTE>(m_TargetFileMapping);
        auto pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(pBase);
        auto pNtHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(pBase + 
          pDosHeader->e_lfanew);

        // If the two pointers are the same then the header is probably nulled 
        // out.
        if (static_cast<PVOID>(pDosHeader) == static_cast<PVOID>(pNtHeader))
        {
          BOOST_THROW_EXCEPTION(FindPatternError() << 
            ErrorFunction("FindPattern::FindPattern") << 
            ErrorString("Could not get pointer to image headers."));
        }

        // Get base of code section
        m_Start =static_cast<PBYTE>(m_TargetFileMapping) + RvaToFileOffset(
          pNtHeader, pNtHeader->OptionalHeader.BaseOfCode);

        // Calculate end of code section
        m_End = static_cast<PBYTE>(m_Start) + pNtHeader->OptionalHeader.
          SizeOfCode;
      }
    }

    // Convert RVA to file offset
    DWORD FindPattern::RvaToFileOffset(PIMAGE_NT_HEADERS pNtHeaders, 
      DWORD Rva)
    {
      // Get number of sections
      WORD NumSections = pNtHeaders->FileHeader.NumberOfSections;
      // Get pointer to first section
      PIMAGE_SECTION_HEADER pCurrentSection = IMAGE_FIRST_SECTION(pNtHeaders);

      // Loop over all sections
      for (WORD i = 0; i < NumSections; ++i)
      {
        // Check if the RVA may be inside the current section
        if (pCurrentSection->VirtualAddress <= Rva)
        {
          // Check if the RVA is inside the current section
          if ((pCurrentSection->VirtualAddress + pCurrentSection->
            Misc.VirtualSize) > Rva)
          {
            // Convert RVA to file (raw) offset
            Rva -= pCurrentSection->VirtualAddress;
            Rva += pCurrentSection->PointerToRawData;
            return Rva;
          }
        }

        // Advance to next section
        ++pCurrentSection;
      }

      // Could not perform conversion. Return number signifying an error.
      return static_cast<DWORD>(-1);
    }
  }
}
