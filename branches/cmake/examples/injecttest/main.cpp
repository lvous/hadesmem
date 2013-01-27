// Copyright (C) 2010-2012 Joshua Boyce.
// See the file COPYING for copying permission.

#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE /*instance*/, DWORD /*reason*/, 
  LPVOID /*reserved*/);

// This is required because of a bug in Clang's dllexport support on Windows, 
// this is worked around by using a linker flag to export all symbols 
// unconditionally.
// TODO: Remove this hack once Clang has been fixed.
#ifdef HADESMEM_CLANG
#define HADESMEM_DLLEXPORT  
#else
#define HADESMEM_DLLEXPORT __declspec(dllexport)
#endif

extern "C" HADESMEM_DLLEXPORT DWORD Load(LPCVOID /*module*/);

extern "C" HADESMEM_DLLEXPORT DWORD Free(LPCVOID /*module*/);

extern "C" __declspec(dllimport) DWORD InjectTestDep_Foo();

bool g_alloced_console = false;

extern "C" HADESMEM_DLLEXPORT DWORD Load(LPCVOID /*module*/)
{
  InjectTestDep_Foo();

  if (!AllocConsole())
  {
    return GetLastError();
  }

  g_alloced_console = true;

  return 0;
}

extern "C" HADESMEM_DLLEXPORT DWORD Free(LPCVOID /*module*/)
{
  if (g_alloced_console)
  {
    if (!FreeConsole())
    {
      return GetLastError();
    }
  }
  
  return 0;
}

BOOL WINAPI DllMain(HINSTANCE /*instance*/, DWORD /*reason*/, 
  LPVOID /*reserved*/)
{
  return TRUE;
}