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
#include <vector>
#include <string>

// Hades
#include "PeFile.h"
#include "NtHeaders.h"

namespace Hades
{
  namespace Memory
  {
    // PE file export directory
    class ExportDir
    {
    public:
      // ExportDir error class
      class Error : public virtual HadesMemError
      { };

      // Constructor
      inline ExportDir(PeFile& MyPeFile);

      // Whether export directory is valid
      inline bool IsValid() const;

      // Ensure export directory is valid
      inline void EnsureValid() const;

      // Get characteristics
      inline DWORD GetCharacteristics() const;

      // Get time date stamp
      inline DWORD GetTimeDateStamp() const;

      // Get major version
      inline WORD GetMajorVersion() const;

      // Get minor version
      inline WORD GetMinorVersion() const;

      // Get module name
      inline std::string GetName() const;

      // Get ordinal base
      inline DWORD GetOrdinalBase() const;

      // Get number of functions
      inline DWORD GetNumberOfFunctions() const;

      // Get number of names
      inline DWORD GetNumberOfNames() const;

      // Get address of functions
      inline DWORD GetAddressOfFunctions() const;

      // Get address of names
      inline DWORD GetAddressOfNames() const;

      // Get address of name ordinals
      inline DWORD GetAddressOfNameOrdinals() const;

      // Get base of export dir
      inline PBYTE GetBase() const;

      // Get raw export dir
      inline IMAGE_EXPORT_DIRECTORY GetExportDirRaw() const;

    private:
      // PE file
      PeFile* m_pPeFile;

      // Memory instance
      MemoryMgr* m_pMemory;
    };

    // Constructor
    ExportDir::ExportDir(PeFile& MyPeFile)
      : m_pPeFile(&MyPeFile), 
      m_pMemory(&m_pPeFile->GetMemoryMgr())
    { }

    // Whether export directory is valid
    bool ExportDir::IsValid() const
    {
      // Get NT headers
      NtHeaders const MyNtHeaders(*m_pPeFile);

      // Get export dir data
      DWORD const DataDirSize(MyNtHeaders.GetDataDirectorySize(NtHeaders::
        DataDir_Export));
      DWORD const DataDirVa(MyNtHeaders.GetDataDirectoryVirtualAddress(
        NtHeaders::DataDir_Export));

      // Export dir is valid if size and rva are valid
      return DataDirSize && DataDirVa;
    }

