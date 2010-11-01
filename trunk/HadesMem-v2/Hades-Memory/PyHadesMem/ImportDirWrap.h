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
#include "Hades-Memory/ImportDir.h"
#include "Hades-Memory/ImportEnum.h"

// Export ImportDir API
inline void ExportImportDir()
{
  boost::python::class_<Hades::Memory::ImportDir, boost::noncopyable>(
    "ImportDir", boost::python::init<Hades::Memory::PeFile&, 
    PIMAGE_IMPORT_DESCRIPTOR>())
    .def("IsValid", &Hades::Memory::ImportDir::IsValid)
    .def("EnsureValid", &Hades::Memory::ImportDir::EnsureValid)
//     .def("GetBase", &Hades::Memory::ImportDir::GetBase)
    .def("Advance", &Hades::Memory::ImportDir::Advance)
    .def("GetCharacteristics", &Hades::Memory::ImportDir::GetCharacteristics)
    .def("GetTimeDateStamp", &Hades::Memory::ImportDir::GetTimeDateStamp)
    .def("GetForwarderChain", &Hades::Memory::ImportDir::GetForwarderChain)
    .def("GetNameRaw", &Hades::Memory::ImportDir::GetNameRaw)
    .def("GetName", &Hades::Memory::ImportDir::GetName)
    .def("GetFirstThunk", &Hades::Memory::ImportDir::GetFirstThunk)
    ;

  boost::python::class_<Hades::Memory::ImportThunk, boost::noncopyable>(
    "ImportThunk", boost::python::init<Hades::Memory::PeFile&, PVOID>())
    .def("IsValid", &Hades::Memory::ImportThunk::IsValid)
    .def("Advance", &Hades::Memory::ImportThunk::Advance)
    .def("GetAddressOfData", &Hades::Memory::ImportThunk::GetAddressOfData)
    .def("GetOrdinalRaw", &Hades::Memory::ImportThunk::GetOrdinalRaw)
    .def("ByOrdinal", &Hades::Memory::ImportThunk::ByOrdinal)
    .def("GetOrdinal", &Hades::Memory::ImportThunk::GetOrdinal)
    .def("GetFunction", &Hades::Memory::ImportThunk::GetFunction)
    .def("GetHint", &Hades::Memory::ImportThunk::GetHint)
    .def("GetName", &Hades::Memory::ImportThunk::GetName)
    .def("SetFunction", &Hades::Memory::ImportThunk::SetFunction)
    ;

  boost::python::class_<Hades::Memory::ImportDirEnum, boost::noncopyable>(
    "ImportDirEnum", boost::python::init<Hades::Memory::PeFile&>())
    .def("First", &Hades::Memory::ImportDirEnum::First)
    .def("Next", &Hades::Memory::ImportDirEnum::Next)
    ;

  boost::python::class_<Hades::Memory::ImportThunkEnum, boost::noncopyable>(
    "ImportThunkEnum", boost::python::init<Hades::Memory::PeFile&, DWORD>())
    .def("First", &Hades::Memory::ImportThunkEnum::First)
    .def("Next", &Hades::Memory::ImportThunkEnum::Next)
    ;
}
