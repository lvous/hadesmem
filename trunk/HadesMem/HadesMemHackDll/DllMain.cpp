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

extern "C" __declspec(dllexport) void Initialize(HMODULE /*MyModule*/)
{
  // Force TLS to be used
  boost::thread_specific_ptr<std::string> MyInt;
  if (!MyInt.get())
    MyInt.reset();
  *MyInt = "asdf";
}

BOOL WINAPI DllMain(HINSTANCE /*hinstDLL*/, DWORD /*fdwReason*/, 
  LPVOID /*lpvReserved*/)
{
  return TRUE;
}
