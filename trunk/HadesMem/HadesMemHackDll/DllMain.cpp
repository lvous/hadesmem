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

extern "C" __declspec(dllexport) void Initialize(HMODULE /*Module*/)
{
  // Test IAT
  MessageBox(NULL, L"Initialize called.", L"HadesMemHackDLL", MB_OK);

  // Force TLS to be used
  boost::thread_specific_ptr<std::wstring> MyString;
  if (!MyString.get())
  {
    MyString.reset(new std::wstring());
  }
  *MyString = L"asdf";
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD /*fdwReason*/, 
  LPVOID /*lpvReserved*/)
{
  return TRUE;
}
