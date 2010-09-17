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

      // Data directory entries
      enum DataDir
      {
        DataDir_Export, 
        DataDir_Import, 
        DataDir_Resource, 
        DataDir_Exception, 
        DataDir_Security, 
        DataDir_BaseReloc, 
        DataDir_Debug, 
        DataDir_Architecture, 
        DataDir_GlobalPTR, 
        DataDir_TLS, 
        DataDir_LoadConfig, 
        DataDir_BoundImport, 
        DataDir_IAT, 
        DataDir_DelayImport, 
        DataDir_COMDescriptor
      };

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

      // Get machine
      inline WORD GetMachine() const;

      // Set machine
      inline void SetMachine(WORD Machine);

      // Get number of sections
      inline WORD GetNumberOfSections() const;

      // Set number of sections
      inline void SetNumberOfSections(WORD NumberOfSections);

      // Get time date stamp
      inline DWORD GetTimeDateStamp() const;

      // Set time date stamp
      inline void SetTimeDateStamp(DWORD TimeDateStamp);

      // Get pointer to symbol table
      inline DWORD GetPointerToSymbolTable() const;

      // Set pointer to symbol table
      inline void SetPointerToSymbolTable(DWORD PointerToSymbolTable);

      // Get number of symbols
      inline DWORD GetNumberOfSymbols() const;

      // Set number of symbols
      inline void SetNumberOfSymbols(DWORD NumberOfSymbols);

      // Get size of optional header
      inline WORD GetSizeOfOptionalHeader() const;

      // Set size of optional header
      inline void SetSizeOfOptionalHeader(WORD SizeOfOptionalHeader);

      // Get characteristics
      inline WORD GetCharacteristics() const;

      // Set characteristics
      inline void SetCharacteristics(WORD Characteristics);

      // Get magic
      inline WORD GetMagic() const;

      // Set magic
      inline void SetMagic(WORD Magic);

      // Get major linker version
      inline BYTE GetMajorLinkerVersion() const;

      // Set major linker version
      inline void SetMajorLinkerVersion(BYTE MajorLinkerVersion);

      // Get minor linker version
      inline BYTE GetMinorLinkerVersion() const;

      // Set major linker version
      inline void SetMinorLinkerVersion(BYTE MinorLinkerVersion);

      // Get minor linker version
      inline DWORD GetSizeOfCode() const;

      // Set major linker version
      inline void SetSizeOfCode(DWORD SizeOfCode);

      // Get minor linker version
      inline DWORD GetSizeOfInitializedData() const;

      // Set major linker version
      inline void SetSizeOfInitializedData(DWORD SizeOfInitializedData);

      // Get minor linker version
      inline DWORD GetSizeOfUninitializedData() const;

      // Set major linker version
      inline void SetSizeOfUninitializedData(DWORD SizeOfUninitializedData);

      // Get minor linker version
      inline DWORD GetAddressOfEntryPoint() const;

      // Set major linker version
      inline void SetAddressOfEntryPoint(DWORD AddressOfEntryPoint);

      // Get base of code
      inline DWORD GetBaseOfCode() const;

      // Set base of code
      inline void SetBaseOfCode(DWORD BaseOfCode);

#if defined(_M_IX86) 
      // Get base of data
      inline DWORD GetBaseOfData() const;

      // Set base of data
      inline void SetBaseOfData(DWORD BaseOfData);
