/*
This file is part of HadesMem.
Copyright © 2010 RaptorFactor (aka Cypherjb, Cypher, Chazwazza). 
<http://www.raptorfactor.com/> <raptorfactor@raptorfactor.com>

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

// Windows API
#include <Windows.h>

// C++ Standard Library
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <functional>

// Boost
#pragma warning(push, 1)
#pragma warning(disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/make_shared.hpp>
#pragma warning(pop)

// Lua
extern "C"
{
#include <lua.h>
}

// LuaBind
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <LuaBind/luabind.hpp>
#include <Luabind/tag_function.hpp>
#include <Luabind/iterator_policy.hpp>
#include <Luabind/exception_handler.hpp>
#pragma warning(pop)

// Hades
#include "Wrappers.h"
#include "Hades-Common/I18n.h"
#include "Hades-Memory/PeLib.h"
#include "Hades-Memory/Memory.h"

// LuaBind extensions for 64-bit number support
// Fixme: Causes truncation to a 32-bit type.
namespace luabind
{
  template <>
  struct default_converter<unsigned long long>
    : native_converter_base<unsigned long long>
  {
    int compute_score(lua_State* L, int index)
    {
      return lua_type(L, index) == LUA_TNUMBER ? 0 : -1;
    };

    unsigned long long from(lua_State* L, int index)
    {
      return static_cast<unsigned long long>(BOOST_PP_CAT(lua_to, integer)
        (L, index));
    }

    void to(lua_State* L, unsigned long long const& value)
    {
      BOOST_PP_CAT(lua_push, integer)(L, BOOST_PP_CAT(as_lua_, integer)(
        value));
    }
  };

  template <>
  struct default_converter<unsigned long long const>
    : default_converter<unsigned long long>
  {};

  template <>
  struct default_converter<unsigned long long const&>
    : default_converter<unsigned long long>
  {};

  template <>
  struct default_converter<signed long long>
    : native_converter_base<signed long long>
  {
    int compute_score(lua_State* L, int index)
    {
      return lua_type(L, index) == LUA_TNUMBER ? 0 : -1;
    };

    signed long long from(lua_State* L, int index)
    {
      return static_cast<signed long long>(BOOST_PP_CAT(lua_to, integer)
        (L, index));
    }

    void to(lua_State* L, signed long long const& value)
    {
      BOOST_PP_CAT(lua_push, integer)(L, BOOST_PP_CAT(as_lua_, integer)(
        value));
    }
  };

  template <>
  struct default_converter<signed long long const>
    : default_converter<signed long long>
  {};

  template <>
  struct default_converter<signed long long const&>
    : default_converter<signed long long>
  {};
}

namespace Hades
{
  namespace Memory
  {
    // LuaState wrapper class for RAII
    class LuaState : private boost::noncopyable
    {
      // Only LuaMgr can create states
      friend class LuaMgr;

      // Underlying lua state
      lua_State* m_State;

    public:
      // Destructor
      ~LuaState();

      // Implicitly act as a lua_State pointer
      operator lua_State*() const;

      // Implicitly act as a lua_State pointer
      operator lua_State*();

    protected:
      // Constructor
      LuaState();
    };

    // Lua managing class
    class LuaMgr : private boost::noncopyable
    {
    public:
      // Lua exception type
      class Error : public virtual HadesMemError 
      { };

      // Constructor
      LuaMgr();

      // Get LUA state
      const LuaState& GetState();

      // Run a LUA script on disk
      void RunFile(std::string const& Path);

      // Run a LUA script from a string
      void RunString(std::string const& Script);

      // Reports an error to the console
      void ReportError(int Status);

    private:
      // Lua state
      LuaState m_State;
    };

    class ScriptMgr : public LuaMgr
    {
    public:
      // Constructor
      ScriptMgr();

      // Custom LuaBind exception filter
      void TranslateException(lua_State* L, std::exception const& e);
    };
  }
}
