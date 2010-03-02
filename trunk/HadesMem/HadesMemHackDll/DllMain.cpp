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
  // But don't run it yet
  volatile bool AlwaysFalse = false;
  if (AlwaysFalse)
  {
    boost::thread_specific_ptr<std::string> MyInt;
    if (!MyInt.get())
      MyInt.reset();
    *MyInt = "asdf";
  }
}

BOOL WINAPI DllMain(HINSTANCE /*hinstDLL*/, DWORD /*fdwReason*/, 
  LPVOID /*lpvReserved*/)
{
  return TRUE;
}