    // Ensure export directory is valid
    void ExportDir::EnsureValid() const
    {
      if (!IsValid())
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("ExportDir::EnsureValid") << 
          ErrorString("Export directory is invalid."));
      }
    }

    // Get characteristics
    DWORD ExportDir::GetCharacteristics() const
    {
      PBYTE const pExportDir = GetBase();
      return m_pMemory->Read<DWORD>(pExportDir + FIELD_OFFSET(
        IMAGE_EXPORT_DIRECTORY, Characteristics));
    }

    // Get time date stamp
    DWORD ExportDir::GetTimeDateStamp() const
    {
      PBYTE const pExportDir = GetBase();
      return m_pMemory->Read<DWORD>(pExportDir + FIELD_OFFSET(
        IMAGE_EXPORT_DIRECTORY, TimeDateStamp));
    }

    // Get major version
    WORD ExportDir::GetMajorVersion() const
    {
      PBYTE const pExportDir = GetBase();
      return m_pMemory->Read<WORD>(pExportDir + FIELD_OFFSET(
        IMAGE_EXPORT_DIRECTORY, MajorVersion));
    }

    // Get minor version
    WORD ExportDir::GetMinorVersion() const
    {
      PBYTE const pExportDir = GetBase();
      return m_pMemory->Read<WORD>(pExportDir + FIELD_OFFSET(
        IMAGE_EXPORT_DIRECTORY, MinorVersion));
    }

    // Get module name
    std::string ExportDir::GetName() const
    {
      // Get base of export dir
      PBYTE const pExpDirBase = GetBase();

      // Read RVA of module name
      DWORD const NameRva = m_pMemory->Read<DWORD>(pExpDirBase + FIELD_OFFSET(
        IMAGE_EXPORT_DIRECTORY, Name));

      // Ensure there is a module name to process
      if (!NameRva)
      {
        return std::string();
      }

      // Read module name
      return m_pMemory->Read<std::string>(m_pPeFile->GetBase() + NameRva);
    }

    // Get ordinal base
    DWORD ExportDir::GetOrdinalBase() const
    {
      PBYTE const pExportDir = GetBase();
      return m_pMemory->Read<DWORD>(pExportDir + FIELD_OFFSET(
        IMAGE_EXPORT_DIRECTORY, Base));
    }

    // Get number of functions
    DWORD ExportDir::GetNumberOfFunctions() const
    {
      PBYTE const pExportDir = GetBase();
      return m_pMemory->Read<DWORD>(pExportDir + FIELD_OFFSET(
        IMAGE_EXPORT_DIRECTORY, NumberOfFunctions));
    }

    // Get number of names
    DWORD ExportDir::GetNumberOfNames() const
    {
      PBYTE const pExportDir = GetBase();
      return m_pMemory->Read<DWORD>(pExportDir + FIELD_OFFSET(
        IMAGE_EXPORT_DIRECTORY, NumberOfNames));
    }

    // Get address of functions
    DWORD ExportDir::GetAddressOfFunctions() const
    {
      PBYTE const pExportDir = GetBase();
      return m_pMemory->Read<DWORD>(pExportDir + FIELD_OFFSET(
        IMAGE_EXPORT_DIRECTORY, AddressOfFunctions));
    }

    // Get address of names
    DWORD ExportDir::GetAddressOfNames() const
    {
      PBYTE const pExportDir = GetBase();
      return m_pMemory->Read<DWORD>(pExportDir + FIELD_OFFSET(
        IMAGE_EXPORT_DIRECTORY, AddressOfNames));
    }

    // Get address of name ordinals
    DWORD ExportDir::GetAddressOfNameOrdinals() const
    {
      PBYTE const pExportDir = GetBase();
      return m_pMemory->Read<DWORD>(pExportDir + FIELD_OFFSET(
        IMAGE_EXPORT_DIRECTORY, AddressOfNameOrdinals));
    }

    // Get base of export dir
    PBYTE ExportDir::GetBase() const
    {
      // Get PE file base
      PBYTE const pBase = m_pPeFile->GetBase();

      // Get NT headers
      NtHeaders const MyNtHeaders(*m_pPeFile);

      // Get export dir data
      DWORD const DataDirSize = MyNtHeaders.GetDataDirectorySize(NtHeaders::
        DataDir_Export);
      DWORD const DataDirVa = MyNtHeaders.GetDataDirectoryVirtualAddress(
        NtHeaders::DataDir_Export);
      if (!DataDirSize || !DataDirVa)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("ExportDir::GetBase") << 
          ErrorString("PE file has no export directory."));
      }

      return pBase + DataDirVa;
    }

    // Get raw export dir
    IMAGE_EXPORT_DIRECTORY ExportDir::GetExportDirRaw() const
    {
      // Get raw export dir
      return m_pMemory->Read<IMAGE_EXPORT_DIRECTORY>(GetBase());
    }

    // PE file export data
    class Export
    {
    public:
      explicit Export(PeFile& MyPeFile) 
        : m_pPeFile(&MyPeFile), 
        m_pMemory(&MyPeFile.GetMemoryMgr()), 
        m_Rva(0), 
        m_Va(nullptr), 
        m_Name(), 
        m_Forwarder(), 
        m_Ordinal(0), 
        m_ByName(false), 
        m_Forwarded(false)
      { }

      Export(PeFile& MyPeFile, DWORD Number) 
        : m_pPeFile(&MyPeFile), 
        m_pMemory(&MyPeFile.GetMemoryMgr()), 
        m_Rva(0), 
        m_Va(nullptr), 
        m_Name(), 
        m_Forwarder(), 
        m_Ordinal(0), 
        m_ByName(false), 
        m_Forwarded(false)
      {
        DosHeader const MyDosHeader(*m_pPeFile);
        NtHeaders const MyNtHeaders(*m_pPeFile);

        ExportDir const MyExportDir(*m_pPeFile);

        WORD* pOrdinals = reinterpret_cast<WORD*>(m_pPeFile->GetBase() + 
          MyExportDir.GetAddressOfFunctions());
        DWORD* pFunctions = reinterpret_cast<DWORD*>(m_pPeFile->GetBase() + 
          MyExportDir.GetAddressOfFunctions());
        DWORD* pNames = reinterpret_cast<DWORD*>(m_pPeFile->GetBase() + 
          MyExportDir.GetAddressOfNames());

        DWORD const DataDirSize = MyNtHeaders.GetDataDirectorySize(NtHeaders::
          DataDir_Export);
        DWORD const DataDirVa = MyNtHeaders.GetDataDirectoryVirtualAddress(
          NtHeaders::DataDir_Export);

        DWORD const ExportDirStart = DataDirVa;
        DWORD const ExportDirEnd = ExportDirStart + DataDirSize;

        if (Number > MyExportDir.GetNumberOfFunctions())
        {
          BOOST_THROW_EXCEPTION(ExportDir::Error() << 
            ErrorFunction("Export::Export") << 
            ErrorString("Invalid export number."));
        }

        m_Ordinal = static_cast<WORD>(Number + MyExportDir.GetOrdinalBase());

        for (std::size_t j = 0; j < MyExportDir.GetNumberOfNames(); ++j)
        {
          if (m_pMemory->Read<WORD>(pOrdinals + j) == Number)
          {
            m_ByName = true;
            m_Name = m_pMemory->Read<std::string>(m_pPeFile->GetBase() + 
              m_pMemory->Read<DWORD>(pNames + j));
          }
        }

        DWORD const FuncRva = m_pMemory->Read<DWORD>(pFunctions + Number);

        if (FuncRva >= ExportDirStart && FuncRva <= ExportDirEnd)
        {
          m_Forwarded = true;
          m_Forwarder = m_pMemory->Read<std::string>(m_pPeFile->GetBase() + 
            FuncRva);
        }
        else
        {
          m_Rva = FuncRva;
          m_Va = m_pPeFile->GetBase() + FuncRva;
        }
      }

      DWORD GetRva() const
      {
        return m_Rva;
      }

      PVOID GetVa() const
      {
        return m_Va;
      }

      std::string GetName() const
      {
        return m_Name;
      }

      std::string GetForwarder() const
      {
        return m_Forwarder;
      }

      WORD GetOrdinal() const
      {
        return m_Ordinal;
      }

      bool ByName() const
      {
        return m_ByName;
      }

      bool Forwarded() const
      {
        return m_Forwarded;
      }

    private:
      PeFile* m_pPeFile;
      MemoryMgr* m_pMemory;

      DWORD m_Rva;
      PVOID m_Va;
      std::string m_Name;
      std::string m_Forwarder;
      WORD m_Ordinal;
      bool m_ByName;
      bool m_Forwarded;
    };
  }
}
