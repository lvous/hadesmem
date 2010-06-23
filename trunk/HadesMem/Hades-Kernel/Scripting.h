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

// Boost
#pragma warning(push, 1)
#include <boost/noncopyable.hpp>
#pragma warning(pop)

// Lua
extern "C"
{
#include "lua.h"
}

// LuaBind
#pragma warning(push, 1)
#include "LuaBind/luabind.hpp"
#include "luabind/tag_function.hpp"
#include "luabind/iterator_policy.hpp"
#pragma warning(pop)

// Hades
#include "Hades-Common/Error.h"

#define HADES_SCRIPTING_TRYCATCH_BEGIN \
  try\
{

#define HADE_SCRIPTING_TRYCATCH_END \
}\
  catch (boost::exception const& e)\
{\
  throw std::exception(boost::diagnostic_information(e).c_str());\
}

namespace Hades
{
  class Kernel;

  namespace Wrappers
  {
    class WriteLn
    {
    public:
      explicit WriteLn(Kernel* pKernel);

      void operator() (std::string const& Input) const;

    private:
      Kernel* m_pKernel;
    };
  }

  // Lua exception type
  class LuaError : public virtual HadesError 
  { };

  // LuaState wrapper class for RAII
  class LuaState : private boost::noncopyable
  {
    // Only LuaMgr can create states
    friend class LuaMgr;

    // Underlying lua state
    lua_State* m_State;

  public:
    // Destructor
    ~LuaState()
    {
      // Close LUA
      lua_close(m_State);
    }

    // Implicitly act as a lua_State pointer
    operator lua_State*() const 
    {
      // Return underlying lua state
      return m_State;
    }

    // Implicitly act as a lua_State pointer
    operator lua_State*() 
    {
      // Return underlying lua state
      return m_State;
    }

  protected:
    // Constructor
    LuaState() 
      : m_State(lua_open()) // Open LUA
    { }
  };

  // Lua managing class
  class LuaMgr
  {
  public:
    // Constructor
    LuaMgr() 
      : m_State()
    {
      // Open LuaBind with Lua state
      luabind::open(m_State);
    }

    // Get LUA state
    const LuaState& GetState() 
    {
      return m_State;
    }

    // Run a LUA script on disk
    void RunFile(std::string const& Path)
    {
      // Load and execute file
      int Status = luaL_dofile(m_State, Path.c_str());
      // Clean up if an error occurred
      if (Status != 0) 
      {
        lua_gc(m_State, LUA_GCCOLLECT, 0);
      }
      // Report any errors
      ReportError(Status);
    }

    // Run a LUA script from a string
    void RunString(std::string const& Script)
    {
      // Load and execute string
      int Status = luaL_dostring(m_State, Script.c_str());
      // Clean up if an error occurred
      if (Status != 0) 
      {
        lua_gc(m_State, LUA_GCCOLLECT, 0);
      }
      // Report any errors
      ReportError(Status);
    }

    // Reports an error to the console
    void ReportError(int Status)
    {
      // Check if an error occurred
      if (Status && !lua_isnil(m_State, -1)) 
      {
        // Get error message as string
        const char* Message = lua_tostring(m_State, -1);
        // If a conversion to string is not possible set that as the message
        if (Message == NULL) 
        {
          Message = "Error object is not a string";
        }
        // Pop error message off stack
        lua_pop(m_State, 1);
        // Throw exception for error
        BOOST_THROW_EXCEPTION(LuaError() << 
          ErrorFunction("LuaMgr::ReportError") << 
          ErrorString(Message));
      }
    }

  private:
    // Lua state
    LuaState m_State;
  };
}
