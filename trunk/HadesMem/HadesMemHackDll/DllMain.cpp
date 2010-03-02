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
  boost::thread_specific_ptr<std::string> MyString;
  if (!MyString.get())
  {
    MyString.reset(new std::string());
  }
  *MyString = "asdf";
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD /*fdwReason*/, 
  LPVOID /*lpvReserved*/)
{
  if (hinstDLL)
  {
    MessageBox(NULL, L"DllMain called.", L"HadesMemHackDLL", MB_OK);
  }

  return TRUE;
}
