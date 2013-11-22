// Copyright (C) 2010-2013 Joshua Boyce.
// See the file COPYING for copying permission.

#include <hadesmem/read.hpp>

#include <array>
#include <string>
#include <vector>

#define BOOST_TEST_MODULE read
#include <hadesmem/detail/warning_disable_prefix.hpp>
#include <boost/test/unit_test.hpp>
#include <hadesmem/detail/warning_disable_suffix.hpp>

#include <hadesmem/alloc.hpp>
#include <hadesmem/error.hpp>
#include <hadesmem/config.hpp>
#include <hadesmem/process.hpp>
#include <hadesmem/detail/winapi.hpp>

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

// TODO: Improve tests by doing checks both before and after writes.
// TODO: Don't read/write data on the stack.

BOOST_TEST_DONT_PRINT_LOG_VALUE(std::wstring)
  
BOOST_AUTO_TEST_CASE(read_pod)
{
  hadesmem::Process const process(::GetCurrentProcessId());
  
  struct TestPODType
  {
    std::int32_t a;
    char* b;
    wchar_t c;
    std::int64_t d;
  };
  
  TestPODType test_pod_type = { 1, 0, L'a', 1234567812345678 };
  auto new_test_pod_type = hadesmem::Read<TestPODType>(process, 
    &test_pod_type);
  BOOST_CHECK_EQUAL(std::memcmp(&test_pod_type, &new_test_pod_type, 
    sizeof(test_pod_type)), 0);

  auto const new_test_array = 
    hadesmem::Read<std::array<char, sizeof(TestPODType)>>(process, 
    &test_pod_type);
  BOOST_CHECK_EQUAL(std::memcmp(&test_pod_type, &new_test_array[0], 
    sizeof(test_pod_type)), 0);

  auto const new_test_array_2 = 
    hadesmem::Read<char, sizeof(TestPODType)>(process, 
    &test_pod_type);
  BOOST_CHECK_EQUAL(std::memcmp(&test_pod_type, &new_test_array_2[0], 
    sizeof(test_pod_type)), 0);

  PVOID const noaccess_page = VirtualAlloc(nullptr, sizeof(void*), 
    MEM_RESERVE | MEM_COMMIT, PAGE_NOACCESS);
  BOOST_REQUIRE(noaccess_page != nullptr);
  hadesmem::Read<void*>(process, noaccess_page);

  PVOID const guard_page = VirtualAlloc(nullptr, sizeof(void*), 
    MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE | PAGE_GUARD);
  BOOST_REQUIRE(guard_page != nullptr);
  BOOST_CHECK_THROW(hadesmem::Read<void*>(process, guard_page), 
    hadesmem::Error);

  PVOID const execute_page = VirtualAlloc(nullptr, sizeof(void*), 
    MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE);
  BOOST_REQUIRE(execute_page != nullptr);
  hadesmem::Read<void*>(process, execute_page);
}

BOOST_AUTO_TEST_CASE(read_string)
{
  hadesmem::Process const process(::GetCurrentProcessId());

  hadesmem::Allocator const str_alloc(process, 0x1000);
  char* const str_mem = static_cast<char*>(str_alloc.GetBase());
  wchar_t* const str_mem_wide = static_cast<wchar_t*>(str_alloc.GetBase());

  std::string test_string = "Narrow test string.";
  std::copy(std::begin(test_string), std::end(test_string), str_mem);
  str_mem[test_string.size()] = '\0';
  auto const new_test_string = hadesmem::ReadString<char>(process, str_mem);
  BOOST_CHECK_EQUAL(new_test_string, test_string);

  std::wstring wide_test_string = L"Wide test string.";
  std::copy(std::begin(wide_test_string), std::end(wide_test_string), 
    str_mem_wide);
  str_mem[wide_test_string.size()] = L'\0';
  auto const wide_new_test_string = hadesmem::ReadString<wchar_t>(process, 
    str_mem_wide);
  BOOST_CHECK_EQUAL(wide_new_test_string, wide_test_string);

  std::string test_string_2 = "Narrow test string.";
  std::copy(std::begin(test_string_2), std::end(test_string_2), str_mem);
  str_mem[test_string_2.size()] = '\0';
  auto const new_test_string_2 = hadesmem::ReadStringEx<char>(
    process, str_mem, 1);
  BOOST_CHECK_EQUAL(new_test_string_2, test_string_2);

  std::wstring wide_test_string_2 = L"Wide test string.";
  std::copy(std::begin(wide_test_string_2), std::end(wide_test_string_2), 
    str_mem_wide);
  str_mem[wide_test_string_2.size()] = L'\0';
  auto const wide_new_test_string_2 = hadesmem::ReadStringEx<wchar_t>(
    process, str_mem_wide, 1);
  BOOST_CHECK_EQUAL(wide_new_test_string_2, wide_test_string_2);
}

BOOST_AUTO_TEST_CASE(read_vector)
{
  hadesmem::Process const process(::GetCurrentProcessId());
  
  std::array<int, 10> int_list = {{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }};
  std::vector<int> int_list_read = hadesmem::ReadVector<int>(
    process, &int_list, 10);
  BOOST_CHECK_EQUAL_COLLECTIONS(int_list.cbegin(), int_list.cend(), 
    int_list_read.cbegin(), int_list_read.cend());
  
  std::vector<int> int_list_read_2;
  hadesmem::ReadVector<int>(process, &int_list, 10, 
    std::back_inserter(int_list_read_2));
  BOOST_CHECK_EQUAL_COLLECTIONS(int_list_read_2.cbegin(), int_list_read_2.cend(), 
    int_list_read.cbegin(), int_list_read.cend());
}

BOOST_AUTO_TEST_CASE(read_cross_region)
{
  SYSTEM_INFO const sys_info = hadesmem::detail::GetSystemInfo();
  DWORD const page_size = sys_info.dwPageSize;
  
  LPVOID const address = VirtualAlloc(nullptr, page_size * 2, MEM_RESERVE | 
    MEM_COMMIT, PAGE_NOACCESS);
  BOOST_REQUIRE(address != 0);
  
  hadesmem::Process const process(::GetCurrentProcessId());
  std::vector<char> buf = hadesmem::ReadVector<char>(process, address, 
    page_size * 2);
  std::vector<char> zero_buf(page_size * 2);
  BOOST_CHECK_EQUAL_COLLECTIONS(buf.cbegin(), buf.cend(), 
    zero_buf.cbegin(), zero_buf.cend());
}
