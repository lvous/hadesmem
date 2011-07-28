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
#include <HadesMemory/Detail/Fwd.hpp>
#include <HadesMemory/Detail/Error.hpp>
#include <HadesMemory/MemoryMgr.hpp>
#include <HadesMemory/PeLib/PeFile.hpp>

// C++ Standard Library
#include <string>
#include <utility>
#include <iterator>

// Boost
#include <boost/optional.hpp>
#include <boost/iterator/iterator_facade.hpp>

// Windows
#include <Windows.h>

namespace HadesMem
{
  // PE file export directory
  class ExportDir
  {
  public:
    // ExportDir error class
    class Error : public virtual HadesMemError
    { };

    // Constructor
    explicit ExportDir(PeFile const& MyPeFile);

    // Whether export directory is valid
    bool IsValid() const;

    // Ensure export directory is valid
    void EnsureValid() const;

    // Get characteristics
    DWORD GetCharacteristics() const;

    // Set characteristics
    void SetCharacteristics(DWORD Characteristics) const;

    // Get time date stamp
    DWORD GetTimeDateStamp() const;

    // Set time date stamp
    void SetTimeDateStamp(DWORD TimeDateStamp) const;

    // Get major version
    WORD GetMajorVersion() const;

    // Set major version
    void SetMajorVersion(WORD MajorVersion) const;

    // Get minor version
    WORD GetMinorVersion() const;

    // Set minor version
    void SetMinorVersion(WORD MinorVersion) const;

    // Get module name
    std::string GetName() const;

    // Get ordinal base
    DWORD GetOrdinalBase() const;

    // Set ordinal base
    void SetOrdinalBase(DWORD OrdinalBase) const;

    // Get number of functions
    DWORD GetNumberOfFunctions() const;

    // Set number of functions
    void SetNumberOfFunctions(DWORD NumberOfFunctions) const;

    // Get number of names
    DWORD GetNumberOfNames() const;

    // Set number of names
    void SetNumberOfNames(DWORD NumberOfNames) const;

    // Get address of functions
    DWORD GetAddressOfFunctions() const;

    // Set address of functions
    void SetAddressOfFunctions(DWORD AddressOfFunctions) const;

    // Get address of names
    DWORD GetAddressOfNames() const;

    // Set address of names
    void SetAddressOfNames(DWORD AddressOfNames) const;

    // Get address of name ordinals
    DWORD GetAddressOfNameOrdinals() const;

    // Set address of name ordinals
    void SetAddressOfNameOrdinals(DWORD AddressOfNameOrdinals) const;

    // Get base of export dir
    PBYTE GetBase() const;

    // Get raw export dir
    IMAGE_EXPORT_DIRECTORY GetExportDirRaw() const;

  private:
    // PE file
    PeFile m_PeFile;

    // Memory instance
    MemoryMgr m_Memory;

    // Base of export dir
    mutable PBYTE m_pBase;
  };

  // PE file export data
  class Export
  {
  public:
    // Constructor
    Export(PeFile const& MyPeFile, DWORD Ordinal);

    // Get RVA
    DWORD GetRva() const;

    // Get VA
    PVOID GetVa() const;

    // Get name
    std::string GetName() const;

    // Get forwarder
    std::string GetForwarder() const;
    
    // Get forwarder module name
    std::string GetForwarderModule() const;
    
    // Get forwarder function name
    std::string GetForwarderFunction() const;

    // Get ordinal
    WORD GetOrdinal() const;

    // If entry is exported by name
    bool ByName() const;

    // If entry is forwarded
    bool Forwarded() const;

  private:
    // PE file instance
    PeFile m_PeFile;
    
    // Memory instance
    MemoryMgr m_Memory;

    // RVA
    DWORD m_Rva;
    
    // VA
    PVOID m_Va;
    
    // Name
    std::string m_Name;
    
    // Forwarder
    std::string m_Forwarder;
    
    // Split forwarder
    std::pair<std::string, std::string> m_ForwarderSplit;
    
    // Ordinal
    WORD m_Ordinal;
    
    // If entry is exported by name
    bool m_ByName;
    
    // If entry is forwarded
    bool m_Forwarded;
  };
  
  // Export enumeration class
  class ExportList
  {
  public:
    // Export list error class
    class Error : public virtual HadesMemError
    { };
      
