// Windows API
#include <Windows.h>

extern "C" __declspec(dllexport) void Initialize(HMODULE /*MyModule*/)
{
  return;
}

BOOL WINAPI DllMain(HINSTANCE /*hinstDLL*/, DWORD /*fdwReason*/, 
  LPVOID /*lpvReserved*/)
{
  return TRUE;
}
