#pragma once

// Windows API
#include <Windows.h>

// C++ Standard Library
#include <iostream>

// Boost
#pragma warning(push, 1)
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/make_shared.hpp>
#pragma warning(pop)

// Lua
extern "C"
{
  #include "lua.h"
}

// LuaBind
#pragma warning(push, 1)
#include "LuaBind/luabind.hpp"
#pragma warning(pop)

// HadesMem
#include "Types.h"
#include "Memory.h"

// Wrapper function generator for MemoryMgr::Read
#define HADESMEM_SCRIPTING_GEN_READ(x) \
inline Types::x Read##x(MemoryMgr const& MyMemory, DWORD_PTR Address)\
{\
  return MyMemory.Read<Types::x>(reinterpret_cast<PVOID>(Address));\
}

// Wrapper function generator for MemoryMgr::Write
#define HADESMEM_SCRIPTING_GEN_WRITE(x) \
inline void Write##x(MemoryMgr const& MyMemory, DWORD_PTR Address, \
  Types::x Data)\
{\
  MyMemory.Write<Types::x>(reinterpret_cast<PVOID>(Address), Data);\
}

namespace Hades
{
  namespace Memory
  {
    namespace Wrappers
    {
      // Console output wrapper
      inline void WriteLn(std::string const& Data)
      {
        std::cout << Data << std::endl;
      }

      // MemoryMgr::MemoryMgr wrappers
      inline boost::shared_ptr<MemoryMgr> CreateMemoryMgr(DWORD ProcId)
      {
        return boost::make_shared<MemoryMgr>(ProcId);
      }
      inline boost::shared_ptr<MemoryMgr> CreateMemoryMgr(
        std::string const& ProcName)
      {
        return boost::make_shared<MemoryMgr>(boost::lexical_cast<std::wstring>(
          ProcName));
      }
      inline boost::shared_ptr<MemoryMgr> CreateMemoryMgr(
        std::string const& WindowName, 
        std::string const* const ClassName)
      {
        std::wstring ClassNameW(ClassName ? boost::lexical_cast<std::wstring>(
          *ClassName) : L"");
        return boost::make_shared<MemoryMgr>(boost::lexical_cast<std::wstring>(
          WindowName), ClassName ? &ClassNameW : nullptr);
      }

      // MemoryMgr::CanRead wrapper
      inline bool CanRead(MemoryMgr const& MyMemory, DWORD_PTR Address)
      {
        return MyMemory.CanRead(reinterpret_cast<PVOID>(Address));
      }

      // MemoryMgr::CanWrite wrapper
      inline bool CanWrite(MemoryMgr const& MyMemory, DWORD_PTR Address)
      {
        return MyMemory.CanWrite(reinterpret_cast<PVOID>(Address));
      }

      // MemoryMgr::Read<T> wrappers
      HADESMEM_SCRIPTING_GEN_READ(Byte)
      HADESMEM_SCRIPTING_GEN_READ(Int16)
      HADESMEM_SCRIPTING_GEN_READ(UInt16)
      HADESMEM_SCRIPTING_GEN_READ(Int32)
      HADESMEM_SCRIPTING_GEN_READ(UInt32)
      HADESMEM_SCRIPTING_GEN_READ(Int64)
      HADESMEM_SCRIPTING_GEN_READ(UInt64)
      HADESMEM_SCRIPTING_GEN_READ(Float)
      HADESMEM_SCRIPTING_GEN_READ(Double)
      HADESMEM_SCRIPTING_GEN_READ(CharNarrow)
      HADESMEM_SCRIPTING_GEN_READ(CharWide)
      HADESMEM_SCRIPTING_GEN_READ(StrNarrow)
      HADESMEM_SCRIPTING_GEN_READ(StrWide)
      HADESMEM_SCRIPTING_GEN_READ(Pointer)

