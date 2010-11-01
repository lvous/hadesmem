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

// Boost
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/python.hpp>
#pragma warning(pop)

// Hades
#include "Hades-Memory/Types.h"
#include "Hades-Memory/MemoryMgr.h"

// Export MemoryMgr API
inline void ExportMemoryMgr()
{
  boost::python::scope MemoryMgrScope = boost::python::class_<Hades::Memory::
    MemoryMgr, boost::noncopyable>("MemoryMgr", boost::python::init<DWORD>())
    .def(boost::python::init<std::basic_string<TCHAR> const&>())
    .def(boost::python::init<std::basic_string<TCHAR> const&, 
      std::basic_string<TCHAR> const&>())
    .def("Call", &Hades::Memory::MemoryMgr::Call)
    .def("ReadInt8", &Hades::Memory::MemoryMgr::Read<Hades::Memory::Types::
      Int8>)
    .def("ReadUInt8", &Hades::Memory::MemoryMgr::Read<Hades::Memory::Types::
      UInt8>)
    .def("ReadInt16", &Hades::Memory::MemoryMgr::Read<Hades::Memory::Types::
      Int16>)
    .def("ReadUInt16", &Hades::Memory::MemoryMgr::Read<Hades::Memory::Types::
      UInt16>)
    .def("ReadInt32", &Hades::Memory::MemoryMgr::Read<Hades::Memory::Types::
      Int32>)
    .def("ReadUInt32", &Hades::Memory::MemoryMgr::Read<Hades::Memory::Types::
      UInt32>)
    .def("ReadInt64", &Hades::Memory::MemoryMgr::Read<Hades::Memory::Types::
      Int64>)
    .def("ReadUInt64", &Hades::Memory::MemoryMgr::Read<Hades::Memory::Types::
      UInt64>)
    .def("ReadFloat", &Hades::Memory::MemoryMgr::Read<Hades::Memory::Types::
      Float>)
    .def("ReadDouble", &Hades::Memory::MemoryMgr::Read<Hades::Memory::Types::
      Double>)
    .def("ReadCharA", &Hades::Memory::MemoryMgr::Read<Hades::Memory::Types::
      CharA>)
    .def("ReadCharW", &Hades::Memory::MemoryMgr::Read<Hades::Memory::Types::
      CharW>)
    .def("ReadStringA", &Hades::Memory::MemoryMgr::Read<Hades::Memory::Types::
      StringA>)
    .def("ReadStringW", &Hades::Memory::MemoryMgr::Read<Hades::Memory::Types::
      StringW>)
//     .def("ReadPointer", &Hades::Memory::MemoryMgr::Read<Hades::Memory::
//       Types::Pointer>)
    .def("WriteInt8", &Hades::Memory::MemoryMgr::Write<Hades::Memory::Types::
      Int8>)
    .def("WriteUInt8", &Hades::Memory::MemoryMgr::Write<Hades::Memory::Types::
      UInt8>)
    .def("WriteInt16", &Hades::Memory::MemoryMgr::Write<Hades::Memory::Types::
      Int16>)
    .def("WriteUInt16", &Hades::Memory::MemoryMgr::Write<Hades::Memory::Types::
      UInt16>)
    .def("WriteInt32", &Hades::Memory::MemoryMgr::Write<Hades::Memory::Types::
      Int32>)
    .def("WriteUInt32", &Hades::Memory::MemoryMgr::Write<Hades::Memory::Types::
      UInt32>)
    .def("WriteInt64", &Hades::Memory::MemoryMgr::Write<Hades::Memory::Types::
      Int64>)
    .def("WriteUInt64", &Hades::Memory::MemoryMgr::Write<Hades::Memory::Types::
      UInt64>)
    .def("WriteFloat", &Hades::Memory::MemoryMgr::Write<Hades::Memory::Types::
      Float>)
    .def("WriteDouble", &Hades::Memory::MemoryMgr::Write<Hades::Memory::Types::
      Double>)
    .def("WriteCharA", &Hades::Memory::MemoryMgr::Write<Hades::Memory::Types::
      CharA>)
    .def("WriteCharW", &Hades::Memory::MemoryMgr::Write<Hades::Memory::Types::
      CharW>)
    .def("WriteStringA", &Hades::Memory::MemoryMgr::Write<Hades::Memory::
      Types::StringA>)
    .def("WriteStringW", &Hades::Memory::MemoryMgr::Write<Hades::Memory::
    Types::StringW>)
    .def("WritePointer", &Hades::Memory::MemoryMgr::Write<Hades::Memory::
      Types::Pointer>)
    .def("CanRead", &Hades::Memory::MemoryMgr::CanRead)
    .def("CanWrite", &Hades::Memory::MemoryMgr::CanWrite)
    .def("IsGuard", &Hades::Memory::MemoryMgr::IsGuard)
//     .def("Alloc", &Hades::Memory::MemoryMgr::Alloc)
    .def("Free", &Hades::Memory::MemoryMgr::Free)
    .def("GetProcessID", &Hades::Memory::MemoryMgr::GetProcessID)
//     .def("GetProcessHandle", &Hades::Memory::MemoryMgr::GetProcessHandle)
//     .def("GetRemoteProcAddress", &Hades::Memory::MemoryMgr::
//       GetRemoteProcAddress)
//     .def("GetRemoteProcAddress", &Hades::Memory::MemoryMgr::
//       GetRemoteProcAddress)
    .def("FlushCache", &Hades::Memory::MemoryMgr::FlushCache)
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
