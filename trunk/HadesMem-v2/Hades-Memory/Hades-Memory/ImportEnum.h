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

// Windows
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
#include "PeFile.h"
#include "ImportDir.h"

namespace Hades
{
  namespace Memory
  {
    // Section enumerator
    class ImportDirEnum : private boost::noncopyable
    {
    public:
      // Constructor
      explicit ImportDirEnum(PeFile const& MyPeFile) 
        : m_PeFile(MyPeFile), 
        m_pImpDesc(nullptr)
      { }

      // Get first import thunk
      std::unique_ptr<ImportDir> First() 
      {
        ImportDir MyImportDir(m_PeFile);
        m_pImpDesc = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(MyImportDir.
          GetBase());
        return MyImportDir.GetCharacteristics() ? 
          std::unique_ptr<ImportDir>(new ImportDir(m_PeFile, m_pImpDesc)) 
          : std::unique_ptr<ImportDir>(nullptr);
      }

      // Get next import thunk
      std::unique_ptr<ImportDir> Next()
      {
        ++m_pImpDesc;
        ImportDir MyImportDir(m_PeFile, m_pImpDesc);
        return MyImportDir.GetCharacteristics() ? 
          std::unique_ptr<ImportDir>(new ImportDir(m_PeFile, m_pImpDesc)) 
          : std::unique_ptr<ImportDir>(nullptr);
      }

      // Section iterator
      class ImportDirIter : public boost::iterator_facade<ImportDirIter, 
        std::unique_ptr<ImportDir>, boost::incrementable_traversal_tag>,  
        private boost::noncopyable
      {
      public:
        // Constructor
        explicit ImportDirIter(ImportDirEnum& MyImportDirEnum) 
          : m_ImportDirEnum(MyImportDirEnum)
        {
          m_Current = m_ImportDirEnum.First();
        }

      private:
        // Allow Boost.Iterator access to internals
        friend class boost::iterator_core_access;

        // For Boost.Iterator
        void increment() 
        {
          m_Current = m_ImportDirEnum.Next();
        }

        // For Boost.Iterator
        std::unique_ptr<ImportDir>& dereference() const
        {
          return m_Current;
        }

        // Parent
        ImportDirEnum& m_ImportDirEnum;

        // Current import dir
        // Mutable due to 'dereference' being marked as 'const'
        mutable std::unique_ptr<ImportDir> m_Current;
      };

    private:
      // Memory instance
      PeFile m_PeFile;

      // Current thunk pointer
      PIMAGE_IMPORT_DESCRIPTOR m_pImpDesc;
    };

    // Section enumerator
    class ImportThunkEnum : private boost::noncopyable
    {
    public:
      // Constructor
      ImportThunkEnum(PeFile const& MyPeFile, DWORD FirstThunk) 
        : m_PeFile(MyPeFile), 
        m_FirstThunk(FirstThunk), 
        m_pThunk(reinterpret_cast<PIMAGE_THUNK_DATA>(m_PeFile.RvaToVa(
          FirstThunk)))
      { }

      // Get first import thunk
      std::unique_ptr<ImportThunk> First() 
      {
        m_pThunk = reinterpret_cast<PIMAGE_THUNK_DATA>(m_PeFile.RvaToVa(
          m_FirstThunk));
        ImportThunk MyImportThunk(m_PeFile, m_pThunk);
        return MyImportThunk.IsValid() ? 
          std::unique_ptr<ImportThunk>(new ImportThunk(m_PeFile, m_pThunk)) 
          : std::unique_ptr<ImportThunk>(nullptr);
      }

      // Get next import thunk
      std::unique_ptr<ImportThunk> Next()
      {
        ++m_pThunk;
        ImportThunk MyImportThunk(m_PeFile, m_pThunk);
        return MyImportThunk.IsValid() ? 
          std::unique_ptr<ImportThunk>(new ImportThunk(m_PeFile, m_pThunk)) 
          : std::unique_ptr<ImportThunk>(nullptr);
      }

      // Section iterator
      class ImportThunkIter : public boost::iterator_facade<ImportThunkIter, 
        std::unique_ptr<ImportThunk>, boost::incrementable_traversal_tag>,  
        private boost::noncopyable
      {
      public:
        // Constructor
        explicit ImportThunkIter(ImportThunkEnum& MyImportThunkEnum) 
          : m_ImportThunkEnum(MyImportThunkEnum)
        {
          m_Current = m_ImportThunkEnum.First();
        }

      private:
        // Allow Boost.Iterator access to internals
        friend class boost::iterator_core_access;

        // For Boost.Iterator
        void increment() 
        {
          m_Current = m_ImportThunkEnum.Next();
        }

        // For Boost.Iterator
        std::unique_ptr<ImportThunk>& dereference() const
        {
          return m_Current;
        }

        // Parent
        ImportThunkEnum& m_ImportThunkEnum;

        // Current import thunk
        // Mutable due to 'dereference' being marked as 'const'
        mutable std::unique_ptr<ImportThunk> m_Current;
      };

    private:
      // Memory instance
      PeFile m_PeFile;

      // Store first thunk RVA so 'First' can be called repeatedly
      DWORD m_FirstThunk;

      // Current thunk pointer
      PIMAGE_THUNK_DATA m_pThunk;
    };
  }
}
