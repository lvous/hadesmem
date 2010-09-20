/*
This file is part of HadesMem.
Copyright � 2010 Cypherjb (aka Chazwazza, aka Cypher). 
<http://www.cypherjb.com/> <cypher.jb@gmail.com>

HadesMem is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

HadesMem is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with HadesMem.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

// Hades
#include "Config.h"

// C++ Standard Library
#include <string>
#include <cvt/wstring>

// Boost
HADES_DISABLE_WARNINGS_PUSH()
#include <boost/lexical_cast.hpp>
HADES_DISABLE_WARNINGS_POP()

// Boost.LexicalCast specialization to allow conversions from wide to narrow 
// strings.
template<> 
inline std::string boost::lexical_cast<std::string, std::wstring>(
  std::wstring const& Source)
{
  return stdext::cvt::wstring_convert<std::codecvt<wchar_t, char, 
    mbstate_t>>().to_bytes(Source);
}

// Boost.LexicalCast specialization to allow conversions from narrow to wide 
// strings.
template<> 
inline std::wstring boost::lexical_cast<std::wstring, std::string>(
  std::string const& Source)
{
  return stdext::cvt::wstring_convert<std::codecvt<wchar_t, char, 
    mbstate_t>>().from_bytes(Source);
}
