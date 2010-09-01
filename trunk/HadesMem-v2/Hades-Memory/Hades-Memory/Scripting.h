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

// Windows API
#include <Windows.h>

// C++ Standard Library
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>

// Boost
#pragma warning(push, 1)
#pragma warning(disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/format.hpp>
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
#include <Luabind/iterator_policy.hpp>
#include <Luabind/exception_handler.hpp>
#pragma warning(pop)

// Hades
#include "Types.h"
#include "PeFile.h"
#include "DosHeader.h"
#include "ModuleWrap.h"
#include "RegionWrap.h"
#include "ScannerWrap.h"
#include "InjectorWrap.h"
#include "ManualMapWrap.h"
#include "MemoryMgrWrap.h"
#include "DisassemblerWrap.h"
#include "Hades-Common/I18n.h"

// LuaBind extensions for 64-bit number support
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
      return static_cast<unsigned long long>(BOOST_PP_CAT(lua_to, number)
        (L, index));
    }

    void to(lua_State* L, unsigned long long const& value)
    {
      BOOST_PP_CAT(lua_push, number)(L, BOOST_PP_CAT(as_lua_, number)(value));
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
      return static_cast<signed long long>(BOOST_PP_CAT(lua_to, number)
        (L, index));
    }

    void to(lua_State* L, signed long long const& value)
    {
      BOOST_PP_CAT(lua_push, number)(L, BOOST_PP_CAT(as_lua_, number)(value));
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
    namespace Wrappers
    {
      // Console output wrapper
      inline void WriteLn(std::string const& Data)
      {
        std::cout << Data << std::endl;
      }

      // Convert number to hex string
      inline std::string ToHexStr(DWORD_PTR Num)
      {
        std::stringstream MyStream;
        MyStream << "0x" << std::hex << std::setfill('0') << 
          std::setw(sizeof(PVOID) * 2) << std::uppercase << Num;
        return MyStream.str();
      }

      // Whether this is a x64 build of HadesMem
      inline bool IsAMD64()
      {
#if defined(_M_AMD64) 
        return true;
#elif defined(_M_IX86) 
        return false;
#else 
#error "Unsupported architecture."
#endif
      }
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

    class ScriptMgr : public LuaMgr
    {
    public:
      ScriptMgr() 
        : LuaMgr() 
      {
        // Register HadesMem exception handler
        luabind::register_exception_handler<std::exception>(std::bind(
          &ScriptMgr::TranslateException, this, std::placeholders::_1, 
          std::placeholders::_2));

        luabind::module(GetState(), "std")
        [
          luabind::class_<std::vector<DWORD_PTR>>("vector_dwordptr")
          .def("push_back", static_cast<void (std::vector<DWORD_PTR>::*)(
            DWORD_PTR const&)>(&std::vector<DWORD_PTR>::push_back))
          .def(luabind::constructor<>())
          .def(luabind::constructor<std::size_t>())
        ];

        // Register HadesMem API
        luabind::module(GetState(), "HadesMem")
        [
          // Bind console output wrapper
          luabind::def("WriteLn", &Wrappers::WriteLn)

          // Number to hex string converter
          ,luabind::def("ToHexStr", &Wrappers::ToHexStr)

          // AMD64 build checker
          ,luabind::def("IsAMD64", &Wrappers::IsAMD64)

          // Bind MemoryMgr class
          ,luabind::class_<Wrappers::MemoryMgrWrappers>("MemoryMgr")

          // Bind MemoryMgr::MemoryMgr
          .def(luabind::constructor<DWORD>())
          .def(luabind::constructor<std::string>())
          .def(luabind::constructor<std::string, std::string>())

          // Bind calling conventions
          .enum_("CallConv")
          [
            luabind::value("CallConv_CDECL", MemoryMgr::CallConv_CDECL),
            luabind::value("CallConv_STDCALL", MemoryMgr::CallConv_STDCALL),
            luabind::value("CallConv_THISCALL", MemoryMgr::CallConv_THISCALL),
            luabind::value("CallConv_FASTCALL", MemoryMgr::CallConv_FASTCALL),
            luabind::value("CallConv_X64", MemoryMgr::CallConv_X64),
            luabind::value("CallConv_Default", MemoryMgr::CallConv_Default)
          ]

          // Bind MemoryMgr::Call wrapper
          .def("Call", &Wrappers::MemoryMgrWrappers::Call) 

          // Bind MemoryMgr::Read<T> wrappers
          .def("ReadInt8", &Wrappers::MemoryMgrWrappers::ReadInt8) 
          .def("ReadUInt8", &Wrappers::MemoryMgrWrappers::ReadUInt8) 
          .def("ReadInt16", &Wrappers::MemoryMgrWrappers::ReadInt16) 
          .def("ReadUInt16", &Wrappers::MemoryMgrWrappers::ReadUInt16)
          .def("ReadInt32", &Wrappers::MemoryMgrWrappers::ReadInt32)
          .def("ReadUInt32", &Wrappers::MemoryMgrWrappers::ReadUInt32)
          .def("ReadInt64", &Wrappers::MemoryMgrWrappers::ReadInt64)
          .def("ReadUInt64", &Wrappers::MemoryMgrWrappers::ReadUInt64)
          .def("ReadFloat", &Wrappers::MemoryMgrWrappers::ReadFloat)
          .def("ReadDouble", &Wrappers::MemoryMgrWrappers::ReadDouble)
          .def("ReadCharNarrow", &Wrappers::MemoryMgrWrappers::ReadCharNarrow)
          .def("ReadCharWide", &Wrappers::MemoryMgrWrappers::ReadCharWide)
          .def("ReadStrNarrow", &Wrappers::MemoryMgrWrappers::ReadStrNarrow)
          .def("ReadStrWide", &Wrappers::MemoryMgrWrappers::ReadStrWide)
          .def("ReadPointer", &Wrappers::MemoryMgrWrappers::ReadPointer)

          // Bind MemoryMgr::Write<T> wrappers
          .def("WriteInt8", &Wrappers::MemoryMgrWrappers::WriteInt8) 
          .def("WriteUInt8", &Wrappers::MemoryMgrWrappers::WriteUInt8) 
          .def("WriteInt16", &Wrappers::MemoryMgrWrappers::WriteInt16) 
          .def("WriteUInt16", &Wrappers::MemoryMgrWrappers::WriteUInt16)
          .def("WriteInt32", &Wrappers::MemoryMgrWrappers::WriteInt32)
          .def("WriteUInt32", &Wrappers::MemoryMgrWrappers::WriteUInt32)
          .def("WriteInt64", &Wrappers::MemoryMgrWrappers::WriteInt64)
          .def("WriteUInt64", &Wrappers::MemoryMgrWrappers::WriteUInt64)
          .def("WriteFloat", &Wrappers::MemoryMgrWrappers::WriteFloat)
          .def("WriteDouble", &Wrappers::MemoryMgrWrappers::WriteDouble)
          .def("WriteCharNarrow", &Wrappers::MemoryMgrWrappers::WriteCharNarrow)
          .def("WriteCharWide", &Wrappers::MemoryMgrWrappers::WriteCharWide)
          .def("WriteStrNarrow", &Wrappers::MemoryMgrWrappers::WriteStrNarrow)
          .def("WriteStrWide", &Wrappers::MemoryMgrWrappers::WriteStrWide)
          .def("WritePointer", &Wrappers::MemoryMgrWrappers::WritePointer)

          // Bind MemoryMgr::CanRead wrapper
          .def("CanRead", &Wrappers::MemoryMgrWrappers::CanRead)

          // Bind MemoryMgr::CanWrite wrapper
          .def("CanWrite", &Wrappers::MemoryMgrWrappers::CanWrite)

          // Bind MemoryMgr::Alloc wrapper
          .def("Alloc", &Wrappers::MemoryMgrWrappers::Alloc)

          // Bind MemoryMgr::Free wrapper
          .def("Free", &Wrappers::MemoryMgrWrappers::Free)

          // Bind MemoryMgr::GetProcessID wrapper
          .def("GetProcessID", &MemoryMgr::GetProcessID)

          // Bind MemoryMgr::GetProcessHandle wrapper
          .def("GetProcessHandle", &Wrappers::MemoryMgrWrappers::
            GetProcessHandle)

          // Bind MemoryMgr::GetRemoteProcAddress wrappers
          .def("GetRemoteProcAddress", &Wrappers::MemoryMgrWrappers::
            GetRemoteProcAddressByOrdinal)
          .def("GetRemoteProcAddress", &Wrappers::MemoryMgrWrappers::
            GetRemoteProcAddressByName)

          // Bind MemoryMgr::FlushCache wrapper
          .def("FlushCache", &Wrappers::MemoryMgrWrappers::FlushCache)

          // Bind Module class
          ,luabind::class_<Wrappers::ModuleWrappers>("Module")

          // Bind Module::Module wrappers
          .def(luabind::constructor<MemoryMgr const&, DWORD_PTR>())
          .def(luabind::constructor<MemoryMgr const&, std::string>())

          // Bind Module::GetBase wrapper
          .def("GetBase", &Wrappers::ModuleWrappers::GetBase)

          // Bind Module::GetSize wrapper
          .def("GetSize", &Wrappers::ModuleWrappers::GetSize)

          // Bind Module::GetName wrapper
          .def("GetName", &Wrappers::ModuleWrappers::GetName)

          // Bind Module::GetPath wrapper
          .def("GetPath", &Wrappers::ModuleWrappers::GetPath)

          // Bind Module::Found wrapper
          .def("Found", &Wrappers::ModuleWrappers::Found)

          // Bind ModuleList class
          ,luabind::class_<Wrappers::ModuleList>("ModuleList")
          .def(luabind::constructor<>())
          .def_readonly("List", &Wrappers::ModuleList::List, 
            luabind::return_stl_iterator)

          // Bind GetModuleList function
          ,luabind::def("GetModuleList", &Wrappers::Module_GetModuleList)

          // Bind Region class
          ,luabind::class_<Wrappers::RegionWrappers>("Region")

          // Bind Scanner::Scanner
          .def(luabind::constructor<MemoryMgr const&, DWORD_PTR>())

          // Bind Region::GetBase wrapper
          .def("GetBase", &Wrappers::RegionWrappers::GetBaseAddress)

          // Bind Region::GetAllocBase wrapper
          .def("GetAllocBase", &Wrappers::RegionWrappers::GetAllocationBase)

          // Bind Region::GetName wrapper
          .def("GetAllocProtect", &Wrappers::RegionWrappers::GetAllocProtect)

          // Bind Region::GetPath wrapper
          .def("GetSize", &Wrappers::RegionWrappers::GetSize)

          // Bind Region::GetState wrapper
          .def("GetState", &Wrappers::RegionWrappers::GetState)

          // Bind Region::GetProtect wrapper
          .def("GetProtect", &Wrappers::RegionWrappers::GetProtect)

          // Bind Region::GetType wrapper
          .def("GetType", &Wrappers::RegionWrappers::GetType)

          // Bind RegionList class
          ,luabind::class_<Wrappers::RegionList>("RegionList")
          .def(luabind::constructor<>())
          .def_readonly("List", &Wrappers::RegionList::List, 
          luabind::return_stl_iterator)

          // Bind GetRegionList function
          ,luabind::def("GetRegionList", &Wrappers::Region_GetRegionList)

          // Bind Injector class
          ,luabind::class_<Wrappers::InjectorWrappers>("Injector")

          // Bind Injector::Injector
          .def(luabind::constructor<MemoryMgr const&>())

          // Bind Injector::InjectDll wrapper
          .def("InjectDll", &Wrappers::InjectorWrappers::InjectDll)

          // Bind Injector::GetBase wrapper
          .def("CallExport", &Wrappers::InjectorWrappers::CallExport)

          // Bind CreateAndInject wrapper
          ,luabind::def("CreateAndInject", &Wrappers::Injector_CreateAndInject)

          // Bind CreateAndInjectInfo class
          ,luabind::class_<Wrappers::CreateAndInjectInfo>(
          "CreateAndInjectInfo")
          .def(luabind::constructor<>())
          .def_readonly("Memory", &Wrappers::CreateAndInjectInfo::Memory)
          .def_readonly("ModBase", &Wrappers::CreateAndInjectInfo::ModBase)
          .def_readonly("ExportRet", &Wrappers::CreateAndInjectInfo::
            ExportRet)

          // Bind StringList class
          ,luabind::class_<Wrappers::StringList>("StringList")
          .def(luabind::constructor<>())
          .def_readonly("List", &Wrappers::StringList::List, 
            luabind::return_stl_iterator)

          // Bind Disassembler class
          ,luabind::class_<Wrappers::DisassemblerWrappers>("Disassembler")

          // Bind Disassembler::Disassembler
          .def(luabind::constructor<MemoryMgr const&>())

          // Bind Disassembler::DisassembleToStr wrapper
          .def("DisassembleToStr", &Wrappers::DisassemblerWrappers::
            DisassembleToStr)

          // Bind Scanner class
          ,luabind::class_<Wrappers::ScannerWrappers>("Scanner")

          // Bind Scanner::Scanner
          .def(luabind::constructor<MemoryMgr const&>())
          .def(luabind::constructor<MemoryMgr const&, DWORD_PTR>())
          .def(luabind::constructor<MemoryMgr const&, DWORD_PTR, DWORD_PTR>())

          // Bind Scanner::Find<T> wrappers
          .def("FindInt8", &Wrappers::ScannerWrappers::FindInt8) 
          .def("FindUInt8", &Wrappers::ScannerWrappers::FindUInt8) 
          .def("FindInt16", &Wrappers::ScannerWrappers::FindInt16) 
          .def("FindUInt16", &Wrappers::ScannerWrappers::FindUInt16)
          .def("FindInt32", &Wrappers::ScannerWrappers::FindInt32)
          .def("FindUInt32", &Wrappers::ScannerWrappers::FindUInt32)
          .def("FindInt64", &Wrappers::ScannerWrappers::FindInt64)
          .def("FindUInt64", &Wrappers::ScannerWrappers::FindUInt64)
          .def("FindFloat", &Wrappers::ScannerWrappers::FindFloat)
          .def("FindDouble", &Wrappers::ScannerWrappers::FindDouble)
          .def("FindCharNarrow", &Wrappers::ScannerWrappers::FindCharNarrow)
          .def("FindCharWide", &Wrappers::ScannerWrappers::FindCharWide)
          .def("FindStrNarrow", &Wrappers::ScannerWrappers::FindStrNarrow)
          .def("FindStrWide", &Wrappers::ScannerWrappers::FindStrWide)
          .def("FindPointer", &Wrappers::ScannerWrappers::FindPointer)

          // Bind Scanner::FindAll<T> wrappers
          .def("FindAllInt8", &Wrappers::ScannerWrappers::FindAllInt8) 
          .def("FindAllUInt8", &Wrappers::ScannerWrappers::FindAllUInt8) 
          .def("FindAllInt16", &Wrappers::ScannerWrappers::FindAllInt16) 
          .def("FindAllUInt16", &Wrappers::ScannerWrappers::FindAllUInt16)
          .def("FindAllInt32", &Wrappers::ScannerWrappers::FindAllInt32)
          .def("FindAllUInt32", &Wrappers::ScannerWrappers::FindAllUInt32)
          .def("FindAllInt64", &Wrappers::ScannerWrappers::FindAllInt64)
          .def("FindAllUInt64", &Wrappers::ScannerWrappers::FindAllUInt64)
          .def("FindAllFloat", &Wrappers::ScannerWrappers::FindAllFloat)
          .def("FindAllDouble", &Wrappers::ScannerWrappers::FindAllDouble)
          .def("FindAllCharNarrow", &Wrappers::ScannerWrappers::
            FindAllCharNarrow)
          .def("FindAllCharWide", &Wrappers::ScannerWrappers::FindAllCharWide)
          .def("FindAllStrNarrow", &Wrappers::ScannerWrappers::
            FindAllStrNarrow)
          .def("FindAllStrWide", &Wrappers::ScannerWrappers::FindAllStrWide)
          .def("FindAllPointer", &Wrappers::ScannerWrappers::FindAllPointer)

          // Bind Scanner::LoadFromXML wrapper
          .def("LoadFromXML", &Wrappers::ScannerWrappers::LoadFromXML)

          // Bind Scanner::operator[] wrapper
          .def("GetAddress", &Wrappers::ScannerWrappers::GetAddress)

          // Bind DwordPtrList class
          ,luabind::class_<Wrappers::ScannerWrappers::DwordPtrList>(
            "DwordPtrList")
          .def(luabind::constructor<>())
          .def_readonly("List", &Wrappers::ScannerWrappers::DwordPtrList::List, 
            luabind::return_stl_iterator)

          // Bind ManualMap class
          ,luabind::class_<Wrappers::ManualMapWrappers>("ManualMap")

          // Bind ManualMap::ManualMap
          .def(luabind::constructor<MemoryMgr const&>())

          // Bind ManualMap::Map wrapper
          .def("Map", &Wrappers::ManualMapWrappers::Map)

          // Bind PeFile class
          ,luabind::class_<PeFile>("PeFile")

          // Bind PE file types
          .enum_("PeFileType")
          [
            luabind::value("PeFileMem", PeFile::PEFileMem),
            luabind::value("PeFileDisk", PeFile::PEFileDisk)
          ]

          // Bind PeFile::PeFile
          .def(luabind::constructor<MemoryMgr const&, PeFile::PeFileType, 
            DWORD_PTR>())

          // Bind DosHeader class
          ,luabind::class_<DosHeader>("DosHeader")

          // Bind DosHeader::DosHeader
          .def(luabind::constructor<PeFile const&>())

          // Bind DosHeader::IsValid
          .def("IsValid", &DosHeader::IsValid)

          // Bind DosHeader::GetMagic
          .def("GetMagic", &DosHeader::GetMagic)

          // Bind DosHeader::SetMagic
          .def("SetMagic", &DosHeader::SetMagic)
        ];
      }

      void TranslateException(lua_State* L, std::exception const& e)
      {
        lua_pushstring(L, boost::diagnostic_information(e).c_str());
      }
    };
  }
}
