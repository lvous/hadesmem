#pragma once

// Windows API
#include <Windows.h>

// C++ Standard Library
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <type_traits>

// Boost
#pragma warning(push, 1)
#pragma warning(disable: 4706)
#include <boost/exception.hpp>
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
    class Disassembler
    {
    public:
      inline explicit Disassembler(MemoryMgr const& MyMemory);

      inline void DisassembleTest();

    private:
      // Disable assignment
      Disassembler& operator= (Disassembler const&);

      MemoryMgr const& m_Memory;
    };

    Disassembler::Disassembler(MemoryMgr const& MyMemory) 
      : m_Memory(MyMemory) 
    { }

    void Disassembler::DisassembleTest() 
    {
      DISASM MyDisasm = { 0 };
      ZeroMemory(&MyDisasm, sizeof(MyDisasm));
      MyDisasm.EIP = reinterpret_cast<long long>(GetModuleHandle(nullptr));
      #if defined(_M_AMD64) 
        MyDisasm.Archi = 64;
      #elif defined(_M_IX86) 
        MyDisasm.Archi = 32;
      #elif 
        #error "Unsupported architecture."
      #endif

      for (int i = 0; i <100; ++i)
      {
        int Len = Disasm(&MyDisasm);
        if (Len != UNKNOWN_OPCODE) 
        {
          std::cout << MyDisasm.CompleteInstr << std::endl;
          MyDisasm.EIP = MyDisasm.EIP + Len;
        }
        else 
        {
          break;
        }
      };
    }
  }
}
