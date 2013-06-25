// Copyright (C) 2010-2013 Joshua Boyce.
// See the file COPYING for copying permission.

#pragma once

#include <array>
#include <iosfwd>
#include <memory>
#include <vector>
#include <cstddef>
#include <ostream>
#include <utility>

#include <hadesmem/detail/warning_disable_prefix.hpp>
#include <boost/assert.hpp>
#include <hadesmem/detail/warning_disable_suffix.hpp>

#include <windows.h>
#include <winnt.h>

#include <hadesmem/read.hpp>
#include <hadesmem/error.hpp>
#include <hadesmem/write.hpp>
#include <hadesmem/config.hpp>
#include <hadesmem/process.hpp>
#include <hadesmem/pelib/pe_file.hpp>
#include <hadesmem/pelib/nt_headers.hpp>

namespace hadesmem
{

class TlsDir
{
public:
  explicit TlsDir(Process const& process, PeFile const& pe_file)
    : process_(&process), 
    pe_file_(&pe_file), 
    base_(nullptr)
  {
    NtHeaders const nt_headers(process, pe_file);

    // TODO: Some sort of API to handle this common case.
    DWORD const data_dir_va = nt_headers.GetDataDirectoryVirtualAddress(
      PeDataDir::TLS);
    // Windows will load images which don't specify a size for the TLS 
    // directory.
    if (!data_dir_va)
    {
      HADESMEM_THROW_EXCEPTION(Error() << 
        ErrorString("PE file has no TLS directory."));
    }

    base_ = static_cast<PBYTE>(RvaToVa(process, pe_file, data_dir_va));
  }

  TlsDir(TlsDir const& other)
    : process_(other.process_), 
    pe_file_(other.pe_file_), 
    base_(other.base_)
  { }
  
  TlsDir& operator=(TlsDir const& other)
  {
    process_ = other.process_;
    pe_file_ = other.pe_file_;
    base_ = other.base_;

    return *this;
  }

  TlsDir(TlsDir&& other) HADESMEM_NOEXCEPT
    : process_(other.process_), 
    pe_file_(other.pe_file_), 
    base_(other.base_)
  { }
  
  TlsDir& operator=(TlsDir&& other) HADESMEM_NOEXCEPT
  {
    process_ = other.process_;
    pe_file_ = other.pe_file_;
    base_ = other.base_;

    return *this;
  }
  
  ~TlsDir() HADESMEM_NOEXCEPT
  { }

  PVOID GetBase() const HADESMEM_NOEXCEPT
  {
    return base_;
  }

  DWORD_PTR GetStartAddressOfRawData() const
  {
    return Read<DWORD_PTR>(*process_, base_ + offsetof(IMAGE_TLS_DIRECTORY, 
      StartAddressOfRawData));
  }

  DWORD_PTR GetEndAddressOfRawData() const
  {
    return Read<DWORD_PTR>(*process_, base_ + offsetof(IMAGE_TLS_DIRECTORY, 
      EndAddressOfRawData));
  }

  DWORD_PTR GetAddressOfIndex() const
  {
    return Read<DWORD_PTR>(*process_, base_ + offsetof(IMAGE_TLS_DIRECTORY, 
      AddressOfIndex));
  }

  DWORD_PTR GetAddressOfCallBacks() const
  {
    return Read<DWORD_PTR>(*process_, base_ + offsetof(IMAGE_TLS_DIRECTORY, 
      AddressOfCallBacks));
  }
  
