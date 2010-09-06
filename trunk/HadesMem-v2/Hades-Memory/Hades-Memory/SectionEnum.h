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

// Boost
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/iterator/iterator_facade.hpp>
#pragma warning(pop)

// Windows API
#include <Windows.h>

// Hades
#include "Section.h"

namespace Hades
{
  namespace Memory
  {
    // Section enumerator
    class SectionEnum
    {
    public:
      // Constructor
      SectionEnum(PeFile const& MyPeFile) 
        : m_PeFile(MyPeFile), 
        m_Current(0)
      {
        ZeroMemory(&m_Current, sizeof(m_Current));
      }

      // Get first region
      boost::shared_ptr<Section> First() 
      {
        Hades::Memory::NtHeaders MyNtHeaders(m_PeFile);
        WORD NumberOfSections(MyNtHeaders.GetNumberOfSections());

        return NumberOfSections ? boost::make_shared<Section>(m_PeFile, 
          m_Current) : boost::shared_ptr<Section>(static_cast<Section*>(
          nullptr));
      }

      // Get next module
      boost::shared_ptr<Section> Next()
      {
        Hades::Memory::NtHeaders MyNtHeaders(m_PeFile);
        WORD NumberOfSections(MyNtHeaders.GetNumberOfSections());

        ++m_Current;

        return (m_Current < NumberOfSections) ? boost::make_shared<Section>(
          m_PeFile, m_Current) : boost::shared_ptr<Section>(
          static_cast<Section*>(nullptr));
      }

      // Section iterator
      class SectionIter : public boost::iterator_facade<SectionIter, 
        boost::shared_ptr<Section>, boost::incrementable_traversal_tag>
      {
      public:
        // Construtor
        SectionIter(SectionEnum& MySectionEnum) 
          : m_SectionEnum(MySectionEnum)
        {
          m_Current = m_SectionEnum.First();
        }

      private:
        // Compiler cannot generate assignment operator
        SectionIter& operator= (SectionIter const& Rhs)
        {
          m_SectionEnum = Rhs.m_SectionEnum;
          m_Current = Rhs.m_Current;
          return *this;
        }

        // Allow Boost.Iterator access to internals
        friend class boost::iterator_core_access;

        // For Boost.Iterator
        void increment() 
        {
          m_Current = m_SectionEnum.Next();
        }

        // For Boost.Iterator
        boost::shared_ptr<Section>& dereference() const
        {
          return m_Current;
        }

        // Parent
        SectionEnum& m_SectionEnum;

        // Current section
        // Mutable due to 'dereference' being marked as 'const'
        mutable boost::shared_ptr<Section> m_Current;
      };

    private:
      // Disable assignmnet
      SectionEnum& operator= (SectionEnum const&);

      // Memory instance
      PeFile const& m_PeFile;

      // Current section number
      WORD m_Current;
    };
  }
}
