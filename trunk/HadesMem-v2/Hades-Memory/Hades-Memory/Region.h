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

namespace Hades
{
  namespace Memory
  {
    // Memory region managing class
    class Region : private boost::noncopyable
    {
    public:
      // MemRegion exception type
      class Error : public virtual HadesMemError 
      { };

      // Constructor
      Region(MemoryMgr& MyMemory, PVOID Address);

      // Constructor
      Region(MemoryMgr& MyMemory, MEMORY_BASIC_INFORMATION const& MyMbi);

      // Get base address
      PVOID GetBase() const;
      
      // Get allocation base
      PVOID GetAllocBase() const;
      
      // Get allocation protection
      DWORD GetAllocProtect() const;
      
      // Get size
      SIZE_T GetSize() const;
      
      // Get state
      DWORD GetState() const;
      
      // Get protection
      DWORD GetProtect() const;
      
      // Get type
      DWORD GetType() const;

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
      RegionEnum(MemoryMgr& MyMemory) 
        : m_pMemory(&MyMemory), 
        m_Address(nullptr), 
        m_Current()
      {
        ZeroMemory(&m_Current, sizeof(m_Current));
      }

      // Get first region
      std::unique_ptr<Region> First() 
      {
        if (!VirtualQueryEx(m_pMemory->GetProcessHandle(), m_Address, 
          &m_Current, sizeof(m_Current)))
        {
          DWORD const LastError = GetLastError();
          BOOST_THROW_EXCEPTION(Region::Error() << 
            ErrorFunction("RegionEnum::First") << 
            ErrorString("Could not get first memory region.") << 
            ErrorCodeWin(LastError));
        }

        return std::unique_ptr<Region>(new Region(*m_pMemory, m_Current));
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
          *m_pMemory, m_Current)) : std::unique_ptr<Region>(nullptr);
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
  }
}
