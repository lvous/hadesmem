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
#include "Hades-Memory/TlsDir.h"

// Export TlsDir API
void ExportTlsDir()
{
  boost::python::class_<Hades::Memory::TlsDir>("TlsDir", boost::python::init<
    Hades::Memory::PeFile const&>())
    .def("IsValid", &Hades::Memory::TlsDir::IsValid)
    .def("EnsureValid", &Hades::Memory::TlsDir::EnsureValid)
    .def("GetStartAddressOfRawData", &Hades::Memory::TlsDir::
    GetStartAddressOfRawData)
    .def("GetEndAddressOfRawData", &Hades::Memory::TlsDir::
    GetEndAddressOfRawData)
    .def("GetAddressOfIndex", &Hades::Memory::TlsDir::GetAddressOfIndex)
    .def("GetAddressOfCallBacks", &Hades::Memory::TlsDir::
    GetAddressOfCallBacks)
    .def("GetSizeOfZeroFill", &Hades::Memory::TlsDir::GetSizeOfZeroFill)
    .def("GetCharacteristics", &Hades::Memory::TlsDir::GetCharacteristics)
    .def("GetCallbacks", &Hades::Memory::TlsDir::GetCallbacks)
    //     .def("GetBase", &Hades::Memory::TlsDir::GetBase)
    .def("GetTlsDirRaw", &Hades::Memory::TlsDir::GetTlsDirRaw)
    ;
}
