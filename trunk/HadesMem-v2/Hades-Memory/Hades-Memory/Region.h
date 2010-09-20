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
#include <vector>
#include <iostream>

// Boost
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/iterator/iterator_facade.hpp>
#pragma warning(pop)

// HadesMem
#include "MemoryMgr.h"

namespace Hades
{
  namespace Memory
  {
    // Memory region managing class
    class Region
    {
    public:
      // MemRegion exception type
      class Error : public virtual HadesMemError 
      { };

      // Constructor
      inline Region(MemoryMgr* MyMemory, PVOID Address);

      // Constructor
      inline Region(MemoryMgr* MyMemory, MEMORY_BASIC_INFORMATION const& 
        MyMbi);

      // Get base address
      inline PVOID GetBase() const;
      // Get allocation base
      inline PVOID GetAllocBase() const;
      // Get allocation protection
      inline DWORD GetAllocProtect() const;
      // Get size
      inline SIZE_T GetSize() const;
      // Get state
      inline DWORD GetState() const;
      // Get protection
      inline DWORD GetProtect() const;
      // Get type
      inline DWORD GetType() const;

    private:
      // MemoryMgr instance
      MemoryMgr* m_pMemory;

      // Region information
      MEMORY_BASIC_INFORMATION m_RegionInfo;
    };

    // Region enumerator
    class RegionEnum
    {
    public:
      // Constructor
      RegionEnum(MemoryMgr* MyMemory) 
        : m_pMemory(MyMemory), 
        m_Address(nullptr), 
        m_Current()
      {
        ZeroMemory(&m_Current, sizeof(m_Current));
      }

      // Get first region
      std::unique_ptr<Region> First() 
      {
        if (!VirtualQueryEx(m_pMemory->GetProcessHandle(), m_Address, &m_Current, 
          sizeof(m_Current)))
        {
          DWORD LastError = GetLastError();
          BOOST_THROW_EXCEPTION(Module::Error() << 
            ErrorFunction("RegionEnum::First") << 
            ErrorString("Could not get first memory region.") << 
            ErrorCodeWin(LastError));
        }

        return std::unique_ptr<Region>(new Region(m_pMemory, m_Current));
      }

      // Get next region
      std::unique_ptr<Region> Next()
      {
        // Advance to next region
        m_Address = reinterpret_cast<PBYTE>(m_Current.BaseAddress) + 
          m_Current.RegionSize;

        // Get region info
        // Todo: Check GetLastError to ensure EOL and throw an exception 
        // on an actual error.
        return VirtualQueryEx(m_pMemory->GetProcessHandle(), m_Address, 
          &m_Current, sizeof(m_Current)) ? std::unique_ptr<Region>(new Region(
          m_pMemory, m_Current)) : std::unique_ptr<Region>(nullptr);
      }

      // Region iterator
      class RegionListIter : public boost::iterator_facade<RegionListIter, 
        std::unique_ptr<Region>, boost::incrementable_traversal_tag>
      {
      public:
        // Constructor
        RegionListIter(RegionEnum& MyRegionEnum) 
          : m_RegionEnum(MyRegionEnum)
        {
          m_Current = m_RegionEnum.First();
        }

      private:
        // Disable assignment
        RegionListIter& operator= (RegionListIter const&);

        // Allow Boost.Iterator access to internals
        friend class boost::iterator_core_access;

        // For Boost.Iterator
        void increment() 
        {
          m_Current = m_RegionEnum.Next();
        }

        // For Boost.Iterator
        std::unique_ptr<Region>& dereference() const
        {
          return m_Current;
        }

        // Parent
        RegionEnum& m_RegionEnum;

        // Current region
        // Mutable due to 'dereference' being marked as 'const'
        mutable std::unique_ptr<Region> m_Current;
      };

    private:
      // Disable assignment
      RegionEnum& operator= (RegionEnum const&);

      // Memory instance
      MemoryMgr* m_pMemory;

      // Current address
      PVOID m_Address;

      // Current region info
      MEMORY_BASIC_INFORMATION m_Current;
    };

    // Constructor
    Region::Region(MemoryMgr* MyMemory, PVOID Address) 
      : m_pMemory(MyMemory), 
      m_RegionInfo() 
    {
      // Clear region info
      ZeroMemory(&m_RegionInfo, sizeof(m_RegionInfo));

      // Query region info
      if (!VirtualQueryEx(m_pMemory->GetProcessHandle(), Address, 
        &m_RegionInfo, sizeof(m_RegionInfo)))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("Region::Region") << 
          ErrorString("Could not query memory region.") << 
          ErrorCodeWin(LastError));
      }
    }

    // Constructor
    Region::Region(MemoryMgr* MyMemory, 
      MEMORY_BASIC_INFORMATION const& MyMbi) 
      : m_pMemory(MyMemory), 
      m_RegionInfo(MyMbi)
    { }

    // Get base address
    PVOID Region::GetBase() const
    {
      return m_RegionInfo.BaseAddress;
    }

    // Get allocation base
    PVOID Region::GetAllocBase() const
    {
      return m_RegionInfo.AllocationBase;
    }

    // Get allocation protection
    DWORD Region::GetAllocProtect() const
    {
      return m_RegionInfo.AllocationProtect;
    }

    // Get size
    SIZE_T Region::GetSize() const
    {
      return m_RegionInfo.RegionSize;
    }

    // Get state
    DWORD Region::GetState() const
    {
      return m_RegionInfo.State;
    }

    // Get protection
    DWORD Region::GetProtect() const
    {
      return m_RegionInfo.Protect;
    }

    // Get type
    DWORD Region::GetType() const
    {
      return m_RegionInfo.Type;
    }
  }
}