  // TODO: Decide whether to throw on a NULL AddressOfCallbacks, return an 
  // empty container, or simply ignore it and consider it a precondition 
  // violation. Update dumper afterwards to reflect decision. (Currently it 
  // is manually checking AddressOfCallbacks.)
  template <typename OutputIterator>
  void GetCallbacks(OutputIterator callbacks) const
  {
    // TODO: Iterator checks for type and category.
    
    ULONG_PTR image_base = GetRuntimeBase(*process_, *pe_file_);
    auto callbacks_raw = reinterpret_cast<PIMAGE_TLS_CALLBACK*>(RvaToVa(
      *process_, *pe_file_, static_cast<DWORD>(GetAddressOfCallBacks() - 
      image_base)));
  
    for (auto callback = Read<PIMAGE_TLS_CALLBACK>(*process_, callbacks_raw); 
      callback; 
      callback = Read<PIMAGE_TLS_CALLBACK>(*process_, ++callbacks_raw))
    {
      DWORD_PTR const callback_offset = reinterpret_cast<DWORD_PTR>(
        callback) - image_base;
      *callbacks = reinterpret_cast<PIMAGE_TLS_CALLBACK>(
        callback_offset);
      ++callbacks;
    }
  }

  DWORD GetSizeOfZeroFill() const
  {
    return Read<DWORD>(*process_, base_ + offsetof(IMAGE_TLS_DIRECTORY, 
      SizeOfZeroFill));
  }

  DWORD GetCharacteristics() const
  {
    return Read<DWORD>(*process_, base_ + offsetof(IMAGE_TLS_DIRECTORY, 
      Characteristics));
  }

  void SetStartAddressOfRawData(DWORD_PTR start_address_of_raw_data)
  {
    Write(*process_, base_ + offsetof(IMAGE_TLS_DIRECTORY, 
      StartAddressOfRawData), start_address_of_raw_data);
  }

  void SetEndAddressOfRawData(DWORD_PTR end_address_of_raw_data)
  {
    Write(*process_, base_ + offsetof(IMAGE_TLS_DIRECTORY, 
      EndAddressOfRawData), end_address_of_raw_data);
  }

  void SetAddressOfIndex(DWORD_PTR address_of_index)
  {
    Write(*process_, base_ + offsetof(IMAGE_TLS_DIRECTORY, 
      AddressOfIndex), address_of_index);
  }

  void SetAddressOfCallBacks(DWORD_PTR address_of_callbacks)
  {
    Write(*process_, base_ + offsetof(IMAGE_TLS_DIRECTORY, 
      AddressOfCallBacks), address_of_callbacks);
  }

  // TODO: SetCallbacks function

  void SetSizeOfZeroFill(DWORD size_of_zero_fill)
  {
    Write(*process_, base_ + offsetof(IMAGE_TLS_DIRECTORY, 
      SizeOfZeroFill), size_of_zero_fill);
  }

  void SetCharacteristics(DWORD characteristics)
  {
    Write(*process_, base_ + offsetof(IMAGE_TLS_DIRECTORY, 
      Characteristics), characteristics);
  }
  
private:
  Process const* process_;
  PeFile const* pe_file_;
  PBYTE base_;
};

inline bool operator==(TlsDir const& lhs, TlsDir const& rhs) HADESMEM_NOEXCEPT
{
  return lhs.GetBase() == rhs.GetBase();
}

inline bool operator!=(TlsDir const& lhs, TlsDir const& rhs) HADESMEM_NOEXCEPT
{
  return !(lhs == rhs);
}

inline bool operator<(TlsDir const& lhs, TlsDir const& rhs) HADESMEM_NOEXCEPT
{
  return lhs.GetBase() < rhs.GetBase();
}

inline bool operator<=(TlsDir const& lhs, TlsDir const& rhs) HADESMEM_NOEXCEPT
{
  return lhs.GetBase() <= rhs.GetBase();
}

inline bool operator>(TlsDir const& lhs, TlsDir const& rhs) HADESMEM_NOEXCEPT
{
  return lhs.GetBase() > rhs.GetBase();
}

inline bool operator>=(TlsDir const& lhs, TlsDir const& rhs) HADESMEM_NOEXCEPT
{
  return lhs.GetBase() >= rhs.GetBase();
}

inline std::ostream& operator<<(std::ostream& lhs, TlsDir const& rhs)
{
  return (lhs << rhs.GetBase());
}

inline std::wostream& operator<<(std::wostream& lhs, TlsDir const& rhs)
{
  return (lhs << rhs.GetBase());
}

}
