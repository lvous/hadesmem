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
#include "Hades-Memory/Injector.h"

class InjectorWrap : public Hades::Memory::Injector
{
public:
  explicit InjectorWrap(Hades::Memory::MemoryMgr const& MyMem) 
    : Hades::Memory::Injector(MyMem)
  { }

  DWORD_PTR InjectDll(std::basic_string<TCHAR> const& Path, 
    bool PathResolution) const
  {
    return reinterpret_cast<DWORD_PTR>(Hades::Memory::Injector::InjectDll(Path, 
      PathResolution));
  }

  DWORD_PTR CallExport(std::basic_string<TCHAR> const& Path, 
    DWORD_PTR ModuleRemote, std::string const& Export) const
  {
    return Hades::Memory::Injector::CallExport(Path, reinterpret_cast<HMODULE>(
      ModuleRemote), Export);
  }
};

// Export Injector API
void ExportInjector()
{
  boost::python::class_<Hades::Memory::Injector>("InjectorBase", 
    boost::python::no_init)
    ;

  boost::python::class_<InjectorWrap, boost::python::bases<Hades::Memory::
    Injector>>("Injector", boost::python::init<
    Hades::Memory::MemoryMgr const&>())
    .def("InjectDll", &InjectorWrap::InjectDll)
    .def("CallExport", &InjectorWrap::CallExport)
    ;
}
