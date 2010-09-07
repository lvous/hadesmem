/*
This file is part of HadesMem.
Copyright � 2010 RaptorFactor (aka Cypherjb, Cypher, Chazwazza). 
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
#include "DosHeader.h"

namespace Hades
{
  namespace Memory
  {
    // PE file NT headers
    class NtHeaders
    {
    public:
      // NT headers error class
      class Error : public virtual HadesMemError
      { };

      // Constructor
      explicit NtHeaders(PeFile const& MyPeFile);

      // Get base of NT headers
      PVOID GetBase() const;

      // Whether signature is valid
      bool IsSignatureValid() const;

      // Ensure signature is valid
      void EnsureSignatureValid() const;

      // Get signature
      DWORD GetSignature() const;

      // Set signature
      void SetSignature(DWORD Signature);

      // Get number of sections
      WORD GetNumberOfSections() const;

      // Set number of sections
      void SetNumberOfSections(WORD NumberOfSections);

      // Get raw NT headers
      IMAGE_NT_HEADERS GetHeadersRaw() const;

    private:
      // Disable assignment
      NtHeaders& operator= (NtHeaders const&);

      // PE file
      PeFile const& m_PeFile;

      // Memory instance
      MemoryMgr const& m_Memory;

      // Dos header
      DosHeader m_DosHeader;
    };

    // Constructor
    NtHeaders::NtHeaders(PeFile const& MyPeFile)
      : m_PeFile(MyPeFile), 
      m_Memory(m_PeFile.GetMemoryMgr()), 
      m_DosHeader(m_PeFile)
    {
      // Ensure signature is valid
      EnsureSignatureValid();
    }

    // Get base of NT headers
    PVOID NtHeaders::GetBase() const
    {
      return static_cast<PBYTE>(m_PeFile.GetBase()) + m_DosHeader.
        GetNewHeaderOffset();
    }

    // Whether signature is valid
    bool NtHeaders::IsSignatureValid() const
    {
      return IMAGE_NT_SIGNATURE == GetSignature();
    }

    // Ensure signature is valid
    void NtHeaders::EnsureSignatureValid() const
    {
      if (!IsSignatureValid())
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::EnsureSignatureValid") << 
          ErrorString("NT headers signature invalid."));
      }
    }

    // Get signature
    DWORD NtHeaders::GetSignature() const
    {
      PBYTE pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<DWORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        Signature));
    }

    // Set signature
    void NtHeaders::SetSignature(DWORD Signature)
    {
      PBYTE pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        Signature), Signature);

      if (GetSignature() != Signature)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetSignature") << 
          ErrorString("Could not set signature. Verification mismatch."));
      }
    }

    // Get number of sections
    WORD NtHeaders::GetNumberOfSections() const
    {
      PBYTE pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<WORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        FileHeader.NumberOfSections));
    }

    // Set number of sections
    void NtHeaders::SetNumberOfSections(WORD NumberOfSections)
    {
      PBYTE pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        FileHeader.NumberOfSections), NumberOfSections);

      if (GetNumberOfSections() != NumberOfSections)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetNumberOfSections") << 
          ErrorString("Could not set numer of sections. Verification "
          "mismatch."));
      }
    }

    // Get raw NT headers
    IMAGE_NT_HEADERS NtHeaders::GetHeadersRaw() const
    {
      return m_Memory.Read<IMAGE_NT_HEADERS>(GetBase());
    }
  }
}
