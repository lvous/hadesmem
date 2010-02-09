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
      inline void DisassembleTest(ULONG_PTR Offset);

    private:
      // Disable assignment
      Disassembler& operator= (Disassembler const&);

      // MemoryMgr instance
      MemoryMgr const& m_Memory;

      // Handle to target file
      CAtlFile m_TargetFile;
      // Handle to target file mapping
      CAtlFileMapping<BYTE> m_TargetFileMapping;
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
    }

    void Disassembler::DisassembleTest(ULONG_PTR Offset) 
    {
      DISASM MyDisasm = { 0 };
      MyDisasm.EIP = reinterpret_cast<long long>(m_TargetFileMapping + Offset);
      #if defined(_M_AMD64) 
        MyDisasm.Archi = 64;
      #elif defined(_M_IX86) 
        MyDisasm.Archi = 32;
      #elif 
        #error "Unsupported architecture."
      #endif

      for (int i = 0; i < 100; ++i)
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
