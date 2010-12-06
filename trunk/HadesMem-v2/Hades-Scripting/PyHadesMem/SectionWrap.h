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
#include "Hades-Memory/Section.h"
#include "Hades-Memory/SectionEnum.h"

// Export Section API
inline void ExportSection()
{
  boost::python::class_<Hades::Memory::Section>("Section", boost::python::init<
    Hades::Memory::PeFile const&, WORD>())
    .def("GetName", &Hades::Memory::Section::GetName)
    .def("GetVirtualAddress", &Hades::Memory::Section::GetVirtualAddress)
    .def("GetVirtualSize", &Hades::Memory::Section::GetVirtualSize)
    .def("GetSizeOfRawData", &Hades::Memory::Section::GetSizeOfRawData)
    .def("GetPointerToRawData", &Hades::Memory::Section::GetPointerToRawData)
    .def("GetPointerToRelocations", &Hades::Memory::Section::
      GetPointerToRelocations)
    .def("GetPointerToLinenumbers", &Hades::Memory::Section::
      GetPointerToLinenumbers)
    .def("GetNumberOfRelocations", &Hades::Memory::Section::
      GetNumberOfRelocations)
    .def("GetNumberOfLinenumbers", &Hades::Memory::Section::
      GetNumberOfLinenumbers)
    .def("GetCharacteristics", &Hades::Memory::Section::GetCharacteristics)
//     .def("GetBase", &Hades::Memory::Section::GetBase)
    .def("GetSectionHeaderRaw", &Hades::Memory::Section::GetSectionHeaderRaw)
    ;

  boost::python::class_<Hades::Memory::SectionEnum, boost::noncopyable>(
    "SectionEnum", boost::python::init<Hades::Memory::PeFile const&>())
    .def("First", &Hades::Memory::SectionEnum::First)
    .def("Next", &Hades::Memory::SectionEnum::Next)
    ;
}
