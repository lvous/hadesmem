/*
This file is part of HadesMem.
Copyright © 2010 Cypherjb (aka Chazwazza, aka Cypher). 
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

// C++ Standard Library
#include <string>
#include <vector>

// Boost
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/lexical_cast.hpp>
#pragma warning(pop)

// Windows API
#include <Windows.h>

// Hades
#include "Error.h"

namespace Hades
{
  namespace I18n
  {
    class Error : public virtual HadesError
    { };
  }
}

// Boost.LexicalCast specialization to allow conversions from wide to narrow 
// strings.
template<> 
inline std::string boost::lexical_cast<std::string, std::wstring>(
  std::wstring const& Source)
{
  int SrcLen = static_cast<int>(Source.length()) + 1;
  int DestLen = WideCharToMultiByte(CP_ACP, 0, Source.c_str(), SrcLen, NULL, 
    0, NULL, NULL);
  if (!DestLen)
  {
    DWORD const LastError = GetLastError();
    BOOST_THROW_EXCEPTION(Hades::I18n::Error() << 
      Hades::ErrorFunction("boost::lexical_cast<std::string, std::wstring>") << 
      Hades::ErrorString("Could not calculate target length.") << 
      Hades::ErrorCodeWin(LastError));
  }

  std::vector<char> Dest(DestLen);
  if (!WideCharToMultiByte(CP_ACP, 0, Source.c_str(), SrcLen, &Dest[0], 
    DestLen, NULL, NULL))
  {
    DWORD const LastError = GetLastError();
    BOOST_THROW_EXCEPTION(Hades::I18n::Error() << 
      Hades::ErrorFunction("boost::lexical_cast<std::string, std::wstring>") << 
      Hades::ErrorString("Could not convert string.") << 
      Hades::ErrorCodeWin(LastError));
  }

  return &Dest[0];
}

// Boost.LexicalCast specialization to allow conversions from narrow to wide 
// strings.
template<> 
inline std::wstring boost::lexical_cast<std::wstring, std::string>(
  std::string const& Source)
{
  int SrcLen = static_cast<int>(Source.length()) + 1;
  int DestLen = MultiByteToWideChar(CP_ACP, 0, Source.c_str(), SrcLen, NULL, 
    0);
  if (!DestLen)
  {
    DWORD const LastError = GetLastError();
    BOOST_THROW_EXCEPTION(Hades::I18n::Error() << 
      Hades::ErrorFunction("boost::lexical_cast<std::wstring, std::string>") << 
      Hades::ErrorString("Could not calculate target length.") << 
      Hades::ErrorCodeWin(LastError));
  }
  
  std::vector<wchar_t> Dest(DestLen);
  if (!MultiByteToWideChar(CP_ACP, 0, Source.c_str(), SrcLen, &Dest[0], 
    DestLen))
  {
    DWORD const LastError = GetLastError();
    BOOST_THROW_EXCEPTION(Hades::I18n::Error() << 
      Hades::ErrorFunction("boost::lexical_cast<std::wstring, std::string>") << 
      Hades::ErrorString("Could not convert string.") << 
      Hades::ErrorCodeWin(LastError));
  }
  
  return &Dest[0];
}
