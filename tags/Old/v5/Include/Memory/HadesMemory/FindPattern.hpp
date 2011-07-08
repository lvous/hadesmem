/*
This file is part of HadesMem.
Copyright (C) 2011 Joshua Boyce (a.k.a. RaptorFactor).
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

// Hades
#include <HadesMemory/Fwd.hpp>
#include <HadesMemory/Error.hpp>
#include <HadesMemory/MemoryMgr.hpp>

// C++ Standard Library
#include <map>
#include <string>
#include <vector>
#include <utility>

// Boost
#include <boost/filesystem.hpp>

// Windows API
#include <Windows.h>

namespace Hades
{
  namespace Memory
  {
    // Pattern finding class
    class FindPattern
    {
    public:
      // FindPattern exception type
      class Error : public virtual HadesMemError 
      { };

      // Constructor
      explicit FindPattern(MemoryMgr const& MyMemory);
      FindPattern(MemoryMgr const& MyMemory, HMODULE Module);

      // Find pattern
      PVOID Find(std::wstring const& Data, std::wstring const& Mask) const;

      // Load patterns from XML file
      void LoadFromXML(boost::filesystem::path const& Path);

      // Get address map
      std::map<std::wstring, PVOID> GetAddresses() const;

      // Operator[] overload to allow retrieving addresses by name
      PVOID operator[](std::wstring const& Name) const;

    private:
      // Search memory
      PVOID Find(std::vector<std::pair<BYTE, bool>> const& Data) const;

      // Memory manager instance
      MemoryMgr m_Memory;
      
      // Target module base
      DWORD_PTR m_Base;

      // Start and end addresses of search region
      PBYTE m_Start;
      PBYTE m_End;

      // Map to hold addresses
      std::map<std::wstring, PVOID> m_Addresses;
    };
  }
}