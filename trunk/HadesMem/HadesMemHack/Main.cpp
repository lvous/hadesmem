// HadesMem
#include "IoAux.h"
#include "Types.h"
#include "HadesMem/Module.h"
#include "HadesMem/Memory.h"
#include "HadesMem/Disassemble.h"
#include "HadesMem/FindPattern.h"

// C++ Standard Library
#include <limits>
#include <memory>
#include <fstream>
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
    auto ProcSelect = static_cast<Detail::ProcSelect>(GetOption(L"process "
      L"selection method", 1, 3));

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

      // Get task
      auto Task = static_cast<Detail::Task>(GetOption(L"task", 1, 8));

      // Check for task 'Read Memory' or 'Write Memory' and output accordingly
      if (Task == Detail::Task_ReadMem || Task == Detail::Task_WriteMem)
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
        auto MyDataType = static_cast<Detail::DataType>(GetOption(L"data type", 
          1, 14));

        // Output
        std::wcout << "Enter target address:" << std::endl;

        // Get address
        auto Address = ReadHexNumericDataFromUser<PVOID>();

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
                std::wcout << "Invalid data." << std::endl;
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
          HandleStringReadOrWrite<Detail::CharNarrow>(*MyMemory, Address, Task, 
            std::cin, std::cout);
          break;

        case Detail::DataType_StrWide:
          HandleStringReadOrWrite<Detail::CharWide>(*MyMemory, Address, Task, 
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
              auto New = ReadHexNumericDataFromUser<Detail::Pointer>();

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
      else if (Task == Detail::Task_FindMod)
      {
        // Output
        std::wcout << "Choose a module search method:" << std::endl;
        std::wcout << "1. Name." << std::endl;
        std::wcout << "2. Handle." << std::endl;
        std::wcout << "3. List." << std::endl;

        // Get module search method
        auto ModSelect = static_cast<Detail::ModSelect>(GetOption(
          L"module search method", 1, 3));

        // Module pointer
        std::shared_ptr<Hades::Memory::Module> MyModule;

        switch (ModSelect) 
        {
          // Search via name
        case Detail::ModSelect_Name:
          {
            // Output
            std::wcout << "Enter target module name:" << std::endl;

            // Get name
            std::wstring ModName;
            while (!std::getline(std::wcin, ModName) || ModName.empty())
            {
              std::wcout << "Invalid name." << std::endl;
              std::wcin.clear();
            }

            // Get module
            MyModule.reset(new Hades::Memory::Module(ModName, *MyMemory));

            break;
          }

          // Search via base
        case Detail::ModSelect_Base:
          {
            // Output
            std::wcout << "Enter target module base:" << std::endl;

            // Get handle
            auto ModHandle = ReadHexNumericDataFromUser<PVOID>();

            // Get module
            MyModule.reset(new Hades::Memory::Module(reinterpret_cast<HMODULE>(
              ModHandle), *MyMemory));

            break;
          }

          // List all modules
        case Detail::ModSelect_List:
          {
            // Get module list
            auto ModList(Hades::Memory::GetModuleList(*MyMemory));

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
        else
        {
          std::wcout << "Module not found." << std::endl;
        }
      }
      // Handle 'Call Function' task
      else if (Task == Detail::Task_CallFunc)
      {
        // Output
        std::wcout << "Enter target address:" << std::endl;

        // Get address
        auto Address = ReadHexNumericDataFromUser<PVOID>();

        // Call remote function
        auto ExitCode = MyMemory->Call<DWORD (DWORD)>(Address, 0);
        
        // Output
        std::wcout << "Thread Exit Code: " << ExitCode << "." << std::endl;
      }
      // Handle 'Disassemble' task
      else if (Task == Detail::Task_Disassemble)
      {
        // Get target offset
        std::wcout << "Enter target offset:" << std::endl;
        auto Offset = ReadHexNumericDataFromUser<PVOID>();

        // Get number of instructions to disassemble
        std::wcout << "Enter number of instructions:" << std::endl;
        auto NumInstructions = ReadNumericDataFromUser<DWORD>();

        // Create disassembler instance
        Hades::Memory::Disassembler MyDisassembler(*MyMemory);
        // Test disassembler
        MyDisassembler.DisassembleTest(Offset, NumInstructions);
      }
      // Handle 'Pattern Scan' task
      else if (Task == Detail::Task_PatternScan)
      {
        // Output
        std::wcout << "Choose a pattern scan method:" << std::endl;
        std::wcout << "1. Manual." << std::endl;
        std::wcout << "2. Automatic (XML)." << std::endl;

        // Get pattern scan method
        auto PatScanMethod = GetOption(L"pattern scan method", 1, 2);

        // Handle manual pattern scanning
        if (PatScanMethod == 1)
        {
          // Get mask
          std::wcout << "Enter mask:" << std::endl;
          std::string Mask;
          while (!(std::cin >> Mask) || Mask.empty())
          {
            std::wcout << "Invalid mask." << std::endl;
            std::cin.clear();
          }

          // Get data
          std::wcout << "Enter data:" << std::endl;
          std::string Data;
          while (!(std::cin >> Data) || Data.empty())
          {
            std::wcout << "Invalid data." << std::endl;
            std::wcin.clear();
          }

          // Ensure data is valid
          if (Data.size() % 2)
          {
            std::wcout << "Invalid data (size of data must be a multiple of "
              "2)." << std::endl;
            continue;
          }

          // Ensure data is valid
          if (Mask.size() * 2 != Data.size())
          {
            std::wcout << "Invalid data (the data must match the mask)." 
              << std::endl;
            continue;
          }

          // Convert data to byte buffer
          std::vector<BYTE> DataReal;
          for (auto i = Data.begin(); i != Data.end(); i += 2)
          {
            std::string CurrentStr(i, i + 2);
            std::stringstream Converter(CurrentStr);
            int Current = 0;
            if (!(Converter >> std::hex >> Current >> std::dec))
            {
              std::cout << "Invalid data (could not convert data to byte "
                "buffer)." << std::endl;
              continue;
            }
            DataReal.push_back(static_cast<BYTE>(Current));
          }

          // Ensure conversion was successful
          if (Data.size() != DataReal.size() * 2)
          {
            std::cout << "Data conversion failed." << std::endl;
            continue;
          }

          // Create pattern scanner
          Hades::Memory::FindPattern MyFindPattern(*MyMemory);

          // Perform pattern scan
          PVOID Address = MyFindPattern.Find(Mask, DataReal);

          // Output
          std::wcout << "Address: " << Address << "." << std::endl;
        }
        // Handle automatic pattern scanning
        else if (PatScanMethod == 2)
        {
          // Create pattern scanner
          Hades::Memory::FindPattern MyFindPattern(*MyMemory);

          // Output
          std::wcout << "Enter pattern file path:" << std::endl;

          // Get name
          std::wstring PatFilePath;
          while (!std::getline(std::wcin, PatFilePath) || PatFilePath.empty())
          {
            std::wcout << "Invalid path." << std::endl;
            std::wcin.clear();
          }

          // Load patterns from XML file
          MyFindPattern.LoadFromXML(PatFilePath);

          // Dump all patterns in scanner
          std::map<std::wstring, PVOID> const Addresses(MyFindPattern.
            GetAddresses());
          for (auto i = Addresses.begin(); i != Addresses.end(); ++i)
          {
            std::wcout << "Name: " << i->first << "." << std::endl;
            std::wcout << "Address: " << i->second << "." << std::endl;
          }

          // Output
          std::wcout << "Enter a path to dump to file (or nothing otherwise):" 
            << std::endl;

          // Get name
          std::wstring DumpPath;
          while (!std::getline(std::wcin, DumpPath))
          {
            std::wcout << "Invalid filename." << std::endl;
            std::wcin.clear();
          }

          // Dump if path specified
          if (!DumpPath.empty())
          {
            std::wofstream MyDumpFile(DumpPath.c_str(), std::ios::trunc);
            MyDumpFile << "// Dump generated by HadesMemHack.\n\n";
            for (auto i = Addresses.begin(); i != Addresses.end(); ++i)
            {
              MyDumpFile << i->second << "\t" << i->first << "\n";
            }
          }

        }
        // Catch unsupported pattern scan methods
        else
        {
          assert(!"Unsupported pattern scan method.");
        }
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
