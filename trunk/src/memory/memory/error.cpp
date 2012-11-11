// Copyright Joshua Boyce 2010-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// This file is part of HadesMem.
// <http://www.raptorfactor.com/> <raptorfactor@raptorfactor.com>

#include "hadesmem/error.hpp"

namespace hadesmem
{

Error::Error()
  : what_()
{
  what_ += "Exception type: ";
  what_ += typeid(*this).name();
}

char const* Error::what() const HADESMEM_NOEXCEPT
{
  return what_.c_str();
}

}
