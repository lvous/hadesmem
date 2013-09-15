// Copyright (C) 2010-2013 Joshua Boyce.
// See the file COPYING for copying permission.

#pragma once

#include <memory>
#include <utility>

#include <windows.h>

#include <hadesmem/config.hpp>
#include <hadesmem/detail/assert.hpp>
#include <hadesmem/detail/protect_region.hpp>
#include <hadesmem/detail/query_region.hpp>
#include <hadesmem/detail/trace.hpp>
#include <hadesmem/error.hpp>
#include <hadesmem/process.hpp>
#include <hadesmem/protect.hpp>

namespace hadesmem
{

namespace detail
{

enum class ProtectGuardType
{
  kRead, 
  kWrite
};

class ProtectGuard
{
public:
  explicit ProtectGuard(Process const& process, PVOID address, 
    ProtectGuardType type)
    : process_(&process), 
    type_(type), 
    can_read_or_write_(false), 
    old_protect_(0), 
    mbi_(Query(process, address))
  {
    if (IsGuard(mbi_))
    {
      HADESMEM_DETAIL_THROW_EXCEPTION(Error() << 
        ErrorString("Attempt to write to guard page."));
    }

    can_read_or_write_ = (type_ == ProtectGuardType::kRead) ? 
      CanRead(mbi_) : CanWrite(mbi_);

    if (!can_read_or_write_)
    {
      old_protect_ = Protect(process, mbi_, PAGE_EXECUTE_READWRITE);
    }
  }

  ProtectGuard(ProtectGuard&& other) HADESMEM_DETAIL_NOEXCEPT
    : process_(other.process_), 
    type_(other.type_), 
    can_read_or_write_(other.can_read_or_write_), 
    old_protect_(other.old_protect_), 
    mbi_(other.mbi_)
  {
    other.old_protect_ = 0;
  }

  ProtectGuard& operator=(ProtectGuard&& other) HADESMEM_DETAIL_NOEXCEPT
  {
    process_ = other.process_;
    type_ = other.type_;
    can_read_or_write_ = other.can_read_or_write_;
    old_protect_ = other.old_protect_;
    mbi_ = other.mbi_;

    other.old_protect_ = 0;

    return *this;
  }

  ~ProtectGuard() HADESMEM_DETAIL_NOEXCEPT
  {
    RestoreUnchecked();
  }
  
  void Restore()
  {
    if (!old_protect_)
    {
      return;
    }

    if (!can_read_or_write_)
    {
      Protect(*process_, mbi_, old_protect_);
    }

    old_protect_ = 0;
  }

  void RestoreUnchecked() HADESMEM_DETAIL_NOEXCEPT
  {
    try
    {
      Restore();
    }
    catch (std::exception const& e)
    {
      // WARNING: Protection is not restored if 'Restore' fails.
      (void)e;
      HADESMEM_DETAIL_TRACE_A(boost::diagnostic_information(e).c_str());
      HADESMEM_DETAIL_ASSERT(false);
    }
  }

private:
  ProtectGuard(ProtectGuard const& other) HADESMEM_DETAIL_DELETED_FUNCTION;
  ProtectGuard& operator=(ProtectGuard const& other) HADESMEM_DETAIL_DELETED_FUNCTION;
  
  Process const* process_;
  ProtectGuardType type_;
  bool can_read_or_write_;
  DWORD old_protect_;
  MEMORY_BASIC_INFORMATION mbi_;
};

}

}
