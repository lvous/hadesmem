#pragma once

// Windows API
#include <Psapi.h>
#include <Windows.h>
#include <atlbase.h>
#include <atlfile.h>
#include <DbgHelp.h>
#pragma comment(lib, "psapi")

// C++ Standard Library
#include <array>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <type_traits>

// Boost
#pragma warning(push, 1)
#pragma warning(disable: 4706)
#include <boost/noncopyable.hpp>
#include <boost/type_traits.hpp>
#pragma warning(pop)

// BeaEngine
#include "BeaEngine/BeaEngine.h"

// HadesMem
#include "Error.h"
#include "Memory.h"
#include "Process.h"

namespace Hades
{
  namespace Memory
  {
    // Memory exception type
    class DisassemblerError : public virtual HadesMemError 
    { };

    // Disassmbler managing class
    class Disassembler
    {
    public:
      // Constructor
      inline explicit Disassembler(MemoryMgr const& MyMemory);

      // Test disassembler
      inline void DisassembleTest(ULONG_PTR Offset, 
        unsigned long NumInstructions);

      // Convert RVA to file offset
      inline DWORD RvaToFileOffset(PIMAGE_NT_HEADERS pNtHeaders, DWORD Rva);

    private:
      // Disable assignment
      Disassembler& operator= (Disassembler const&);

      // MemoryMgr instance
      MemoryMgr const& m_Memory;

      // Handle to target file
      CAtlFile m_TargetFile;
      // Handle to target file mapping
      CAtlFileMapping<BYTE> m_TargetFileMapping;
      // Base of code section in mapping
      PBYTE m_BaseOfCode;
    };

    // Constructor
    Disassembler::Disassembler(MemoryMgr const& MyMemory) 
      : m_Memory(MyMemory), 
      m_TargetFile(), 
      m_TargetFileMapping()
    {
      // Get path to target
      std::array<wchar_t, MAX_PATH> PathToTargetBuf = { 0 };
      if (!GetModuleFileNameEx(m_Memory.GetProcessHandle(), nullptr, 
        &PathToTargetBuf[0], MAX_PATH))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(DisassemblerError() << 
          ErrorFunction("Disassembler:Disassembler") << 
          ErrorString("Could not get path to target.") << 
          ErrorCodeWin(LastError));
      }

      // Open target file
      if (FAILED(m_TargetFile.Create(&PathToTargetBuf[0], GENERIC_READ, 
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 
        OPEN_EXISTING)))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(DisassemblerError() << 
          ErrorFunction("Disassembler:Disassembler") << 
          ErrorString("Could not open target file.") << 
          ErrorCodeWin(LastError));
      }

      // Map target file into memory
      if (FAILED(m_TargetFileMapping.MapFile(m_TargetFile, 0, 0, PAGE_READONLY, 
        FILE_MAP_READ)))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(DisassemblerError() << 
          ErrorFunction("Disassembler:Disassembler") << 
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
        BOOST_THROW_EXCEPTION(DisassemblerError() << 
          ErrorFunction("Disassembler:Disassembler") << 
          ErrorString("Could not get pointer to image headers."));
      }

      // Get base of code section
      m_BaseOfCode =
        static_cast<PBYTE>(m_TargetFileMapping) + RvaToFileOffset(pNtHeader, 
        pNtHeader->OptionalHeader.BaseOfCode);
    }

    // Test disassembler
    void Disassembler::DisassembleTest(ULONG_PTR Offset, 
      unsigned long NumInstructions) 
    {
      // Calculate target address
      PBYTE Target = m_BaseOfCode + Offset;

      // Debug output
      std::wcout << "Base of Mapping:" << static_cast<PBYTE>(
        m_TargetFileMapping) << std::endl;
      std::wcout << "Base of Code:" << m_BaseOfCode << std::endl;
      std::wcout << "Target: " << Target << std::endl;

      // Set up disasm structure for BeaEngine
      DISASM MyDisasm = { 0 };
      MyDisasm.EIP = reinterpret_cast<long long>(Target);
      #if defined(_M_AMD64) 
        MyDisasm.Archi = 64;
      #elif defined(_M_IX86) 
        MyDisasm.Archi = 32;
      #elif 
        #error "Unsupported architecture."
      #endif

      // Disassemble instructions
      for (unsigned int i = 0; i < NumInstructions; ++i)
      {
        // Disassemble current instruction
        int Len = Disasm(&MyDisasm);
        // Ensure disassembly succeeded
        if (Len != UNKNOWN_OPCODE) 
        {
          // Output current instruction
          std::cout << MyDisasm.CompleteInstr << std::endl;
          // Advance to next instruction
          MyDisasm.EIP = MyDisasm.EIP + Len;
        }
        // If disassembly failed then break out
        else 
        {
          break;
        }
      };
    }

    // Convert RVA to file offset
    DWORD Disassembler::RvaToFileOffset(PIMAGE_NT_HEADERS pNtHeaders, 
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
