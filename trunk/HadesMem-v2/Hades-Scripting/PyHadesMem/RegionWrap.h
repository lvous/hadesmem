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
#include "Hades-Memory/Region.h"

// Export Region API
inline void ExportRegion()
{
  boost::python::class_<Hades::Memory::Region>("Region", boost::python::init<
    Hades::Memory::MemoryMgr&, PVOID>())
//     .def("GetBase", &Hades::Memory::Region::GetBase)
//     .def("GetAllocBase", &Hades::Memory::Region::GetAllocBase)
    .def("GetAllocProtect", &Hades::Memory::Region::GetAllocProtect)
    .def("GetSize", &Hades::Memory::Region::GetSize)
    .def("GetState", &Hades::Memory::Region::GetState)
    .def("GetProtect", &Hades::Memory::Region::GetProtect)
    .def("GetType", &Hades::Memory::Region::GetType)
    ;
}
