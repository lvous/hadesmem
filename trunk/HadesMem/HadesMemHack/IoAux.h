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
#include <iostream>

// Hades Memory
#include "HadesMem/Memory.h"

// Read string from user
template <typename CharT>
std::basic_string<CharT> ReadStringDataFromUser(std::basic_istream<CharT>& In, 
  std::basic_ostream<CharT>& Out) 
{
  // Read data
  std::basic_string<CharT> Data;
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

// Read numeric type from user
template <typename T>
T ReadHexNumericDataFromUser() 
{
  // Read data
  T Data = T();
  while (!(std::wcin >> std::hex >> Data >> std::dec))
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

// Get option ID
template <typename T>
inline T GetOption(std::wstring const& Option, int Min, int Max)
{
  // Get option ID
  int Value = 0;
  while (!(std::wcin >> Value) || Value < Min  || Value > Max)
  {
    std::wcout << L"Invalid " + Option + L"." << std::endl;
    std::wcin.clear();
    std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), L'\n');
  }
  std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), L'\n');

  // Return option ID
  return static_cast<T>(Value);
}

// Print data type list
inline void PrintDataTypeList()
{
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
}
