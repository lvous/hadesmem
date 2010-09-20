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
      ExportDir(PeFile* MyPeFile);

      // Get module name
      std::string GetName() const;

      // Get base of export dir
      PBYTE GetBase() const;

      // Get raw export dir
      IMAGE_EXPORT_DIRECTORY GetExportDirRaw() const;

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

    // Get raw export dir
    IMAGE_EXPORT_DIRECTORY ExportDir::GetExportDirRaw() const
    {
      // Get raw export dir
      return m_pMemory->Read<IMAGE_EXPORT_DIRECTORY>(GetBase());
    }
  }
}
