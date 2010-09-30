/*
This file is part of HadesMem.
Copyright © 2010 RaptorFactor (aka Cypherjb, Cypher, Chazwazza). 
<http://www.raptorfactor.com/> <raptorfactor@raptorfactor.com>

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

// Boost
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#pragma warning(pop)

// Windows API
#include <crtdbg.h>
#include <Windows.h>

// Hades
#include "Hades-Common/Logger.h"

typedef struct _EXCEPTION_REGISTRATION_RECORD
{
  struct _EXCEPTION_REGISTRATION_RECORD *Next;
  PEXCEPTION_ROUTINE Handler;
} EXCEPTION_REGISTRATION_RECORD, *PEXCEPTION_REGISTRATION_RECORD;

typedef struct _DISPATCHER_CONTEXT
{
  PEXCEPTION_REGISTRATION_RECORD RegistrationPointer;
} DISPATCHER_CONTEXT, *PDISPATCHER_CONTEXT;

#define EXCEPTION_CHAIN_END ((PEXCEPTION_REGISTRATION_RECORD)-1)

LONG CALLBACK VectoredHandler(__in PEXCEPTION_POINTERS ExceptionInfo)
{
  PVOID pTeb = NtCurrentTeb();

  PEXCEPTION_REGISTRATION_RECORD pExceptionList = 
    *reinterpret_cast<PEXCEPTION_REGISTRATION_RECORD*>(pTeb);

  while (pExceptionList != EXCEPTION_CHAIN_END)
  {
    DISPATCHER_CONTEXT DispatcherContext = { 0 };

    EXCEPTION_DISPOSITION Disposition = pExceptionList->Handler(
      ExceptionInfo->ExceptionRecord, 
      pExceptionList, 
      ExceptionInfo->ContextRecord, 
      &DispatcherContext);

    switch (Disposition)
    {
    case ExceptionContinueExecution:
      return EXCEPTION_CONTINUE_EXECUTION;

    case ExceptionContinueSearch:
      pExceptionList = pExceptionList->Next;
      break;

    case ExceptionNestedException:
    case ExceptionCollidedUnwind:
      std::abort();

    default:
      assert(!"Unknown exception disposition.");
    }
  }

  return EXCEPTION_CONTINUE_SEARCH;
}

#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
void TestSEH()
{
  // Test SEH
  __try 
  {
    int* pInt = 0;
    *pInt = 0;
  }
  __except (EXCEPTION_EXECUTE_HANDLER)
  {
    MessageBoxW(NULL, L"Testing SEH.", L"Hades-MMHelper", MB_OK);
  }
}
#pragma warning(pop)

void TestRelocs()
{
  MessageBoxW(NULL, L"Testing relocations.", L"Hades-MMHelper", MB_OK);
}

extern "C" __declspec(dllexport) DWORD __stdcall Test(HMODULE /*Module*/)
{
  // Break to debugger if present
  if (IsDebuggerPresent())
  {
    DebugBreak();
  }

  // Add VEH
  if (!AddVectoredExceptionHandler(1, &VectoredHandler))
  {
    MessageBoxW(NULL, L"Failed to add VEH.", L"Hades-MMHelper", MB_OK);
  }

  // Test IAT
  MessageBoxW(NULL, L"Testing IAT.", L"Hades-MMHelper", MB_OK);

  // Test TLS
  boost::thread_specific_ptr<std::wstring> TlsTest;
  TlsTest.reset(new std::wstring(L"Testing TLS."));
  MessageBoxW(NULL, TlsTest->c_str(), L"Hades-MMHelper", MB_OK);

  // Test relocs
  typedef void (* tTestRelocs)();
  tTestRelocs pTestRelocs = reinterpret_cast<tTestRelocs>(&TestRelocs);
  pTestRelocs();

  // Test SEH
  TestSEH();

  // Test C++ EH
  try
  {
    throw std::runtime_error("Testing C++ EH.");
  }
  catch (std::exception const& e)
  {
    MessageBoxA(NULL, boost::diagnostic_information(e).c_str(), 
      "Hades-MMHelper", MB_OK);
  }

  // Test return values
  return 1337;
}

extern "C" __declspec(dllexport) DWORD __stdcall Initialize(HMODULE /*Module*/)
{
  // Break to debugger if present
  if (IsDebuggerPresent())
  {
    DebugBreak();
  }

  // Test return values
  return 1234;
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
