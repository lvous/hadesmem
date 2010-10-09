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

// Boost
#pragma warning(push, 1)
#pragma warning(disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#pragma warning(pop)

// Windows API
#include <Windows.h>

// Hades
#include "MemoryMgrWrap.h"
#include "Hades-Common/I18n.h"
#include "Hades-Memory/Injector.h"
#include "Hades-Memory/MemoryMgr.h"

namespace Hades
{
  namespace Memory
  {
    namespace Wrappers
    {
      struct CreateAndInjectInfo
      {
        boost::shared_ptr<Wrappers::MemoryMgrWrappers> Memory;
        DWORD_PTR ModBase;
        DWORD_PTR ExportRet;
      };

      // CreateAndInject wrapper
      inline CreateAndInjectInfo Injector_CreateAndInject(
        std::string const& Path, std::string const& Args, 
        std::string const& Module, std::string const& Export)
      {
        CreateAndInjectData MyData(CreateAndInject(
          boost::lexical_cast<std::wstring>(Path), 
          boost::lexical_cast<std::wstring>(Args), 
          boost::lexical_cast<std::wstring>(Module), 
          Export));

        CreateAndInjectInfo MyInfo;
        // This is dangerous, but I haven't had time to think about the 
        // 'proper' solution yet, so this should work for now, but needs 
        // to be fixed in the future.
        // Todo: Fix this monstrosity.
        MemoryMgr* pMemory = MyData.pMemory.release();
        boost::shared_ptr<Wrappers::MemoryMgrWrappers> pMemoryShared(
          static_cast<Wrappers::MemoryMgrWrappers*>(pMemory));
        MyInfo.Memory = pMemoryShared;
        MyInfo.ModBase = reinterpret_cast<DWORD_PTR>(MyData.ModuleBase);
        MyInfo.ExportRet = MyData.ExportRet;

        return MyInfo;
      }

      class InjectorWrappers : public Injector
      {
      public:
        explicit InjectorWrappers(MemoryMgr& MyMemory) 
          : Injector(MyMemory)
        { }

        // Injector::InjectDll wrapper
        DWORD_PTR InjectDll(std::string const& Path, bool PathResolution)
        {
          return reinterpret_cast<DWORD_PTR>(Injector::InjectDll(
            boost::lexical_cast<std::wstring>(Path), PathResolution));
        }

        // Injector::CallExport wrapper
        DWORD_PTR CallExport(std::string const& ModulePath, 
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
