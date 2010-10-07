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

// Windows API
#include <Windows.h>

// Hades
#include "Hades-Memory/PeFile.h"
#include "Hades-Memory/MemoryMgr.h"

namespace Hades
{
  namespace Memory
  {
    namespace Wrappers
    {
      class PeFileWrappers : public PeFile
      {
      public:
        PeFileWrappers(MemoryMgr& MyMemory, DWORD_PTR Address) 
          : PeFile(MyMemory, reinterpret_cast<PVOID>(Address))
        { }
      };

      class PeFileAsDataWrappers : public PeFileAsData
      {
      public:
        PeFileAsDataWrappers(MemoryMgr& MyMemory, DWORD_PTR Address) 
          : PeFileAsData(MyMemory, reinterpret_cast<PVOID>(Address))
        { }
      };
    }
  }
}
