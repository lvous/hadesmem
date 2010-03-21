/*
This file is part of HadesMem.

HadesMem is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

HadesMem is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

// Windows API
#include <Windows.h>

// C++ Standard Library
#include <vector>

// HadesMem
#include "Memory.h"

namespace Hades
{
  namespace Memory
  {
    // MemRegion exception type
    class MemRegionError : public virtual HadesMemError 
    { };

    // Get memory region list
    inline std::vector<std::shared_ptr<class MemoryRegion>> GetRegionList(
      MemoryMgr const& MyMemory);

    // Region wide stream overload
    inline std::wostream& operator<< (std::wostream& Out, 
      class Region const& In);

    // Memory region managing class
    class MemoryRegion
    {
    public:
      // Constructor
      inline explicit MemoryRegion(MemoryMgr const& MyMemory, 
        MEMORY_BASIC_INFORMATION const& RegionInfo);

      // Get base address
      inline PVOID GetBaseAddress() const;
      // Get allocation base
      inline PVOID GetAllocationBase() const;
      // Get allocation protection
      inline DWORD GetAllocationProtect() const;
      // Get size
      inline SIZE_T GetRegionSize() const;
      // Get state
      inline DWORD GetState() const;
      // Get protection
      inline DWORD GetProtect() const;
      // Get type
      inline DWORD GetType() const;

    private:
      // Disable assignment
      MemoryRegion& operator= (MemoryRegion const&);

      // MemoryMgr instance
      MemoryMgr const& m_Memory;

      // Region information
      MEMORY_BASIC_INFORMATION m_RegionInfo;
    };

    // Get memory region list
    std::vector<std::shared_ptr<MemoryRegion>> GetRegionList(
      MemoryMgr const& MyMemory)
    {
      // Region list
      std::vector<std::shared_ptr<MemoryRegion>> RegionList;

      // Loop over all memory regions
      PBYTE Address = nullptr;
      MEMORY_BASIC_INFORMATION MyMbi = { 0 };
      while (VirtualQueryEx(MyMemory.GetProcessHandle(), Address, &MyMbi, 
        sizeof(MyMbi)))
      {
        // Add current region to list
        RegionList.push_back(std::make_shared<MemoryRegion>(MyMemory, MyMbi));
        // Advance to next region
        Address = reinterpret_cast<PBYTE>(MyMbi.BaseAddress) + 
          MyMbi.RegionSize;
      }

      // Return region list
      return RegionList;
    }

    // Region wide stream overload
    std::wostream& operator<< (std::wostream& Out, MemoryRegion const& In)
    {
      Out << "Base Address: " << In.GetBaseAddress() << "." << std::endl;
      Out << "Allocation Base: " << In.GetAllocationBase() << "." << std::endl;
      Out << "Allocation Protect: " << In.GetAllocationProtect() << "." << 
        std::endl;
      Out << "Region Size: " << In.GetRegionSize() << "." << std::endl;
      Out << "State: " << In.GetState() << "." << std::endl;
      Out << "Protect: " << In.GetProtect() << "." << std::endl;
      Out << "Type: " << In.GetType() << "." << std::endl;
      return Out;
    }

    // Constructor
    MemoryRegion::MemoryRegion(MemoryMgr const& MyMemory, 
      MEMORY_BASIC_INFORMATION const& RegionInfo) 
      : m_Memory(MyMemory), 
      m_RegionInfo(RegionInfo) 
    { }

    // Get base address
    PVOID MemoryRegion::GetBaseAddress() const
    {
      return m_RegionInfo.BaseAddress;
    }

    // Get allocation base
    PVOID MemoryRegion::GetAllocationBase() const
    {
      return m_RegionInfo.AllocationBase;
    }

    // Get allocation protection
    DWORD MemoryRegion::GetAllocationProtect() const
    {
      return m_RegionInfo.AllocationProtect;
    }

    // Get size
    SIZE_T MemoryRegion::GetRegionSize() const
    {
      return m_RegionInfo.RegionSize;
    }

    // Get state
    DWORD MemoryRegion::GetState() const
    {
      return m_RegionInfo.State;
    }

    // Get protection
    DWORD MemoryRegion::GetProtect() const
    {
      return m_RegionInfo.Protect;
    }

    // Get type
    DWORD MemoryRegion::GetType() const
    {
      return m_RegionInfo.Type;
    }
  }
}
