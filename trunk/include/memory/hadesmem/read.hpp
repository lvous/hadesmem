// Copyright Joshua Boyce 2010-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// This file is part of HadesMem.
// <http://www.raptorfactor.com/> <raptorfactor@raptorfactor.com>

#pragma once

#include <array>
#include <string>
#include <vector>
#include <cstddef>
#include <exception>
#include <type_traits>

#include "hadesmem/detail/warning_disable_prefix.hpp"
#include <boost/assert.hpp>
#include "hadesmem/detail/warning_disable_suffix.hpp"

#include <windows.h>

#include "hadesmem/error.hpp"
#include "hadesmem/protect.hpp"
#include "hadesmem/detail/read_impl.hpp"
#include "hadesmem/detail/query_region.hpp"
#include "hadesmem/detail/type_traits.hpp"
#include "hadesmem/detail/static_assert.hpp"

namespace hadesmem
{

class Process;

namespace detail
{

template <typename T>
T ReadUnchecked(Process const& process, PVOID address)
{
  HADESMEM_STATIC_ASSERT(detail::IsTriviallyCopyable<T>::value);
  
  T data;
  ReadUnchecked(process, address, &data, sizeof(data));
  return data;
}

}

template <typename T>
T Read(Process const& process, PVOID address)
{
  HADESMEM_STATIC_ASSERT(detail::IsTriviallyCopyable<T>::value);
  
  T data;
  detail::Read(process, address, &data, sizeof(data));
  return data;
}

template <typename T, std::size_t N>
std::array<T, N> Read(Process const& process, PVOID address)
{
  HADESMEM_STATIC_ASSERT(detail::IsTriviallyCopyable<T>::value);

  HADESMEM_STATIC_ASSERT(detail::IsDefaultConstructible<T>::value);

  std::array<T, N> data;
  detail::Read(process, address, data.data(), sizeof(T) * N);
  return data;
}

template <typename T>
std::basic_string<T> ReadString(Process const& process, PVOID address)
{
  HADESMEM_STATIC_ASSERT(detail::IsCharType<T>::value);

  MEMORY_BASIC_INFORMATION const mbi = detail::Query(process, address);

  if (detail::IsGuard(mbi))
  {
    BOOST_THROW_EXCEPTION(Error() << 
      ErrorString("Attempt to read from guard page."));
  }

  bool const can_read = detail::CanRead(mbi);

  DWORD old_protect = 0;
  if (!can_read)
  {
    old_protect = Protect(process, address, PAGE_EXECUTE_READWRITE);
  }

  std::basic_string<T> data;

  try
  {
    T* current_address = static_cast<T*>(address);
    for (T current = detail::ReadUnchecked<T>(process, current_address); 
      current != T(); 
      current = detail::ReadUnchecked<T>(process, ++current_address))
    {
      data.push_back(current);
    }
  }
  catch (std::exception const& /*e*/)
  {
    if (!can_read)
    {
      try
      {
        Protect(process, address, old_protect);
      }
      catch (std::exception const& e)
      {
        (void)e;
        BOOST_ASSERT_MSG(false, boost::diagnostic_information(e).c_str());
      }
    }

    throw;
  }

  if (!can_read)
  {
    Protect(process, address, old_protect);
  }

  return data;
}

template <typename T>
std::basic_string<T> ReadString(Process const& process, PVOID address, 
  std::size_t max_len)
{
  HADESMEM_STATIC_ASSERT(detail::IsCharType<T>::value);

  std::basic_string<T> data(max_len, T());
  detail::Read(process, address, &data[0], sizeof(T) * max_len);

  std::size_t const null_offs = data.find(T());
  if (null_offs != std::basic_string<T>::npos)
  {
    data.erase(null_offs);
  }

  return data;
}

template <typename T>
std::vector<T> ReadVector(Process const& process, PVOID address, 
  std::size_t size)
{
  HADESMEM_STATIC_ASSERT(detail::IsTriviallyCopyable<T>::value);

  HADESMEM_STATIC_ASSERT(detail::IsDefaultConstructible<T>::value);
  
  std::vector<T> data(size);
  detail::Read(process, address, data.data(), sizeof(T) * size);
  return data;
}

}
