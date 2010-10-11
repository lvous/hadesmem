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
#include <tchar.h>
#include <Windows.h>
#include <TlHelp32.h>

// C++ Standard Library
#include <string>
#include <memory>

// Boost
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/noncopyable.hpp>
#include <boost/iterator/iterator_facade.hpp>
#pragma warning(pop)

// Hades
#include "Fwd.h"
#include "Error.h"
#include "MemoryMgr.h"
#include "Hades-Common/EnsureCleanup.h"

namespace Hades
{
  namespace Memory
  {
    // Module managing class
    class Module : private boost::noncopyable
    {
    public:
      // Module exception type
      class Error : public virtual HadesMemError
      { };

      // Create module
      Module(MemoryMgr& MyMemory, MODULEENTRY32 const& ModuleEntry);

      // Find module by handle
      Module(MemoryMgr& MyMemory, HMODULE Handle);

      // Find module by name
      Module(MemoryMgr& MyMemory, std::basic_string<TCHAR> const& ModuleName);

      // Get module base
      HMODULE GetBase() const;
      // Get module size
      DWORD GetSize() const;

      // Get module name
      std::basic_string<TCHAR> GetName() const;
      // Get module path
      std::basic_string<TCHAR> GetPath() const;

    private:
      // Memory instance
      MemoryMgr* m_pMemory;

      // Module base address
      HMODULE m_Base;
      // Module size
      DWORD m_Size;
      // Module name
      std::basic_string<TCHAR> m_Name;
      // Module path
      std::basic_string<TCHAR> m_Path;
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
          DWORD const LastError = GetLastError();
          BOOST_THROW_EXCEPTION(Module::Error() << 
            ErrorFunction("ModuleEnum::First") << 
            ErrorString("Could not get module snapshot.") << 
            ErrorCodeWin(LastError));
        }

        // Get first module entry
        if (!Module32First(m_Snap, &m_ModuleEntry))
        {
          DWORD const LastError = GetLastError();
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
        // Fixme: Check GetLastError to ensure EOL and throw an exception 
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
  }
}
