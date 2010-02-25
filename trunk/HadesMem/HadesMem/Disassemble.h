#pragma once

// Windows API
#include <Psapi.h>
#include <Windows.h>
#include <atlbase.h>
#include <atlfile.h>
#pragma comment(lib, "psapi")

// C++ Standard Library
#include <array>
#include <memory>
#include <string>
#include <iostream>

// BeaEngine
#include "BeaEngine/BeaEngine.h"

// HadesMem
#include "Error.h"
#include "Module.h"
#include "Memory.h"
#include "Process.h"

namespace Hades
{
  namespace Memory
  {
    // Disassembler exception type
    class DisassemblerError : public virtual HadesMemError 
    { };

    // Disassembler managing class
    class Disassembler
    {
    public:
      // Constructor
      inline explicit Disassembler(MemoryMgr const& MyMemory);

      // Test disassembler
      inline void DisassembleTest(PVOID Address, DWORD NumInstructions);

    private:
      // Convert RVA to file offset
      inline DWORD RvaToFileOffset(PIMAGE_NT_HEADERS pNtHeaders, DWORD Rva);

      // Disable assignment
      Disassembler& operator= (Disassembler const&);

      // MemoryMgr instance
      MemoryMgr const& m_Memory;
    };

    // Constructor
    Disassembler::Disassembler(MemoryMgr const& MyMemory) 
      : m_Memory(MyMemory)
    { }

    // Test disassembler
    void Disassembler::DisassembleTest(PVOID Address, DWORD NumInstructions) 
    {
      // Calculate target address
      PBYTE Target = static_cast<PBYTE>(Address);

      // Read data into buffer
      int MaxInstructionSize = 30;
      auto Buffer(m_Memory.Read<std::vector<BYTE>>(Target, NumInstructions * 
        MaxInstructionSize));

      // Set up disasm structure for BeaEngine
      DISASM MyDisasm = { 0 };
      MyDisasm.EIP = reinterpret_cast<long long>(&Buffer[0]);
      #if defined(_M_AMD64) 
        MyDisasm.Archi = 64;
      #elif defined(_M_IX86) 
        MyDisasm.Archi = 32;
      #elif 
        #error "Unsupported architecture."
      #endif

      // Disassemble instructions
      for (DWORD i = 0; i < NumInstructions; ++i)
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
