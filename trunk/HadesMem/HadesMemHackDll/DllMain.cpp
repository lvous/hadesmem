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

extern "C" __declspec(dllexport) void __stdcall Initialize(HMODULE /*Module*/)
{
  // Break to debugger if present
  if (IsDebuggerPresent())
  {
    DebugBreak();
  }

  // Test IAT
  MessageBox(NULL, L"Initialize called.", L"HadesMemHackDll", MB_OK);

  // Test TLS callbacks
  boost::thread_specific_ptr<std::wstring> MyString;
  if (!MyString.get())
  {
    MyString.reset(new std::wstring());
  }
  *MyString = L"asdf";
}

BOOL WINAPI DllMain(HINSTANCE /*hinstDLL*/, DWORD /*fdwReason*/, 
  LPVOID /*lpvReserved*/)
{
  return TRUE;
}
