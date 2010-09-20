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
    // PE file export data
    struct Export
    {
      DWORD Rva;
      PVOID Va;
      std::string Name;
      std::string Forwarder;
      WORD Ordinal;
      bool ByName;
      bool Forwarded;
    };

    // PE file export directory
    class ExportDir
    {
    public:
      // ExportDir error class
      class Error : public virtual HadesMemError
      { };

      // Constructor
      inline ExportDir(PeFile* MyPeFile);

      // Whether export directory is valid
      inline bool IsValid() const;

      // Ensure export directory is valid
      inline void EnsureValid() const;

      // Get module name
      inline std::string GetName() const;

      // Get all exports
      inline std::vector<Export> GetAllExports() const;

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
    ExportDir::ExportDir(PeFile* MyPeFile)
      : m_pPeFile(MyPeFile), 
      m_pMemory(m_pPeFile->GetMemoryMgr())
    { }

    // Get module name
    std::string ExportDir::GetName() const
    {
      // Get base of export dir
      PBYTE pExpDirBase(GetBase());

      // Read RVA of module name
      auto NameRva(m_pMemory->Read<DWORD>(pExpDirBase + FIELD_OFFSET(
        IMAGE_EXPORT_DIRECTORY, Name)));

      // Ensure there is a module name to process
      if (!NameRva)
      {
        return std::string();
      }

      // Read module name
      return m_pMemory->Read<std::string>(m_pPeFile->GetBase() + NameRva);
    }

    // Get base of export dir
    PBYTE ExportDir::GetBase() const
    {
      // Get PE file base
      PBYTE pBase(m_pPeFile->GetBase());

      // Get NT headers
      NtHeaders MyNtHeaders(m_pPeFile);

      // Get export dir data
      DWORD DataDirSize(MyNtHeaders.GetDataDirectorySize(NtHeaders::
        DataDir_Export));
      DWORD DataDirVa(MyNtHeaders.GetDataDirectoryVirtualAddress(NtHeaders::
        DataDir_Export));
      if (!DataDirSize || !DataDirVa)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("ExportDir::GetBase") << 
          ErrorString("PE file has no export directory."));
      }

      return pBase + DataDirVa;
    }

    // Get all exports
    std::vector<Export> ExportDir::GetAllExports() const
    {
      std::vector<Export> Exports;

      DosHeader MyDosHeader(m_pPeFile);
      NtHeaders MyNtHeaders(m_pPeFile);

      IMAGE_EXPORT_DIRECTORY const ExportDirRaw(GetExportDirRaw());

      WORD* pOrdinals(reinterpret_cast<WORD*>(m_pPeFile->GetBase() + 
        ExportDirRaw.AddressOfNameOrdinals));
      DWORD* pFunctions(reinterpret_cast<DWORD*>(m_pPeFile->GetBase() + 
        ExportDirRaw.AddressOfFunctions));
      DWORD* pNames(reinterpret_cast<DWORD*>(m_pPeFile->GetBase() + 
        ExportDirRaw.AddressOfNames));

      DWORD const DataDirSize(MyNtHeaders.GetDataDirectorySize(NtHeaders::
        DataDir_Export));
      DWORD const DataDirVa(MyNtHeaders.GetDataDirectoryVirtualAddress(
        NtHeaders::DataDir_Export));

      DWORD const ExportDirStart = DataDirVa;
      DWORD const ExportDirEnd = ExportDirStart + DataDirSize;

      for (std::size_t i(0); i < ExportDirRaw.NumberOfFunctions; ++i)
      {
        Export MyExport = { 0 };

        MyExport.Ordinal = static_cast<WORD>(i + ExportDirRaw.Base);

        for (std::size_t j(0); j < ExportDirRaw.NumberOfNames; ++j)
        {
          if (m_pMemory->Read<WORD>(pOrdinals + j) == i)
          {
            MyExport.ByName = true;
            MyExport.Name = m_pMemory->Read<std::string>(m_pPeFile->
              GetBase() + m_pMemory->Read<DWORD>(pNames + j));
          }
        }

        DWORD FuncRva = m_pMemory->Read<DWORD>(pFunctions + i);

        if (FuncRva >= ExportDirStart && FuncRva <= ExportDirEnd)
        {
          MyExport.Forwarded = true;
          MyExport.Forwarder = m_pMemory->Read<std::string>(m_pPeFile->
            GetBase() + FuncRva);
        }
        else
        {
          MyExport.Rva = FuncRva;
          MyExport.Va = m_pPeFile->GetBase() + FuncRva;
        }

        Exports.push_back(MyExport);
      }

      return Exports;
    }

    // Get raw export dir
    IMAGE_EXPORT_DIRECTORY ExportDir::GetExportDirRaw() const
    {
      // Get raw export dir
      return m_pMemory->Read<IMAGE_EXPORT_DIRECTORY>(GetBase());
    }

    // Whether export directory is valid
    bool ExportDir::IsValid() const
    {
      // Get NT headers
      NtHeaders MyNtHeaders(m_pPeFile);

      // Get export dir data
      DWORD DataDirSize(MyNtHeaders.GetDataDirectorySize(NtHeaders::
        DataDir_Export));
      DWORD DataDirVa(MyNtHeaders.GetDataDirectoryVirtualAddress(NtHeaders::
        DataDir_Export));

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
  }
}
