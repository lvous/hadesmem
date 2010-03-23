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

// Hades
#include "HadesMem/IatHook.h"

std::shared_ptr<Hades::Memory::MemoryMgr> MyMemory;
std::shared_ptr<Hades::Memory::IatHook> GetCurProcIdHook;

DWORD WINAPI GetCurrentProcessId_Hook()
{
  MessageBox(NULL, L"Hook called!", L"GetCurrentProcessId", MB_OK);
  return GetCurProcIdHook->GetOrig<DWORD (WINAPI*)()>()();
}

extern "C" __declspec(dllexport) DWORD __stdcall Initialize(HMODULE Module)
{
  try
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

    // Test IAT hook class
    MyMemory.reset(new Hades::Memory::MemoryMgr(GetCurrentProcessId()));
    GetCurProcIdHook.reset(new Hades::Memory::IatHook(*MyMemory, 
      L"kernel32.dll", L"GetCurrentProcessId", &GetCurrentProcessId_Hook, 
      Module));
    GetCurrentProcessId();
  }
  catch (boost::exception const& e)
  {
    // Dump error information
    MessageBoxA(NULL, boost::diagnostic_information(e).c_str(), 
      "HadesMemHackDll", MB_OK);
  }
  catch (std::exception const& e)
  {
    // Dump error information
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
