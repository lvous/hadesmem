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

#pragma once

// Windows API
#include <Windows.h>

// C++ Standard Library
#include <vector>
#include <iostream>

// Boost
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
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
      inline Region(MemoryMgr const& MyMemory, PVOID Address);

      // Constructor
      inline Region(MemoryMgr const& MyMemory, 
        MEMORY_BASIC_INFORMATION const& MyMbi);

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
      // Disable assignment
      Region& operator= (Region const&);

      // MemoryMgr instance
      MemoryMgr const& m_Memory;

      // Region information
      MEMORY_BASIC_INFORMATION m_RegionInfo;
    };

    // Region enumerator
    class RegionEnum
    {
    public:
      // Constructor
      RegionEnum(MemoryMgr const& MyMemory) 
        : m_Memory(MyMemory), 
        m_Address(nullptr), 
        m_Current()
      {
        ZeroMemory(&m_Current, sizeof(m_Current));
      }

      // Get first region
      boost::shared_ptr<Region> First() 
      {
        if (!VirtualQueryEx(m_Memory.GetProcessHandle(), m_Address, &m_Current, 
          sizeof(m_Current)))
        {
          DWORD LastError = GetLastError();
          BOOST_THROW_EXCEPTION(Module::Error() << 
            ErrorFunction("RegionEnum::First") << 
            ErrorString("Could not get first memory region.") << 
            ErrorCodeWin(LastError));
        }

        return boost::make_shared<Region>(m_Memory, m_Current);
      }

      // Get next module
      boost::shared_ptr<Region> Next()
      {
        // Advance to next region
        m_Address = reinterpret_cast<PBYTE>(m_Current.BaseAddress) + 
          m_Current.RegionSize;

        // Get region info
        // Todo: Check GetLastError to ensure EOL and throw an exception 
        // on an actual error.
        return 
          VirtualQueryEx(m_Memory.GetProcessHandle(), m_Address, &m_Current, 
          sizeof(m_Current)) ? boost::make_shared<Region>(m_Memory, m_Current) 
          : boost::shared_ptr<Region>(static_cast<Region*>(nullptr));
      }

      // Region iterator
      class RegionListIter : public boost::iterator_facade<RegionListIter, 
        boost::shared_ptr<Region>, boost::incrementable_traversal_tag>
      {
      public:
        // Construtor
        RegionListIter(RegionEnum& MyRegionEnum) 
          : m_RegionEnum(MyRegionEnum)
        {
          m_Current = m_RegionEnum.First();
        }

      private:
        // Compiler cannot generate assignment operator
        RegionListIter& operator= (RegionListIter const& Rhs)
        {
          m_RegionEnum = Rhs.m_RegionEnum;
          m_Current = Rhs.m_Current;
          return *this;
        }

        // Allow Boost.Iterator access to internals
        friend class boost::iterator_core_access;

        // For Boost.Iterator
        void increment() 
        {
          m_Current = m_RegionEnum.Next();
        }

        // For Boost.Iterator
        boost::shared_ptr<Region>& dereference() const
        {
          return m_Current;
        }

        // Parent
        RegionEnum& m_RegionEnum;

        // Current region
        // Mutable due to 'dereference' being marked as 'const'
        mutable boost::shared_ptr<Region> m_Current;
      };

    private:
      // Disable assignmnet
      RegionEnum& operator= (RegionEnum const&);

      // Memory instance
      MemoryMgr const& m_Memory;

      // Current address
      PVOID m_Address;

      // Current region info
      MEMORY_BASIC_INFORMATION m_Current;
    };

    // Get memory region list
    std::vector<boost::shared_ptr<Region>> GetMemoryRegionList(
      MemoryMgr const& MyMemory)
    {
      // Region list
      std::vector<boost::shared_ptr<Region>> RegionList;

      // Loop over all memory regions
      PBYTE Address = nullptr;
      MEMORY_BASIC_INFORMATION MyMbi = { 0 };
      while (VirtualQueryEx(MyMemory.GetProcessHandle(), Address, &MyMbi, 
        sizeof(MyMbi)))
      {
        // Add current region to list
        RegionList.push_back(boost::make_shared<Region>(MyMemory, 
          MyMbi.BaseAddress));
        // Advance to next region
        Address = reinterpret_cast<PBYTE>(MyMbi.BaseAddress) + 
          MyMbi.RegionSize;
      }

      // Return region list
      return RegionList;
    }

    // Constructor
    Region::Region(MemoryMgr const& MyMemory, PVOID Address) 
      : m_Memory(MyMemory), 
      m_RegionInfo() 
    {
      // Clear region info
      ZeroMemory(&m_RegionInfo, sizeof(m_RegionInfo));

      // Query region info
      if (!VirtualQueryEx(m_Memory.GetProcessHandle(), Address, 
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
    Region::Region(MemoryMgr const& MyMemory, 
      MEMORY_BASIC_INFORMATION const& MyMbi) 
      : m_Memory(MyMemory), 
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
