// Copyright (C) 2010-2013 Joshua Boyce.
// See the file COPYING for copying permission.

#include "hadesmem/detail/initialize.hpp"

#include <ctime>
#include <random>
#include <iostream>

#include <hadesmem/detail/warning_disable_prefix.hpp>
#include <boost/assert.hpp>
#include <boost/locale.hpp>
#include <boost/filesystem.hpp>
#include <hadesmem/detail/warning_disable_suffix.hpp>

#include <hadesmem/config.hpp>
#include <windows.h>
// Required for ::_CrtSetDbgFlag.
// Does not compile under GCC.
#if !defined(HADESMEM_GCC)
// Macro unused under Clang.
#if !defined(HADESMEM_CLANG)
#define _CRTDBG_MAP_ALLOC
#endif // #if !defined(HADESMEM_CLANG)
#include <stdlib.h>
#include <crtdbg.h>
#endif // #if !defined(HADESMEM_GCC)

#include <hadesmem/error.hpp>

#ifndef PROCESS_CALLBACK_FILTER_ENABLED
#define PROCESS_CALLBACK_FILTER_ENABLED 0x1UL
#endif // #ifndef PROCESS_CALLBACK_FILTER_ENABLED

namespace hadesmem
{

namespace detail
{

void DisableUserModeCallbackExceptionFilter()
{
  HMODULE const k32_mod = ::GetModuleHandle(L"kernel32");
  if (!k32_mod)
  {
    DWORD const last_error = ::GetLastError();
    HADESMEM_THROW_EXCEPTION(hadesmem::Error() << 
      hadesmem::ErrorString("GetModuleHandle failed.") << 
      hadesmem::ErrorCodeWinLast(last_error));
  }

  typedef BOOL (NTAPI *GetProcessUserModeExceptionPolicyPtr)(LPDWORD lpFlags);
  typedef BOOL (NTAPI *SetProcessUserModeExceptionPolicyPtr)(DWORD dwFlags);

  // These APIs are not available by default until Windows 7 SP1, so they 
  // must be called dynamically.
  auto const get_policy = 
    reinterpret_cast<GetProcessUserModeExceptionPolicyPtr>(
    ::GetProcAddress(k32_mod, "GetProcessUserModeExceptionPolicy"));
  auto const set_policy = 
    reinterpret_cast<SetProcessUserModeExceptionPolicyPtr>(
    ::GetProcAddress(k32_mod, "SetProcessUserModeExceptionPolicy"));

  DWORD flags;
  if (get_policy && set_policy && get_policy(&flags))
  {
    set_policy(flags & ~PROCESS_CALLBACK_FILTER_ENABLED);
  }
}

void EnableCrtDebugFlags()
{
  // Only enable in debug mode.
#if defined(_DEBUG)
  // Does not compile under GCC.
#if !defined(HADESMEM_GCC)
  int dbg_flags = ::_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  dbg_flags |= _CRTDBG_ALLOC_MEM_DF;
  dbg_flags |= _CRTDBG_LEAK_CHECK_DF;
  ::_CrtSetDbgFlag(dbg_flags);
#endif // #if !defined(HADESMEM_GCC)
#endif // #if defined(_DEBUG)
}

void EnableTerminationOnHeapCorruption()
{
  if (!::HeapSetInformation(::GetProcessHeap(), 
    HeapEnableTerminationOnCorruption, nullptr, 0))
  {
    DWORD const last_error = ::GetLastError();
    HADESMEM_THROW_EXCEPTION(hadesmem::Error() << 
      hadesmem::ErrorString("HeapSetInformation failed.") << 
      hadesmem::ErrorCodeWinLast(last_error));
  }
}

void EnableBottomUpRand()
{
  // This is a defense-in-depth measure, so the time should be sufficient 
  // entropy in all but the rarest cases.
  std::mt19937 rand(static_cast<unsigned int>(std::time(nullptr)));
  std::uniform_int_distribution<unsigned int> uint_dist(0, 256);
  unsigned int const num_allocs = uint_dist(rand);
  for (unsigned int i = 0; i != num_allocs; ++i)
  {
    // Reserve memory only, as committing it would be a waste.
    DWORD const kAllocSize64K = 64 * 1024;
    if (!::VirtualAlloc(NULL, kAllocSize64K, MEM_RESERVE, PAGE_NOACCESS))
    {
      DWORD const last_error = ::GetLastError();
      HADESMEM_THROW_EXCEPTION(hadesmem::Error() << 
        hadesmem::ErrorString("VirtualAlloc failed.") << 
        hadesmem::ErrorCodeWinLast(last_error));
    }
  }
}

std::locale ImbueAllDefault()
{
  // Use the Windows API backend to (hopefully) provide consistent behaviour 
  // across different compilers and standard library implementations (as 
  // opposed to the standard library backend).
  auto backend = boost::locale::localization_backend_manager::global();
  backend.select("winapi"); 

  boost::locale::generator gen(backend);

  std::locale const locale = gen("");

  // Ensure the locale uses a UTF-8 backend. 
  BOOST_ASSERT(std::use_facet<boost::locale::info>(locale).utf8());

  return ImbueAll(locale);
}

std::locale ImbueAll(std::locale const& locale)
{
  auto const old_loc = std::locale::global(locale);

  std::cin.imbue(locale);
  std::cout.imbue(locale);
  std::cerr.imbue(locale);
  std::clog.imbue(locale);

  std::wcin.imbue(locale);
  std::wcout.imbue(locale);
  std::wcerr.imbue(locale);
  std::wclog.imbue(locale);

  boost::filesystem::path::imbue(locale);

  // According to comments in the Boost.Locale examples, this is needed 
  // to prevent the C standard library performing string conversions 
  // instead of the C++ standard library on some platforms. Unfortunately, 
  // it is not specified which platforms this applies to, so I'm unsure 
  // if this is ever relevant to Windows, but it's better safe than sorry.
  std::ios_base::sync_with_stdio(false); 

  return old_loc;
}

}

}
