// Copyright Joshua Boyce 2010-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// This file is part of HadesMem.
// <http://www.raptorfactor.com/> <raptorfactor@raptorfactor.com>

#pragma once

#include "hadesmem/detail/warning_disable_prefix.hpp"
#include <boost/config.hpp>
#include <boost/optional.hpp>
#include "hadesmem/detail/warning_disable_suffix.hpp"

#include <windows.h>

#include "hadesmem/region.hpp"

namespace hadesmem
{

class Process;

namespace detail
{

struct RegionIteratorImpl
{
  RegionIteratorImpl() BOOST_NOEXCEPT
    : process_(nullptr), 
    region_()
  { }
  
  Process const* process_;
  boost::optional<Region> region_;
  
private:
  RegionIteratorImpl(RegionIteratorImpl const&);
  RegionIteratorImpl& operator=(RegionIteratorImpl const&);
};

}

}
