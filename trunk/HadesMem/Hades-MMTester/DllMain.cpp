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

// Windows API
#include <crtdbg.h>
#include <Windows.h>
#include <Winternl.h>
#include <WinNT.h>

// C++ Standard Library
#include <string>
#include <iostream>
#include <exception>

// Boost
#pragma warning(push, 1)
#include <boost/format.hpp>
#include <boost/thread/tss.hpp>
#include <boost/thread/thread.hpp>
#include <boost/exception/all.hpp>
#pragma warning(pop)

// Hades
#include "Hades-Common/Logger.h"

// Using a VEH to do exception dispatching. Necessary when the target 
// has DEP enabled.
LONG CALLBACK MyVectoredHandler(__in  PEXCEPTION_POINTERS /*ExceptionInfo*/)
{
  PVOID pTeb = NtCurrentTeb();
  pTeb;
  return EXCEPTION_CONTINUE_SEARCH;
}

extern "C" __declspec(dllexport) DWORD __stdcall Initialize(HMODULE /*Module*/)
{
  // Break to debugger if present
  if (IsDebuggerPresent())
  {
    DebugBreak();
  }
  
  // Register VEH
  if (!AddVectoredExceptionHandler(1, &MyVectoredHandler))
  {
    MessageBox(nullptr, L"AddVEH failed!", L"Hades-MMTester", MB_OK);
  }

  // Test TLS callbacks
  boost::thread_specific_ptr<DWORD> TlsTest;
  if (!TlsTest.get())
  {
    TlsTest.reset(new DWORD(0));
  }
  *TlsTest = 1022;

  // Test EH
  try
  {
    throw std::exception("Test EH!");
  }
  catch (std::exception const& e)
  {
    // Test imports
    MessageBoxA(nullptr, e.what(), "Hades-MMTester", MB_OK);
  }

  // Test imports
  MessageBox(nullptr, L"Test IAT", L"Hades-MMTester", MB_OK);

  // Test return values
  return 0;
}

BOOL WINAPI DllMain(HINSTANCE /*hinstDLL*/, DWORD /*fdwReason*/, 
  LPVOID /*lpvReserved*/)
{
  // Attempt to detect memory leaks in debug mode
#ifdef _DEBUG
  int CurrentFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  int NewFlags = (_CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF | 
    _CRTDBG_CHECK_ALWAYS_DF);
  _CrtSetDbgFlag(CurrentFlags | NewFlags);
#endif

  return TRUE;
}
