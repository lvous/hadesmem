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

// Hades
#include <HadesMemory/Module.hpp>
#include <HadesMemory/MemoryMgr.hpp>
#include <HadesMemory/PeLib/PeFile.hpp>
#include <HadesMemory/PeLib/Section.hpp>
#include <HadesMemory/PeLib/DosHeader.hpp>
#include <HadesMemory/PeLib/NtHeaders.hpp>

// C++ Standard Library
#include <algorithm>

// Boost
#define BOOST_TEST_MODULE SectionTest
#include <boost/test/unit_test.hpp>

// Section component tests
BOOST_AUTO_TEST_CASE(BOOST_TEST_MODULE)
{
  // Create memory manager for self
  HadesMem::MemoryMgr const MyMemory(GetCurrentProcessId());
    
  // Enumerate module list and run section tests on all modules
  HadesMem::ModuleList Modules(MyMemory);
  std::for_each(Modules.begin(), Modules.end(), 
    [&] (HadesMem::Module const& Mod) 
    {
      // Open module as a memory-based PeFile
      // Todo: Also test FileType_Data
      HadesMem::PeFile const MyPeFile(MyMemory, Mod.GetHandle());
      HadesMem::DosHeader const MyDosHeader(MyPeFile);
      HadesMem::NtHeaders const MyNtHeaders(MyPeFile);
      
      // Enumerate sections for module
      WORD Number = 0;
      HadesMem::SectionList Sections(MyPeFile);
      if (Mod.GetHandle() == GetModuleHandle(NULL))
      {
        BOOST_CHECK(Sections.begin() != Sections.end());
      }
      std::for_each(Sections.begin(), Sections.end(), 
        [&] (HadesMem::Section const& S)
        {
          // Check Section::GetNumber
          BOOST_CHECK_EQUAL(S.GetNumber(), Number);
          ++Number;
          
          // Test Section::Section
          HadesMem::Section Test(MyPeFile, S.GetNumber());
          
          // Get raw section header data
          auto const HdrRaw = MyMemory.Read<IMAGE_SECTION_HEADER>(
            Test.GetBase());
          
          // Ensure all member functions are called without exception, and 
          // overwrite the value of each field with the existing value
          Test.SetName(Test.GetName());
          Test.SetVirtualAddress(Test.GetVirtualAddress());
          Test.SetVirtualSize(Test.GetVirtualSize());
          Test.SetSizeOfRawData(Test.GetSizeOfRawData());
          Test.SetPointerToRawData(Test.GetPointerToRawData());
          Test.SetPointerToRelocations(Test.GetPointerToRelocations());
          Test.SetPointerToLinenumbers(Test.GetPointerToLinenumbers());
          Test.SetNumberOfRelocations(Test.GetNumberOfRelocations());
          Test.SetNumberOfLinenumbers(Test.GetNumberOfLinenumbers());
          Test.SetCharacteristics(Test.GetCharacteristics());
            
          // Get raw section header data again
          auto const HdrRawNew = MyMemory.Read<IMAGE_SECTION_HEADER>(
            Test.GetBase());
          
          // Ensure TlsDir getters/setters 'match' by checking that the data is 
          // unchanged
          BOOST_CHECK_EQUAL(std::memcmp(&HdrRaw, &HdrRawNew, sizeof(
            HdrRaw)), 0);
        });
    });
}
