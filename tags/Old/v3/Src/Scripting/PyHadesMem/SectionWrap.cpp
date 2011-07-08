/*
This file is part of HadesMem.
Copyright (C) 2010 Joshua Boyce (aka RaptorFactor, Cypherjb, Cypher, Chazwazza).
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
#include <boost/python.hpp>
#pragma warning(pop)

// Hades
#include "HadesMemory/Section.hpp"
#include "HadesMemory/SectionEnum.hpp"

class SectionWrap : public Hades::Memory::Section
{
public:
  SectionWrap(Hades::Memory::PeFile const& MyPeFile, WORD Number)
    : Hades::Memory::Section(MyPeFile, Number)
  { }

  DWORD_PTR GetBase() const
  {
    return reinterpret_cast<DWORD_PTR>(Hades::Memory::Section::GetBase());
  }
};

struct SectionIterWrap
{
  static SectionWrap next(Hades::Memory::SectionIter& o)
  {
    if (!*o)
    {
      PyErr_SetString(PyExc_StopIteration, "No more data.");
      boost::python::throw_error_already_set();
    }

    auto MySection(*static_cast<SectionWrap*>(&**o));

    ++o;

    return MySection;
  }

  static boost::python::object pass_through(boost::python::object const& o) 
  {
    return o;
  }

  static void wrap(const char* python_name)
  {
    boost::python::class_<Hades::Memory::SectionIter>(python_name, 
      boost::python::init<Hades::Memory::PeFile const&>())
      .def("next", next)
      .def("__iter__", pass_through)
      ;
  }
};

// Export Section API
void ExportSection()
{
  boost::python::class_<Hades::Memory::Section>("SectionBase", 
    boost::python::no_init)
    ;

  boost::python::class_<SectionWrap, boost::python::bases<
    Hades::Memory::Section>>("Section", boost::python::init<
    Hades::Memory::PeFile const&, WORD>())
    .def("GetName", &Hades::Memory::Section::GetName)
    .def("GetVirtualAddress", &SectionWrap::GetVirtualAddress)
    .def("GetVirtualSize", &SectionWrap::GetVirtualSize)
    .def("GetSizeOfRawData", &SectionWrap::GetSizeOfRawData)
    .def("GetPointerToRawData", &SectionWrap::GetPointerToRawData)
    .def("GetPointerToRelocations", &SectionWrap::GetPointerToRelocations)
    .def("GetPointerToLinenumbers", &SectionWrap::GetPointerToLinenumbers)
    .def("GetNumberOfRelocations", &SectionWrap::GetNumberOfRelocations)
    .def("GetNumberOfLinenumbers", &SectionWrap::GetNumberOfLinenumbers)
    .def("GetCharacteristics", &SectionWrap::GetCharacteristics)
    .def("GetBase", &SectionWrap::GetBase)
    .def("GetSectionHeaderRaw", &SectionWrap::GetSectionHeaderRaw)
    ;

  SectionIterWrap::wrap("SectionIter"); 
}