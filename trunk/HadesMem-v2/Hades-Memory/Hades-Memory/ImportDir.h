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
#include <string>

// Boost
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/noncopyable.hpp>
#include <boost/iterator/iterator_facade.hpp>
#pragma warning(pop)

// Hades
#include "Fwd.h"
#include "Error.h"

namespace Hades
{
  namespace Memory
  {
    // Import directory wrapper
    class ImportDir : private boost::noncopyable
    {
    public:
      // ImportDir error type
      class Error : public virtual HadesMemError
      { };

      // Constructor
      explicit ImportDir(PeFile& MyPeFile, PIMAGE_IMPORT_DESCRIPTOR pImpDesc = 
        nullptr);

      // Whether import directory is valid
      bool IsValid() const;

      // Ensure import directory is valid
      void EnsureValid() const;

      // Get import directory base
      PBYTE GetBase() const;

      // Advance to next descriptor
      void Advance() const;

      // Get characteristics
      DWORD GetCharacteristics() const;

      // Get time and date stamp
      DWORD GetTimeDateStamp() const;

      // Get forwarder chain
      DWORD GetForwarderChain() const;

      // Get name (raw)
      DWORD GetNameRaw() const;

      // Get name
      std::string GetName() const;

      // Get first think
      DWORD GetFirstThunk() const;

    private:
      PeFile* m_pPeFile;

      MemoryMgr* m_pMemory;

      mutable PIMAGE_IMPORT_DESCRIPTOR m_pImpDesc;
    };

    // Section enumerator
    class ImportDirEnum : private boost::noncopyable
    {
    public:
      // Constructor
      explicit ImportDirEnum(PeFile& MyPeFile) 
        : m_pPeFile(&MyPeFile), 
        m_pImpDesc(nullptr)
      { }

      // Get first import thunk
      std::unique_ptr<ImportDir> First() 
      {
        ImportDir MyImportDir(*m_pPeFile);
        m_pImpDesc = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(MyImportDir.
          GetBase());
        return MyImportDir.GetCharacteristics() ? 
          std::unique_ptr<ImportDir>(new ImportDir(*m_pPeFile)) 
          : std::unique_ptr<ImportDir>(nullptr);
      }

      // Get next import thunk
      std::unique_ptr<ImportDir> Next()
      {
        ++m_pImpDesc;
        ImportDir MyImportDir(*m_pPeFile, m_pImpDesc);
        return MyImportDir.GetCharacteristics() ? 
          std::unique_ptr<ImportDir>(new ImportDir(*m_pPeFile, m_pImpDesc)) 
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
        // Disable assignment
        ImportDirIter& operator= (ImportDirIter const&);

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
      // Disable assignment
      ImportDirEnum& operator= (ImportDirEnum const&);

      // Memory instance
      PeFile* m_pPeFile;

      // Current thunk pointer
      PIMAGE_IMPORT_DESCRIPTOR m_pImpDesc;
    };

    // Import thunk wrapper
    class ImportThunk : private boost::noncopyable
    {
    public:
      // ImportDir error type
      class Error : public virtual HadesMemError
      { };

      // Constructor
      ImportThunk(PeFile& MyPeFile, PVOID pThunk);

      // Whether thunk is valid
      bool IsValid() const;

      // Advance to next thunk
      void Advance() const;

      // Get address of data
      DWORD_PTR GetAddressOfData() const;

      // Get ordinal (raw)
      DWORD_PTR GetOrdinalRaw() const;
      
      // Whether import is by ordinal
      bool ByOrdinal() const;

      // Get ordinal
      WORD GetOrdinal() const;

      // Get function
      DWORD_PTR GetFunction() const;

      // Get hint
      WORD GetHint() const;

      // Get name
      std::string GetName() const;

      // Set function
      void SetFunction(DWORD_PTR Function) const;

    private:
      PeFile* m_pPeFile;

      MemoryMgr* m_pMemory;

      mutable PIMAGE_THUNK_DATA m_pThunk;

      mutable PBYTE m_pBase;
    };

    // Section enumerator
    class ImportThunkEnum : private boost::noncopyable
    {
    public:
      // Constructor
      ImportThunkEnum(PeFile& MyPeFile, DWORD FirstThunk) 
        : m_pPeFile(&MyPeFile), 
        m_pThunk(reinterpret_cast<PIMAGE_THUNK_DATA>(
          m_pPeFile->RvaToVa(FirstThunk)))
      { }

      // Get first import thunk
      std::unique_ptr<ImportThunk> First() 
      {
        ImportThunk MyImportThunk(*m_pPeFile, m_pThunk);
        return MyImportThunk.IsValid() ? 
          std::unique_ptr<ImportThunk>(new ImportThunk(*m_pPeFile, m_pThunk)) 
          : std::unique_ptr<ImportThunk>(nullptr);
      }

      // Get next import thunk
      std::unique_ptr<ImportThunk> Next()
      {
        ++m_pThunk;
        ImportThunk MyImportThunk(*m_pPeFile, m_pThunk);
        return MyImportThunk.IsValid() ? 
          std::unique_ptr<ImportThunk>(new ImportThunk(*m_pPeFile, m_pThunk)) 
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
        // Disable assignment
        ImportThunkIter& operator= (ImportThunkIter const&);

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
      // Disable assignment
      ImportThunkEnum& operator= (ImportThunkEnum const&);

      // Memory instance
      PeFile* m_pPeFile;

      // Current thunk pointer
      PIMAGE_THUNK_DATA m_pThunk;
    };
  }
}
