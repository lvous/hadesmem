// Windows API
#include <Windows.h>

// C++ Standard Library
#include <cstdio>
#include <vector>

extern "C" __declspec(dllexport) void Initialize(HMODULE /*MyModule*/)
{ }

BOOL WINAPI DllMain(HINSTANCE /*hinstDLL*/, DWORD /*fdwReason*/, 
  LPVOID /*lpvReserved*/)
{
  return TRUE;
}