    // Export iterator
    class ExportIter : public boost::iterator_facade<ExportIter, Export, 
      boost::forward_traversal_tag>
    {
    public:
      // Export iterator error class
      class Error : public virtual HadesMemError
      { };

      // Constructor
      ExportIter() 
        : m_pParent(nullptr), 
        m_PeFile(), 
        m_NumFuncs(static_cast<DWORD>(-1)), 
        m_OrdBase(static_cast<DWORD>(-1)), 
        m_Export(), 
        m_CurNum(static_cast<DWORD>(-1))
      { }
      
      // Constructor
      ExportIter(ExportList& Parent) 
        : m_pParent(&Parent), 
        m_PeFile(Parent.m_PeFile), 
        m_NumFuncs(0), 
        m_OrdBase(0), 
        m_Export(), 
        m_CurNum(0)
      {
        ExportDir const MyExportDir(*m_PeFile);
        if (!MyExportDir.IsValid() || !MyExportDir.GetNumberOfFunctions())
        {
          m_pParent = nullptr;
          m_PeFile = boost::optional<PeFile>();
          m_NumFuncs = static_cast<DWORD>(-1);
          m_OrdBase = static_cast<DWORD>(-1);
          m_Export = boost::optional<Export>();
          m_CurNum = static_cast<DWORD>(-1);
        }
        else
        {
          m_NumFuncs = MyExportDir.GetNumberOfFunctions();
          m_OrdBase = MyExportDir.GetOrdinalBase();
          m_Export = Export(*m_PeFile, MyExportDir.GetOrdinalBase());
        }
      }
      
      // Copy constructor
      ExportIter(ExportIter const& Rhs) 
        : m_pParent(Rhs.m_pParent), 
        m_PeFile(Rhs.m_PeFile), 
        m_NumFuncs(Rhs.m_NumFuncs), 
        m_OrdBase(Rhs.m_OrdBase), 
        m_Export(Rhs.m_Export), 
        m_CurNum(Rhs.m_CurNum)
      { }
      
      // Assignment operator
      ExportIter& operator=(ExportIter const& Rhs) 
      {
        m_pParent = Rhs.m_pParent;
        m_PeFile = Rhs.m_PeFile;
        m_NumFuncs = Rhs.m_NumFuncs;
        m_OrdBase = Rhs.m_OrdBase;
        m_Export = Rhs.m_Export;
        m_CurNum = Rhs.m_CurNum;
        return *this;
      }

    private:
      // Give Boost.Iterator access to internals
      friend class boost::iterator_core_access;

      // Increment iterator
      void increment() 
      {
        ++m_CurNum;
        DWORD const NextOrdinal = m_Export->GetOrdinal() + 1;
        if (NextOrdinal - m_OrdBase < m_NumFuncs)
        {
          m_Export = Export(*m_PeFile, NextOrdinal);
        }
        else
        {
          m_pParent = nullptr;
          m_PeFile = boost::optional<PeFile>();
          m_NumFuncs = static_cast<DWORD>(-1);
          m_OrdBase = static_cast<DWORD>(-1);
          m_Export = boost::optional<Export>();
          m_CurNum = static_cast<DWORD>(-1);
        }
      }
      
      // Check iterator for equality
      bool equal(ExportIter const& Rhs) const
      {
        return this->m_pParent == Rhs.m_pParent && 
          this->m_CurNum == Rhs.m_CurNum;
      }
  
      // Dereference iterator
      Export& dereference() const 
      {
        return *m_Export;
      }

      // Parent
      class ExportList* m_pParent;
      // PE file
      boost::optional<PeFile> m_PeFile;
      // Number of functions
      DWORD m_NumFuncs;
      // Ordinal base
      DWORD m_OrdBase;
      // Export object
      mutable boost::optional<Export> m_Export;
      // Current export number
      DWORD m_CurNum;
    };
    
    // Export list iterator types
    typedef ExportIter iterator;
    
    // Constructor
    ExportList(PeFile const& MyPeFile)
      : m_PeFile(MyPeFile)
    { }
    
    // Get start of export list
    iterator begin()
    {
      return iterator(*this);
    }
    
    // Get end of export list
    iterator end()
    {
      return iterator();
    }
    
  private:
    // Give iterator access to internals
    friend class ExportIter;
    
    // PE file
    PeFile m_PeFile;
  };
}
