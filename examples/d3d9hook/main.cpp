// Copyright (C) 2010-2013 Joshua Boyce.
// See the file COPYING for copying permission.

#include <windows.h>

#include <hadesmem/error.hpp>
#include <hadesmem/config.hpp>
#include <hadesmem/detail/trace.hpp>

#include "d3d9_manager.hpp"

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved);

extern "C" HADESMEM_DETAIL_DLLEXPORT DWORD_PTR Load();

extern "C" HADESMEM_DETAIL_DLLEXPORT DWORD_PTR Free();

extern "C" HADESMEM_DETAIL_DLLEXPORT DWORD_PTR Load()
{
  try
  {
    InitializeD3D9Hooks();
  }
  catch (...)
  {
    HADESMEM_DETAIL_TRACE_A(boost::current_exception_diagnostic_information()
      .c_str());
  }

  return 0;
}

extern "C" HADESMEM_DETAIL_DLLEXPORT DWORD_PTR Free()
{
  try
  {
    UninitializeD3D9Hooks();
  }
  catch (...)
  {
    HADESMEM_DETAIL_TRACE_A(boost::current_exception_diagnostic_information()
      .c_str());
  }

  return 0;
}

BOOL WINAPI DllMain(HINSTANCE /*instance*/, DWORD /*reason*/, 
  LPVOID /*reserved*/)
{
  return TRUE;
}