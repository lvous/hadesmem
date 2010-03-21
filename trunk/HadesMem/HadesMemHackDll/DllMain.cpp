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

// Windows API
#include <Windows.h>

// C++ Standard Library
#include <cstdio>
#include <vector>
#include <string>

// Boost
#pragma warning(push, 1)
#include <boost/thread.hpp>
#pragma warning(pop)

extern "C" __declspec(dllexport) DWORD __stdcall Initialize(HMODULE Module)
{
  // Break to debugger if present
  if (IsDebuggerPresent())
  {
    DebugBreak();
  }

  // Test IAT
  MessageBox(NULL, L"Initialize called.", L"HadesMemHackDll", MB_OK);

  // Test parameter
  if (!Module)
  {
    MessageBox(NULL, L"Invalid parameter.", L"HadesMemHackDll", MB_OK);
  }

  // Test TLS callbacks
  boost::thread_specific_ptr<std::wstring> MyString;
  if (!MyString.get())
  {
    MyString.reset(new std::wstring());
  }
  *MyString = L"asdf";

  // Test C++ EH
  try
  {
    throw std::runtime_error("Testing C++ EH.");
  }
  catch (std::exception const& e)
  {
    MessageBoxA(NULL, e.what(), "HadesMemHackDll", MB_OK);
  }

  // Test return values
  return 1337;
}

BOOL WINAPI DllMain(HINSTANCE /*hinstDLL*/, DWORD /*fdwReason*/, 
  LPVOID /*lpvReserved*/)
{
  return TRUE;
}
