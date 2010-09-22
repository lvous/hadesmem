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
#include <TlHelp32.h>

// C++ Standard Library
#include <string>
#include <vector>
#include <iostream>

// Boost
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/algorithm/string.hpp>
#include <boost/iterator/iterator_facade.hpp>
#pragma warning(pop)

// Hades
#include "MemoryMgr.h"

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
      inline Module(MemoryMgr& MyMemory, 
        MODULEENTRY32 const& ModuleEntry);

      // Find module by handle
      inline Module(MemoryMgr& MyMemory, HMODULE Handle);

      // Find module by name
      inline Module(MemoryMgr& MyMemory, std::wstring const& ModuleName);

      // Get module base
      inline HMODULE GetBase() const;
      // Get module size
      inline DWORD GetSize() const;

      // Get module name
      inline std::wstring GetName() const;
      // Get module path
      inline std::wstring GetPath() const;

    private:
      // Memory instance
      MemoryMgr* m_pMemory;

      // Module base address
      HMODULE m_Base;
      // Module size
      DWORD m_Size;
      // Module name
      std::wstring m_Name;
      // Module path
      std::wstring m_Path;
    };

    // Module enumerator
    class ModuleEnum
    {
    public:
      // Constructor
      ModuleEnum(MemoryMgr& MyMemory) 
        : m_pMemory(&MyMemory), 
        m_Snap(), 
        m_ModuleEntry()
      {
        ZeroMemory(&m_ModuleEntry, sizeof(m_ModuleEntry));
        m_ModuleEntry.dwSize = sizeof(m_ModuleEntry);
      }

      // Get first module
      std::unique_ptr<Module> First() 
      {
        // Grab a new snapshot of the process
        m_Snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, m_pMemory->
          GetProcessID());
        if (m_Snap == INVALID_HANDLE_VALUE)
        {
          DWORD LastError = GetLastError();
          BOOST_THROW_EXCEPTION(Module::Error() << 
            ErrorFunction("ModuleEnum::First") << 
            ErrorString("Could not get module snapshot.") << 
            ErrorCodeWin(LastError));
        }

        // Get first module entry
        if (!Module32First(m_Snap, &m_ModuleEntry))
        {
          DWORD LastError = GetLastError();
          BOOST_THROW_EXCEPTION(Module::Error() << 
            ErrorFunction("ModuleEnum::First") << 
            ErrorString("Could not get module info.") << 
            ErrorCodeWin(LastError));
        }

        return std::unique_ptr<Module>(new Module(*m_pMemory, m_ModuleEntry));
      }

      // Get next module
      std::unique_ptr<Module> Next()
      {
        // Todo: Check GetLastError to ensure EOL and throw an exception 
        // on an actual error.
        return Module32Next(m_Snap, &m_ModuleEntry) ? std::unique_ptr<Module>(
          new Module(*m_pMemory, m_ModuleEntry)) : std::unique_ptr<Module>(
          nullptr);
      }

      // Module iterator
      class ModuleListIter : public boost::iterator_facade<ModuleListIter, 
        std::unique_ptr<Module>, boost::incrementable_traversal_tag>
      {
      public:
        // Constructor
        ModuleListIter(ModuleEnum& MyModuleList) 
          : m_ModuleEnum(MyModuleList)
        {
          m_Current = m_ModuleEnum.First();
        }

      private:
        // Disable assignment
        ModuleListIter& operator= (ModuleListIter const&);

        // Allow Boost.Iterator access to internals
        friend class boost::iterator_core_access;
        
        // For Boost.Iterator
        void increment() 
        {
          m_Current = m_ModuleEnum.Next();
        }

        // For Boost.Iterator
        std::unique_ptr<Module>& dereference() const
        {
          return m_Current;
        }

        // Parent
        ModuleEnum& m_ModuleEnum;

        // Current module
        // Mutable due to 'dereference' being marked as 'const'
        mutable std::unique_ptr<Module> m_Current;
      };

    private:
      // Disable assignment
      ModuleEnum& operator= (ModuleEnum const&);

      // Memory instance
      MemoryMgr* m_pMemory;

      // Toolhelp32 snapshot handle
      Windows::EnsureCloseSnap m_Snap;
      
      // Current module entry
      MODULEENTRY32 m_ModuleEntry;
    };

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
        DWORD LastError = GetLastError();
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
        DWORD LastError = GetLastError();
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
      m_Size(ModuleEntry.dwSize), 
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
