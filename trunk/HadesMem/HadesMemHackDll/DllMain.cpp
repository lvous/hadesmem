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
#include "HadesMem/Patcher.h"

std::shared_ptr<Hades::Memory::MemoryMgr> MyMemory;
std::shared_ptr<Hades::Memory::PatchDetour> MyPatch;

BOOL WINAPI IsWow64Process_Hook(HANDLE hProcess, PBOOL Wow64Process)
{
  MessageBox(NULL, L"IsWow64Process_Hook", L"HadesMemHackDll", MB_OK);
  typedef BOOL (WINAPI* tIsWow64Proc)(HANDLE, PBOOL);
  return reinterpret_cast<tIsWow64Proc>(MyPatch->GetTrampoline())(hProcess, 
    Wow64Process);
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

    MyMemory.reset(new Hades::Memory::MemoryMgr(GetCurrentProcessId()));
    HMODULE HookMod = GetModuleHandle(L"kernelbase.dll") ? GetModuleHandle(
      L"kernelbase.dll") : GetModuleHandle(L"kernel32.dll");
    MyPatch.reset(new Hades::Memory::PatchDetour(*MyMemory, GetProcAddress(
      HookMod, "IsWow64Process"), &IsWow64Process_Hook));
    MyPatch->Apply();
    BOOL IsWoW64 = FALSE;
    IsWow64Process(GetCurrentProcess(), &IsWoW64);
    MyPatch->Remove();
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
