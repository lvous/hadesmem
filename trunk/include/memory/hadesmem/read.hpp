// Copyright (C) 2010-2013 Joshua Boyce.
// See the file COPYING for copying permission.

#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>
#include <cstddef>
#include <exception>
#include <type_traits>

#include <windows.h>

#include <hadesmem/error.hpp>
#include <hadesmem/protect.hpp>
#include <hadesmem/detail/assert.hpp>
#include <hadesmem/detail/read_impl.hpp>
#include <hadesmem/detail/type_traits.hpp>
#include <hadesmem/detail/query_region.hpp>
#include <hadesmem/detail/protect_guard.hpp>
#include <hadesmem/detail/static_assert.hpp>

// NOTE: Reads which span across region boundaries are not explicitly handled 
// or supported. They may work simply by chance (or if the user changes the 
// memory page protections preemptively in preparation for the read), however 
// this is not guaranteed to work, even in the aforementioned scenario.

namespace hadesmem
{

class Process;

template <typename T>
T Read(Process const& process, PVOID address)
{
  HADESMEM_ASSERT(address != nullptr);
  
  return detail::Read<T>(process, address);
}

template <typename T, std::size_t N>
std::array<T, N> Read(Process const& process, PVOID address)
{
  HADESMEM_ASSERT(address != nullptr);

  return detail::Read<std::array<T, N>>(process, address);
}

// TODO: Clean up this function.
template <typename T>
std::basic_string<T> ReadString(Process const& process, PVOID address, 
  std::size_t chunk_len = 128)
{
  HADESMEM_STATIC_ASSERT(detail::IsCharType<T>::value);

  HADESMEM_ASSERT(chunk_len != 0);

  std::basic_string<T> data;

  for (;;)
  {
    detail::ProtectGuard protect_guard(process, address, 
      detail::ProtectGuardType::kRead);

    MEMORY_BASIC_INFORMATION const mbi = detail::Query(process, address);
    PVOID const region_next = static_cast<PBYTE>(mbi.BaseAddress) + 
      mbi.RegionSize;

    T* cur = static_cast<T*>(address);
    while (cur + 1 < region_next)
    {
      std::size_t const len_to_end = reinterpret_cast<DWORD_PTR>(region_next) - 
        reinterpret_cast<DWORD_PTR>(cur);
      std::size_t const buf_len_bytes = (std::min)(chunk_len * sizeof(T), 
        len_to_end);
      std::size_t const buf_len = buf_len_bytes / sizeof(T);

      std::vector<T> buf(buf_len);
      detail::ReadUnchecked(process, cur, buf.data(), buf.size() * sizeof(T));

      auto const iter = std::find(std::begin(buf), std::end(buf), T());
      std::copy(std::begin(buf), iter, std::back_inserter(data));

      if (iter != std::end(buf))
      {
        protect_guard.Restore();
        return data;
      }

      cur += buf_len;
    }

    address = region_next;
  }
}

template <typename T>
std::vector<T> ReadVector(Process const& process, PVOID address, 
  std::size_t count)
{
  HADESMEM_STATIC_ASSERT(detail::IsTriviallyCopyable<T>::value);
  HADESMEM_STATIC_ASSERT(detail::IsDefaultConstructible<T>::value);
  
  HADESMEM_ASSERT(address != nullptr);
  HADESMEM_ASSERT(count != 0);
  
  std::vector<T> data(count);
  detail::Read(process, address, data.data(), sizeof(T) * count);
  return data;
}

}
