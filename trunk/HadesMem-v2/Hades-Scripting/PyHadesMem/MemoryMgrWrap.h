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

// Boost
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/python.hpp>
#pragma warning(pop)

// Hades
#include "Hades-Memory/Types.h"
#include "Hades-Memory/MemoryMgr.h"

class MemoryMgrWrap : public Hades::Memory::MemoryMgr
{
public:
  explicit MemoryMgrWrap(DWORD ProcID) 
    : Hades::Memory::MemoryMgr(ProcID)
  { }

  explicit MemoryMgrWrap(std::basic_string<TCHAR> const& ProcName) 
    : Hades::Memory::MemoryMgr(ProcName)
  { }

  explicit MemoryMgrWrap(std::basic_string<TCHAR> const& WindowName, 
    std::basic_string<TCHAR> const& ClassName) 
    : Hades::Memory::MemoryMgr(WindowName, ClassName)
  { }

  DWORD_PTR Call(DWORD_PTR Address, std::vector<DWORD_PTR> const& Args, 
    CallConv MyCallConv) const
  {
    std::vector<PVOID> ArgsNew;
    ArgsNew.reserve(Args.size());
    std::transform(Args.begin(), Args.end(), std::back_inserter(ArgsNew), 
      [] (DWORD_PTR Current) 
    {
      return reinterpret_cast<PVOID>(Current);
    });

    return Hades::Memory::MemoryMgr::Call(reinterpret_cast<PVOID>(Address), 
      ArgsNew, MyCallConv);
  }

  DWORD_PTR Alloc(SIZE_T Size) const
  {
    return reinterpret_cast<DWORD_PTR>(Hades::Memory::MemoryMgr::Alloc(Size));
  }

  DWORD_PTR GetProcessHandle() const
  {
    return reinterpret_cast<DWORD_PTR>(Hades::Memory::MemoryMgr::
      GetProcessHandle());
  }
};

// Export MemoryMgr API
inline void ExportMemoryMgr()
{
  boost::python::scope MemoryMgrScope = boost::python::class_<MemoryMgrWrap>(
    "MemoryMgr", boost::python::init<DWORD>())
    .def(boost::python::init<std::basic_string<TCHAR> const&>())
    .def(boost::python::init<std::basic_string<TCHAR> const&, 
      std::basic_string<TCHAR> const&>())
    .def("Call", &MemoryMgrWrap::Call)
    .def("ReadInt8", &MemoryMgrWrap::Read<Hades::Memory::Types::Int8>)
    .def("ReadUInt8", &MemoryMgrWrap::Read<Hades::Memory::Types::UInt8>)
    .def("ReadInt16", &MemoryMgrWrap::Read<Hades::Memory::Types::Int16>)
    .def("ReadUInt16", &MemoryMgrWrap::Read<Hades::Memory::Types::UInt16>)
    .def("ReadInt32", &MemoryMgrWrap::Read<Hades::Memory::Types::Int32>)
    .def("ReadUInt32", &MemoryMgrWrap::Read<Hades::Memory::Types::UInt32>)
    .def("ReadInt64", &MemoryMgrWrap::Read<Hades::Memory::Types::Int64>)
    .def("ReadUInt64", &MemoryMgrWrap::Read<Hades::Memory::Types::UInt64>)
    .def("ReadFloat", &MemoryMgrWrap::Read<Hades::Memory::Types::Float>)
    .def("ReadDouble", &MemoryMgrWrap::Read<Hades::Memory::Types::Double>)
    .def("ReadCharA", &MemoryMgrWrap::Read<Hades::Memory::Types::CharA>)
    .def("ReadCharW", &MemoryMgrWrap::Read<Hades::Memory::Types::CharW>)
    .def("ReadStringA", &MemoryMgrWrap::Read<Hades::Memory::Types::StringA>)
    .def("ReadStringW", &MemoryMgrWrap::Read<Hades::Memory::Types::StringW>)
//     .def("ReadPointer", &MemoryMgrWrap::Read<Hades::Memory::Types::Pointer>)
    .def("WriteInt8", &MemoryMgrWrap::Write<Hades::Memory::Types::Int8>)
    .def("WriteUInt8", &MemoryMgrWrap::Write<Hades::Memory::Types::UInt8>)
    .def("WriteInt16", &MemoryMgrWrap::Write<Hades::Memory::Types::Int16>)
    .def("WriteUInt16", &MemoryMgrWrap::Write<Hades::Memory::Types::UInt16>)
    .def("WriteInt32", &MemoryMgrWrap::Write<Hades::Memory::Types::Int32>)
    .def("WriteUInt32", &MemoryMgrWrap::Write<Hades::Memory::Types::UInt32>)
    .def("WriteInt64", &MemoryMgrWrap::Write<Hades::Memory::Types::Int64>)
    .def("WriteUInt64", &MemoryMgrWrap::Write<Hades::Memory::Types::UInt64>)
    .def("WriteFloat", &MemoryMgrWrap::Write<Hades::Memory::Types::Float>)
    .def("WriteDouble", &MemoryMgrWrap::Write<Hades::Memory::Types::Double>)
    .def("WriteCharA", &MemoryMgrWrap::Write<Hades::Memory::Types::CharA>)
    .def("WriteCharW", &MemoryMgrWrap::Write<Hades::Memory::Types::CharW>)
    .def("WriteStringA", &MemoryMgrWrap::Write<Hades::Memory::Types::StringA>)
    .def("WriteStringW", &MemoryMgrWrap::Write<Hades::Memory::Types::StringW>)
    .def("WritePointer", &MemoryMgrWrap::Write<Hades::Memory::Types::Pointer>)
    .def("CanRead", &MemoryMgrWrap::CanRead)
    .def("CanWrite", &MemoryMgrWrap::CanWrite)
    .def("IsGuard", &MemoryMgrWrap::IsGuard)
    .def("Alloc", &MemoryMgrWrap::Alloc)
    .def("Free", &MemoryMgrWrap::Free)
    .def("GetProcessID", &MemoryMgrWrap::GetProcessID)
    .def("GetProcessHandle", &MemoryMgrWrap::GetProcessHandle)
//     .def("GetRemoteProcAddress", &MemoryMgrWrap::GetRemoteProcAddress)
//     .def("GetRemoteProcAddress", &MemoryMgrWrap::GetRemoteProcAddress)
    .def("FlushCache", &MemoryMgrWrap::FlushCache)
    ;

  boost::python::enum_<Hades::Memory::MemoryMgr::CallConv>("CallConv")
    .value("CDECL", Hades::Memory::MemoryMgr::CallConv_CDECL)
    .value("Import", Hades::Memory::MemoryMgr::CallConv_STDCALL)
    .value("LoadConfig", Hades::Memory::MemoryMgr::CallConv_THISCALL)
    .value("BoundImport", Hades::Memory::MemoryMgr::CallConv_FASTCALL)
    .value("IAT", Hades::Memory::MemoryMgr::CallConv_X64)
    .value("DelayImport", Hades::Memory::MemoryMgr::CallConv_Default)
    ;
}
