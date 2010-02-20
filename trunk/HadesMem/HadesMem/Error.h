#pragma once

// Windows
#include <Windows.h>

// C++ Standard Library
#include <string>
#include <stdexcept>

// Boost
#pragma warning(push, 1)
#pragma warning(disable: 4706)
#include <boost/exception/all.hpp>
#pragma warning(pop)

namespace Hades
{
  namespace Memory
  {
    // Error info (function name)
    typedef boost::error_info<struct TagErrorFunc, std::string> ErrorFunction;
    // Error info (error string)
    typedef boost::error_info<struct TagErrorString, std::string> ErrorString;
    // Error info (Windows error code)
    typedef boost::error_info<struct TagErrorCodeWin, DWORD> ErrorCodeWin;

    // Base exception class
    class HadesMemError : public virtual std::exception, 
      public virtual boost::exception
    { };
  }
}
