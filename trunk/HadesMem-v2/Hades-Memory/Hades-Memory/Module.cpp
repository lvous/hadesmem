/*
This file is part of HadesMem.
Copyright � 2010 RaptorFactor (aka Cypherjb, Cypher, Chazwazza). 
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

// Boost
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/algorithm/string.hpp>
#pragma warning(pop)

// Hades
#include "Module.h"

namespace Hades
{
  namespace Memory
  {
    // Find module by name
    Module::Module(MemoryMgr& MyMemory, std::wstring const& ModuleName) 
      : m_pMemory(&MyMemory), 
      m_Base(nullptr), 
      m_Size(0), 
      m_Name(), 
      m_Path()
    {
      // Grab a new snapshot of the process
      Windows::EnsureCloseSnap const Snap(CreateToolhelp32Snapshot(
        TH32CS_SNAPMODULE, MyMemory.GetProcessID()));
      if (Snap == INVALID_HANDLE_VALUE)
      {
        DWORD const LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("Module::Module") << 
          ErrorString("Could not get module snapshot.") << 
          ErrorCodeWin(LastError));
      }

      // Convert module name to lowercase
      std::wstring const ModuleNameLower(boost::to_lower_copy(ModuleName));

      // Search for process
      MODULEENTRY32 ModEntry = { sizeof(ModEntry) };
      bool Found = false;
      for (BOOL MoreMods = Module32First(Snap, &ModEntry); MoreMods; 
        MoreMods = Module32Next(Snap, &ModEntry)) 
      {
        Found = (boost::to_lower_copy(std::wstring(ModEntry.szModule)) == 
          ModuleNameLower) || (boost::to_lower_copy(std::wstring(
          ModEntry.szExePath)) == ModuleNameLower);
        if (Found)
        {
          break;
        }
      }

      // Check process was found
      if (!Found)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("Module::Module") << 
          ErrorString("Could not find module."));
      }

      // Get module data
      m_Base = ModEntry.hModule;
      m_Size = ModEntry.modBaseSize;
      m_Name = ModEntry.szModule;
      m_Path = ModEntry.szExePath;
    }

    // Find module by handle
    Module::Module(MemoryMgr& MyMemory, HMODULE Handle) 
      : m_pMemory(&MyMemory), 
      m_Base(nullptr), 
      m_Size(0), 
      m_Name(), 
      m_Path()
    {
      // Grab a new snapshot of the process
      Windows::EnsureCloseSnap const Snap(CreateToolhelp32Snapshot(
        TH32CS_SNAPMODULE, MyMemory.GetProcessID()));
      if (Snap == INVALID_HANDLE_VALUE)
      {
        DWORD const LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("Module::Module") << 
          ErrorString("Could not get module snapshot.") << 
          ErrorCodeWin(LastError));
      }

      // Search for process
      MODULEENTRY32 ModEntry = { sizeof(ModEntry) };
      bool Found = false;
      for (BOOL MoreMods = Module32First(Snap, &ModEntry); MoreMods; 
        MoreMods = Module32Next(Snap, &ModEntry)) 
      {
        Found = (ModEntry.hModule == Handle);
        if (Found)
        {
          break;
        }
      }

      // Check process was found
      if (!Found)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("Module::Module") << 
          ErrorString("Could not find module."));
      }

      // Get module data
      m_Base = ModEntry.hModule;
      m_Size = ModEntry.modBaseSize;
      m_Name = ModEntry.szModule;
      m_Path = ModEntry.szExePath;
    }

    Module::Module(MemoryMgr& MyMemory, MODULEENTRY32 const& ModuleEntry) 
      : m_pMemory(&MyMemory), 
      m_Base(ModuleEntry.hModule), 
      m_Size(ModuleEntry.modBaseSize), 
      m_Name(ModuleEntry.szModule), 
      m_Path(ModuleEntry.szExePath)
    { }

    // Get module base address
    HMODULE Module::GetBase() const
    {
      return m_Base;
    }

    // Get module size
    DWORD Module::GetSize() const
    {
      return m_Size;
    }

    // Get module name
    std::wstring Module::GetName() const
    {
      return m_Name;
    }

    // Get module path
    std::wstring Module::GetPath() const
    {
      return m_Path;
    }
  }
}
