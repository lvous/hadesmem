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
#include "Injector.h"
#include "MemoryMgr.h"
#include "Hades-Common/I18n.h"

namespace Hades
{
  namespace Memory
  {
    namespace Wrappers
    {
      struct CreateAndInjectInfo
      {
        boost::shared_ptr<MemoryMgr> Memory;
        DWORD_PTR ModBase;
        DWORD ExportRet;
      };

      // CreateAndInject wrapper
      inline CreateAndInjectInfo Injector_CreateAndInject(
        std::string const& Path, std::string const& Args, 
        std::string const& Module, std::string const& Export)
      {
        HMODULE ModBase = nullptr;
        DWORD ExportRet = 0;
        auto MyMemory = CreateAndInject(
          boost::lexical_cast<std::wstring>(Path), 
          boost::lexical_cast<std::wstring>(Args), 
          boost::lexical_cast<std::wstring>(Module), 
          Export, 
          &ModBase, 
          &ExportRet);

        CreateAndInjectInfo MyInfo;
        MyInfo.Memory = MyMemory;
        MyInfo.ModBase = reinterpret_cast<DWORD_PTR>(ModBase);
        MyInfo.ExportRet = ExportRet;

        return MyInfo;
      }

      class InjectorWrappers : public Injector
      {
      public:
        explicit InjectorWrappers(MemoryMgr const& MyMemory) 
          : Injector(MyMemory)
        { }

        // Injector::InjectDll wrapper
        DWORD_PTR InjectDll(std::string const& Path, bool PathResolution)
        {
          return reinterpret_cast<DWORD_PTR>(Injector::InjectDll(
            boost::lexical_cast<std::wstring>(Path), PathResolution));
        }

        // Injector::CallExport wrapper
        DWORD CallExport(std::string const& ModulePath, 
          DWORD_PTR ModuleRemote, std::string const& Export)
        {
          return Injector::CallExport(boost::lexical_cast<std::wstring>(
            ModulePath), reinterpret_cast<HMODULE>(ModuleRemote), 
            Export);
        }
      };
    }
  }
}
