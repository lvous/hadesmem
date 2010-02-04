// HadesMem
#include "IoAux.h"
#include "Types.h"
#include "HadesMem/Module.h"
#include "HadesMem/Memory.h"

// C++ Standard Library
#include <limits>
#include <memory>
#include <iostream>

// Windows API
#include <Windows.h>

// Program entry-point.
int wmain(int /*argc*/, wchar_t* /*argv*/[], wchar_t* /*envp*/[])
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
    auto ProcSelect = static_cast<Detail::ProcSelect>(GetOption("process "
      "selection method", 1, 3));

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

      // Get task
      auto Task = static_cast<Detail::Task>(GetOption("task", 1, 6));

      // Check for task 'Read Memory' or 'Write Memory' and output accordingly
      if (Task == Detail::Task_ReadMem || Task == Detail::Task_ReadMem)
      {
        // Output
        std::wcout << "Choose a data type:" << std::endl;
        std::wcout << "1. Byte." << std::endl;
        std::wcout << "2. Int16." << std::endl;
        std::wcout << "3. UInt16." << std::endl;
        std::wcout << "4. Int32." << std::endl;
        std::wcout << "5. UInt32." << std::endl;
        std::wcout << "6. Int64." << std::endl;
        std::wcout << "7. UInt64." << std::endl;
        std::wcout << "8. Float." << std::endl;
        std::wcout << "9. Double." << std::endl;
        std::wcout << "10. String (Narrow)." << std::endl;
        std::wcout << "11. String (Wide)." << std::endl;
        std::wcout << "12. Char (Narrow)." << std::endl;
        std::wcout << "13. Char (Wide)." << std::endl;
        std::wcout << "14. Pointer." << std::endl;

        // Get data type
        auto MyDataType = static_cast<Detail::DataType>(GetOption("data type", 
          1, 14));

        // Output
        std::wcout << "Enter target address:" << std::endl;

        // Get address
        PVOID Address = 0;
        while (!(std::wcin >> std::hex >> Address >> std::dec) || !Address)
        {
          std::cout << "Invalid address." << std::endl;
          std::wcin.clear();
          std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        }
        std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');

        // Handle selected data type
        switch (MyDataType)
        {
        case Detail::DataType_Byte:
          {
            // Read data
            auto Current = MyMemory->Read<Detail::Byte>(Address);
            std::wcout << "Value: " << static_cast<Detail::Int32>(Current) 
              << std::endl;

            // Handle 'Write Memory' task
            if (Task == Detail::Task_WriteMem)
            {
              // Output
              std::wcout << "Enter new value:" << std::endl;

              // Get data
              Detail::Int32 New = 0;
              while (!(std::wcin >> New) || New > 256)
              {
                std::cout << "Invalid data." << std::endl;
                std::wcin.clear();
                std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), 
                  '\n');
              }
              std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), 
                '\n');

              // Write data
              MyMemory->Write(Address, static_cast<Detail::Byte>(New));
            }

            break;
          }

        case Detail::DataType_Int16:
          HandleNumericReadOrWrite<Detail::Int16>(*MyMemory, Address, Task);
          break;

        case Detail::DataType_UInt16:
          HandleNumericReadOrWrite<Detail::UInt16>(*MyMemory, Address, Task);
          break;

        case Detail::DataType_Int32:
          HandleNumericReadOrWrite<Detail::Int32>(*MyMemory, Address, Task);
          break;

        case Detail::DataType_UInt32:
          HandleNumericReadOrWrite<Detail::UInt32>(*MyMemory, Address, Task);
          break;

        case Detail::DataType_Int64:
          HandleNumericReadOrWrite<Detail::Int64>(*MyMemory, Address, Task);
          break;

        case Detail::DataType_UInt64:
          HandleNumericReadOrWrite<Detail::UInt64>(*MyMemory, Address, Task);
          break;

        case Detail::DataType_Float:
          HandleNumericReadOrWrite<Detail::Float>(*MyMemory, Address, Task);
          break;

        case Detail::DataType_Double:
          HandleNumericReadOrWrite<Detail::Double>(*MyMemory, Address, Task);
          break;

        case Detail::DataType_StrNarrow:
          HandleStringReadOrWrite<Detail::StrNarrow>(*MyMemory, Address, Task, 
            std::cin, std::cout);
          break;

        case Detail::DataType_StrWide:
          HandleStringReadOrWrite<Detail::StrWide>(*MyMemory, Address, Task, 
            std::wcin, std::wcout);
          break;

        case Detail::DataType_CharNarrow:
          HandleCharReadOrWrite<Detail::CharNarrow>(*MyMemory, Address, Task, 
            std::cin, std::cout);
          break;

        case Detail::DataType_CharWide:
          HandleCharReadOrWrite<Detail::CharWide>(*MyMemory, Address, Task, 
            std::wcin, std::wcout);
          break;

        case Detail::DataType_Pointer:
          {
            // Read data
            auto Current = MyMemory->Read<Detail::Pointer>(Address);
            std::wcout << "Value: " << Current << std::endl;

            // Handle 'Write Memory' task
            if (Task == Detail::Task_WriteMem)
            {
              // Output
              std::wcout << "Enter new value:" << std::endl;

              // Get data
              Detail::Pointer New = 0;
              while (!(std::wcin >> std::hex >> New >> std::dec))
              {
                std::cout << "Invalid address." << std::endl;
                std::wcin.clear();
                std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), 
                  '\n');
              }
              std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), 
                '\n');

              // Write data
              MyMemory->Write(Address, New);
            }

            break;
          }

        default:
          // Catch unsupported type
          assert(!"Unsupported data type.");
        }
      }
      // Check for task 'Find Module'
      if (Task == Detail::Task_FindMod)
      {
        // Output
        std::wcout << "Choose a module search method:" << std::endl;
        std::wcout << "1. Name." << std::endl;
        std::wcout << "2. Handle." << std::endl;
        std::wcout << "3. List." << std::endl;

        // Get module search method
        auto ModSelect = static_cast<Detail::ModSelect>(GetOption(
          "module search method", 1, 3));

        // Module pointer
        std::shared_ptr<Hades::Memory::Module> MyModule;

        switch (ModSelect) 
        {
        case Detail::ModSelect_Name:
          {
            // Output
            std::wcout << "Enter target module name:" << std::endl;

            // Get name
            std::wstring ModName;
            while (!std::getline(std::wcin, ModName) || ModName.empty())
            {
              std::cout << "Invalid name." << std::endl;
              std::wcin.clear();
            }

            // Get module
            MyModule.reset(new Hades::Memory::Module(ModName, *MyMemory));

            break;
          }

        case Detail::ModSelect_Base:
          {
            // Output
            std::wcout << "Enter target module base:" << std::endl;

            // Get handle
            PVOID ModHandle = 0;
            while (!(std::wcin >> std::hex >> ModHandle >> std::dec))
            {
              std::cout << "Invalid handle." << std::endl;
              std::wcin.clear();
              std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), 
                '\n');
            }
            std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), 
              '\n');

            // Get module
            MyModule.reset(new Hades::Memory::Module(reinterpret_cast<HMODULE>(
              ModHandle), *MyMemory));

            break;
          }

        case Detail::ModSelect_List:
          {
            // Get module list
            auto ModList(GetModuleList(*MyMemory));

            // Dump module info
            std::for_each(ModList.begin(), ModList.end(), 
              [] (std::shared_ptr<Hades::Memory::Module> MyModule)
            {
              std::wcout << *MyModule;
            });

            // Finished
            continue;
          }

        default:
          assert(!"Unsupported module search method.");
        }

        // Dump module info if found
        if (MyModule->Found())
        {
          std::wcout << *MyModule;
        }
        // Output if not found
        {
          std::wcout << "Module not found." << std::endl;
        }
      }
      // Handle 'Call Function' task
      if (Task == Detail::Task_CallFunc)
      {
        // Output
        std::wcout << "Enter target address:" << std::endl;

        // Get address
        PVOID Address = 0;
        while (!(std::wcin >> std::hex >> Address >> std::dec))
        {
          std::cout << "Invalid handle." << std::endl;
          std::wcin.clear();
          std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), 
            '\n');
        }
        std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), 
          '\n');

        // Call remote function
        auto ExitCode = MyMemory->Call(Address);
        
        // Output
        std::wcout << "Thread Exit Code: " << reinterpret_cast<PVOID>(
          ExitCode) << "." << std::endl;
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

  // Stop console window closing
  std::wcin.clear();
  std::wcin.sync();
  std::wcin.get();
}
