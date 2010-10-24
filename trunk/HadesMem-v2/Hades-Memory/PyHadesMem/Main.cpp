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

// Windows API
#include <crtdbg.h>
#include <Windows.h>

// C++ Standard Library
#include <string>
#include <iostream>

// Boost
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/python.hpp>
#include <boost/exception/all.hpp>
#pragma warning(pop)

// Hades
#include "Hades-Common/Error.h"
#include "Hades-Memory/AutoLink.h"
#include "Hades-Memory/MemoryMgr.h"

// Custom error translator
void HadesErrorTranslator(std::exception const& e)
{
  PyErr_SetString(PyExc_RuntimeError, boost::diagnostic_information(e).
    c_str());
}

// Export MemoryMgr API
void ExportMemoryMgr()
{
  using namespace boost::python;
  class_<Hades::Memory::MemoryMgr, boost::noncopyable>("MemoryMgr", 
    init<DWORD>())
    .def(init<std::basic_string<TCHAR> const&>())
    .def(init<std::basic_string<TCHAR> const&, 
    std::basic_string<TCHAR> const&>())
    ;
}

#if defined(_M_AMD64) 
BOOST_PYTHON_MODULE(PyHadesMem_AMD64)
#elif defined(_M_IX86) 
BOOST_PYTHON_MODULE(PyHadesMem_IA32)
#else 
#error "Unsupported architecture."
#endif
{
  using namespace boost::python;
  register_exception_translator<std::exception>(&HadesErrorTranslator);
  ExportMemoryMgr();
}

BOOL WINAPI DllMain(HINSTANCE /*hinstDLL*/, DWORD /*fdwReason*/, 
  LPVOID /*lpvReserved*/)
{
  // Attempt to detect memory leaks in debug mode
#ifdef _DEBUG
  int CurrentFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  int NewFlags = (_CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF | 
    _CRTDBG_CHECK_ALWAYS_DF);
  _CrtSetDbgFlag(CurrentFlags | NewFlags);
#endif

  return TRUE;
}
