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
#include "Hades-Memory/ExportDir.h"
#include "Hades-Memory/ExportEnum.h"

// Export ExportDir API
inline void ExportExportDir()
{
  boost::python::class_<Hades::Memory::ExportDir>("ExportDir", 
    boost::python::init<Hades::Memory::PeFile const&>())
    .def("IsValid", &Hades::Memory::ExportDir::IsValid)
    .def("EnsureValid", &Hades::Memory::ExportDir::EnsureValid)
    .def("GetCharacteristics", &Hades::Memory::ExportDir::GetCharacteristics)
    .def("GetTimeDateStamp", &Hades::Memory::ExportDir::GetTimeDateStamp)
    .def("GetMajorVersion", &Hades::Memory::ExportDir::GetMajorVersion)
    .def("GetMinorVersion", &Hades::Memory::ExportDir::GetMinorVersion)
    .def("GetName", &Hades::Memory::ExportDir::GetName)
    .def("GetOrdinalBase", &Hades::Memory::ExportDir::GetOrdinalBase)
    .def("GetNumberOfFunctions", &Hades::Memory::ExportDir::GetNumberOfFunctions)
    .def("GetNumberOfNames", &Hades::Memory::ExportDir::GetNumberOfNames)
    .def("GetAddressOfFunctions", &Hades::Memory::ExportDir::
    GetAddressOfFunctions)
    .def("GetAddressOfNames", &Hades::Memory::ExportDir::GetAddressOfNames)
    .def("GetAddressOfNameOrdinals", &Hades::Memory::ExportDir::
    GetAddressOfNameOrdinals)
//     .def("GetBase", &Hades::Memory::ExportDir::GetBase)
    .def("GetExportDirRaw", &Hades::Memory::ExportDir::GetExportDirRaw)
    ;

  boost::python::class_<Hades::Memory::ExportEnum, boost::noncopyable>(
    "ExportEnum", boost::python::init<Hades::Memory::PeFile const&>())
    .def("First", &Hades::Memory::ExportEnum::First)
    .def("Next", &Hades::Memory::ExportEnum::Next)
    ;
}
