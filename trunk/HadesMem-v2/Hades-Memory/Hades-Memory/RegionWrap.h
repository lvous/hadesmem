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
#include <vector>

// Boost
#pragma warning(push, 1)
#pragma warning(disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/shared_ptr.hpp>
#pragma warning(pop)

// Windows API
#include <Windows.h>

// HadesMem
#include "Region.h"
#include "MemoryMgr.h"

namespace Hades
{
  namespace Memory
  {
    namespace Wrappers
    {
      // Module list wrapper
      struct RegionList
      {
        std::vector<boost::shared_ptr<Region>> List;
      };

      // GetRegionList wrapper
      inline RegionList Region_GetRegionList(MemoryMgr const& MyMemory)
      {
        RegionList MyRegionList;
        MyRegionList.List = GetMemoryRegionList(MyMemory);
        return MyRegionList;
      }

      class RegionWrappers : public Region
      {
      public:
        RegionWrappers(MemoryMgr const& MyMemory, DWORD_PTR Address)
          : Region(MyMemory, reinterpret_cast<PVOID>(Address))
        { }

        // Region::GetBase wrapper
        DWORD_PTR GetBaseAddress()
        {
          return reinterpret_cast<DWORD_PTR>(Region::GetBase());
        }

        // Region::GetAllocBase wrapper
        DWORD_PTR GetAllocationBase()
        {
          return reinterpret_cast<DWORD_PTR>(Region::GetAllocBase());
        }
      };
    }
  }
}
