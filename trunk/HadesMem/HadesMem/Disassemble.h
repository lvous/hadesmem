/*
This file is part of HadesMem.

HadesMem is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

HadesMem is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

// Windows API
#include <Windows.h>

// C++ Standard Library
#include <vector>
#include <string>

// BeaEngine
#include "BeaEngine/BeaEngine.h"

// HadesMem
#include "Module.h"
#include "Memory.h"

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
      inline std::vector<std::string> Disassemble(PVOID Address, 
        DWORD_PTR NumInstructions);

    private:
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
    std::vector<std::string> Disassembler::Disassemble(PVOID Address, 
      DWORD_PTR NumInstructions) 
    {
      // Read data into buffer
      int MaxInstructionSize = 15;
      auto Buffer(m_Memory.Read<std::vector<BYTE>>(Address, NumInstructions * 
        MaxInstructionSize));

      // Set up disasm structure for BeaEngine
      DISASM MyDisasm = { 0 };
      MyDisasm.EIP = reinterpret_cast<long long>(&Buffer[0]);
      MyDisasm.VirtualAddr = reinterpret_cast<long long>(Address);
      #if defined(_M_AMD64) 
        MyDisasm.Archi = 64;
      #elif defined(_M_IX86) 
        MyDisasm.Archi = 32;
      #else 
        #error "Unsupported architecture."
      #endif

      // Container to hold disassembled code as a string
      std::vector<std::string> Results;

      // Disassemble instructions
      for (DWORD_PTR i = 0; i < NumInstructions; ++i)
      {
        // Disassemble current instruction
        int Len = Disasm(&MyDisasm);
        // Ensure disassembly succeeded
        if (Len == UNKNOWN_OPCODE)
        {
          break;
        }

        // Add current instruction to list
        Results.push_back(MyDisasm.CompleteInstr);

        // Advance to next instruction
        MyDisasm.EIP += Len;
        MyDisasm.VirtualAddr += Len;
      }

      // Return disassembled data
      return Results;
    }
  }
}
