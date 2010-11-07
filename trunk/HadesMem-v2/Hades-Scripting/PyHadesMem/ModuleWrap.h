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
#include "Hades-Memory/Module.h"

// Export Module API
inline void ExportModule()
{
  boost::python::class_<Hades::Memory::Module>("Module", boost::python::init<
    Hades::Memory::MemoryMgr&, HMODULE>())
    .def(boost::python::init<Hades::Memory::MemoryMgr&, 
    std::basic_string<TCHAR> const&>())
//     .def("GetBase", &Hades::Memory::Module::GetBase)
    .def("GetSize", &Hades::Memory::Module::GetSize)
    .def("GetName", &Hades::Memory::Module::GetName)
    .def("GetPath", &Hades::Memory::Module::GetPath)
    ;
}
