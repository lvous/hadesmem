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

// Boost
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/python.hpp>
#pragma warning(pop)

// Hades
#include "Hades-Memory/NtHeaders.h"

// Export NtHeaders API
inline void ExportNtHeaders()
{
  boost::python::scope NtHeadersScope = boost::python::class_<
    Hades::Memory::NtHeaders, boost::noncopyable>("NtHeaders", 
    boost::python::init<Hades::Memory::PeFile&>())
//     .def("GetBase", &Hades::Memory::NtHeaders::GetBase)
    .def("IsSignatureValid", &Hades::Memory::NtHeaders::IsSignatureValid)
    .def("EnsureSignatureValid", &Hades::Memory::NtHeaders::
      EnsureSignatureValid)
    .def("GetSignature", &Hades::Memory::NtHeaders::GetSignature)
    .def("SetSignature", &Hades::Memory::NtHeaders::SetSignature)
    .def("GetMachine", &Hades::Memory::NtHeaders::GetMachine)
    .def("SetMachine", &Hades::Memory::NtHeaders::SetMachine)
    .def("GetNumberOfSections", &Hades::Memory::NtHeaders::GetNumberOfSections)
    .def("SetNumberOfSections", &Hades::Memory::NtHeaders::SetNumberOfSections)
    .def("GetTimeDateStamp", &Hades::Memory::NtHeaders::GetTimeDateStamp)
    .def("SetTimeDateStamp", &Hades::Memory::NtHeaders::SetTimeDateStamp)
    .def("GetPointerToSymbolTable", &Hades::Memory::NtHeaders::
      GetPointerToSymbolTable)
    .def("SetPointerToSymbolTable", &Hades::Memory::NtHeaders::
      SetPointerToSymbolTable)
    .def("GetNumberOfSymbols", &Hades::Memory::NtHeaders::GetNumberOfSymbols)
    .def("SetNumberOfSymbols", &Hades::Memory::NtHeaders::SetNumberOfSymbols)
    .def("GetSizeOfOptionalHeader", &Hades::Memory::NtHeaders::
      GetSizeOfOptionalHeader)
    .def("SetSizeOfOptionalHeader", &Hades::Memory::NtHeaders::
      SetSizeOfOptionalHeader)
    .def("GetCharacteristics", &Hades::Memory::NtHeaders::GetCharacteristics)
    .def("SetCharacteristics", &Hades::Memory::NtHeaders::SetCharacteristics)
    .def("GetMagic", &Hades::Memory::NtHeaders::GetMagic)
    .def("SetMagic", &Hades::Memory::NtHeaders::SetMagic)
    .def("GetMajorLinkerVersion", &Hades::Memory::NtHeaders::
      GetMajorLinkerVersion)
    .def("SetMajorLinkerVersion", &Hades::Memory::NtHeaders::
      SetMajorLinkerVersion)
    .def("GetMinorLinkerVersion", &Hades::Memory::NtHeaders::
      GetMinorLinkerVersion)
    .def("SetMinorLinkerVersion", &Hades::Memory::NtHeaders::
      SetMinorLinkerVersion)
    .def("GetSizeOfCode", &Hades::Memory::NtHeaders::GetSizeOfCode)
    .def("SetSizeOfCode", &Hades::Memory::NtHeaders::SetSizeOfCode)
    .def("GetSizeOfInitializedData", &Hades::Memory::NtHeaders::
      GetSizeOfInitializedData)
    .def("SetSizeOfInitializedData", &Hades::Memory::NtHeaders::
      SetSizeOfInitializedData)
    .def("GetSizeOfUninitializedData", &Hades::Memory::NtHeaders::
      GetSizeOfUninitializedData)
    .def("SetSizeOfUninitializedData", &Hades::Memory::NtHeaders::
      SetSizeOfUninitializedData)
    .def("GetAddressOfEntryPoint", &Hades::Memory::NtHeaders::
      GetAddressOfEntryPoint)
    .def("SetAddressOfEntryPoint", &Hades::Memory::NtHeaders::
      SetAddressOfEntryPoint)
    .def("GetBaseOfCode", &Hades::Memory::NtHeaders::GetBaseOfCode)
    .def("SetBaseOfCode", &Hades::Memory::NtHeaders::SetBaseOfCode)
#if defined(_M_IX86) 
    .def("GetBaseOfData", &Hades::Memory::NtHeaders::GetBaseOfData)
    .def("SetBaseOfData", &Hades::Memory::NtHeaders::SetBaseOfData)
#endif
    .def("GetImageBase", &Hades::Memory::NtHeaders::GetImageBase)
    .def("SetImageBase", &Hades::Memory::NtHeaders::SetImageBase)
    .def("GetSectionAlignment", &Hades::Memory::NtHeaders::GetSectionAlignment)
    .def("SetSectionAlignment", &Hades::Memory::NtHeaders::SetSectionAlignment)
    .def("GetFileAlignment", &Hades::Memory::NtHeaders::GetFileAlignment)
    .def("SetFileAlignment", &Hades::Memory::NtHeaders::SetFileAlignment)
    .def("GetMajorOperatingSystemVersion", &Hades::Memory::NtHeaders::
      GetMajorOperatingSystemVersion)
    .def("SetMajorOperatingSystemVersion", &Hades::Memory::NtHeaders::
      SetMajorOperatingSystemVersion)
    .def("GetMinorOperatingSystemVersion", &Hades::Memory::NtHeaders::
      GetMinorOperatingSystemVersion)
    .def("SetMinorOperatingSystemVersion", &Hades::Memory::NtHeaders::
      SetMinorOperatingSystemVersion)
    .def("GetMajorImageVersion", &Hades::Memory::NtHeaders::
      GetMajorImageVersion)
    .def("SetMajorImageVersion", &Hades::Memory::NtHeaders::
      SetMajorImageVersion)
    .def("GetMinorImageVersion", &Hades::Memory::NtHeaders::
      GetMinorImageVersion)
    .def("SetMinorImageVersion", &Hades::Memory::NtHeaders::
      SetMinorImageVersion)
    .def("GetMajorSubsystemVersion", &Hades::Memory::NtHeaders::
      GetMajorSubsystemVersion)
    .def("SetMajorSubsystemVersion", &Hades::Memory::NtHeaders::
      SetMajorSubsystemVersion)
    .def("GetMinorSubsystemVersion", &Hades::Memory::NtHeaders::
      GetMinorSubsystemVersion)
    .def("SetMinorSubsystemVersion", &Hades::Memory::NtHeaders::
      SetMinorSubsystemVersion)
    .def("GetWin32VersionValue", &Hades::Memory::NtHeaders::
      GetWin32VersionValue)
    .def("SetWin32VersionValue", &Hades::Memory::NtHeaders::
      SetWin32VersionValue)
    .def("GetSizeOfImage", &Hades::Memory::NtHeaders::GetSizeOfImage)
    .def("SetSizeOfImage", &Hades::Memory::NtHeaders::SetSizeOfImage)
    .def("GetSizeOfHeaders", &Hades::Memory::NtHeaders::GetSizeOfHeaders)
    .def("SetSizeOfHeaders", &Hades::Memory::NtHeaders::SetSizeOfHeaders)
    .def("GetCheckSum", &Hades::Memory::NtHeaders::GetCheckSum)
    .def("SetCheckSum", &Hades::Memory::NtHeaders::SetCheckSum)
    .def("GetSubsystem", &Hades::Memory::NtHeaders::GetSubsystem)
    .def("SetSubsystem", &Hades::Memory::NtHeaders::SetSubsystem)
    .def("GetDllCharacteristics", &Hades::Memory::NtHeaders::
      GetDllCharacteristics)
    .def("SetDllCharacteristics", &Hades::Memory::NtHeaders::
      SetDllCharacteristics)
    .def("GetSizeOfStackReserve", &Hades::Memory::NtHeaders::
      GetSizeOfStackReserve)
    .def("SetSizeOfStackReserve", &Hades::Memory::NtHeaders::
      SetSizeOfStackReserve)
    .def("GetSizeOfStackCommit", &Hades::Memory::NtHeaders::
      GetSizeOfStackCommit)
    .def("SetSizeOfStackCommit", &Hades::Memory::NtHeaders::
      SetSizeOfStackCommit)
    .def("GetSizeOfHeapReserve", &Hades::Memory::NtHeaders::
      GetSizeOfHeapReserve)
    .def("SetSizeOfHeapReserve", &Hades::Memory::NtHeaders::
      SetSizeOfHeapReserve)
    .def("GetSizeOfHeapCommit", &Hades::Memory::NtHeaders::GetSizeOfHeapCommit)
    .def("SetSizeOfHeapCommit", &Hades::Memory::NtHeaders::SetSizeOfHeapCommit)
    .def("GetLoaderFlags", &Hades::Memory::NtHeaders::
      GetLoaderFlags)
    .def("SetLoaderFlags", &Hades::Memory::NtHeaders::
      SetLoaderFlags)
    .def("GetNumberOfRvaAndSizes", &Hades::Memory::NtHeaders::
      GetNumberOfRvaAndSizes)
    .def("SetNumberOfRvaAndSizes", &Hades::Memory::NtHeaders::
      SetNumberOfRvaAndSizes)
    .def("GetDataDirectoryVirtualAddress", &Hades::Memory::NtHeaders::
      GetDataDirectoryVirtualAddress)
    .def("SetDataDirectoryVirtualAddress", &Hades::Memory::NtHeaders::
      SetDataDirectoryVirtualAddress)
    .def("GetDataDirectorySize", &Hades::Memory::NtHeaders::
      GetDataDirectorySize)
    .def("SetDataDirectorySize", &Hades::Memory::NtHeaders::
      SetDataDirectorySize)
    .def("GetHeadersRaw", &Hades::Memory::NtHeaders::GetHeadersRaw)
    ;

  boost::python::enum_<Hades::Memory::NtHeaders::DataDir>("DataDir")
    .value("Export", Hades::Memory::NtHeaders::DataDir_Export)
    .value("Import", Hades::Memory::NtHeaders::DataDir_Import)
    .value("Resource", Hades::Memory::NtHeaders::DataDir_Resource)
    .value("Exception", Hades::Memory::NtHeaders::DataDir_Exception)
    .value("Security", Hades::Memory::NtHeaders::DataDir_Security)
    .value("BaseReloc", Hades::Memory::NtHeaders::DataDir_BaseReloc)
    .value("Debug", Hades::Memory::NtHeaders::DataDir_Debug)
    .value("Architecture", Hades::Memory::NtHeaders::DataDir_Architecture)
    .value("GlobalPTR", Hades::Memory::NtHeaders::DataDir_GlobalPTR)
    .value("TLS", Hades::Memory::NtHeaders::DataDir_TLS)
    .value("LoadConfig", Hades::Memory::NtHeaders::DataDir_LoadConfig)
    .value("BoundImport", Hades::Memory::NtHeaders::DataDir_BoundImport)
    .value("IAT", Hades::Memory::NtHeaders::DataDir_IAT)
    .value("DelayImport", Hades::Memory::NtHeaders::DataDir_DelayImport)
    .value("COMDescriptor", Hades::Memory::NtHeaders::DataDir_COMDescriptor)
    ;
}