      // MemoryMgr::Write<T> wrappers
      HADESMEM_SCRIPTING_GEN_WRITE(Byte)
      HADESMEM_SCRIPTING_GEN_WRITE(Int16)
      HADESMEM_SCRIPTING_GEN_WRITE(UInt16)
      HADESMEM_SCRIPTING_GEN_WRITE(Int32)
      HADESMEM_SCRIPTING_GEN_WRITE(UInt32)
      HADESMEM_SCRIPTING_GEN_WRITE(Int64)
      HADESMEM_SCRIPTING_GEN_WRITE(UInt64)
      HADESMEM_SCRIPTING_GEN_WRITE(Float)
      HADESMEM_SCRIPTING_GEN_WRITE(Double)
      HADESMEM_SCRIPTING_GEN_WRITE(CharNarrow)
      HADESMEM_SCRIPTING_GEN_WRITE(CharWide)
      HADESMEM_SCRIPTING_GEN_WRITE(StrNarrow)
      HADESMEM_SCRIPTING_GEN_WRITE(StrWide)
      HADESMEM_SCRIPTING_GEN_WRITE(Pointer)
    }

    // Lua exception type
    class LuaError : public virtual HadesMemError 
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
      void RunFile(const std::string& Path)
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
      void RunString(const std::string& Script)
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

    class ScriptMgr : public LuaMgr
    {
    public:
      ScriptMgr() 
        : LuaMgr() 
      {
        // Register HadesMem API
        luabind::module(GetState(), "HadesMem")
        [
          // Bind MemoryMgr class
          luabind::class_<MemoryMgr>("MemoryMgr")

          // Bind MemoryMgr::MemoryMgr wrappers
          ,luabind::def("CreateMemoryMgr", static_cast<boost::shared_ptr<
          MemoryMgr> (*)(DWORD)>(&Wrappers::CreateMemoryMgr)) 
          ,luabind::def("CreateMemoryMgr", static_cast<boost::shared_ptr<
          MemoryMgr> (*)(std::string const&)>(&Wrappers::CreateMemoryMgr)) 
          ,luabind::def("CreateMemoryMgr", static_cast<boost::shared_ptr<
          MemoryMgr> (*)(std::string const& WindowName, std::string const* 
          const ClassName)>(&Wrappers::CreateMemoryMgr)) 

          // Bind MemoryMgr::Read<T> wrappers
          ,luabind::def("ReadByte", &Wrappers::ReadByte) 
          ,luabind::def("ReadInt16", &Wrappers::ReadInt16) 
          ,luabind::def("ReadUInt16", &Wrappers::ReadUInt16)
          ,luabind::def("ReadInt32", &Wrappers::ReadInt32)
          ,luabind::def("ReadUInt32", &Wrappers::ReadUInt32)
          ,luabind::def("ReadInt64", &Wrappers::ReadInt64)
          ,luabind::def("ReadUInt64", &Wrappers::ReadUInt64)
          ,luabind::def("ReadFloat", &Wrappers::ReadFloat)
          ,luabind::def("ReadDouble", &Wrappers::ReadDouble)
          ,luabind::def("ReadCharNarrow", &Wrappers::ReadCharNarrow)
          ,luabind::def("ReadCharWide", &Wrappers::ReadCharWide)
          ,luabind::def("ReadStrNarrow", &Wrappers::ReadStrNarrow)
          ,luabind::def("ReadStrWide", &Wrappers::ReadStrWide)
          ,luabind::def("ReadPointer", &Wrappers::ReadPointer)

          // Bind MemoryMgr::Write<T> wrappers
          ,luabind::def("WriteByte", &Wrappers::WriteByte) 
          ,luabind::def("WriteInt16", &Wrappers::WriteInt16) 
          ,luabind::def("WriteUInt16", &Wrappers::WriteUInt16)
          ,luabind::def("WriteInt32", &Wrappers::WriteInt32)
          ,luabind::def("WriteUInt32", &Wrappers::WriteUInt32)
          ,luabind::def("WriteInt64", &Wrappers::WriteInt64)
          ,luabind::def("WriteUInt64", &Wrappers::WriteUInt64)
          ,luabind::def("WriteFloat", &Wrappers::WriteFloat)
          ,luabind::def("WriteDouble", &Wrappers::WriteDouble)
          ,luabind::def("WriteCharNarrow", &Wrappers::WriteCharNarrow)
          ,luabind::def("WriteCharWide", &Wrappers::WriteCharWide)
          ,luabind::def("WriteStrNarrow", &Wrappers::WriteStrNarrow)
          ,luabind::def("WriteStrWide", &Wrappers::WriteStrWide)
          ,luabind::def("WritePointer", &Wrappers::WritePointer)

          // Bind console output wrapper
          ,luabind::def("WriteLn", &Wrappers::WriteLn)
        ];
      }
    };
  }
}
