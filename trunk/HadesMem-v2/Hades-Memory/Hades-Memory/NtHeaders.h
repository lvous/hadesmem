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
      inline explicit NtHeaders(PeFile const& MyPeFile);

      // Get base of NT headers
      inline PVOID GetBase() const;

      // Whether signature is valid
      inline bool IsSignatureValid() const;

      // Ensure signature is valid
      inline void EnsureSignatureValid() const;

      // Get signature
      inline DWORD GetSignature() const;

      // Set signature
      inline void SetSignature(DWORD Signature);

      // Get number of sections
      inline WORD GetNumberOfSections() const;

      // Set number of sections
      inline void SetNumberOfSections(WORD NumberOfSections);

      // Get base of code
      inline DWORD GetBaseOfCode() const;

      // Set base of code
      inline void SetBaseOfCode(DWORD BaseOfCode);

      // Get size of image
      inline DWORD GetSizeOfImage() const;
      
      // Set size of image
      inline void SetSizeOfImage(DWORD SizeOfImage);

      // Get raw NT headers
      inline IMAGE_NT_HEADERS GetHeadersRaw() const;

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
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<DWORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        Signature));
    }

    // Set signature
    void NtHeaders::SetSignature(DWORD Signature)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
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
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<WORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        FileHeader.NumberOfSections));
    }

    // Set number of sections
    void NtHeaders::SetNumberOfSections(WORD NumberOfSections)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
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
    
    // Get base of code
    DWORD NtHeaders::GetBaseOfCode() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<DWORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.BaseOfCode));
    }

    // Set base of code
    void NtHeaders::SetBaseOfCode(DWORD BaseOfCode)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.BaseOfCode), BaseOfCode);

      if (GetBaseOfCode() != BaseOfCode)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetBaseOfCode") << 
          ErrorString("Could not set base of code. Verification mismatch."));
      }
    }

    // Get size of image
    DWORD NtHeaders::GetSizeOfImage() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<DWORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.SizeOfImage));
    }

    // Set size of image
    void NtHeaders::SetSizeOfImage(DWORD SizeOfImage)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.SizeOfImage), SizeOfImage);

      if (GetSizeOfImage() != SizeOfImage)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetSizeOfImage") << 
          ErrorString("Could not set size of image. Verification mismatch."));
      }
    }

    // Get raw NT headers
    IMAGE_NT_HEADERS NtHeaders::GetHeadersRaw() const
    {
      return m_Memory.Read<IMAGE_NT_HEADERS>(GetBase());
    }
  }
}
