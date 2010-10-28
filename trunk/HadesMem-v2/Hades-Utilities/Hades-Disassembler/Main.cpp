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

// Windows API
#include <tchar.h>
#include <crtdbg.h>
#include <Windows.h>

// C++ Standard Library
#include <vector>
#include <string>
#include <iterator>
#include <iostream>
#include <exception>

// Boost
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/timer.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#pragma warning(pop)

// Hades
#include "Hades-Memory/Memory.h"
#include "Hades-Memory/AutoLink.h"

// Program entry-point.
int _tmain(int argc, TCHAR* argv[])
{
  // Program timer
  boost::timer ProgTimer;

  try
  {
    // Attempt to detect memory leaks in debug mode
#ifdef _DEBUG
    int CurrentFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    int NewFlags = (_CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF | 
      _CRTDBG_CHECK_ALWAYS_DF);
    _CrtSetDbgFlag(CurrentFlags | NewFlags);
#endif

    // Hades version number
    std::wstring const VerNum(L"TRUNK");

    // Version and copyright output
#if defined(_M_X64)
    std::wcout << "Hades-Disassembler AMD64 [Version " << VerNum << "]\n";
#elif defined(_M_IX86)
    std::wcout << "Hades-Disassembler IA32 [Version " << VerNum << "]\n";
#else
#error Unsupported platform!
#endif
    std::wcout << "Copyright (C) 2010 RaptorFactor. All rights reserved." << 
      std::endl;
    std::wcout << "Website: http://www.raptorfactor.com/, "
      "Email: raptorfactor@raptorfactor.com." << std::endl;
    std::wcout << "Built on " << __DATE__ << " at " << __TIME__ << "." << 
      std::endl << std::endl;

    // Auto-close flag (Set by Boost.ProgramOptions)
    bool KeepOpen = false;
    // Target address (Set by Boost.ProgramOptions)
    std::wstring AddressStr;
    // Number of instructions (Set by Boost.ProgramOptions)
    DWORD_PTR NumInstructions = 0;
    // Target process ID (Set by Boost.ProgramOptions)
    DWORD ProcID = 0;

    // Set program option descriptions
    boost::program_options::options_description OptsDesc("Allowed options");
    OptsDesc.add_options()
      ("help", "display help")
      ("keep-open", boost::program_options::wvalue<bool>(&KeepOpen)->
        zero_tokens(), "keep console window open")
      ("address", boost::program_options::wvalue<std::wstring>(
        &AddressStr), "target address")
      ("instructions", boost::program_options::wvalue<DWORD_PTR>(
        &NumInstructions), "number of instructions")
      ("process-id", boost::program_options::wvalue<DWORD>(&ProcID), 
        "target process id")
      ;

    // Parse program options
    boost::program_options::variables_map Opts;
    boost::program_options::store(boost::program_options::parse_command_line(
      argc, argv, OptsDesc), Opts);
    boost::program_options::notify(Opts);

    // Print help if requested
    if (Opts.count("help")) 
    {
      // Print help
      std::cout << OptsDesc << std::endl;

      // Stop window from automatically closing if required
      if (KeepOpen)
      {
        std::wcin.clear();
        std::wcin.sync();
        std::wcin.get();
      }

      // Quit
      return 1;
    }

    // Sanity check
    if (!ProcID)
    {
      BOOST_THROW_EXCEPTION(Hades::HadesError() << 
        Hades::ErrorFunction("_tmain") << 
        Hades::ErrorString("No process ID specified."));
    }
    
    // Get address as integer
    DWORD_PTR Address = 0;
    std::wstringstream AddressConverter;
    AddressConverter << std::hex << AddressStr;
    AddressConverter >> Address;

    // Sanity check
    if (!Address)
    {
      BOOST_THROW_EXCEPTION(Hades::HadesError() << 
        Hades::ErrorFunction("_tmain") << 
        Hades::ErrorString("No instruction count specified."));
    }

    // Sanity check
    if (!NumInstructions)
    {
      BOOST_THROW_EXCEPTION(Hades::HadesError() << 
        Hades::ErrorFunction("_tmain") << 
        Hades::ErrorString("No instruction count specified."));
    }

    // Create memory manager
    Hades::Memory::MemoryMgr MyMemory(ProcID);

    // Create disassembler and disassemble target
    Hades::Memory::Disassembler MyDisassembler(MyMemory);
    std::vector<Hades::Memory::DisasmData> MyDisasmData (MyDisassembler.
      Disassemble(reinterpret_cast<PVOID>(Address), NumInstructions));

    // Dump disassembled instructions
    std::for_each(MyDisasmData.cbegin(), MyDisasmData.cend(), 
      [] (Hades::Memory::DisasmData const& Current)
    {
      std::wcout << reinterpret_cast<PVOID>(Current.Disasm.VirtualAddr) << " " 
        << Current.Disasm.CompleteInstr << std::endl;
    });

    // Print elapsed time
    std::wcout << "\nElapsed Time: " << ProgTimer.elapsed() << "." << 
      std::endl;

    // Stop window from automatically closing if required
    if (KeepOpen)
    {
      std::wcin.clear();
      std::wcin.sync();
      std::wcin.get();
    }
  }
  catch (std::exception const& e)
  {
    // Dump error information
    std::cout << boost::diagnostic_information(e);

    // Print elapsed time
    std::wcout << "\nElapsed Time: " << ProgTimer.elapsed() << "." << 
      std::endl;

    // Always keep window open in case of an error
    std::wcin.clear();
    std::wcin.sync();
    std::wcin.get();
  }
}
