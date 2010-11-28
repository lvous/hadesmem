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

class ModuleWrap : public Hades::Memory::Module
{
public:
  ModuleWrap(Hades::Memory::MemoryMgr const& MyMem, DWORD_PTR Handle) 
    : Hades::Memory::Module(MyMem, reinterpret_cast<HMODULE>(Handle))
  { }

  ModuleWrap(Hades::Memory::MemoryMgr const& MyMem, 
    std::basic_string<TCHAR> const& Name) 
    : Hades::Memory::Module(MyMem, Name)
  { }

  DWORD_PTR GetBase() const
  {
    return reinterpret_cast<DWORD_PTR>(Hades::Memory::Module::GetBase());
  }
};

// Export Module API
inline void ExportModule()
{
  boost::python::class_<ModuleWrap>("Module", boost::python::init<
    Hades::Memory::MemoryMgr const&, DWORD_PTR>())
    .def(boost::python::init<Hades::Memory::MemoryMgr const&, 
      std::basic_string<TCHAR> const&>())
    .def("GetBase", &ModuleWrap::GetBase)
    .def("GetSize", &ModuleWrap::GetSize)
    .def("GetName", &ModuleWrap::GetName)
    .def("GetPath", &ModuleWrap::GetPath)
    ;
}
