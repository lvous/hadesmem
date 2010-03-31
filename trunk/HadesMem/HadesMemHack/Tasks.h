/*
This file is part of HadesMem.
Copyright © 2010 Cypherjb (aka Chazwazza, aka Cypher). 
<http://www.cypherjb.com/> <cypher.jb@gmail.com>

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

// C++ Standard Library
#include <string>
#include <memory>
#include <vector>
#include <iostream>

// HadesMem
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

// Create Memory object using process name from user
inline void CreateMemoryFromProcName(std::shared_ptr<Hades::Memory::MemoryMgr>& 
  MyMemory)
{
  // Output
  std::wcout << "Please enter the target process name." << std::endl;

  // Get process name
  std::wstring ProcessName;
  while (!std::getline(std::wcin, ProcessName) || ProcessName.empty())
  {
    std::wcout << "Invalid process name." << std::endl;
    std::wcin.clear();
  }

  // Create memory manager
  MyMemory.reset(new Hades::Memory::MemoryMgr(ProcessName));
}

// Create Memory object using process ID from user
inline void CreateMemoryFromProcID(std::shared_ptr<Hades::Memory::MemoryMgr>& 
  MyMemory)
{
  // Output
  std::wcout << "Please enter the target process ID." << std::endl;

  // Get process ID
  DWORD ProcessID;
  while (!(std::wcin >> ProcessID) || !ProcessID)
  {
    std::wcout << "Invalid process ID." << std::endl;
    std::wcin.clear();
    std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), 
      '\n');
  }
  std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');

  // Create memory manager
  MyMemory.reset(new Hades::Memory::MemoryMgr(ProcessID));
}

// Create Memory object using window name from user
inline void CreateMemoryFromWindowName(std::shared_ptr<
  Hades::Memory::MemoryMgr>& MyMemory)
{
  // Output
  std::wcout << "Please enter the target window name." << std::endl;

  // Get window name
  std::wstring WindowName;
  while (!std::getline(std::wcin, WindowName) || WindowName.empty())
  {
    std::wcout << "Invalid window name." << std::endl;
    std::wcin.clear();
  }

  // Create memory manager
  MyMemory.reset(new Hades::Memory::MemoryMgr(WindowName, nullptr));
}

// Handle 'MemoryRead' or 'MemoryWrite' task for string types
template <typename CharT>
void HandleStringReadOrWrite(Hades::Memory::MemoryMgr const& MyMemory, 
  PVOID Address, int Task, std::basic_istream<CharT>& In, 
  std::basic_ostream<CharT>& Out)
{
  // Read data
  auto Current = MyMemory.Read<std::basic_string<CharT>>(Address);
  Out << "Value: " << Current << std::endl;

  // Handle 'Write Memory' task
  if (Task == Detail::Task_WriteMem)
  {
    // Output
    Out << "Enter new value:" << std::endl;

    // Get data
    auto New = ReadStringDataFromUser<CharT>(In, Out);

    // Write data
    MyMemory.Write(Address, New);
  }
}

// Handle 'MemoryRead' or 'MemoryWrite' task for char types
template <typename T, typename CharT>
void HandleCharReadOrWrite(Hades::Memory::MemoryMgr const& MyMemory, 
  PVOID Address, int Task, std::basic_istream<CharT>& In, 
  std::basic_ostream<CharT>& Out)
{
  // Read data
  auto Current = MyMemory.Read<T>(Address);
  Out << "Value: " << Current << std::endl;

  // Handle 'Write Memory' task
  if (Task == Detail::Task_WriteMem)
  {
    // Output
    Out << "Enter new value:" << std::endl;

    // Get data
    auto New = ReadCharDataFromUser<T, CharT>(In, Out);

    // Write data
    MyMemory.Write(Address, New);
  }
}

// Handle 'MemoryRead' or 'MemoryWrite' task for numeric types
template <typename T>
void HandleNumericReadOrWrite(Hades::Memory::MemoryMgr const& MyMemory, 
  PVOID Address, int Task)
{
  // Read data
  auto Current = MyMemory.Read<T>(Address);
  std::wcout << "Value: " << Current << std::endl;

  // Handle 'Write Memory' task
  if (Task == Detail::Task_WriteMem)
  {
    // Output
    std::wcout << "Enter new value:" << std::endl;

    // Get data
    auto New = ReadNumericDataFromUser<T>();

    // Write data
    MyMemory.Write(Address, New);
  }
}

// Handle 'Search memory' task for generic types
template <typename T>
void HandleGenericSearch(Hades::Memory::MemoryMgr const& MyMemory, 
  PVOID Start, PVOID End, int SearchMode, T Data)
{
  // Find data
  Hades::Memory::Scanner MyScanner(MyMemory, Start, End);
  if (SearchMode == 1)
  {
    // Find data
    PVOID Address = MyScanner.Find(Data);

    // Output
    std::wcout << "Address: " << Address << std::endl;
  }
  else if (SearchMode == 2)
  {
    // Find data
    auto Addresses(MyScanner.FindAll(Data));

    // Output
    std::for_each(Addresses.begin(), Addresses.end(), 
      [] (PVOID Address)
    {
      std::wcout << "Address: " << Address << std::endl;
    });
  }
  else
  {
    assert(!"Unsupported search mode.");
  }
}

// Handle 'Search memory' task for numeric types
template <typename T>
void HandleNumericSearch(Hades::Memory::MemoryMgr const& MyMemory, 
  PVOID Start, PVOID End, int SearchMode)
{
  // Output
  std::wcout << "Enter data:" << std::endl;

  // Get data
  auto Data = ReadNumericDataFromUser<T>();

  // Find data
  HandleGenericSearch(MyMemory, Start, End, SearchMode, Data);
}

// Handle 'Search memory' task for string types
template <typename CharT>
void HandleStringSearch(Hades::Memory::MemoryMgr const& MyMemory, 
  PVOID Start, PVOID End, std::basic_istream<CharT>& In, 
  std::basic_ostream<CharT>& Out, int SearchMode)
{
  // Output
  std::wcout << "Enter data:" << std::endl;

  // Get data
  auto Data = ReadStringDataFromUser<CharT>(In, Out);

  // Find data
  HandleGenericSearch(MyMemory, Start, End, SearchMode, Data);
}

// Handle 'Search memory' task for char types
template <typename T, typename CharT>
void HandleCharSearch(Hades::Memory::MemoryMgr const& MyMemory, 
  PVOID Start, PVOID End, std::basic_istream<CharT>& In, 
  std::basic_ostream<CharT>& Out, int SearchMode)
{
  // Output
  std::wcout << "Enter data:" << std::endl;

  // Get data
  auto Data = ReadCharDataFromUser<CharT>(In, Out);

  // Find data
  HandleGenericSearch(MyMemory, Start, End, SearchMode, Data);
}

// Handle 'Memory read' or 'Memory write' task
inline void HandleMemReadOrWrite(Detail::Task Task, 
  std::shared_ptr<Hades::Memory::MemoryMgr> MyMemory)
{
  // Output
  PrintDataTypeList();

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
      auto Current = MyMemory->Read<Hades::Memory::Types::Byte>(
        Address);
      std::wcout << "Value: " << static_cast<Hades::Memory::Types::
        Int32>(Current) << std::endl;

      // Handle 'Write Memory' task
      if (Task == Detail::Task_WriteMem)
      {
        // Output
        std::wcout << "Enter new value:" << std::endl;

        // Get data
        Hades::Memory::Types::Int32 New = 0;
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
        MyMemory->Write(Address, static_cast<Hades::Memory::Types::
          Byte>(New));
      }

      break;
    }

  case Detail::DataType_Int16:
    HandleNumericReadOrWrite<Hades::Memory::Types::Int16>(*MyMemory, 
      Address, Task);
    break;

  case Detail::DataType_UInt16:
    HandleNumericReadOrWrite<Hades::Memory::Types::UInt16>(*MyMemory, 
      Address, Task);
    break;

  case Detail::DataType_Int32:
    HandleNumericReadOrWrite<Hades::Memory::Types::Int32>(*MyMemory, 
      Address, Task);
    break;

  case Detail::DataType_UInt32:
    HandleNumericReadOrWrite<Hades::Memory::Types::UInt32>(*MyMemory, 
      Address, Task);
    break;

  case Detail::DataType_Int64:
    HandleNumericReadOrWrite<Hades::Memory::Types::Int64>(*MyMemory, 
      Address, Task);
    break;

  case Detail::DataType_UInt64:
    HandleNumericReadOrWrite<Hades::Memory::Types::UInt64>(*MyMemory, 
      Address, Task);
    break;

  case Detail::DataType_Float:
    HandleNumericReadOrWrite<Hades::Memory::Types::Float>(*MyMemory, 
      Address, Task);
    break;

  case Detail::DataType_Double:
    HandleNumericReadOrWrite<Hades::Memory::Types::Double>(*MyMemory, 
      Address, Task);
    break;

  case Detail::DataType_StrNarrow:
    HandleStringReadOrWrite<Hades::Memory::Types::CharNarrow>(
      *MyMemory, Address, Task, std::cin, std::cout);
    break;

  case Detail::DataType_StrWide:
    HandleStringReadOrWrite<Hades::Memory::Types::CharWide>(*MyMemory, 
      Address, Task, std::wcin, std::wcout);
    break;

  case Detail::DataType_CharNarrow:
    HandleCharReadOrWrite<Hades::Memory::Types::CharNarrow>(*MyMemory, 
      Address, Task, std::cin, std::cout);
    break;

  case Detail::DataType_CharWide:
    HandleCharReadOrWrite<Hades::Memory::Types::CharWide>(*MyMemory, 
      Address, Task, std::wcin, std::wcout);
    break;

  case Detail::DataType_Pointer:
    {
      // Read data
      auto Current = MyMemory->Read<Hades::Memory::Types::Pointer>(
        Address);
      std::wcout << "Value: " << Current << std::endl;

      // Handle 'Write Memory' task
      if (Task == Detail::Task_WriteMem)
      {
        // Output
        std::wcout << "Enter new value:" << std::endl;

        // Get data
        auto New = ReadHexNumericDataFromUser<Hades::Memory::Types::
          Pointer>();

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

// Handle 'Find module' task
inline void HandleFindMod(Detail::Task /*Task*/, 
  std::shared_ptr<Hades::Memory::MemoryMgr> MyMemory)
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

      break;
    }

  default:
    assert(!"Unsupported module search method.");
  }

  // Dump module info if found
  if (MyModule)
  {
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
}

