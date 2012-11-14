// Copyright (C) 2010-2012 Joshua Boyce.
// See the file COPYING for copying permission.

#include "hadesmem/protect.hpp"

#define BOOST_TEST_MODULE protect
#include "hadesmem/detail/warning_disable_prefix.hpp"
#include <boost/test/unit_test.hpp>
#include "hadesmem/detail/warning_disable_suffix.hpp"

#include "hadesmem/error.hpp"
#include "hadesmem/process.hpp"

// Boost.Test causes the following warning under GCC:
// error: base class 'struct boost::unit_test::ut_detail::nil_t' has a 
// non-virtual destructor [-Werror=effc++]
#if defined(HADESMEM_GCC)
#pragma GCC diagnostic ignored "-Weffc++"
#endif // #if defined(HADESMEM_GCC)

// Boost.Test causes the following warning under Clang:
// error: declaration requires a global constructor 
// [-Werror,-Wglobal-constructors]
#if defined(HADESMEM_CLANG)
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#endif // #if defined(HADESMEM_CLANG)

BOOST_AUTO_TEST_CASE(query)
{
  hadesmem::Process const process(::GetCurrentProcessId());
  
  HMODULE const this_mod = GetModuleHandle(nullptr);
  BOOST_CHECK(CanRead(process, this_mod));
  BOOST_CHECK(!CanWrite(process, this_mod));
  BOOST_CHECK(!CanExecute(process, this_mod));
  BOOST_CHECK(!IsGuard(process, this_mod));
}

BOOST_AUTO_TEST_CASE(protect)
{
  hadesmem::Process const process(::GetCurrentProcessId());
  
  PVOID address = VirtualAlloc(nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, 
    PAGE_EXECUTE_READWRITE);
  BOOST_REQUIRE(address);
  BOOST_CHECK(CanRead(process, address));
  BOOST_CHECK(CanWrite(process, address));
  BOOST_CHECK(CanExecute(process, address));
  BOOST_CHECK(!IsGuard(process, address));
  BOOST_CHECK_EQUAL(Protect(process, address, PAGE_NOACCESS), 
    static_cast<DWORD>(PAGE_EXECUTE_READWRITE));
  BOOST_CHECK(!CanRead(process, address));
  BOOST_CHECK(!CanWrite(process, address));
  BOOST_CHECK(!CanExecute(process, address));
  BOOST_CHECK(!IsGuard(process, address));
  BOOST_CHECK_EQUAL(Protect(process, address, PAGE_EXECUTE), 
    static_cast<DWORD>(PAGE_NOACCESS));
  BOOST_CHECK(CanExecute(process, address));
}

BOOST_AUTO_TEST_CASE(query_and_protect_invalid)
{
  hadesmem::Process const process(::GetCurrentProcessId());
  
  LPVOID const invalid_address = reinterpret_cast<LPVOID>(
    static_cast<DWORD_PTR>(-1));
  BOOST_CHECK_THROW(CanRead(process, invalid_address), 
    hadesmem::Error);
  BOOST_CHECK_THROW(CanWrite(process, invalid_address), 
    hadesmem::Error);
  BOOST_CHECK_THROW(CanExecute(process, invalid_address), 
    hadesmem::Error);
  BOOST_CHECK_THROW(IsGuard(process, invalid_address), 
    hadesmem::Error);
  BOOST_CHECK_THROW(Protect(process, invalid_address, PAGE_EXECUTE_READWRITE), 
    hadesmem::Error);
}