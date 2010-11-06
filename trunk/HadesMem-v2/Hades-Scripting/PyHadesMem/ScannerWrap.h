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
#include "Hades-Memory/Scanner.h"

// Export Scanner API
inline void ExportScanner()
{
  boost::python::class_<Hades::Memory::Scanner, boost::noncopyable>(
    "Scanner", boost::python::init<Hades::Memory::MemoryMgr&, PVOID, PVOID>())
//     .def("FindInt8", &Hades::Memory::Scanner::Find<Hades::Memory::Types::
//       Int8>)
//     .def("FindUInt8", &Hades::Memory::Scanner::Find<Hades::Memory::Types::
//       UInt8>)
//     .def("FindInt16", &Hades::Memory::Scanner::Find<Hades::Memory::Types::
//       Int16>)
//     .def("FindUInt16", &Hades::Memory::Scanner::Find<Hades::Memory::Types::
//       UInt16>)
//     .def("FindInt32", &Hades::Memory::Scanner::Find<Hades::Memory::Types::
//       Int32>)
//     .def("FindUInt32", &Hades::Memory::Scanner::Find<Hades::Memory::Types::
//       UInt32>)
//     .def("FindInt64", &Hades::Memory::Scanner::Find<Hades::Memory::Types::
//       Int64>)
//     .def("FindUInt64", &Hades::Memory::Scanner::Find<Hades::Memory::Types::
//       UInt64>)
//     .def("FindFloat", &Hades::Memory::Scanner::Find<Hades::Memory::Types::
//       Float>)
//     .def("FindDouble", &Hades::Memory::Scanner::Find<Hades::Memory::Types::
//       Double>)
//     .def("FindCharA", &Hades::Memory::Scanner::Find<Hades::Memory::Types::
//       CharA>)
//     .def("FindCharW", &Hades::Memory::Scanner::Find<Hades::Memory::Types::
//       CharW>)
//     .def("FindStringA", &Hades::Memory::Scanner::Find<Hades::Memory::Types::
//       StringA>)
//     .def("FindStringW", &Hades::Memory::Scanner::Find<Hades::Memory::Types::
//       StringW>)
//     .def("FindPointer", &Hades::Memory::Scanner::Find<Hades::Memory::Types::
//       Pointer>)
//     .def("FindAllInt8", &Hades::Memory::Scanner::FindAll<Hades::Memory::
//       Types::Int8>)
//     .def("FindAllUInt8", &Hades::Memory::Scanner::FindAll<Hades::Memory::
//       Types::UInt8>)
//     .def("FindAllInt16", &Hades::Memory::Scanner::FindAll<Hades::Memory::
//       Types::Int16>)
//     .def("FindAllUInt16", &Hades::Memory::Scanner::FindAll<Hades::Memory::
//       Types::UInt16>)
//     .def("FindAllInt32", &Hades::Memory::Scanner::FindAll<Hades::Memory::
//       Types::Int32>)
//     .def("FindAllUInt32", &Hades::Memory::Scanner::FindAll<Hades::Memory::
//       Types::UInt32>)
//     .def("FindAllInt64", &Hades::Memory::Scanner::FindAll<Hades::Memory::
//       Types::Int64>)
//     .def("FindAllUInt64", &Hades::Memory::Scanner::FindAll<Hades::Memory::
//       Types::UInt64>)
//     .def("FindAllFloat", &Hades::Memory::Scanner::FindAll<Hades::Memory::
//       Types::Float>)
//     .def("FindAllDouble", &Hades::Memory::Scanner::FindAll<Hades::Memory::
//       Types::Double>)
//     .def("FindAllCharA", &Hades::Memory::Scanner::FindAll<Hades::Memory::
//       Types::CharA>)
//     .def("FindAllCharW", &Hades::Memory::Scanner::FindAll<Hades::Memory::
//       Types::CharW>)
//     .def("FindAllStringA", &Hades::Memory::Scanner::FindAll<Hades::Memory::
//       Types::StringA>)
//     .def("FindAllStringW", &Hades::Memory::Scanner::FindAll<Hades::Memory::
//       Types::StringW>)
//     .def("FindAllPointer", &Hades::Memory::Scanner::FindAll<Hades::Memory::
//       Types::Pointer>)
    ;
}
