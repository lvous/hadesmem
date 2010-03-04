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
  MessageBox(NULL, L"Invalid parameter.", L"HadesMemHackDll", MB_OK);

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

  // Test return values
  return 1337;
}

BOOL WINAPI DllMain(HINSTANCE /*hinstDLL*/, DWORD /*fdwReason*/, 
  LPVOID /*lpvReserved*/)
{
  return TRUE;
}