#endif

      // Get base of code
      inline ULONG_PTR GetImageBase() const;

      // Set base of code
      inline void SetImageBase(ULONG_PTR ImageBase);

      // Get base of code
      inline DWORD GetSectionAlignment() const;

      // Set base of code
      inline void SetSectionAlignment(DWORD SectionAlignment);

      // Get base of code
      inline DWORD GetFileAlignment() const;

      // Set base of code
      inline void SetFileAlignment(DWORD FileAlignment);

      // Get base of code
      inline WORD GetMajorOperatingSystemVersion() const;

      // Set base of code
      inline void SetMajorOperatingSystemVersion(
        WORD MajorOperatingSystemVersion);

      // Get base of code
      inline WORD GetMinorOperatingSystemVersion() const;

      // Set base of code
      inline void SetMinorOperatingSystemVersion(
        WORD MinorOperatingSystemVersion);

      // Get base of code
      inline WORD GetMajorImageVersion() const;

      // Set base of code
      inline void SetMajorImageVersion(WORD MajorImageVersion);

      // Get base of code
      inline WORD GetMinorImageVersion() const;

      // Set base of code
      inline void SetMinorImageVersion(WORD MinorImageVersion);

      // Get base of code
      inline WORD GetMajorSubsystemVersion() const;

      // Set base of code
      inline void SetMajorSubsystemVersion(WORD MajorSubsystemVersion);

      // Get base of code
      inline WORD GetMinorSubsystemVersion() const;

      // Set base of code
      inline void SetMinorSubsystemVersion(WORD MinorSubsystemVersion);

      // Get base of code
      inline DWORD GetWin32VersionValue() const;

      // Set base of code
      inline void SetWin32VersionValue(DWORD Win32VersionValue);

      // Get size of image
      inline DWORD GetSizeOfImage() const;
      
      // Set size of image
      inline void SetSizeOfImage(DWORD SizeOfImage);

      // Get base of code
      inline DWORD GetSizeOfHeaders() const;

      // Set base of code
      inline void SetSizeOfHeaders(DWORD SizeOfHeaders);

      // Get base of code
      inline DWORD GetCheckSum() const;

      // Set base of code
      inline void SetCheckSum(DWORD CheckSum);

      // Get base of code
      inline WORD GetSubsystem() const;

      // Set base of code
      inline void SetSubsystem(WORD Subsystem);

      // Get base of code
      inline WORD GetDllCharacteristics() const;

      // Set base of code
      inline void SetDllCharacteristics(WORD DllCharacteristics);

      // Get base of code
      inline ULONG_PTR GetSizeOfStackReserve() const;

      // Set base of code
      inline void SetSizeOfStackReserve(ULONG_PTR SizeOfStackReserve);

      // Get base of code
      inline ULONG_PTR GetSizeOfStackCommit() const;

      // Set base of code
      inline void SetSizeOfStackCommit(ULONG_PTR SizeOfStackCommit);

      // Get base of code
      inline ULONG_PTR GetSizeOfHeapReserve() const;

      // Set base of code
      inline void SetSizeOfHeapReserve(ULONG_PTR SizeOfHeapReserve);

      // Get base of code
      inline ULONG_PTR GetSizeOfHeapCommit() const;

      // Set base of code
      inline void SetSizeOfHeapCommit(ULONG_PTR SizeOfHeapCommit);

      // Get base of code
      inline DWORD GetLoaderFlags() const;

      // Set base of code
      inline void SetLoaderFlags(DWORD LoaderFlags);

      // Get base of code
      inline DWORD GetNumberOfRvaAndSizes() const;

      // Set base of code
      inline void SetNumberOfRvaAndSizes(DWORD NumberOfRvaAndSizes);

      // Get base of code
      inline DWORD GetDataDirectoryVirtualAddress(DataDir MyDataDir) const;

      // Set base of code
      inline void SetDataDirectoryVirtualAddress(DataDir MyDataDir, 
        DWORD DataDirectoryVirtualAddress);

      // Get base of code
      inline DWORD GetDataDirectorySize(DataDir MyDataDir) const;

      // Set base of code
      inline void SetDataDirectorySize(DataDir MyDataDir, 
        DWORD DataDirectorySize);

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
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, Signature), 
        Signature);

      if (GetSignature() != Signature)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetSignature") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get machine
    WORD NtHeaders::GetMachine() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<WORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        FileHeader.Machine));
    }

    // Set machine
    void NtHeaders::SetMachine(WORD Machine)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, FileHeader.
        Machine), Machine);

      if (GetMachine() != Machine)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetMachine") << 
          ErrorString("Could not set data. Verification mismatch."));
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
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, FileHeader.
        NumberOfSections), NumberOfSections);

      if (GetNumberOfSections() != NumberOfSections)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetNumberOfSections") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get time date stamp
    DWORD NtHeaders::GetTimeDateStamp() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<DWORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        FileHeader.TimeDateStamp));
    }

    // Set time date stamp
    void NtHeaders::SetTimeDateStamp(DWORD TimeDateStamp)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, FileHeader.
        TimeDateStamp), TimeDateStamp);

      if (GetTimeDateStamp() != TimeDateStamp)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetTimeDateStamp") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get pointer to symbol table
    DWORD NtHeaders::GetPointerToSymbolTable() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<DWORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        FileHeader.PointerToSymbolTable));
    }

    // Set pointer to symbol table
    void NtHeaders::SetPointerToSymbolTable(DWORD PointerToSymbolTable)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, FileHeader.
        PointerToSymbolTable), PointerToSymbolTable);

      if (GetPointerToSymbolTable() != PointerToSymbolTable)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetPointerToSymbolTable") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get number of symbols
    DWORD NtHeaders::GetNumberOfSymbols() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<DWORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        FileHeader.NumberOfSymbols));
    }

    // Set number of symbols
    void NtHeaders::SetNumberOfSymbols(DWORD NumberOfSymbols)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, FileHeader.
        NumberOfSymbols), NumberOfSymbols);

      if (GetNumberOfSymbols() != NumberOfSymbols)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetNumberOfSymbols") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get size of optional header
    WORD NtHeaders::GetSizeOfOptionalHeader() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<WORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        FileHeader.SizeOfOptionalHeader));
    }

    // Set size of optional header
    void NtHeaders::SetSizeOfOptionalHeader(WORD SizeOfOptionalHeader)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, FileHeader.
        SizeOfOptionalHeader), SizeOfOptionalHeader);

      if (GetSizeOfOptionalHeader() != SizeOfOptionalHeader)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetSizeOfOptionalHeader") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get characteristics
    WORD NtHeaders::GetCharacteristics() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<WORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        FileHeader.Characteristics));
    }

    // Set characteristics
    void NtHeaders::SetCharacteristics(WORD Characteristics)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, FileHeader.
        Characteristics), Characteristics);

      if (GetCharacteristics() != Characteristics)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetCharacteristics") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get magic
    WORD NtHeaders::GetMagic() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<WORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.Magic));
    }

    // Set magic
    void NtHeaders::SetMagic(WORD Magic)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        Magic), Magic);

      if (GetMagic() != Magic)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetMagic") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get major linker version
    BYTE NtHeaders::GetMajorLinkerVersion() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<BYTE>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.MajorLinkerVersion));
    }

    // Set major linker version
    void NtHeaders::SetMajorLinkerVersion(BYTE MajorLinkerVersion)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        MajorLinkerVersion), MajorLinkerVersion);

      if (GetMajorLinkerVersion() != MajorLinkerVersion)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetMajorLinkerVersion") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get minor linker version
    BYTE NtHeaders::GetMinorLinkerVersion() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<BYTE>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.MinorLinkerVersion));
    }

    // Set minor linker version
    void NtHeaders::SetMinorLinkerVersion(BYTE MinorLinkerVersion)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        MinorLinkerVersion), MinorLinkerVersion);

      if (GetMinorLinkerVersion() != MinorLinkerVersion)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetMinorLinkerVersion") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get size of code
    DWORD NtHeaders::GetSizeOfCode() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<DWORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.SizeOfCode));
    }

    // Set size of code
    void NtHeaders::SetSizeOfCode(DWORD SizeOfCode)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        SizeOfCode), SizeOfCode);

      if (GetSizeOfCode() != SizeOfCode)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetSizeOfCode") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get size of initialized data
    DWORD NtHeaders::GetSizeOfInitializedData() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<DWORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.SizeOfInitializedData));
    }

    // Set size of initialized data
    void NtHeaders::SetSizeOfInitializedData(DWORD SizeOfInitializedData)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        SizeOfInitializedData), SizeOfInitializedData);

      if (GetSizeOfInitializedData() != SizeOfInitializedData)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetSizeOfInitializedData") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get size of uninitialized data
    DWORD NtHeaders::GetSizeOfUninitializedData() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<DWORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.SizeOfUninitializedData));
    }

    // Set size of uninitialized data
    void NtHeaders::SetSizeOfUninitializedData(DWORD SizeOfUninitializedData)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        SizeOfUninitializedData), SizeOfUninitializedData);

      if (GetSizeOfUninitializedData() != SizeOfUninitializedData)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetSizeOfUninitializedData") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get address of entry point
    DWORD NtHeaders::GetAddressOfEntryPoint() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<DWORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.SizeOfUninitializedData));
    }

    // Set address of entry point
    void NtHeaders::SetAddressOfEntryPoint(DWORD AddressOfEntryPoint)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        AddressOfEntryPoint), AddressOfEntryPoint);

      if (GetAddressOfEntryPoint() != AddressOfEntryPoint)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetAddressOfEntryPoint") << 
          ErrorString("Could not set data. Verification mismatch."));
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
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        BaseOfCode), BaseOfCode);

      if (GetBaseOfCode() != BaseOfCode)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetBaseOfCode") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

