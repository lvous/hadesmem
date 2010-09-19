/*
This file is part of HadesMem.
Copyright � 2010 RaptorFactor (aka Cypherjb, Cypher, Chazwazza). 
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
#include <Luabind/tag_function.hpp>
#include <Luabind/iterator_policy.hpp>
#include <Luabind/exception_handler.hpp>
#pragma warning(pop)

// Hades
#include "Memory.h"
#include "PeFileWrap.h"
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
      // Lua exception type
      class Error : public virtual HadesMemError 
      { };

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
          BOOST_THROW_EXCEPTION(Error() << 
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
          // This is potentially bad. See 17.4.4.4/2.
          // Todo: Investigate and fix if required.
          .def("push_back", static_cast<void (std::vector<DWORD_PTR>::*)(
            DWORD_PTR const&)>(&std::vector<DWORD_PTR>::push_back))
          .def(luabind::constructor<>())
          .def(luabind::constructor<std::size_t>())
        ];

        // Register HadesMem API
        luabind::module(GetState(), "HadesMem")
        [
          // Bind RunFile
          luabind::def("RunFile", luabind::tag_function<void (
            std::string const&)>(std::bind(&LuaMgr::RunFile, this, 
            std::placeholders::_1)))

          // Bind console output wrapper
          ,luabind::def("WriteLn", &Wrappers::WriteLn)

          // Number to hex string converter
          ,luabind::def("ToHexStr", &Wrappers::ToHexStr)

          // AMD64 build checker
          ,luabind::def("IsAMD64", &Wrappers::IsAMD64)

          // Bind MemoryMgr class
          ,luabind::class_<MemoryMgr>("MemoryMgrBase")
          ,luabind::class_<Wrappers::MemoryMgrWrappers, MemoryMgr>("MemoryMgr")
          .def(luabind::constructor<DWORD>())
          .def(luabind::constructor<std::string>())
          .def(luabind::constructor<std::string, std::string>())
          .enum_("CallConv")
          [
            luabind::value("CallConv_CDECL", MemoryMgr::CallConv_CDECL),
            luabind::value("CallConv_STDCALL", MemoryMgr::CallConv_STDCALL),
            luabind::value("CallConv_THISCALL", MemoryMgr::CallConv_THISCALL),
            luabind::value("CallConv_FASTCALL", MemoryMgr::CallConv_FASTCALL),
            luabind::value("CallConv_X64", MemoryMgr::CallConv_X64),
            luabind::value("CallConv_Default", MemoryMgr::CallConv_Default)
          ]
          .def("Call", &Wrappers::MemoryMgrWrappers::Call) 
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
          .def("CanRead", &Wrappers::MemoryMgrWrappers::CanRead)
          .def("CanWrite", &Wrappers::MemoryMgrWrappers::CanWrite)
          .def("Alloc", &Wrappers::MemoryMgrWrappers::Alloc)
          .def("Free", &Wrappers::MemoryMgrWrappers::Free)
          .def("GetProcessID", &MemoryMgr::GetProcessID)
          .def("GetProcessHandle", &Wrappers::MemoryMgrWrappers::
            GetProcessHandle)
          .def("GetRemoteProcAddress", &Wrappers::MemoryMgrWrappers::
            GetRemoteProcAddressByOrdinal)
          .def("GetRemoteProcAddress", &Wrappers::MemoryMgrWrappers::
            GetRemoteProcAddressByName)
          .def("FlushCache", &Wrappers::MemoryMgrWrappers::FlushCache)

          // Bind Module class
          ,luabind::class_<Module>("ModuleBase")
          ,luabind::class_<Wrappers::ModuleWrappers, Module>("Module")
          .def(luabind::constructor<MemoryMgr*, DWORD_PTR>())
          .def(luabind::constructor<MemoryMgr*, std::string>())
          .def("GetBase", &Wrappers::ModuleWrappers::GetBase)
          .def("GetSize", &Wrappers::ModuleWrappers::GetSize)
          .def("GetName", &Wrappers::ModuleWrappers::GetName)
          .def("GetPath", &Wrappers::ModuleWrappers::GetPath)

          // Bind ModuleEnum class
          ,luabind::class_<Wrappers::ModuleEnumWrap>("ModuleEnum")
          .def(luabind::constructor<MemoryMgr*>())
          .def("First", &Wrappers::ModuleEnumWrap::First)
          .def("Next", &Wrappers::ModuleEnumWrap::Next)

          // Bind Region class
          ,luabind::class_<Region>("RegionBase")
          ,luabind::class_<Wrappers::RegionWrappers, Region>("Region")
          .def(luabind::constructor<MemoryMgr*, DWORD_PTR>())
          .def("GetBase", &Wrappers::RegionWrappers::GetBaseAddress)
          .def("GetAllocBase", &Wrappers::RegionWrappers::GetAllocationBase)
          .def("GetAllocProtect", &Wrappers::RegionWrappers::GetAllocProtect)
          .def("GetSize", &Wrappers::RegionWrappers::GetSize)
          .def("GetState", &Wrappers::RegionWrappers::GetState)
          .def("GetProtect", &Wrappers::RegionWrappers::GetProtect)
          .def("GetType", &Wrappers::RegionWrappers::GetType)

          // Bind RegionEnum class
          ,luabind::class_<Wrappers::RegionEnumWrap>("RegionEnum")
          .def(luabind::constructor<MemoryMgr*>())
          .def("First", &Wrappers::RegionEnumWrap::First)
          .def("Next", &Wrappers::RegionEnumWrap::Next)

          // Bind Injector class
          ,luabind::class_<Injector>("InjectorBase")
          ,luabind::class_<Wrappers::InjectorWrappers, Injector>("Injector")
          .def(luabind::constructor<MemoryMgr*>())
          .def("InjectDll", &Wrappers::InjectorWrappers::InjectDll)
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
          ,luabind::class_<Disassembler>("DisassemblerBase")
          ,luabind::class_<Wrappers::DisassemblerWrappers, Disassembler>(
            "Disassembler")
          .def(luabind::constructor<MemoryMgr*>())
          .def("DisassembleToStr", &Wrappers::DisassemblerWrappers::
            DisassembleToStr)

          // Bind Scanner class
          ,luabind::class_<Scanner>("ScannerBase")
          ,luabind::class_<Wrappers::ScannerWrappers, Scanner>("Scanner")
          .def(luabind::constructor<MemoryMgr*>())
          .def(luabind::constructor<MemoryMgr*, DWORD_PTR>())
          .def(luabind::constructor<MemoryMgr*, DWORD_PTR, DWORD_PTR>())
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
          .def("LoadFromXML", &Wrappers::ScannerWrappers::LoadFromXML)
          .def("GetAddress", &Wrappers::ScannerWrappers::GetAddress)

          // Bind DwordPtrList class
          ,luabind::class_<Wrappers::ScannerWrappers::DwordPtrList>(
            "DwordPtrList")
          .def(luabind::constructor<>())
          .def_readonly("List", &Wrappers::ScannerWrappers::DwordPtrList::List, 
            luabind::return_stl_iterator)

          // Bind ManualMap class
          ,luabind::class_<ManualMap>("ManualMapBase")
          ,luabind::class_<Wrappers::ManualMapWrappers, ManualMap>("ManualMap")
          .def(luabind::constructor<MemoryMgr*>())
          .def("Map", &Wrappers::ManualMapWrappers::Map)

          // Bind PeFile class
          ,luabind::class_<PeFile>("PeFileBase")
          ,luabind::class_<Wrappers::PeFileWrappers, PeFile>("PeFile")
          .def(luabind::constructor<MemoryMgr*, DWORD_PTR>())

          // Bind PeFileAsData class
          ,luabind::class_<PeFileAsData>("PeFileAsDataBase")
          ,luabind::class_<Wrappers::PeFileAsDataWrappers, PeFileAsData>(
            "PeFileAsData")
          .def(luabind::constructor<MemoryMgr*, DWORD_PTR>())

          // Bind DosHeader class
          ,luabind::class_<DosHeader>("DosHeader")
          .def(luabind::constructor<PeFile*>())
          .def("IsMagicValid", &DosHeader::IsMagicValid)
          .def("EnsureMagicValid", &DosHeader::EnsureMagicValid)
          .def("GetMagic", &DosHeader::GetMagic)
          .def("GetBytesOnLastPage", &DosHeader::GetBytesOnLastPage)
          .def("GetPagesInFile", &DosHeader::GetPagesInFile)
          .def("GetRelocations", &DosHeader::GetRelocations)
          .def("GetSizeOfHeaderInParagraphs", &DosHeader::GetSizeOfHeaderInParagraphs)
          .def("GetMinExtraParagraphs", &DosHeader::GetMinExtraParagraphs)
          .def("GetMaxExtraParagraphs", &DosHeader::GetMaxExtraParagraphs)
          .def("GetInitialSS", &DosHeader::GetInitialSS)
          .def("GetInitialSP", &DosHeader::GetInitialSP)
          .def("GetChecksum", &DosHeader::GetChecksum)
          .def("GetInitialIP", &DosHeader::GetInitialIP)
          .def("GetInitialCS", &DosHeader::GetInitialCS)
          .def("GetRelocTableFileAddr", &DosHeader::GetRelocTableFileAddr)
          .def("GetOverlayNum", &DosHeader::GetOverlayNum)
          .def("GetReservedWords1", &DosHeader::GetReservedWords1)
          .def("GetOEMID", &DosHeader::GetOEMID)
          .def("GetOEMInfo", &DosHeader::GetOEMInfo)
          .def("GetReservedWords2", &DosHeader::GetReservedWords2)
          .def("GetNewHeaderOffset", &DosHeader::GetNewHeaderOffset)
          .def("SetMagic", &DosHeader::SetMagic)
          .def("SetBytesOnLastPage", &DosHeader::SetBytesOnLastPage)
          .def("SetPagesInFile", &DosHeader::SetPagesInFile)
          .def("SetRelocations", &DosHeader::SetRelocations)
          .def("SetSizeOfHeaderInParagraphs", &DosHeader::SetSizeOfHeaderInParagraphs)
          .def("SetMinExtraParagraphs", &DosHeader::SetMinExtraParagraphs)
          .def("SetMaxExtraParagraphs", &DosHeader::SetMaxExtraParagraphs)
          .def("SetInitialSS", &DosHeader::SetInitialSS)
          .def("SetInitialSP", &DosHeader::SetInitialSP)
          .def("SetChecksum", &DosHeader::SetChecksum)
          .def("SetInitialIP", &DosHeader::SetInitialIP)
          .def("SetInitialCS", &DosHeader::SetInitialCS)
          .def("SetRelocTableFileAddr", &DosHeader::SetRelocTableFileAddr)
          .def("SetOverlayNum", &DosHeader::SetOverlayNum)
          .def("SetReservedWords1", &DosHeader::SetReservedWords1)
          .def("SetOEMID", &DosHeader::SetOEMID)
          .def("SetOEMInfo", &DosHeader::SetOEMInfo)
          .def("SetReservedWords2", &DosHeader::SetReservedWords2)
          .def("SetNewHeaderOffset", &DosHeader::SetNewHeaderOffset)

          // Bind NtHeaders class
          ,luabind::class_<NtHeaders>("NtHeaders")
          .enum_("DataDir")
          [
            luabind::value("DataDir_Export", NtHeaders::DataDir_Export),
            luabind::value("DataDir_Import", NtHeaders::DataDir_Import),
            luabind::value("DataDir_Resource", NtHeaders::DataDir_Resource),
            luabind::value("DataDir_Exception", NtHeaders::DataDir_Exception),
            luabind::value("DataDir_Security", NtHeaders::DataDir_Security),
            luabind::value("DataDir_BaseReloc", NtHeaders::DataDir_BaseReloc),
            luabind::value("DataDir_Debug", NtHeaders::DataDir_Debug),
            luabind::value("DataDir_Architecture", NtHeaders::
              DataDir_Architecture),
            luabind::value("DataDir_GlobalPTR", NtHeaders::DataDir_GlobalPTR),
            luabind::value("DataDir_TLS", NtHeaders::DataDir_TLS),
            luabind::value("DataDir_LoadConfig", NtHeaders::
              DataDir_LoadConfig),
            luabind::value("DataDir_BoundImport", NtHeaders::
              DataDir_BoundImport),
            luabind::value("DataDir_IAT", NtHeaders::DataDir_IAT),
            luabind::value("DataDir_DelayImport", NtHeaders::
              DataDir_DelayImport),
            luabind::value("DataDir_COMDescriptor", NtHeaders::
              DataDir_COMDescriptor)
          ]
          .def(luabind::constructor<PeFile*>())
          .def("GetBase", &NtHeaders::GetBase)
          .def("IsSignatureValid", &NtHeaders::IsSignatureValid)
          .def("EnsureSignatureValid", &NtHeaders::EnsureSignatureValid)
          .def("GetSignature", &NtHeaders::GetSignature)
          .def("SetSignature", &NtHeaders::SetSignature)
          .def("GetMachine", &NtHeaders::GetMachine)
          .def("SetMachine", &NtHeaders::SetMachine)
          .def("GetNumberOfSections", &NtHeaders::GetNumberOfSections)
          .def("SetNumberOfSections", &NtHeaders::SetNumberOfSections)
          .def("GetTimeDateStamp", &NtHeaders::GetTimeDateStamp)
          .def("SetTimeDateStamp", &NtHeaders::SetTimeDateStamp)
          .def("GetPointerToSymbolTable", &NtHeaders::GetPointerToSymbolTable)
          .def("SetPointerToSymbolTable", &NtHeaders::SetPointerToSymbolTable)
          .def("GetNumberOfSymbols", &NtHeaders::GetNumberOfSymbols)
          .def("SetNumberOfSymbols", &NtHeaders::SetNumberOfSymbols)
          .def("GetSizeOfOptionalHeader", &NtHeaders::GetSizeOfOptionalHeader)
          .def("SetSizeOfOptionalHeader", &NtHeaders::SetSizeOfOptionalHeader)
          .def("GetCharacteristics", &NtHeaders::GetCharacteristics)
          .def("SetCharacteristics", &NtHeaders::SetCharacteristics)
          .def("GetMagic", &NtHeaders::GetMagic)
          .def("SetMagic", &NtHeaders::SetMagic)
          .def("GetMajorLinkerVersion", &NtHeaders::GetMajorLinkerVersion)
          .def("SetMajorLinkerVersion", &NtHeaders::SetMajorLinkerVersion)
          .def("GetMinorLinkerVersion", &NtHeaders::GetMinorLinkerVersion)
          .def("SetMinorLinkerVersion", &NtHeaders::SetMinorLinkerVersion)
          .def("GetSizeOfCode", &NtHeaders::GetSizeOfCode)
          .def("SetSizeOfCode", &NtHeaders::SetSizeOfCode)
          .def("GetSizeOfInitializedData", &NtHeaders::
            GetSizeOfInitializedData)
          .def("SetSizeOfInitializedData", &NtHeaders::
            SetSizeOfInitializedData)
          .def("GetSizeOfUninitializedData", &NtHeaders::
            GetSizeOfUninitializedData)
          .def("SetSizeOfUninitializedData", &NtHeaders::
            SetSizeOfUninitializedData)
          .def("GetAddressOfEntryPoint", &NtHeaders::GetAddressOfEntryPoint)
          .def("SetAddressOfEntryPoint", &NtHeaders::SetAddressOfEntryPoint)
          .def("GetBaseOfCode", &NtHeaders::GetBaseOfCode)
          .def("SetBaseOfCode", &NtHeaders::SetBaseOfCode)
#if defined(_M_IX86) 
          .def("GetBaseOfData", &NtHeaders::GetBaseOfData)
          .def("SetBaseOfData", &NtHeaders::SetBaseOfData)
#endif
          .def("GetImageBase", &NtHeaders::GetImageBase)
          .def("SetImageBase", &NtHeaders::SetImageBase)
          .def("GetSectionAlignment", &NtHeaders::GetSectionAlignment)
          .def("SetSectionAlignment", &NtHeaders::SetSectionAlignment)
          .def("GetFileAlignment", &NtHeaders::GetFileAlignment)
          .def("SetFileAlignment", &NtHeaders::SetFileAlignment)
          .def("GetMajorOperatingSystemVersion", &NtHeaders::
            GetMajorOperatingSystemVersion)
          .def("SetMajorOperatingSystemVersion", &NtHeaders::
            SetMajorOperatingSystemVersion)
          .def("GetMinorOperatingSystemVersion", &NtHeaders::
            GetMinorOperatingSystemVersion)
          .def("SetMinorOperatingSystemVersion", &NtHeaders::
            SetMinorOperatingSystemVersion)
          .def("GetMajorImageVersion", &NtHeaders::GetMajorImageVersion)
          .def("SetMajorImageVersion", &NtHeaders::SetMajorImageVersion)
          .def("GetMinorImageVersion", &NtHeaders::GetMinorImageVersion)
          .def("SetMinorImageVersion", &NtHeaders::SetMinorImageVersion)
          .def("GetMajorSubsystemVersion", &NtHeaders::
            GetMajorSubsystemVersion)
          .def("SetMajorSubsystemVersion", &NtHeaders::
            SetMajorSubsystemVersion)
          .def("GetMinorSubsystemVersion", &NtHeaders::GetMinorSubsystemVersion)
          .def("SetMinorSubsystemVersion", &NtHeaders::SetMinorSubsystemVersion)
          .def("GetWin32VersionValue", &NtHeaders::GetWin32VersionValue)
          .def("SetWin32VersionValue", &NtHeaders::SetWin32VersionValue)
          .def("GetSizeOfImage", &NtHeaders::GetSizeOfImage)
          .def("SetSizeOfImage", &NtHeaders::SetSizeOfImage)
          .def("GetSizeOfHeaders", &NtHeaders::GetSizeOfHeaders)
          .def("SetSizeOfHeaders", &NtHeaders::SetSizeOfHeaders)
          .def("GetCheckSum", &NtHeaders::GetCheckSum)
          .def("SetCheckSum", &NtHeaders::SetCheckSum)
          .def("GetSubsystem", &NtHeaders::GetSubsystem)
          .def("SetSubsystem", &NtHeaders::SetSubsystem)
          .def("GetDllCharacteristics", &NtHeaders::GetDllCharacteristics)
          .def("SetDllCharacteristics", &NtHeaders::SetDllCharacteristics)
          .def("GetSizeOfStackReserve", &NtHeaders::GetSizeOfStackReserve)
          .def("SetSizeOfStackReserve", &NtHeaders::SetSizeOfStackReserve)
          .def("GetSizeOfStackCommit", &NtHeaders::GetSizeOfStackCommit)
          .def("SetSizeOfStackCommit", &NtHeaders::SetSizeOfStackCommit)
          .def("GetSizeOfHeapReserve", &NtHeaders::GetSizeOfHeapReserve)
          .def("SetSizeOfHeapReserve", &NtHeaders::SetSizeOfHeapReserve)
          .def("GetSizeOfHeapCommit", &NtHeaders::GetSizeOfHeapCommit)
          .def("SetSizeOfHeapCommit", &NtHeaders::SetSizeOfHeapCommit)
          .def("GetLoaderFlags", &NtHeaders::GetLoaderFlags)
          .def("SetLoaderFlags", &NtHeaders::SetLoaderFlags)
          .def("GetNumberOfRvaAndSizes", &NtHeaders::GetNumberOfRvaAndSizes)
          .def("SetNumberOfRvaAndSizes", &NtHeaders::SetNumberOfRvaAndSizes)
          .def("GetDataDirectoryVirtualAddress", &NtHeaders::
            GetDataDirectoryVirtualAddress)
          .def("SetDataDirectoryVirtualAddress", &NtHeaders::
            SetDataDirectoryVirtualAddress)
          .def("GetDataDirectorySize", &NtHeaders::GetDataDirectorySize)
          .def("SetDataDirectorySize", &NtHeaders::SetDataDirectorySize)
          .def("GetHeadersRaw", &NtHeaders::GetHeadersRaw)

          // Bind Section class
          ,luabind::class_<Section>("Section")
          .def(luabind::constructor<PeFile*, WORD>())
          .def("GetName", &Section::GetName)

          // Bind SectionEnum class
          ,luabind::class_<SectionEnum>("SectionEnum")
          .def(luabind::constructor<PeFile*>())
          .def("First", &SectionEnum::First)
          .def("Next", &SectionEnum::Next)

          // Bind ExportDir class
          ,luabind::class_<ExportDir>("ExportDir")
          .def(luabind::constructor<PeFile*>())
          .def("GetName", &ExportDir::GetName)
        ];
      }

      // Custom LuaBind exception filter
      void TranslateException(lua_State* L, std::exception const& e)
      {
        // Get debug information
        lua_Debug d;
        lua_getstack(L, 1, &d);
        lua_getinfo(L, "Sln", &d);

        // Error message
        std::stringstream ErrorMsg;

        // Add file and line name to error message
        ErrorMsg << d.short_src << ":" << d.currentline;

        // Add 'function' name/type to error message
        if (d.name != 0)
        {
          ErrorMsg << "(" << d.namewhat << " " << d.name << ")";
        }

        // Add exception data to error message
        ErrorMsg << "\n" << boost::diagnostic_information(e);

        // Push error message onto lua stack
        lua_pushstring(L, ErrorMsg.str().c_str());
      }
    };
  }
}
