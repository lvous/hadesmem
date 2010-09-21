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

// Windows API
#include <Windows.h>

// C++ Standard Library
#include <vector>
#include <string>
#include <utility>
#include <iterator>
#include <algorithm>

// BeaEngine
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include "BeaEngine/BeaEngine.h"
#pragma warning(pop)

// Hades
#include "MemoryMgr.h"

namespace Hades
{
  namespace Memory
  {
    // Disassembler data
    struct DisasmData
    {
      DISASM Disasm;
      int Len;
      std::vector<BYTE> Raw;
    };

    // Disassembler managing class
    class Disassembler
    {
    public:
      // Disassembler exception type
      class Error : public virtual HadesMemError 
      { };

      // Constructor
      inline explicit Disassembler(MemoryMgr& pMyMemory);

      // Disassemble target and get results as strings
      inline std::vector<std::string> DisassembleToStr(PVOID Address, 
        DWORD_PTR NumInstructions) const;

      // Disassemble target and get full disasm data back
      inline std::vector<DisasmData> Disassemble(PVOID Address, 
        DWORD_PTR NumInstructions) const;

    private:
      // MemoryMgr instance
      MemoryMgr* m_pMemory;
    };

    // Constructor
    Disassembler::Disassembler(MemoryMgr& pMyMemory) 
      : m_pMemory(&pMyMemory)
    { }

    // Test disassembler
    std::vector<std::string> Disassembler::DisassembleToStr(PVOID Address, 
      DWORD_PTR NumInstructions) const
    {
      // Disassemble target
      auto MyDisasmData(Disassemble(Address, NumInstructions));

      // Container to hold disassembled code as a string
      std::vector<std::string> Results;
      std::transform(MyDisasmData.begin(), MyDisasmData.end(), 
        std::back_inserter(Results), 
        [] (DisasmData const& MyDisasm) 
      {
        return MyDisasm.Disasm.CompleteInstr;
      });

      // Return disassembled data
      return Results;
    }

    std::vector<DisasmData> Disassembler::Disassemble(PVOID Address, 
      DWORD_PTR NumInstructions) const
    {
      // Read data into buffer
      int MaxInstructionSize = 15;
      auto Buffer(m_pMemory->Read<std::vector<BYTE>>(Address, 
        NumInstructions * MaxInstructionSize));

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
      std::vector<DisasmData> Results;

      // DisassembleToStr instructions
      for (DWORD_PTR i = 0; i < NumInstructions; ++i)
      {
        // DisassembleToStr current instruction
        int Len = Disasm(&MyDisasm);
        // Ensure disassembly succeeded
        if (Len == UNKNOWN_OPCODE)
        {
          break;
        }

        DisasmData CurData;
        CurData.Disasm = MyDisasm;
        CurData.Len = Len;
        CurData.Raw = m_pMemory->Read<std::vector<BYTE>>(
          reinterpret_cast<PVOID>(MyDisasm.EIP), Len);

        // Add current instruction to list
        Results.push_back(CurData);

        // Advance to next instruction
        MyDisasm.EIP += Len;
        MyDisasm.VirtualAddr += Len;
      }

      // Return disassembled data
      return Results;
    }
  }
}