#if defined(_M_IX86) 
    // Get base of data
    DWORD NtHeaders::GetBaseOfData() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<DWORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.BaseOfData));
    }

    // Set base of data
    void NtHeaders::SetBaseOfData(DWORD BaseOfData)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        BaseOfData), BaseOfData);

      if (GetBaseOfData() != BaseOfData)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetBaseOfData") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }
#endif

    // Get image base
    ULONG_PTR NtHeaders::GetImageBase() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<ULONG_PTR>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.ImageBase));
    }

    // Set image base
    void NtHeaders::SetImageBase(ULONG_PTR ImageBase)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        ImageBase), ImageBase);

      if (GetImageBase() != ImageBase)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetImageBase") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get section alignment
    DWORD NtHeaders::GetSectionAlignment() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<DWORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.SectionAlignment));
    }

    // Set section alignment
    void NtHeaders::SetSectionAlignment(DWORD SectionAlignment)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        SectionAlignment), SectionAlignment);

      if (GetSectionAlignment() != SectionAlignment)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetSectionAlignment") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get file alignment
    DWORD NtHeaders::GetFileAlignment() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<DWORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.FileAlignment));
    }

    // Set file alignment
    void NtHeaders::SetFileAlignment(DWORD FileAlignment)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        FileAlignment), FileAlignment);

      if (GetFileAlignment() != FileAlignment)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetFileAlignment") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get major operating system version
    WORD NtHeaders::GetMajorOperatingSystemVersion() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<WORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.FileAlignment));
    }

    // Set major operating system version
    void NtHeaders::SetMajorOperatingSystemVersion(
      WORD MajorOperatingSystemVersion)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        MajorOperatingSystemVersion), MajorOperatingSystemVersion);

      if (GetMajorOperatingSystemVersion() != MajorOperatingSystemVersion)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetMajorOperatingSystemVersion") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get minor operating system version
    WORD NtHeaders::GetMinorOperatingSystemVersion() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<WORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.MinorOperatingSystemVersion));
    }

    // Set minor operating system version
    void NtHeaders::SetMinorOperatingSystemVersion(
      WORD MinorOperatingSystemVersion)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        MinorOperatingSystemVersion), MinorOperatingSystemVersion);

      if (GetMinorOperatingSystemVersion() != MinorOperatingSystemVersion)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetMinorOperatingSystemVersion") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get major image version
    WORD NtHeaders::GetMajorImageVersion() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<WORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.MajorImageVersion));
    }

    // Set major image version
    void NtHeaders::SetMajorImageVersion(WORD MajorImageVersion)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        MajorImageVersion), MajorImageVersion);

      if (GetMajorImageVersion() != MajorImageVersion)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetMajorImageVersion") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get minor image version
    WORD NtHeaders::GetMinorImageVersion() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<WORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.MinorImageVersion));
    }

    // Set minor image version
    void NtHeaders::SetMinorImageVersion(WORD MinorImageVersion)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        MinorImageVersion), MinorImageVersion);

      if (GetMinorImageVersion() != MinorImageVersion)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetMinorImageVersion") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get major subsystem version
    WORD NtHeaders::GetMajorSubsystemVersion() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<WORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.MajorSubsystemVersion));
    }

    // Set major subsystem version
    void NtHeaders::SetMajorSubsystemVersion(WORD MajorSubsystemVersion)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        MajorSubsystemVersion), MajorSubsystemVersion);

      if (GetMajorSubsystemVersion() != MajorSubsystemVersion)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetMajorSubsystemVersion") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get minor subsystem version
    WORD NtHeaders::GetMinorSubsystemVersion() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<WORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.MinorSubsystemVersion));
    }

    // Set minor subsystem version
    void NtHeaders::SetMinorSubsystemVersion(WORD MinorSubsystemVersion)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        MinorSubsystemVersion), MinorSubsystemVersion);

      if (GetMinorSubsystemVersion() != MinorSubsystemVersion)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetMinorSubsystemVersion") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get Win32 version value
    DWORD NtHeaders::GetWin32VersionValue() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<DWORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.Win32VersionValue));
    }

    // Set Win32 version value
    void NtHeaders::SetWin32VersionValue(DWORD Win32VersionValue)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        Win32VersionValue), Win32VersionValue);

      if (GetWin32VersionValue() != Win32VersionValue)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetWin32VersionValue") << 
          ErrorString("Could not set data. Verification mismatch."));
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
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        SizeOfImage), SizeOfImage);

      if (GetSizeOfImage() != SizeOfImage)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetSizeOfImage") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get size of headers
    DWORD NtHeaders::GetSizeOfHeaders() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<DWORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.SizeOfHeaders));
    }

    // Set size of headers
    void NtHeaders::SetSizeOfHeaders(DWORD SizeOfHeaders)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        SizeOfHeaders), SizeOfHeaders);

      if (GetSizeOfHeaders() != SizeOfHeaders)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetSizeOfHeaders") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get checksum
    DWORD NtHeaders::GetCheckSum() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<DWORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.CheckSum));
    }

    // Set checksum
    void NtHeaders::SetCheckSum(DWORD CheckSum)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        CheckSum), CheckSum);

      if (GetCheckSum() != CheckSum)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetCheckSum") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get subsystem
    WORD NtHeaders::GetSubsystem() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<WORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.Subsystem));
    }

    // Set subsystem
    void NtHeaders::SetSubsystem(WORD Subsystem)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        Subsystem), Subsystem);

      if (GetSubsystem() != Subsystem)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetSubsystem") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get DLL characteristics
    WORD NtHeaders::GetDllCharacteristics() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<WORD>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.DllCharacteristics));
    }

    // Set DLL characteristics
    void NtHeaders::SetDllCharacteristics(WORD DllCharacteristics)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        DllCharacteristics), DllCharacteristics);

      if (GetDllCharacteristics() != DllCharacteristics)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetDllCharacteristics") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get size of stack reserve
    ULONG_PTR NtHeaders::GetSizeOfStackReserve() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<ULONG_PTR>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.SizeOfStackReserve));
    }

    // Set size of stack reserve
    void NtHeaders::SetSizeOfStackReserve(ULONG_PTR SizeOfStackReserve)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        SizeOfStackReserve), SizeOfStackReserve);

      if (GetSizeOfStackReserve() != SizeOfStackReserve)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetSizeOfStackReserve") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get size of stack commit
    ULONG_PTR NtHeaders::GetSizeOfStackCommit() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<ULONG_PTR>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.SizeOfStackCommit));
    }

    // Set size of stack commit
    void NtHeaders::SetSizeOfStackCommit(ULONG_PTR SizeOfStackCommit)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        SizeOfStackCommit), SizeOfStackCommit);

      if (GetSizeOfStackCommit() != SizeOfStackCommit)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetSizeOfStackCommit") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get size of heap reserve
    ULONG_PTR NtHeaders::GetSizeOfHeapReserve() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<ULONG_PTR>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.SizeOfHeapReserve));
    }

    // Set size of heap reserve
    void NtHeaders::SetSizeOfHeapReserve(ULONG_PTR SizeOfHeapReserve)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        SizeOfHeapReserve), SizeOfHeapReserve);

      if (GetSizeOfHeapReserve() != SizeOfHeapReserve)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetSizeOfHeapReserve") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get size of heap commit
    ULONG_PTR NtHeaders::GetSizeOfHeapCommit() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<ULONG_PTR>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.SizeOfHeapCommit));
    }

    // Set size of heap commit
    void NtHeaders::SetSizeOfHeapCommit(ULONG_PTR SizeOfHeapCommit)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        SizeOfHeapCommit), SizeOfHeapCommit);

      if (GetSizeOfHeapCommit() != SizeOfHeapCommit)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetSizeOfHeapCommit") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get loader flags
    DWORD NtHeaders::GetLoaderFlags() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<ULONG_PTR>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.LoaderFlags));
    }

    // Set loader flags
    void NtHeaders::SetLoaderFlags(DWORD LoaderFlags)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        LoaderFlags), LoaderFlags);

      if (GetLoaderFlags() != LoaderFlags)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetLoaderFlags") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get number of RVA and sizes
    DWORD NtHeaders::GetNumberOfRvaAndSizes() const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<ULONG_PTR>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.NumberOfRvaAndSizes));
    }

    // Set number of RVA and sizes
    void NtHeaders::SetNumberOfRvaAndSizes(DWORD NumberOfRvaAndSizes)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        NumberOfRvaAndSizes), NumberOfRvaAndSizes);

      if (GetNumberOfRvaAndSizes() != NumberOfRvaAndSizes)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetNumberOfRvaAndSizes") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get data directory virtual address
    DWORD NtHeaders::GetDataDirectoryVirtualAddress(DataDir MyDataDir) const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<ULONG_PTR>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.DataDirectory[0]) + MyDataDir * sizeof(
        IMAGE_DATA_DIRECTORY) + FIELD_OFFSET(IMAGE_DATA_DIRECTORY, 
        VirtualAddress));
    }

    // Set data directory virtual address
    void NtHeaders::SetDataDirectoryVirtualAddress(DataDir MyDataDir, 
      DWORD DataDirectoryVirtualAddress)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        DataDirectory[0]) + MyDataDir * sizeof(IMAGE_DATA_DIRECTORY) + 
        FIELD_OFFSET(IMAGE_DATA_DIRECTORY, VirtualAddress), 
        DataDirectoryVirtualAddress);

      if (GetDataDirectoryVirtualAddress(MyDataDir) != 
        DataDirectoryVirtualAddress)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetDataDirectoryVirtualAddress") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get data directory size
    DWORD NtHeaders::GetDataDirectorySize(DataDir MyDataDir) const
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      return m_Memory.Read<ULONG_PTR>(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, 
        OptionalHeader.DataDirectory[0]) + MyDataDir * sizeof(
        IMAGE_DATA_DIRECTORY) + FIELD_OFFSET(IMAGE_DATA_DIRECTORY, 
        Size));
    }

    // Set data directory size
    void NtHeaders::SetDataDirectorySize(DataDir MyDataDir, 
      DWORD DataDirectorySize)
    {
      auto pBase(static_cast<PBYTE>(GetBase()));
      m_Memory.Write(pBase + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.
        DataDirectory[0]) + MyDataDir * sizeof(IMAGE_DATA_DIRECTORY) + 
        FIELD_OFFSET(IMAGE_DATA_DIRECTORY, Size), DataDirectorySize);

      if (GetDataDirectorySize(MyDataDir) != DataDirectorySize)
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("NtHeaders::SetDataDirectorySize") << 
          ErrorString("Could not set data. Verification mismatch."));
      }
    }

    // Get raw NT headers
    IMAGE_NT_HEADERS NtHeaders::GetHeadersRaw() const
    {
      return m_Memory.Read<IMAGE_NT_HEADERS>(GetBase());
    }
  }
}
