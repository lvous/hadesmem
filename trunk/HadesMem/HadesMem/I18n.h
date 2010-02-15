#pragma once

// C++ Standard Library
#include <string>
#include <cctype>
#include <locale>
#include <algorithm>

namespace Hades
{
  namespace I18n
  {
    // Convert a wide string to a narrow string
    inline std::string ConvertStr(std::wstring const& Source)
    {
      auto const Loc(std::locale(""));
      auto const& MyCType(std::use_facet<std::ctype<wchar_t>>(Loc));

      std::string Dest;
      std::for_each(Source.begin(), Source.end(), 
        [&Dest, &MyCType] (wchar_t Current)
      {
        Dest += MyCType.narrow(Current);
      });

      return Dest;
    }

    // Convert a narrow string to a wide string
    inline std::wstring ConvertStr(std::string const& Source)
    {
      auto const Loc(std::locale(""));
      auto const& MyCType(std::use_facet<std::ctype<char>>(Loc));

      std::wstring Dest;
      std::for_each(Source.begin(), Source.end(), 
        [&Dest, &MyCType] (char Current)
      {
        Dest += MyCType.widen(Current);
      });

      return Dest;
    }

    // Convert string to lowercase
    template <typename CharT>
    inline std::basic_string<CharT> ToLower(std::basic_string<CharT> const& 
      Str)
    {
      auto const Loc(std::locale(""));
      auto const& MyCType(std::use_facet<std::ctype<CharT>>(Loc));

      std::basic_string<CharT> Dest(Str);
      std::transform(Dest.begin(), Dest.end(), Dest.begin(), 
        [&MyCType] (CharT x) 
      {
        return MyCType.tolower(x);
      });

      return Dest;
    }

    // Convert string to lowercase
    template <typename CharT>
    inline std::basic_string<CharT> ToUpper(std::basic_string<CharT> const& 
      Str)
    {
      auto const Loc(std::locale(""));
      auto const& MyCType(std::use_facet<std::ctype<CharT>>(Loc));

      std::basic_string<CharT> Dest(Str);
      std::transform(Dest.begin(), Dest.end(), Dest.begin(), 
        [&MyCType] (CharT x) 
      {
        return MyCType.toupper(x);
      });

      return Dest;
    }
  }
}
