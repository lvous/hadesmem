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

#pragma once

// C++ Standard Library
#include <string>

// Boost
#pragma warning(push, 1)
#pragma warning(disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/lexical_cast.hpp>
#pragma warning(pop)

// Windows API
#include <Windows.h>

// Hades
#include "Hades-Common/I18n.h"
#include "Hades-Memory/ManualMap.h"
#include "Hades-Memory/MemoryMgr.h"

namespace Hades
{
  namespace Memory
  {
    namespace Wrappers
    {
      class ManualMapWrappers : public ManualMap
      {
      public:
        ManualMapWrappers(MemoryMgr& MyMemory) 
          : ManualMap(MyMemory)
        { }

        // Wrapper function for ManualMap::Map
        DWORD_PTR Map(std::string const& Path, std::string const& Export, 
          bool InjectHelper)
        {
          return reinterpret_cast<DWORD_PTR>(ManualMap::Map(
            boost::lexical_cast<std::basic_string<TCHAR>>(Path), Export, 
            InjectHelper));
        }
      };
    }
  }
}
