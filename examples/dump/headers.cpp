// Copyright (C) 2010-2013 Joshua Boyce.
// See the file COPYING for copying permission.

#include "headers.hpp"

#include <iostream>

#include <hadesmem/detail/warning_disable_prefix.hpp>
#include <udis86.h>
#include <hadesmem/detail/warning_disable_suffix.hpp>

#include <hadesmem/config.hpp>
#include <hadesmem/detail/str_conv.hpp>
#include <hadesmem/pelib/dos_header.hpp>
#include <hadesmem/pelib/nt_headers.hpp>
#include <hadesmem/pelib/pe_file.hpp>
#include <hadesmem/process.hpp>

#include "main.hpp"

// TODO: Detect when the file has a writable PE header (both methods). See
// "Writable PE header" in the ReversingLabs "Undocumented PECOFF" whitepaper.

// TODO: Detect using relocations as an obfuscation mechanism. See "File
// encryption via relocations" in ReversingLabs "Undocumented PECOFF" whitepaper
// for more information.

namespace
{

void DumpDosHeader(hadesmem::Process const& process,
                   hadesmem::PeFile const& pe_file)
{
  std::wostream& out = std::wcout;

  WriteNewline(out);
  WriteNormal(out, L"DOS Header:", 1);

  hadesmem::DosHeader dos_hdr(process, pe_file);
  WriteNewline(out);
  WriteNamedHex(out, L"Magic", dos_hdr.GetMagic(), 2);
  WriteNamedHex(out, L"BytesOnLastPage", dos_hdr.GetBytesOnLastPage(), 2);
  WriteNamedHex(out, L"PagesInFile", dos_hdr.GetPagesInFile(), 2);
  WriteNamedHex(out, L"Relocations", dos_hdr.GetRelocations(), 2);
  WriteNamedHex(
    out, L"SizeOfHeaderInParagraphs", dos_hdr.GetSizeOfHeaderInParagraphs(), 2);
  WriteNamedHex(out, L"MinExtraParagraphs", dos_hdr.GetMinExtraParagraphs(), 2);
  WriteNamedHex(out, L"MaxExtraParagraphs", dos_hdr.GetMaxExtraParagraphs(), 2);
  WriteNamedHex(out, L"InitialSS", dos_hdr.GetInitialSS(), 2);
  WriteNamedHex(out, L"InitialSP", dos_hdr.GetInitialSP(), 2);
  WriteNamedHex(out, L"Checksum", dos_hdr.GetChecksum(), 2);
  WriteNamedHex(out, L"InitialIP", dos_hdr.GetInitialIP(), 2);
  WriteNamedHex(out, L"InitialCS", dos_hdr.GetInitialCS(), 2);
  WriteNamedHex(out, L"RelocTableFileAddr", dos_hdr.GetRelocTableFileAddr(), 2);
  WriteNamedHex(out, L"OverlayNum", dos_hdr.GetOverlayNum(), 2);
  auto const reserved_words_1 = dos_hdr.GetReservedWords1();
  WriteNamedHexContainer(out, L"ReservedWords1", reserved_words_1, 2);
  // ReservedWords1 is officially defined as reserved and should be null.
  // However, if non-null it overrides OS version values in the PEB after
  // loading.
  // Sample: winver.exe (Corkami PE Corpus)
  if (std::find_if(std::begin(reserved_words_1),
                   std::end(reserved_words_1),
                   [](WORD w)
  { return !!w; }) != std::end(reserved_words_1))
  {
    WriteNormal(out, L"WARNING! Detected non-zero data in ReservedWords1.", 2);
    WarnForCurrentFile(WarningType::kSuspicious);
  }
  WriteNamedHex(out, L"OEMID", dos_hdr.GetOEMID(), 2);
  WriteNamedHex(out, L"OEMInfo", dos_hdr.GetOEMInfo(), 2);
  WriteNamedHexContainer(
    out, L"ReservedWords2", dos_hdr.GetReservedWords2(), 2);
  // TODO: Detect when e_lfanew is in the overlay and will not be mapped when
  // loaded into memory. See "Self-destructing PE header" in the ReversingLabs
  // "Undocumented PECOFF" whitepaper. Also investigate the second part of that
  // trick in regards to FileAlignment and NtSizeOfHeaders.
  // TODO: Detect when e_lfanew is set in a way that will cause the NT headers
  // to overlap physical and virtual parts of the file, causing an 'on disk'
  // header and an 'in memory' header. See "Dual PE header" in the ReversingLabs
  // "Undocumented PECOFF" whitepaper.
  // TODO: Warn on a non-standard value.
  WriteNamedHex(out, L"NewHeaderOffset", dos_hdr.GetNewHeaderOffset(), 2);
}

std::wstring GetDataDirName(DWORD num)
{
  switch (static_cast<hadesmem::PeDataDir>(num))
  {
  case hadesmem::PeDataDir::Export:
    return L"Export";

  case hadesmem::PeDataDir::Import:
    return L"Import";

  case hadesmem::PeDataDir::Resource:
    return L"Resource";

  case hadesmem::PeDataDir::Exception:
    return L"Exception";

  case hadesmem::PeDataDir::Security:
    return L"Security";

  case hadesmem::PeDataDir::BaseReloc:
    return L"BaseReloc";

  case hadesmem::PeDataDir::Debug:
    return L"Debug";

  case hadesmem::PeDataDir::Architecture:
    return L"Architecture";

  case hadesmem::PeDataDir::GlobalPTR:
    return L"GlobalPTR";

  case hadesmem::PeDataDir::TLS:
    return L"TLS";

  case hadesmem::PeDataDir::LoadConfig:
    return L"LoadConfig";

  case hadesmem::PeDataDir::BoundImport:
    return L"BoundImport";

  case hadesmem::PeDataDir::IAT:
    return L"IAT";

  case hadesmem::PeDataDir::DelayImport:
    return L"DelayImport";

  case hadesmem::PeDataDir::COMDescriptor:
    return L"COMDescriptor";

  case hadesmem::PeDataDir::Reserved:
    return L"Reserved";

  default:
    HADESMEM_DETAIL_ASSERT(false);
    return L"UKNOWN";
  }
}

bool IsUnsupportedDataDir(DWORD num)
{
  switch (static_cast<hadesmem::PeDataDir>(num))
  {
  case hadesmem::PeDataDir::Export:
  case hadesmem::PeDataDir::Import:
  case hadesmem::PeDataDir::TLS:
  case hadesmem::PeDataDir::BoundImport:
    return false;

  case hadesmem::PeDataDir::Resource:
  case hadesmem::PeDataDir::Exception:
  case hadesmem::PeDataDir::Security:
  case hadesmem::PeDataDir::BaseReloc:
  case hadesmem::PeDataDir::Debug:
  case hadesmem::PeDataDir::Architecture:
  case hadesmem::PeDataDir::GlobalPTR:
  case hadesmem::PeDataDir::LoadConfig:
  case hadesmem::PeDataDir::IAT:
  case hadesmem::PeDataDir::DelayImport:
  case hadesmem::PeDataDir::COMDescriptor:
  case hadesmem::PeDataDir::Reserved:
    return true;

  default:
    HADESMEM_DETAIL_ASSERT(false);
    return true;
  }
}

void DumpNtHeaders(hadesmem::Process const& process,
                   hadesmem::PeFile const& pe_file)
{
  std::wostream& out = std::wcout;

  WriteNewline(out);
  WriteNormal(out, L"DOS Header:", 1);

  hadesmem::NtHeaders nt_hdrs(process, pe_file);
  WriteNewline(out);
  WriteNamedHex(out, L"Signature", nt_hdrs.GetSignature(), 2);
  WriteNamedHex(out, L"Machine", nt_hdrs.GetMachine(), 2);
  auto const num_sections = nt_hdrs.GetNumberOfSections();
  WriteNamedHex(out, L"NumberOfSections", num_sections, 2);
  if (!num_sections)
  {
    WriteNormal(out, L"WARNING! No sections.", 2);
    WarnForCurrentFile(WarningType::kSuspicious);
  }
  if (num_sections > 96U)
  {
    WriteNormal(out, L"WARNING! More than 96 sections.", 2);
    WarnForCurrentFile(WarningType::kSuspicious);
  }
  WriteNamedHex(out, L"TimeDateStamp", nt_hdrs.GetTimeDateStamp(), 2);
  WriteNamedHex(
    out, L"PointerToSymbolTable", nt_hdrs.GetPointerToSymbolTable(), 2);
  WriteNamedHex(out, L"NumberOfSymbols", nt_hdrs.GetNumberOfSymbols(), 2);
  // TODO: Detect when SizeOfOptionalHeader has been set to put the section
  // table in unmapped space (e.g. the overlay).
  WriteNamedHex(
    out, L"SizeOfOptionalHeader", nt_hdrs.GetSizeOfOptionalHeader(), 2);
  WriteNamedHex(out, L"Characteristics", nt_hdrs.GetCharacteristics(), 2);
  WriteNamedHex(out, L"Magic", nt_hdrs.GetMagic(), 2);
  WriteNamedHex(out, L"MajorLinkerVersion", nt_hdrs.GetMajorLinkerVersion(), 2);
  WriteNamedHex(out, L"MinorLinkerVersion", nt_hdrs.GetMinorLinkerVersion(), 2);
  WriteNamedHex(out, L"SizeOfCode", nt_hdrs.GetSizeOfCode(), 2);
  WriteNamedHex(
    out, L"SizeOfInitializedData", nt_hdrs.GetSizeOfInitializedData(), 2);
  WriteNamedHex(
    out, L"SizeOfUninitializedData", nt_hdrs.GetSizeOfUninitializedData(), 2);
  // TODO: Detect EP outside of the file (i.e. pointing to another non-relocated
  // module). See "AddressOfEntryPoint" in ReversingLabs "Undocumented PECOFF"
  // for more inforamtion. Also see AddressOfEntryPoint in Corkami PE info.
  // TODO: For valid EPs inside the file, dump the section that it is in, and
  // also disassemble the first N instructions (for some reasonable value of N).
  // TODO: Detect virtual overlap EP. (Sample: virtEP.exe from Corkami)
  DWORD const addr_of_ep = nt_hdrs.GetAddressOfEntryPoint();
  WriteNamedHex(out, L"AddressOfEntryPoint", addr_of_ep, 2);
  // Entry point can be null. For DLLs this is fine, because it simply means the
  // EP is not called, but for non-DLLs it means that execution starts at
  // ImageBase, executing 'MZ' as 'dec ebp/pop edx'.
  // Sample: nullEP.exe (Corkami PE Corpus).
  // The EP can also be null in the case where it is 'patched' via TLS, but this
  // applies to all cases not just when the EP is null (it's just more likely in
  // the case where it's null).
  if (!addr_of_ep && !!(nt_hdrs.GetCharacteristics() & IMAGE_FILE_DLL))
  {
    WriteNormal(out, L"WARNING! Detected zero EP in non-DLL PE.", 2);
    WarnForCurrentFile(WarningType::kSuspicious);
  }
  auto const ep_va = RvaToVa(process, pe_file, addr_of_ep);
  if (addr_of_ep && !ep_va)
  {
    // Not actually unsupported, we just want to identify potential samples for
    // now.
    WriteNormal(out, L"WARNING! Unable to resolve EP to file offset.", 2);
    WarnForCurrentFile(WarningType::kUnsupported);
  }
  if (ep_va)
  {
    ud_t ud_obj;
    ud_init(&ud_obj);
    // TODO: Fix this so we don't risk overflow etc.
    std::size_t size = 0U;
    if (pe_file.GetType() == hadesmem::PeFileType::Data)
    {
      // TODO: Don't read so much unnecessary data. We know the maximum
      // instruction length for the architecture, so we should at least clamp it
      // based on that (and the max number of instructions to disassemble). This
      // could also fail for 'hostile' PE files.
      size = (reinterpret_cast<std::uintptr_t>(pe_file.GetBase()) +
              pe_file.GetSize()) -
             reinterpret_cast<std::uintptr_t>(ep_va);
    }
    else
    {
      // TODO: Fix this.
      auto const mbi = hadesmem::detail::Query(process, ep_va);
      size = reinterpret_cast<std::uintptr_t>(mbi.BaseAddress) +
             mbi.RegionSize - reinterpret_cast<std::uintptr_t>(ep_va);
    }
    ud_set_input_buffer(&ud_obj, static_cast<std::uint8_t*>(ep_va), size);
    ud_set_syntax(&ud_obj, UD_SYN_INTEL);
    ud_set_pc(&ud_obj, nt_hdrs.GetImageBase());
#if defined(_M_AMD64)
    ud_set_mode(&ud_obj, 64);
#elif defined(_M_IX86)
    ud_set_mode(&ud_obj, 32);
#else
#error "[HadesMem] Unsupported architecture."
#endif

    // TODO: Experiment to find the "right" number of instructions to try and
    // disassemble.
    for (std::size_t i = 0; i < 10; ++i)
    {
      std::uint32_t const len = ud_disassemble(&ud_obj);
      if (len == 0)
      {
        WriteNormal(out, L"WARNING! Disassembly failed.", 3);
        WarnForCurrentFile(WarningType::kUnsupported);
        break;
      }

      char const* const asm_str = ud_insn_asm(&ud_obj);
      HADESMEM_DETAIL_ASSERT(asm_str);
      char const* const asm_bytes_str = ud_insn_hex(&ud_obj);
      HADESMEM_DETAIL_ASSERT(asm_bytes_str);
      auto const diasm_line =
        hadesmem::detail::MultiByteToWideChar(asm_str) + L". [" +
        hadesmem::detail::MultiByteToWideChar(asm_bytes_str) + L"].";
      WriteNormal(out, diasm_line, 3);
    }
  }
  WriteNamedHex(out, L"BaseOfCode", nt_hdrs.GetBaseOfCode(), 2);
#if defined(HADESMEM_DETAIL_ARCH_X86)
  WriteNamedHex(out, L"BaseOfData", nt_hdrs.GetBaseOfData(), 2);
#endif
  ULONG_PTR const image_base = nt_hdrs.GetImageBase();
  WriteNamedHex(out, L"ImageBase", image_base, 2);
  // ImageBase can be null under XP. In this case the binary is relocated to
  // 0x10000.
  // Sample: ibnullXP.exe (Corkami PE corpus).
  if (!image_base)
  {
    WriteNormal(out, L"WARNING! Detected zero ImageBase.", 2);
    WarnForCurrentFile(WarningType::kSuspicious);
  }
  // If ImageBase is in the kernel address range it's relocated to 0x1000.
  // Sample: ibkernel.exe (Corkami PE corpus).
  if (nt_hdrs.GetMachine() == IMAGE_FILE_MACHINE_I386 &&
      image_base >= (1UL << 31))
  {
    WriteNormal(out, L"WARNING! Detected kernel space ImageBase.", 2);
    WarnForCurrentFile(WarningType::kSuspicious);
  }
  // Not sure if this is actually possible under x64.
  // TODO: Check whether the image is allowed to load (similar to x86) in this
  // case.
  else if (nt_hdrs.GetMachine() == IMAGE_FILE_MACHINE_AMD64 &&
           image_base >= (0xFFFFULL << 48))
  {
    // User space is 0x00000000`00000000 - 0x0000FFFF`FFFFFFFF
    // Kernel space is 0xFFFF0000`00000000 - 0xFFFFFFFF`FFFFFFFF
    // TODO: Also check the gap?
    WriteNormal(out, L"WARNING! Detected kernel space ImageBase.", 2);
    WarnForCurrentFile(WarningType::kUnsupported);
  }
  // ImageBase must be a multiple of 0x10000
  if (!!(image_base & 0xFFFF))
  {
    WriteNormal(
      out,
      L"WARNING! Detected invalid ImageBase (not a multiple of 0x10000).",
      2);
    WarnForCurrentFile(WarningType::kSuspicious);
  }
  DWORD const section_alignment = nt_hdrs.GetSectionAlignment();
  WriteNamedHex(out, L"SectionAlignment", section_alignment, 2);
  // Sample: bigalign.exe (Corkami PE corpus).
  // Sample: nosection*.exe (Corkami PE corpus).
  if (section_alignment < 0x200 || section_alignment > 0x1000)
  {
    WriteNormal(out, L"WARNING! Unusual section alignment.", 2);
    WarnForCurrentFile(WarningType::kSuspicious);
  }
  DWORD const file_alignment = nt_hdrs.GetFileAlignment();
  WriteNamedHex(out, L"FileAlignment", file_alignment, 2);
  // Sample: bigalign.exe (Corkami PE corpus).
  // Sample: nosection*.exe (Corkami PE corpus).
  if (file_alignment < 0x200 || file_alignment > 0x1000)
  {
    WriteNormal(out, L"WARNING! Unusual file alignment.", 2);
    WarnForCurrentFile(WarningType::kSuspicious);
  }
  if (section_alignment < 0x800 && section_alignment != file_alignment)
  {
    WriteNormal(out, L"WARNING! Unusual alignment.", 2);
    WarnForCurrentFile(WarningType::kUnsupported);
  }
  if (file_alignment > section_alignment)
  {
    WriteNormal(out, L"WARNING! Invalid alignment.", 2);
    WarnForCurrentFile(WarningType::kUnsupported);
  }
  WriteNamedHex(out,
                L"MajorOperatingSystemVersion",
                nt_hdrs.GetMajorOperatingSystemVersion(),
                2);
  WriteNamedHex(out,
                L"MinorOperatingSystemVersion",
                nt_hdrs.GetMinorOperatingSystemVersion(),
                2);
  WriteNamedHex(out, L"MajorImageVersion", nt_hdrs.GetMajorImageVersion(), 2);
  WriteNamedHex(out, L"MinorImageVersion", nt_hdrs.GetMinorImageVersion(), 2);
  WriteNamedHex(
    out, L"MajorSubsystemVersion", nt_hdrs.GetMajorSubsystemVersion(), 2);
  WriteNamedHex(
    out, L"MinorSubsystemVersion", nt_hdrs.GetMinorSubsystemVersion(), 2);
  if (nt_hdrs.GetMajorSubsystemVersion() < 3 ||
      (nt_hdrs.GetMajorSubsystemVersion() == 3 &&
       nt_hdrs.GetMinorSubsystemVersion() < 10))
  {
    WriteNormal(out, L"WARNING! Invalid subsystem version.", 2);
    WarnForCurrentFile(WarningType::kSuspicious);
  }
  WriteNamedHex(out, L"Win32VersionValue", nt_hdrs.GetWin32VersionValue(), 2);
  WriteNamedHex(out, L"SizeOfImage", nt_hdrs.GetSizeOfImage(), 2);
  WriteNamedHex(out, L"SizeOfHeaders", nt_hdrs.GetSizeOfHeaders(), 2);
  WriteNamedHex(out, L"CheckSum", nt_hdrs.GetCheckSum(), 2);
  WriteNamedHex(out, L"Subsystem", nt_hdrs.GetSubsystem(), 2);
  WriteNamedHex(out, L"DllCharacteristics", nt_hdrs.GetDllCharacteristics(), 2);
  WriteNamedHex(out, L"SizeOfStackReserve", nt_hdrs.GetSizeOfStackReserve(), 2);
  WriteNamedHex(out, L"SizeOfStackCommit", nt_hdrs.GetSizeOfStackCommit(), 2);
  WriteNamedHex(out, L"SizeOfHeapReserve", nt_hdrs.GetSizeOfHeapReserve(), 2);
  WriteNamedHex(out, L"SizeOfHeapCommit", nt_hdrs.GetSizeOfHeapCommit(), 2);
  WriteNamedHex(out, L"LoaderFlags", nt_hdrs.GetLoaderFlags(), 2);
  DWORD const num_dirs = nt_hdrs.GetNumberOfRvaAndSizes();
  WriteNamedHex(out, L"NumberOfRvaAndSizes", num_dirs, 2);
  DWORD const num_dirs_clamped = GetNumberOfRvaAndSizesClamped(nt_hdrs);
  WriteNamedHex(out, L"NumberOfRvaAndSizes (Clamped)", num_dirs_clamped, 2);
  if (num_dirs > num_dirs_clamped)
  {
    WriteNormal(
      out, L"WARNING! Detected an invalid number of data directories.", 2);
    WarnForCurrentFile(WarningType::kSuspicious);
  }
  for (DWORD i = 0; i < num_dirs_clamped; ++i)
  {
    auto const data_dir_va = nt_hdrs.GetDataDirectoryVirtualAddress(
      static_cast<hadesmem::PeDataDir>(i));
    auto const data_dir_size =
      nt_hdrs.GetDataDirectorySize(static_cast<hadesmem::PeDataDir>(i));
    auto const data_dir_name = GetDataDirName(i);
    WriteNamedHexSuffix(
      out, L"Data Directory VA", data_dir_va, data_dir_name, 2);
    WriteNamedHexSuffix(
      out, L"Data Directory Size", data_dir_size, data_dir_name, 2);
    if (IsUnsupportedDataDir(i))
    {
      WriteNormal(out,
                  L"WARNING! " + data_dir_name +
                    L" data directory is unsupported.",
                  2);
      WarnForCurrentFile(WarningType::kUnsupported);
    }
  }
}
}

void DumpHeaders(hadesmem::Process const& process,
                 hadesmem::PeFile const& pe_file)
{
  DumpDosHeader(process, pe_file);

  DumpNtHeaders(process, pe_file);
}
