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

// C++ Standard Library
#include <memory>

// Boost
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/iterator/iterator_facade.hpp>
#pragma warning(pop)

// Windows API
#include <Windows.h>

// Hades
#include "PeFile.h"
#include "Section.h"
#include "NtHeaders.h"

namespace Hades
{
  namespace Memory
  {
    // Section enumerator
    class SectionEnum : private boost::noncopyable
    {
    public:
      // Constructor
      explicit SectionEnum(PeFile& MyPeFile) 
        : m_pPeFile(&MyPeFile), 
        m_Current(0)
      { }

      // Get first section
      std::unique_ptr<Section> First() 
      {
        NtHeaders const MyNtHeaders(*m_pPeFile);
        WORD NumberOfSections = MyNtHeaders.GetNumberOfSections();

        m_Current = 0;

        return NumberOfSections ? std::unique_ptr<Section>(new Section(
          *m_pPeFile, m_Current)) : std::unique_ptr<Section>(nullptr);
      }

      // Get next section
      std::unique_ptr<Section> Next()
      {
        NtHeaders const MyNtHeaders(*m_pPeFile);
        WORD const NumberOfSections = MyNtHeaders.GetNumberOfSections();

        ++m_Current;

        return (m_Current < NumberOfSections) ? std::unique_ptr<Section>(
          new Section(*m_pPeFile, m_Current)) : std::unique_ptr<Section>(
          nullptr);
      }

      // Section iterator
      class SectionIter : public boost::iterator_facade<SectionIter, 
        std::unique_ptr<Section>, boost::incrementable_traversal_tag>,  
        private boost::noncopyable
      {
      public:
        // Constructor
        explicit SectionIter(SectionEnum& MySectionEnum) 
          : m_SectionEnum(MySectionEnum)
        {
          m_Current = m_SectionEnum.First();
        }

      private:
        // Disable assignment
        SectionIter& operator= (SectionIter const&);

        // Allow Boost.Iterator access to internals
        friend class boost::iterator_core_access;

        // For Boost.Iterator
        void increment() 
        {
          m_Current = m_SectionEnum.Next();
        }

        // For Boost.Iterator
        std::unique_ptr<Section>& dereference() const
        {
          return m_Current;
        }

        // Parent
        SectionEnum& m_SectionEnum;

        // Current section
        // Mutable due to 'dereference' being marked as 'const'
        mutable std::unique_ptr<Section> m_Current;
      };

    private:
      // Disable assignment
      SectionEnum& operator= (SectionEnum const&);

      // Memory instance
      PeFile* m_pPeFile;

      // Current section number
      WORD m_Current;
    };
  }
}
