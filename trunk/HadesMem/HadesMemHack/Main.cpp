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

// C++ Standard Library
#include <limits>
#include <memory>
#include <fstream>
#include <iterator>
#include <iostream>

// HadesMem
#include "IoAux.h"
#include "Types.h"
#include "Tasks.h"
#include "HadesMem/Memory.h"

// Windows API
#include <Windows.h>

// Program entry-point.
int wmain(int /*argc*/, wchar_t* /*argv*/[], wchar_t* /*envp*/[])
{
  for (;;)
  {
    try
    {
      // Output
      std::wcout << "Welcome to HadesMemHack." << std::endl;

      // Output
      std::wcout << "Choose a process selection method:" << std::endl;
      std::wcout << "1. Process name." << std::endl;
      std::wcout << "2. Process ID." << std::endl;
      std::wcout << "3. Window name." << std::endl;

      // Get process selection method
      auto ProcSelect = GetOption<Detail::ProcSelect>(L"process selection "
        L"method", 1, 3);

      // Get process selection data and create memory manager
      std::shared_ptr<Hades::Memory::MemoryMgr> MyMemory;
      switch (ProcSelect)
      {
        // Select via process name
      case Detail::ProcSelect_ProcName:
        CreateMemoryFromProcName(MyMemory);
        break;

        // Select via process ID
      case Detail::ProcSelect_ProcID:
        CreateMemoryFromProcID(MyMemory);
        break;

        // Select via window name
      case Detail::ProcSelect_WindowName:
        CreateMemoryFromWindowName(MyMemory);
        break;

      default:
        assert(!"Unsupported process selection method.");
      }

      // Just keep prompting for input until an exception is thrown
      for (;;)
      {
        // Output
        std::wcout << "Choose a task:" << std::endl;
        std::wcout << "1. Read memory." << std::endl;
        std::wcout << "2. Write memory." << std::endl;
        std::wcout << "3. Find module." << std::endl;
        std::wcout << "4. Call function." << std::endl;
        std::wcout << "5. Allocate memory." << std::endl;
        std::wcout << "6. Free memory." << std::endl;
        std::wcout << "7. Disassemble." << std::endl;
        std::wcout << "8. Pattern scan." << std::endl;
        std::wcout << "9. Inject DLL." << std::endl;
        std::wcout << "10. Manually map DLL." << std::endl;
        std::wcout << "11. Search memory." << std::endl;
        std::wcout << "12. Query memory." << std::endl;
        std::wcout << "13. Run script." << std::endl;

        // Get task
        auto Task = GetOption<Detail::Task>(L"task", 1, 13);

        // Check for task 'Read Memory' or 'Write Memory' and output 
        // accordingly
        if (Task == Detail::Task_ReadMem || Task == Detail::Task_WriteMem)
        {
          HandleMemReadOrWrite(Task, MyMemory);
        }
        // Check for task 'Find Module'
        else if (Task == Detail::Task_FindMod)
        {
          HandleFindMod(Task, MyMemory);
        }
        // Handle 'Call function' task
        else if (Task == Detail::Task_CallFunc)
        {
          HandleCallFunc(Task, MyMemory);
        }
        // Handle 'Allocate memory' task
        else if (Task == Detail::Task_AllocMem)
        {
          HandleAllocMem(Task, MyMemory);
        }
        // Handle 'Free memory' task
        else if (Task == Detail::Task_FreeMem)
        {
          HandleFreeMem(Task, MyMemory);
        }
        // Handle 'Disassemble' task
        else if (Task == Detail::Task_Disassemble)
        {
          HandleDisassemble(Task, MyMemory);
        }
        // Handle 'Pattern Scan' task
        else if (Task == Detail::Task_PatternScan)
        {
          HandlePatternScan(Task, MyMemory);
        }
        // Handle 'Inject DLL' task
        else if (Task == Detail::Task_InjectDLL)
        {
          HandleInjectDLL(Task, MyMemory);
        }
        // Handle 'Manually map DLL' task
        else if (Task == Detail::Task_ManualMap)
        {
          HandleManualMap(Task, MyMemory);
        }
        // Handle 'Search memory' task
        else if (Task == Detail::Task_SearchMem)
        {
          HandleSearchMem(Task, MyMemory);
        }
        // Handle 'Query memory' task
        else if (Task == Detail::Task_QueryMem)
        {
          HandleQueryMem(Task, MyMemory);
        }
        // Handle 'Run script' task
        else if (Task == Detail::Task_RunScript)
        {
          HandleRunScript(Task, MyMemory);
        }
        // Output for all currently unhandled tasks
        else
        {
          std::wcout << "Sorry, that task is not yet supported." << std::endl;
        }
      }
    }
    catch (boost::exception const& e)
    {
      // Dump error information
      std::cout << boost::diagnostic_information(e);
    }
    catch (std::exception const& e)
    {
      // Dump error information
      std::wcout << "Error! " << e.what() << std::endl;
    }

    // Pause for input before continuing
    std::wcin.clear();
    std::wcin.sync();
    std::wcin.get();
  }
}
