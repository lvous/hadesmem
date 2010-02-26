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

    // Get region list
    inline std::vector<std::shared_ptr<class Region>> GetRegionList(
      MemoryMgr const& MyMemory);

    // Module wide stream overload
    inline std::wostream& operator<< (std::wostream& Out, 
      class Region const& In);

    // Memory region managing class
    class MemoryRegion
    {
    public:
      // Constructor
      inline explicit MemoryRegion(MemoryMgr const& MyMemory, 
        MEMORY_BASIC_INFORMATION RegionInfo);

    private:
      // Disable assignment
      MemoryRegion& operator= (MemoryRegion const&);

      // MemoryMgr instance
      MemoryMgr const& m_Memory;

      // Region information
      MEMORY_BASIC_INFORMATION m_RegionInfo;
    };

    // Constructor
    MemoryRegion::MemoryRegion(MemoryMgr const& MyMemory, 
      MEMORY_BASIC_INFORMATION RegionInfo) 
      : m_Memory(MyMemory), 
      m_RegionInfo(RegionInfo) 
    { }
  }
}