// Handle 'Call function' task
inline void HandleCallFunc(Detail::Task /*Task*/, 
  std::shared_ptr<Hades::Memory::MemoryMgr> MyMemory)
{
  // Output
  std::wcout << "Enter target address:" << std::endl;

  // Get address
  auto Address = ReadHexNumericDataFromUser<PVOID>();

  // Call remote function
  std::vector<PVOID> CallArgs;
  CallArgs.push_back(0);
  auto ExitCode = MyMemory->Call(Address, CallArgs);

  // Output
  std::wcout << "Thread Exit Code: " << ExitCode << "." << std::endl;
}

// Handle 'Allocate memory' task
inline void HandleAllocMem(Detail::Task /*Task*/, 
  std::shared_ptr<Hades::Memory::MemoryMgr> MyMemory)
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
inline void HandleFreeMem(Detail::Task /*Task*/, 
  std::shared_ptr<Hades::Memory::MemoryMgr> MyMemory)
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

// Handle 'Disasseble' task
inline void HandleDisassemble(Detail::Task /*Task*/, 
  std::shared_ptr<Hades::Memory::MemoryMgr> MyMemory)
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
  auto DisasmResults(MyDisassembler.DisassembleToStr(Offset, 
    NumInstructions));

  // Print results
  std::copy(DisasmResults.begin(), DisasmResults.end(), 
    std::ostream_iterator<std::string>(std::cout, "\n"));
}

