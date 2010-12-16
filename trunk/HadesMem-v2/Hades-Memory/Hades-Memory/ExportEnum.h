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
#include <boost/optional.hpp>
#include <boost/iterator/iterator_facade.hpp>
#pragma warning(pop)

// Windows API
#include <Windows.h>

// Hades
#include "ExportDir.h"

namespace Hades
{
  namespace Memory
  {
    // Export enumerator
    class ExportEnum : private boost::noncopyable
    {
    public:
      // Constructor
      explicit ExportEnum(PeFile const& MyPeFile) 
        : m_PeFile(MyPeFile), 
        m_Current()
      { }

      // Get first section
      std::unique_ptr<Export> First() 
      {
        ExportDir const MyExportDir(m_PeFile);
        DWORD NumberOfFunctions = MyExportDir.GetNumberOfFunctions();
        if (NumberOfFunctions)
        {
          m_Current.reset(Export(m_PeFile, MyExportDir.GetOrdinalBase()));
          return std::unique_ptr<Export>(new Export(m_Current.get()));
        }
        else
        {
          return std::unique_ptr<Export>(nullptr);
        }
      }

      // Get next section
      std::unique_ptr<Export> Next()
      {
        ExportDir const MyExportDir(m_PeFile);
        DWORD NumberOfFunctions = MyExportDir.GetNumberOfFunctions();
        DWORD NextOrdinal = m_Current->GetOrdinal() + 1;
        if (NextOrdinal - MyExportDir.GetOrdinalBase() < NumberOfFunctions)
        {
          m_Current.reset(Export(m_PeFile, NextOrdinal));
          return std::unique_ptr<Export>(new Export(m_Current.get()));
        }
        else
        {
          return std::unique_ptr<Export>(nullptr);
        }
      }

      // Section iterator
      class ExportIter : public boost::iterator_facade<ExportIter, 
        std::unique_ptr<Export>, boost::incrementable_traversal_tag>, 
        private boost::noncopyable
      {
      public:
        // Constructor
        explicit ExportIter(ExportEnum& MyExportEnum) 
          : m_ExportEnum(MyExportEnum)
        {
          m_Current = m_ExportEnum.First();
        }

      private:
        // Allow Boost.Iterator access to internals
        friend class boost::iterator_core_access;

        // For Boost.Iterator
        void increment() 
        {
          m_Current = m_ExportEnum.Next();
        }

        // For Boost.Iterator
        std::unique_ptr<Export>& dereference() const
        {
          return m_Current;
        }

        // Parent
        ExportEnum& m_ExportEnum;

        // Current section
        // Mutable due to 'dereference' being marked as 'const'
        mutable std::unique_ptr<Export> m_Current;
      };

    private:
      // Memory instance
      PeFile m_PeFile;

      // Current export
      boost::optional<Export> m_Current;
    };
  }
}
