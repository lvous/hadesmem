// Windows API
#include <Windows.h>

// C++ Standard Library
#include <cstdio>
#include <vector>

#pragma comment(linker, "/INCLUDE:__tls_used")
#pragma comment(linker, "/INCLUDE:_tls_entry")
#pragma data_seg(".CRT$XLB")

void NTAPI MyTlsEntry(PVOID /*hinstDLL*/, DWORD /*fdwReason*/, 
  PVOID /*lpvReserved*/)
{ }

extern "C" PIMAGE_TLS_CALLBACK tls_entry = MyTlsEntry;

extern "C" __declspec(dllexport) void Initialize(HMODULE /*MyModule*/)
{ }

BOOL WINAPI DllMain(HINSTANCE /*hinstDLL*/, DWORD /*fdwReason*/, 
  LPVOID /*lpvReserved*/)
{
  return TRUE;
}
