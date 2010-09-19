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
      // TODO: Implement this in a more efficient manner. (i.e. Read out 
      // only the data that's necessary rather than the whole structure)
      
      // Get export dir
      auto const ExportDirRaw(GetExportDirRaw());

      // Ensure there is a module name to process
      if (!ExportDirRaw.Name)
      {
        return std::string();
      }

      // Read module name
      return m_pMemory->Read<std::string>(static_cast<PBYTE>(m_pPeFile->GetBase()) 
        + ExportDirRaw.Name);
    }

    // Get raw export dir
    IMAGE_EXPORT_DIRECTORY ExportDir::GetExportDirRaw() const
    {
      // Get PE file base
      auto pBase(static_cast<PBYTE>(m_pPeFile->GetBase()));

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
          ErrorFunction("ExportDir::GetExportDirRaw") << 
          ErrorString("PE file has no export directory."));
      }

      // Get raw export dir
      return m_pMemory->Read<IMAGE_EXPORT_DIRECTORY>(pBase + DataDirVa);
    }
  }
}
