// Copyright Joshua Boyce 2010-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// This file is part of HadesMem.
// <http://www.raptorfactor.com/> <raptorfactor@raptorfactor.com>

#pragma once

#include <windows.h>

#include "hadesmem/config.hpp"
#include "hadesmem/region.hpp"
#include "hadesmem/detail/optional.hpp"

namespace hadesmem
{

class Process;

namespace detail
{

struct RegionIteratorImpl
{
  RegionIteratorImpl() HADESMEM_NOEXCEPT
    : process_(nullptr), 
    region_()
  { }
  
  Process const* process_;
  Optional<Region> region_;
  
private:
  RegionIteratorImpl(RegionIteratorImpl const&) HADESMEM_DELETED_FUNCTION;
  RegionIteratorImpl& operator=(RegionIteratorImpl const&) 
    HADESMEM_DELETED_FUNCTION;
};

}

}
