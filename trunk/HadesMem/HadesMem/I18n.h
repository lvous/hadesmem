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
    inline std::string ConvertStr(const std::wstring& Source)
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
    inline std::wstring ConvertStr(const std::string& Source)
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
    template <typename charT>
    inline std::basic_string<charT> ToLower(const std::basic_string<charT>& 
      Str)
    {
      auto const Loc(std::locale(""));
      auto const& MyCType(std::use_facet<std::ctype<charT>>(Loc));

      std::basic_string<charT> Dest(Str);
      std::transform(Dest.begin(), Dest.end(), Dest.begin(), 
        [&MyCType] (charT x) 
      {
        return MyCType.tolower(x);
      });

      return Dest;
    }

    // Convert string to lowercase
    template <typename charT>
    inline std::basic_string<charT> ToUpper(const std::basic_string<charT>& 
      Str)
    {
      auto const Loc(std::locale(""));
      auto const& MyCType(std::use_facet<std::ctype<charT>>(Loc));

      std::basic_string<charT> Dest(Str);
      std::transform(Dest.begin(), Dest.end(), Dest.begin(), 
        [&MyCType] (charT x) 
      {
        return MyCType.toupper(x);
      });

      return Dest;
    }
  }
}
