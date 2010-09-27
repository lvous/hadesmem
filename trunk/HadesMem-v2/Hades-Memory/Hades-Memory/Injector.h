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

// C++ Standard Library
#include <memory>
#include <string>
#include <utility>

// Boost
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/filesystem.hpp>
#include <boost/noncopyable.hpp>
#pragma warning(pop)

// Hades
#include "Fwd.h"
#include "Error.h"

// Image base linker 'trick'
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace Hades
{
  namespace Memory
  {
    // DLL injection class
    class Injector : private boost::noncopyable
    {
    public:
      // Injector exception type
      class Error : public virtual HadesMemError 
      { };

      // Constructor
      Injector(MemoryMgr& MyMemory);

      // Inject DLL
      HMODULE InjectDll(boost::filesystem::path const& Path, 
        bool PathResolution = true) const;

      // Call export
      DWORD CallExport(boost::filesystem::path const& ModulePath, 
        HMODULE ModuleRemote, std::string const& Export) const;

    private:
      // MemoryMgr instance
      MemoryMgr* m_pMemory;
    };
    
    // Data returned by CreateAndInject API
    struct CreateAndInjectData : private boost::noncopyable
    {
      CreateAndInjectData() 
        : pMemory(), 
        ModuleBase(nullptr), 
        ExportRet(0)
      { }

      CreateAndInjectData(CreateAndInjectData&& Other)
        : pMemory(), 
        ModuleBase(nullptr), 
        ExportRet(0)
      {
        *this = std::move(Other);
      }

      CreateAndInjectData& operator=(CreateAndInjectData&& Other)
      {
        this->pMemory = std::move(Other.pMemory);

        this->ModuleBase = Other.ModuleBase;
        Other.ModuleBase = nullptr;

        this->ExportRet = Other.ExportRet;
        Other.ExportRet = 0;

        return *this;
      }

      std::unique_ptr<MemoryMgr> pMemory;
      HMODULE ModuleBase;
      DWORD ExportRet;
    };

    // Create process (as suspended) and inject DLL
    CreateAndInjectData CreateAndInject(std::wstring const& Path, 
      std::wstring const& Args, std::wstring const& Module, 
      std::string const& Export);
  }
}
