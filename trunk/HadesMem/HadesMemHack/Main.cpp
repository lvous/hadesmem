// HadesMem
#include "IoAux.h"
#include "Types.h"
#include "HadesMem/Types.h"
#include "HadesMem/Module.h"
#include "HadesMem/Memory.h"
#include "HadesMem/Region.h"
#include "HadesMem/Scanner.h"
#include "HadesMem/Injector.h"
#include "HadesMem/ManualMap.h"
#include "HadesMem/Scripting.h"
#include "HadesMem/Disassemble.h"

// C++ Standard Library
#include <limits>
#include <memory>
#include <fstream>
#include <iterator>
#include <iostream>

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
          auto MyDataType = GetOption<Detail::DataType>(L"data type", 1, 14);

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
                  std::wcin.ignore((std::numeric_limits<std::streamsize>::max)
                    (), '\n');
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
            HandleStringReadOrWrite<Detail::CharNarrow>(*MyMemory, Address, 
              Task, std::cin, std::cout);
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
          auto ModSelect = GetOption<Detail::ModSelect>(L"module search "
            L"method", 1, 3);

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
              MyModule.reset(new Hades::Memory::Module(
                reinterpret_cast<HMODULE>(ModHandle), *MyMemory));

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
                std::wcout << *MyModule << std::endl;
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
        // Handle 'Call function' task
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
        // Handle 'Allocate memory' task
        else if (Task == Detail::Task_AllocMem)
        {
          // Output
          std::wcout << "Enter region size:" << std::endl;

          // Get region size
          auto Size = ReadHexNumericDataFromUser<SIZE_T>();

          // Allocate memory
          auto Address = MyMemory->Alloc(Size);

          // Output
          std::wcout << "Memory allocated at address " << Address << "." 
            << std::endl;

        }
        // Handle 'Free memory' task
        else if (Task == Detail::Task_FreeMem)
        {
          // Output
          std::wcout << "Enter region address:" << std::endl;

          // Get region address
          auto Address = ReadHexNumericDataFromUser<PVOID>();

          // Free memory
          MyMemory->Free(Address);

          // Output
          std::wcout << "Memory freed at address " << Address << "." 
            << std::endl;
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
          auto DisasmResults(MyDisassembler.Disassemble(Offset, 
            NumInstructions));

          // Print results
          std::copy(DisasmResults.begin(), DisasmResults.end(), 
            std::ostream_iterator<std::string>(std::cout, "\n"));
        }
        // Handle 'Pattern Scan' task
        else if (Task == Detail::Task_PatternScan)
        {
          // Output
          std::wcout << "Choose a pattern scan method:" << std::endl;
          std::wcout << "1. Manual." << std::endl;
          std::wcout << "2. Automatic (XML)." << std::endl;

          // Get pattern scan method
          auto PatScanMethod = GetOption<int>(L"pattern scan method", 1, 2);

          // Handle manual pattern scanning
          if (PatScanMethod == 1)
          {
            // Get mask
            std::wcout << "Enter mask:" << std::endl;
            std::wstring Mask;
            while (!(std::wcin >> Mask) || Mask.empty())
            {
              std::wcout << "Invalid mask." << std::endl;
              std::wcin.clear();
            }

            // Get data
            std::wcout << "Enter data:" << std::endl;
            std::string Data;
            while (!(std::cin >> Data) || Data.empty())
            {
              std::wcout << "Invalid data." << std::endl;
              std::cin.clear();
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
            Hades::Memory::Scanner MyScanner(*MyMemory);

            // Perform pattern scan
            PVOID Address = MyScanner.Find(DataReal, Mask);

            // Output
            std::wcout << "Address: " << Address << "." << std::endl;
          }
          // Handle automatic pattern scanning
          else if (PatScanMethod == 2)
          {
            // Create pattern scanner
            Hades::Memory::Scanner MyScanner(*MyMemory);

            // Output
            std::wcout << "Enter pattern file path:" << std::endl;

            // Get name
            std::wstring PatFilePath;
            while (!std::getline(std::wcin, PatFilePath) || 
              PatFilePath.empty())
            {
              std::wcout << "Invalid path." << std::endl;
              std::wcin.clear();
            }

            // Load patterns from XML file
            MyScanner.LoadFromXML(PatFilePath);

            // Dump all patterns in scanner
            std::map<std::wstring, PVOID> const Addresses(MyScanner.
              GetAddresses());
            for (auto i = Addresses.begin(); i != Addresses.end(); ++i)
            {
              std::wcout << "Name: " << i->first << "." << std::endl;
              std::wcout << "Address: " << i->second << "." << std::endl;
            }

            // Output
            std::wcout << "Enter a path to dump to file (or nothing "
              "otherwise):" << std::endl;

            // Get name
            std::wstring DumpPath;
            while (!std::getline(std::wcin, DumpPath))
            {
              std::wcout << "Invalid path." << std::endl;
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
        // Handle 'Inject DLL' task
        else if (Task == Detail::Task_InjectDLL)
        {
          // Output
          std::wcout << "Enter path to DLL:" << std::endl;

          // Get name
          std::wstring LibPath;
          while (!std::getline(std::wcin, LibPath) || LibPath.empty())
          {
            std::wcout << "Invalid path." << std::endl;
            std::wcin.clear();
          }

          // Output
          std::wcout << "Enter name of export (optional):" << std::endl;

          // Get export
          std::string Export;
          while (!std::getline(std::cin, Export))
          {
            std::wcout << "Invalid export." << std::endl;
            std::cin.clear();
          }

          // Create DLL injector
          Hades::Memory::Injector MyInjector(*MyMemory);
          DWORD ReturnValue = 0;
          HMODULE ModBase = MyInjector.InjectDll(LibPath, Export, ReturnValue);

          // Output
          std::wcout << "Module Base: " << ModBase << "." << std::endl;
        }
        // Handle 'Manuall map DLL' task
        else if (Task == Detail::Task_ManualMap)
        {
          // Output
          std::wcout << "Enter path to DLL:" << std::endl;

          // Get name
          std::wstring LibPath;
          while (!std::getline(std::wcin, LibPath) || LibPath.empty())
          {
            std::wcout << "Invalid path." << std::endl;
            std::wcin.clear();
          }

          // Output
          std::wcout << "Enter name of export (optional):" << std::endl;

          // Get export
          std::string Export;
          while (!std::getline(std::cin, Export))
          {
            std::wcout << "Invalid export." << std::endl;
            std::cin.clear();
          }

          // Create DLL injector
          Hades::Memory::ManualMap MyManualMapper(*MyMemory);
          PVOID ModuleBase = MyManualMapper.Map(LibPath, Export);

          // Output
          std::wcout << "Module Base: " << ModuleBase << "." << std::endl;
        }
        // Handle 'Search memory' task
        else if (Task == Detail::Task_SearchMem)
        {

          // Output
          std::wcout << "Enter start address:" << std::endl;

          // Get start address
          auto Start = ReadHexNumericDataFromUser<PVOID>();

          // Output
          std::wcout << "Enter end address:" << std::endl;

          // Get end address
          auto End = ReadHexNumericDataFromUser<PVOID>();

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
          auto MyDataType = GetOption<Detail::DataType>(L"data type", 1, 14);

          // Handle selected data type
          switch (MyDataType)
          {
          case Detail::DataType_Byte:
            {
              // Output
              std::wcout << "Enter data:" << std::endl;

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

              // Real data
              auto Data = static_cast<Detail::Byte>(New);

              // Find data
              Hades::Memory::Scanner MyScanner(*MyMemory, Start, End);
              PVOID Address = MyScanner.Find(Data);

              // Output
              std::wcout << "Address: " << Address << std::endl;

              break;
            }

          case Detail::DataType_Int16:
            HandleNumericSearch<Detail::Int16>(*MyMemory, Start, End);
            break;

          case Detail::DataType_UInt16:
            HandleNumericSearch<Detail::UInt16>(*MyMemory, Start, End);
            break;

          case Detail::DataType_Int32:
            HandleNumericSearch<Detail::Int32>(*MyMemory, Start, End);
            break;

          case Detail::DataType_UInt32:
            HandleNumericSearch<Detail::UInt32>(*MyMemory, Start, End);
            break;

          case Detail::DataType_Int64:
            HandleNumericSearch<Detail::Int64>(*MyMemory, Start, End);
            break;

          case Detail::DataType_UInt64:
            HandleNumericSearch<Detail::UInt64>(*MyMemory, Start, End);
            break;

          case Detail::DataType_Float:
            HandleNumericSearch<Detail::Float>(*MyMemory, Start, End);
            break;

          case Detail::DataType_Double:
            HandleNumericSearch<Detail::Double>(*MyMemory, Start, End);
            break;

        case Detail::DataType_StrNarrow:
          HandleStringSearch<Detail::CharNarrow>(*MyMemory, Start, End, 
            std::cin, std::cout);
          break;

        case Detail::DataType_StrWide:
          HandleStringSearch<Detail::CharWide>(*MyMemory, Start, End, 
            std::wcin, std::wcout);
          break;

        case Detail::DataType_CharNarrow:
          HandleCharSearch<Detail::CharNarrow>(*MyMemory, Start, End, std::cin, 
            std::cout);
          break;

        case Detail::DataType_CharWide:
          HandleCharSearch<Detail::CharWide>(*MyMemory, Start, End, std::wcin, 
            std::wcout);
          break;

          case Detail::DataType_Pointer:
            {
              // Output
              std::wcout << "Enter data:" << std::endl;

              // Get data
              auto Data = ReadHexNumericDataFromUser<Detail::Pointer>();

              // Find data
              Hades::Memory::Scanner MyScanner(*MyMemory, Start, End);
              PVOID Address = MyScanner.Find(Data);

              // Output
              std::wcout << "Address: " << Address << std::endl;

              break;
            }

          default:
            // Catch unsupported type
            assert(!"Unsupported data type.");
          }
        }
        // Handle 'Query memory' task
        else if (Task == Detail::Task_QueryMem)
        {
          auto RegionList(GetRegionList(*MyMemory));
          std::for_each(RegionList.begin(), RegionList.end(), 
            [] (std::shared_ptr<Hades::Memory::MemoryRegion> Current) 
          {
            std::wcout << *Current << std::endl;
          });
        }
        // Handle 'Run script' task
        else if (Task == Detail::Task_RunScript)
        {
          // Create script manager
          Hades::Memory::ScriptMgr MyScriptMgr;

          // Output
          std::wcout << "Enter script:" << std::endl;

          // Get script
          std::string Script;
          while (!std::getline(std::cin, Script) || Script.empty())
          {
            std::wcout << "Invalid script." << std::endl;
            std::cin.clear();
          }

          // Run script
          MyScriptMgr.RunString(Script);
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
