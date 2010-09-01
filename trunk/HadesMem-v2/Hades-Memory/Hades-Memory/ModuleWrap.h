/*
This file is part of HadesMem.
Copyright © 2010 Cypherjb (aka Chazwazza, aka Cypher). 
<http://www.cypherjb.com/> <cypher.jb@gmail.com>

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

// Boost
#pragma warning(push, 1)
#pragma warning(disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#pragma warning(pop)

// Windows API
#include <Windows.h>

// HadesMem
#include "Module.h"
#include "MemoryMgr.h"
#include "Hades-Common/I18n.h"

namespace Hades
{
  namespace Memory
  {
    namespace Wrappers
    {
      // Module list wrapper
      struct ModuleList
      {
        std::vector<boost::shared_ptr<Module>> List;
      };

      // GetModuleList wrapper
      inline ModuleList Module_GetModuleList(MemoryMgr const& MyMemory)
      {
        ModuleList MyModuleList = { GetModuleList(MyMemory) };
        return MyModuleList;
      }

      class ModuleWrappers : public Module
      {
      public:
        ModuleWrappers(MemoryMgr const& MyMemory, DWORD_PTR Handle) 
          : Module(MyMemory, reinterpret_cast<HMODULE>(Handle))
        { }

        ModuleWrappers(MemoryMgr const& MyMemory, 
          std::string const& ModuleName) 
          : Module(MyMemory, boost::lexical_cast<std::wstring>(ModuleName))
        { }

        // Module::GetBase wrapper
        DWORD_PTR GetBase()
        {
          return reinterpret_cast<DWORD_PTR>(Module::GetBase());
        }

        // Module::GetName wrapper
        std::string GetName()
        {
          return boost::lexical_cast<std::string>(Module::GetName());
        }

        // Module::GetPath wrapper
        std::string GetPath()
        {
          return boost::lexical_cast<std::string>(Module::GetPath());
        }
      };
    }
  }
}
