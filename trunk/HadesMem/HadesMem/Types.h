/*
This file is part of HadesMem.

HadesMem is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

HadesMem is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

// C++ Standard Library
#include <string>

namespace Hades
{
  namespace Memory
  {
    namespace Types
    {
      // Declare fixed-size types
      typedef char                  Byte;
      typedef short                 Int16;
      typedef unsigned short        UInt16;
      typedef int                   Int32;
      typedef unsigned int          UInt32;
      typedef long long             Int64;
      typedef unsigned long long    UInt64;
      typedef float                 Float;
      typedef double                Double;
      typedef char                  CharNarrow;
      typedef wchar_t               CharWide;
      typedef std::string           StrNarrow;
      typedef std::wstring          StrWide;
      typedef void*                 Pointer;

      // Ensure data type are correct
      static_assert(sizeof(char) == 1, "Size of Byte is wrong.");
      static_assert(sizeof(Int16) == 2, "Size of Int16 is wrong.");
      static_assert(sizeof(UInt16) == 2, "Size of UInt16 is wrong.");
      static_assert(sizeof(Int32) == 4, "Size of Int32 is wrong.");
      static_assert(sizeof(UInt32) == 4, "Size of UInt32 is wrong.");
      static_assert(sizeof(Int64) == 8, "Size of Int64 is wrong.");
      static_assert(sizeof(UInt64) == 8, "Size of ""UInt64 is wrong.");
      static_assert(sizeof(Float) == 4, "Size of Float is wrong.");
      static_assert(sizeof(Double) == 8, "Size of Double is wrong.");
    }
  }
}
