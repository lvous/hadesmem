/*
This file is part of HadesMem.
Copyright (C) 2010 Joshua Boyce (aka RaptorFactor, Cypherjb, Cypher, Chazwazza).
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

// Windows API
#include <Windows.h>
#include <TlHelp32.h>

// Hades
#include "Fwd.hpp"
#include "Error.hpp"
#include "MemoryMgr.hpp"

namespace Hades
{
  namespace Memory
  {
    // Module managing class
    class Module
    {
    public:
      // Module exception type
      class Error : public virtual HadesMemError
      { };

      // Create module
      Module(MemoryMgr const& MyMemory, MODULEENTRY32 const& ModuleEntry);

      // Find module by handle
      Module(MemoryMgr const& MyMemory, HMODULE Handle);

      // Find module by name
      Module(MemoryMgr const& MyMemory, std::wstring const& ModuleName);

      // Get module base
      HMODULE GetBase() const;
      // Get module size
      DWORD GetSize() const;

      // Get module name
      std::wstring GetName() const;
      // Get module path
      std::wstring GetPath() const;

    private:
      // Memory instance
      MemoryMgr m_Memory;

      // Module base address
      HMODULE m_Base;
      // Module size
      DWORD m_Size;
      // Module name
      std::wstring m_Name;
      // Module path
      std::wstring m_Path;
    };
  }
}