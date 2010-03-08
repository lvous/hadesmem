// Windows API
#include <Windows.h>

// Boost
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

// Start and end of target module's memory region
PVOID RegionStart = nullptr;
PVOID RegionEnd = nullptr;

// Get a pointer to the TEB of the current thread
PVOID GetCurrentTeb()
{
  #if defined(_M_AMD64) 
    return reinterpret_cast<PVOID>(__readgsqword(6 * sizeof(PVOID)));
  #elif defined(_M_IX86) 
    return reinterpret_cast<PVOID>(__readfsdword(6 * sizeof(PVOID)));
  #else 
    #error "Unsupported architecture."
  #endif
}

// Exception registration record
typedef struct _EXCEPTION_REGISTRATION_RECORD 
{ 
  struct _EXCEPTION_REGISTRATION_RECORD *Next; 
  PEXCEPTION_ROUTINE                     Handler; 
} EXCEPTION_REGISTRATION_RECORD, *PEXCEPTION_REGISTRATION_RECORD;

LONG CALLBACK MyVectoredHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
  // Check if the exception occurred within the target memory range
  if (ExceptionInfo->ExceptionRecord->ExceptionAddress >= RegionStart && 
    ExceptionInfo->ExceptionRecord->ExceptionAddress <= RegionEnd)
  {
    // Get exception list for current thread
    PEXCEPTION_REGISTRATION_RECORD ExceptionList = 
      *reinterpret_cast<PEXCEPTION_REGISTRATION_RECORD*>(GetCurrentTeb());

    // Get handler
    std::wstring HandlerStr((boost::wformat(L"Exception caught!\n Handler: %p") 
      %ExceptionList->Handler).str());

    // Display handler
    MessageBox(NULL, HandlerStr.c_str(), L"HadesMMHelper", MB_OK);
  }

  // Unhandled exception. Continue down chain.
  return EXCEPTION_CONTINUE_SEARCH;
}

extern "C" __declspec(dllexport) DWORD __stdcall Initialize(HMODULE /*Module*/, 
  PVOID Start, PVOID End)
{
  // Break to debugger if present
  if (IsDebuggerPresent())
  {
    DebugBreak();
  }

  // Set global region start and end
  RegionStart = Start;
  RegionEnd = End;

  // Test IAT
  MessageBox(NULL, L"Initialize called.", L"HadesMMHelper", MB_OK);

  // Sanity checks
  if (!RegionStart || !RegionEnd || (RegionStart >= RegionEnd))
  {
    MessageBox(NULL, L"Invalid memory region.", L"HadesMMHelper", MB_OK);
  }

  // Add our VEH handler (used for manual exception dispatching)
  if (!AddVectoredExceptionHandler(1, &MyVectoredHandler))
  {
    MessageBox(NULL, L"Could not register VEH.", L"HadesMMHelper", MB_OK);
  }

  // Test return values
  return 0;
}

BOOL WINAPI DllMain(HINSTANCE /*hinstDLL*/, DWORD /*fdwReason*/, 
  LPVOID /*lpvReserved*/)
{
  return TRUE;
}
