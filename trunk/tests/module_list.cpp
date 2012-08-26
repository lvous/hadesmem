// Copyright Joshua Boyce 2010-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// This file is part of HadesMem.
// <http://www.raptorfactor.com/> <raptorfactor@raptorfactor.com>

#include "hadesmem/module_list.hpp"

#define BOOST_TEST_MODULE module_list
#include "hadesmem/detail/warning_disable_prefix.hpp"
#include <boost/concept_check.hpp>
#include <boost/test/unit_test.hpp>
#include "hadesmem/detail/warning_disable_suffix.hpp"

#include "hadesmem/module.hpp"
#include "hadesmem/process.hpp"

// Boost.Test causes the following warning under GCC:
// error: base class 'struct boost::unit_test::ut_detail::nil_t' has a 
// non-virtual destructor [-Werror=effc++]
#if defined(HADESMEM_GCC)
#pragma GCC diagnostic ignored "-Weffc++"
#endif // #if defined(HADESMEM_GCC)

BOOST_TEST_DONT_PRINT_LOG_VALUE(hadesmem::ModuleList::iterator)

BOOST_AUTO_TEST_CASE(module_list)
{
  BOOST_CONCEPT_ASSERT((boost::InputIterator<hadesmem::ModuleList::iterator>));
  BOOST_CONCEPT_ASSERT((boost::InputIterator<hadesmem::ModuleList::
    const_iterator>));
  
  hadesmem::Process const process(::GetCurrentProcessId());
  
  using std::begin;
  using std::end;
  
  hadesmem::ModuleList const module_list_1(&process);
  hadesmem::ModuleList module_list_2(module_list_1);
  hadesmem::ModuleList module_list_3(std::move(module_list_2));
  module_list_2 = std::move(module_list_3);
  BOOST_CHECK_NE(begin(module_list_2), end(module_list_2));
  
  auto iter = begin(module_list_1);
  hadesmem::Module const this_mod(&process, nullptr);
  BOOST_CHECK_NE(iter, end(module_list_1));
  BOOST_CHECK_EQUAL(*iter, this_mod);
  hadesmem::Module const ntdll_mod(&process, L"NtDll.DlL");
  BOOST_CHECK_NE(++iter, end(module_list_1));
  BOOST_CHECK_EQUAL(*iter, ntdll_mod);
  hadesmem::Module const kernel32_mod(&process, L"kernel32.dll");
  BOOST_CHECK_NE(++iter, end(module_list_1));
  BOOST_CHECK_EQUAL(*iter, kernel32_mod);
}

BOOST_AUTO_TEST_CASE(module_list_algorithm)
{
  hadesmem::Process const process(::GetCurrentProcessId());
  
  using std::begin;
  using std::end;
  
  hadesmem::ModuleList const module_list_1(&process);
  
  for (auto const& module : module_list_1)
  {
    BOOST_CHECK_NE(module.GetHandle(), static_cast<void*>(nullptr));
    BOOST_CHECK_NE(module.GetSize(), 0U);
    BOOST_CHECK(!module.GetName().empty());
    BOOST_CHECK(!module.GetPath().empty());
  }
  
  auto const user32_iter = std::find_if(begin(module_list_1), 
    end(module_list_1), 
    [] (hadesmem::Module const& module)
    {
      return module.GetHandle() == GetModuleHandle(L"user32.dll");
    });
  BOOST_CHECK_NE(user32_iter, end(module_list_1));
}