// Handle 'Pattern scan' task
inline void HandlePatternScan(Detail::Task /*Task*/, 
  std::shared_ptr<Hades::Memory::MemoryMgr> MyMemory)
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
      return;
    }

    // Ensure data is valid
    if (Mask.size() * 2 != Data.size())
    {
      std::wcout << "Invalid data (the data must match the mask)." 
        << std::endl;
      return;
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
      return;
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
inline void HandleInjectDLL(Detail::Task /*Task*/, 
  std::shared_ptr<Hades::Memory::MemoryMgr> MyMemory)
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

  // Inject DLL
  HMODULE ModBase = MyInjector.InjectDll(LibPath);

  // Output
  std::wcout << "Module Base: " << ModBase << "." << std::endl;

  // If export has been specified
  if (!Export.empty())
  {
    // Call remote export
    DWORD ExportRet = MyInjector.CallExport(LibPath, ModBase, Export);

    // Debug output
    std::wcout << "Export Returned: " << ExportRet << "." << std::endl;
  }
}

// Handle 'Manually map DLL' task
inline void HandleManualMap(Detail::Task /*Task*/, 
  std::shared_ptr<Hades::Memory::MemoryMgr> MyMemory)
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
inline void HandleSearchMem(Detail::Task /*Task*/, 
  std::shared_ptr<Hades::Memory::MemoryMgr> MyMemory)
{
  // Output
  std::wcout << "Choose a memory searching mode:" << std::endl;
  std::wcout << "1. Find first instance." << std::endl;
  std::wcout << "2. Find all instances." << std::endl;

  // Get memory searching mode
  auto SearchMode = GetOption<int>(L"memory searching mode", 1, 2);

  // Output
  std::wcout << "Enter start address:" << std::endl;

  // Get start address
  auto Start = ReadHexNumericDataFromUser<PVOID>();

  // Output
  std::wcout << "Enter end address:" << std::endl;

  // Get end address
  auto End = ReadHexNumericDataFromUser<PVOID>();

  // Output
  PrintDataTypeList();

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
      Hades::Memory::Types::Int32 New = 0;
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
      auto Data = static_cast<Hades::Memory::Types::Byte>(New);

      // Find data
      HandleGenericSearch(*MyMemory, Start, End, SearchMode, Data);

      break;
    }

  case Detail::DataType_Int16:
    HandleNumericSearch<Hades::Memory::Types::Int16>(*MyMemory, Start, 
      End, SearchMode);
    break;

  case Detail::DataType_UInt16:
    HandleNumericSearch<Hades::Memory::Types::UInt16>(*MyMemory, Start, 
      End, SearchMode);
    break;

  case Detail::DataType_Int32:
    HandleNumericSearch<Hades::Memory::Types::Int32>(*MyMemory, Start, 
      End, SearchMode);
    break;

  case Detail::DataType_UInt32:
    HandleNumericSearch<Hades::Memory::Types::UInt32>(*MyMemory, Start, 
      End, SearchMode);
    break;

  case Detail::DataType_Int64:
    HandleNumericSearch<Hades::Memory::Types::Int64>(*MyMemory, Start, 
      End, SearchMode);
    break;

  case Detail::DataType_UInt64:
    HandleNumericSearch<Hades::Memory::Types::UInt64>(*MyMemory, Start, 
      End, SearchMode);
    break;

  case Detail::DataType_Float:
    HandleNumericSearch<Hades::Memory::Types::Float>(*MyMemory, Start, 
      End, SearchMode);
    break;

  case Detail::DataType_Double:
    HandleNumericSearch<Hades::Memory::Types::Double>(*MyMemory, Start, 
      End, SearchMode);
    break;

  case Detail::DataType_StrNarrow:
    HandleStringSearch<Hades::Memory::Types::CharNarrow>(*MyMemory, 
      Start, End, std::cin, std::cout, SearchMode);
    break;

  case Detail::DataType_StrWide:
    HandleStringSearch<Hades::Memory::Types::CharWide>(*MyMemory, Start, 
      End, std::wcin, std::wcout, SearchMode);
    break;

  case Detail::DataType_CharNarrow:
    HandleCharSearch<Hades::Memory::Types::CharNarrow>(*MyMemory, Start, 
      End, std::cin, std::cout, SearchMode);
    break;

  case Detail::DataType_CharWide:
    HandleCharSearch<Hades::Memory::Types::CharWide>(*MyMemory, Start, 
      End, std::wcin, std::wcout, SearchMode);
    break;

  case Detail::DataType_Pointer:
    {
      // Output
      std::wcout << "Enter data:" << std::endl;

      // Get data
      auto Data = ReadHexNumericDataFromUser<Hades::Memory::Types::
        Pointer>();

      // Find data
      HandleGenericSearch(*MyMemory, Start, End, SearchMode, Data);

      break;
    }

  default:
    // Catch unsupported type
    assert(!"Unsupported data type.");
  }
}

// Handle 'Query memory' task
inline void HandleQueryMem(Detail::Task /*Task*/, 
  std::shared_ptr<Hades::Memory::MemoryMgr> MyMemory)
{
  auto RegionList(GetRegionList(*MyMemory));
  std::for_each(RegionList.begin(), RegionList.end(), 
    [] (std::shared_ptr<Hades::Memory::MemoryRegion> Current) 
  {
    std::wcout << *Current << std::endl;
  });
}

// Handle 'Run script' task
inline void HandleRunScript(Detail::Task /*Task*/, 
  std::shared_ptr<Hades::Memory::MemoryMgr> MyMemory)
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
