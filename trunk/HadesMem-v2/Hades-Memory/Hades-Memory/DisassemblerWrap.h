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
#include <vector>

// Windows API
#include <Windows.h>

// HadesMem
#include "MemoryMgr.h"
#include "Disassembler.h"

namespace Hades
{
  namespace Memory
  {
    namespace Wrappers
    {
      // String list wrapper
      struct StringList
      {
        std::vector<std::string> List;
      };

      class DisassemblerWrappers : public Disassembler
      {
      public:
        explicit DisassemblerWrappers(MemoryMgr const& MyMemory) 
          : Disassembler(MyMemory)
        { }

        // Disassembler::DisassembleToStr wrapper
        inline StringList DisassembleToStr(DWORD_PTR Address, 
          DWORD_PTR NumInstructions)
        {
          StringList MyStringList;
          MyStringList.List =  Disassembler::DisassembleToStr(
            reinterpret_cast<PVOID>(Address), NumInstructions);
          return MyStringList;
        }
      };
    }
  }
}
