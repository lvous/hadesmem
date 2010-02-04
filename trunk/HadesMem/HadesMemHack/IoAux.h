#pragma once

// HadesMem
#include "HadesMem/Memory.h"

// C++ Standard Library
#include <string>
#include <memory>
#include <iostream>

// Read string from user
template <typename T, typename CharT>
T ReadStringDataFromUser(std::basic_istream<CharT>& In, 
  std::basic_ostream<CharT>& Out) 
{
  // Read data
  T Data;
  while (!std::getline(In, Data) || Data.empty())
  {
    Out << "Invalid data." << std::endl;
    In.clear();
  }

  // Return data
  return Data;
}

// Read character from user
template <typename T, typename CharT>
T ReadCharDataFromUser(std::basic_istream<CharT>& In, 
  std::basic_ostream<CharT>& Out) 
{
  // Read data
  T Data = T();
  while (!(In >> Data))
  {
    Out << "Invalid data." << std::endl;
    In.clear();
  }

  // Return data
  return Data;
}

// Read numeric type from user
template <typename T>
T ReadNumericDataFromUser() 
{
  // Read data
  T Data = T();
  while (!(std::wcin >> Data))
  {
    std::cout << "Invalid data." << std::endl;
    std::wcin.clear();
    std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), 
      '\n');
  }
  std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), 
    '\n');

// Return data
  return Data;
}

// Handle 'MemoryRead' or 'MemoryWrite' task for string types
template <typename T, typename CharT>
void HandleStringReadOrWrite(Hades::Memory::MemoryMgr const& MyMemory, 
  PVOID Address, int Task, std::basic_istream<CharT>& In, 
  std::basic_ostream<CharT>& Out)
{
  // Read data
  auto Current = MyMemory.Read<T>(Address);
  Out << "Value: " << Current << std::endl;

  // Handle 'Write Memory' task
  if (Task == 2)
  {
    // Output
    Out << "Enter new value:" << std::endl;

    // Get data
    auto New = ReadStringDataFromUser<T, CharT>(In, Out);

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
  if (Task == 2)
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
  if (Task == 2)
  {
    // Output
    std::wcout << "Enter new value:" << std::endl;

    // Get data
    auto New = ReadNumericDataFromUser<T>();

    // Write data
    MyMemory.Write(Address, New);
  }
}

// Get option ID
inline int GetOption(std::string Option, int Min, int Max)
{
  // Get option ID
  int Value = 0;
  while (!(std::wcin >> Value) || Value < Min  || Value > Max)
  {
    std::cout << "Invalid " + Option + "." << std::endl;
    std::wcin.clear();
    std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
  }
  std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');

  // Return option ID
  return Value;
}

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
inline void CreateMemoryFromWindowName(std::shared_ptr<Hades::Memory::MemoryMgr>& 
  MyMemory)
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
