// Windows API
#include <Windows.h>

extern "C" __declspec(dllexport) void Initialize(HMODULE /*MyModule*/)
{
  MessageBox(nullptr, L"It works!", L"HadesMemHackDll", MB_OK);
  return;
}

BOOL WINAPI DllMain(HINSTANCE /*hinstDLL*/, DWORD /*fdwReason*/, 
  LPVOID /*lpvReserved*/)
{
  return TRUE;
}
