// Copyright (C) 2010-2013 Joshua Boyce.
// See the file COPYING for copying permission.

#include "imports.hpp"

#include <iostream>
#include <iterator>

#include <hadesmem/pelib/bound_import_dir.hpp>
#include <hadesmem/pelib/bound_import_dir_list.hpp>
#include <hadesmem/pelib/import_dir.hpp>
#include <hadesmem/pelib/import_dir_list.hpp>
#include <hadesmem/pelib/import_thunk.hpp>
#include <hadesmem/pelib/import_thunk_list.hpp>
#include <hadesmem/pelib/pe_file.hpp>
#include <hadesmem/process.hpp>

#include "main.hpp"

// TODO: Detect imports which simply point back to exports from the same module
// (also detect if the exports are forwarded, and also detect infinite loops).
// Remember that all exports can have the same name, so we need to use the hint
// first, then only use the name if we fail to find a match using the hint. See
// "Import name table" and "Import name hint" in ReversingLabs "Undocumented
// PECOFF" whitepaper for more information.

// TODO: Support old style bound imports and bound forwarded imports.

// TODO: Are any fixes needed to properly support in-memory images, rather than
// just on-disk files?

namespace
{

bool HasBoundImportDir(hadesmem::Process const& process,
                       hadesmem::PeFile const& pe_file)
{
  hadesmem::NtHeaders const nt_headers(process, pe_file);
  // Intentionally not checking whether the RVA is valid, because we will detect
  // an empty list in that case, at which point we want to warn because an
  // invalid RVA is suspicious (even though it won't stop the file from
  // loading).
  return (nt_headers.GetNumberOfRvaAndSizes() >
            static_cast<int>(hadesmem::PeDataDir::BoundImport) &&
          nt_headers.GetDataDirectoryVirtualAddress(
            hadesmem::PeDataDir::BoundImport));
}

bool HasValidNonEmptyBoundImportDescList(hadesmem::Process const& process,
                                         hadesmem::PeFile const& pe_file)
{
  hadesmem::BoundImportDirList bound_import_dirs(process, pe_file);

  return (std::begin(bound_import_dirs) != std::end(bound_import_dirs));
}

void DumpImportThunk(hadesmem::ImportThunk const& thunk, bool is_bound)
{
  std::wostream& out = std::wcout;

  WriteNewline(out);

  bool const by_ordinal = thunk.ByOrdinal();

  // This check needs to be first, because it's possible to have invalid data in
  // the IAT (i.e. -1) which will cause ByOrdinal to be true!
  if (is_bound)
  {
    WriteNamedHex(out, L"Function", thunk.GetFunction(), 3);
  }
  else if (by_ordinal)
  {
    WriteNamedHex(out, L"OrdinalRaw", thunk.GetOrdinalRaw(), 3);
    WriteNamedHex(out, L"Ordinal", thunk.GetOrdinal(), 3);
  }
  else
  {
    try
    {
      WriteNamedHex(out, L"AddressOfData", thunk.GetAddressOfData(), 3);
      WriteNamedHex(out, L"Hint", thunk.GetHint(), 3);
      // TODO: Do something similar to how export names are handled and handle
      // cases where the name is unprintable, extremely long, etc.
      WriteNamedNormal(out, L"Name", thunk.GetName().c_str(), 3);
    }
    catch (std::exception const& /*e*/)
    {
      WriteNormal(out, L"WARNING! Invalid import thunk name data.", 3);
      WarnForCurrentFile(WarningType::kUnsupported);
    }
  }
}
}

// TODO: Fix the code so the bound import out-param is not necessary.
void DumpImports(hadesmem::Process const& process,
                 hadesmem::PeFile const& pe_file,
                 bool& has_new_bound_imports_any)
{
  std::wostream& out = std::wcout;

  hadesmem::ImportDirList import_dirs(process, pe_file);

  if (std::begin(import_dirs) != std::end(import_dirs))
  {
    WriteNewline(out);
    WriteNormal(out, L"Import Dirs:", 1);
  }
  else
  {
    // Currently set to Unsupported rather than Suspicious in order to more
    // quickly identify files with broken RVA resolution (because broken RVA
    // resolution is far more common than actual files with no imports, and even
    // in the case of files with no imports they're typically interesting for
    // other reasons anyway).
    WriteNewline(out);
    WriteNormal(out, L"WARNING! Empty or invalid import directory.", 1);
    WarnForCurrentFile(WarningType::kUnsupported);
  }

  std::uint32_t num_import_dirs = 0U;
  for (auto const& dir : import_dirs)
  {
    WriteNewline(out);

    if (dir.IsVirtualTerminated())
    {
      WriteNormal(
        out,
        L"WARNING! Detected virtual termination trick. Stopping enumeration.",
        2);
      WarnForCurrentFile(WarningType::kSuspicious);
      break;
    }

    if (dir.IsTlsAoiTerminated())
    {
      WriteNormal(out,
                  L"WARNING! Detected TLS AOI trick! Assuming a "
                  L"Windows 7 style loader and stopping enumeration early.",
                  2);
      WarnForCurrentFile(WarningType::kSuspicious);
      break;
    }

    if (dir.IsVirtualBegin())
    {
      WriteNormal(
        out, L"WARNING! Detected virtual descriptor overlap trick.", 2);
      WarnForCurrentFile(WarningType::kSuspicious);
    }

    DWORD const iat = dir.GetFirstThunk();
    bool const iat_valid = !!hadesmem::RvaToVa(process, pe_file, iat);
    DWORD const ilt = dir.GetOriginalFirstThunk();
    bool const use_ilt = !!ilt && ilt != iat;
    hadesmem::ImportThunkList ilt_thunks(process, pe_file, use_ilt ? ilt : iat);
    bool const ilt_empty = std::begin(ilt_thunks) == std::end(ilt_thunks);
    bool const ilt_valid = !!hadesmem::RvaToVa(process, pe_file, ilt);

    // TODO: Come up with a better solution to this.
    if (num_import_dirs++ == 1000)
    {
      WriteNormal(
        out,
        L"WARNING! Processed 1000 import dirs. Stopping early to avoid "
        L"resource exhaustion attacks. Check PE file for TLS AOI trick, "
        L"virtual terminator trick, or other similar attacks.",
        2);
      WarnForCurrentFile(WarningType::kUnsupported);
      break;
    }

    {
      // If the IAT is empty then the descriptor is skipped, and the name can
      // be invalid because it's ignored. Note that we simply skip here rather
      // than terminate, because it's possible to have such 'invalid' entries
      // in-between real entries.
      hadesmem::ImportThunkList iat_thunks(process, pe_file, iat);
      if (std::begin(iat_thunks) == std::end(iat_thunks))
      {
        WriteNormal(out,
                    L"WARNING! IAT is " +
                      std::wstring(iat_valid ? L"empty" : L"invalid") +
                      L". Skipping directory.",
                    2);
        WarnForCurrentFile(WarningType::kSuspicious);
        continue;
      }
    }

    // Apparently it's okay for the ILT to be invalid and 0xFFFFFFFF or 0. This is handled below in our ILT valid/empty checks (after dumping the dir data, but before dumping the thunks).
    // Sample: maxvals.exe (Corkami PE Corpus)
    // Sample: dllmaxvals.dll (Corkami PE Corpus)
    // For anything else though treat the directory as invalid and stop.
    // TODO: Verify this is correct. Probably easiest just to hot-patch the Corkami samples to give them a random invalid RVA and see if they still run.
    if (!ilt_valid && ilt != 0xFFFFFFFF && ilt != 0)
    {
      // TODO: Come up with a less stupid message for this.
      WriteNormal(
        out, L"WARNING! ILT is extra invalid. Stopping enumeration.", 2);
      WarnForCurrentFile(WarningType::kUnsupported);
      break;
    }

    WriteNamedHex(out, L"OriginalFirstThunk", dir.GetOriginalFirstThunk(), 2);
    DWORD const time_date_stamp = dir.GetTimeDateStamp();
    std::wstring time_date_stamp_str;
    if (!ConvertTimeStamp(time_date_stamp, time_date_stamp_str))
    {
      WriteNormal(out, L"WARNING! Invalid timestamp.", 2);
      WarnForCurrentFile(WarningType::kSuspicious);
    }
    WriteNamedHexSuffix(
      out, L"TimeDateStamp", time_date_stamp, time_date_stamp_str, 2);
    bool has_new_bound_imports = (time_date_stamp == static_cast<DWORD>(-1));
    if (has_new_bound_imports)
    {
      has_new_bound_imports_any = true;
    }
    bool has_old_bound_imports = (!has_new_bound_imports && time_date_stamp);
    if (has_new_bound_imports)
    {
      // Don't just check whether the ILT is invalid, but also ensure that
      // there's a valid bound import dir. In the case where the bound import
      // dir is invalid we just treat the IAT as the ILT on disk. See
      // dllmaxvals.dll for a PE file which has TimeDateStamp of 0xFFFFFFFF, no
      // ILT, and no bound import dir.
      // TODO: Is this allowed? I guess this is legal in the case where you
      // bind a DLL that doesn't have an ILT to begin wtih, at which point it
      // won't load if the bindings don't match, but we need to confirm this.
      // Warn so we can find samples for further investigation.
      if (!ilt_valid && HasValidNonEmptyBoundImportDescList(process, pe_file))
      {
        WriteNormal(out,
                    L"WARNING! Detected new style bound imports "
                    L"with an invalid ILT. Currently unhandled.",
                    2);
        WarnForCurrentFile(WarningType::kUnsupported);
      }
    }
    DWORD const forwarder_chain = dir.GetForwarderChain();
    WriteNamedHex(out, L"ForwarderChain", forwarder_chain, 2);
    if (forwarder_chain == static_cast<DWORD>(-1))
    {
      if (has_old_bound_imports)
      {
        // Not sure how common this is or if it's even allowed. I think it
        // probably just gets ignored by the loader, but mark as unsupported to
        // identify potential samples just in case.
        WriteNormal(out,
                    L"WARNING! Detected new style forwarder chain with "
                    L"no new style bound imports. Currently unhandled.",
                    2);
        WarnForCurrentFile(WarningType::kUnsupported);
      }

      if (!time_date_stamp)
      {
        // Not sure how common this is or if it's even allowed. I think it
        // probably just gets ignored by the loader, but mark as unsupported to
        // identify potential samples just in case.
        WriteNormal(out,
                    L"WARNING! Detected new style forwarder chain "
                    L"with no bound imports. Currently unhandled.",
                    2);
        WarnForCurrentFile(WarningType::kUnsupported);
      }
    }
    if (forwarder_chain != 0 && forwarder_chain != static_cast<DWORD>(-1))
    {
      if (has_new_bound_imports)
      {
        // Not sure how common this is or if it's even allowed. I think it
        // probably just gets ignored by the loader, but mark as unsupported to
        // identify potential samples just in case.
        WriteNormal(out,
                    L"WARNING! Detected old style forwarder chain "
                    L"with new bound imports. Currently unhandled.",
                    2);
        WarnForCurrentFile(WarningType::kUnsupported);
      }
      else if (has_old_bound_imports)
      {
        WriteNormal(out,
                    L"WARNING! Detected old style forwarder chain "
                    L"with old bound imports. Currently unhandled.",
                    2);
        WarnForCurrentFile(WarningType::kUnsupported);
      }
      else
      {
        // Not sure how common this is or if it's even allowed. I think it
        // probably just gets ignored by the loader, but mark as unsupported to
        // identify potential samples just in case.
        WriteNormal(out,
                    L"WARNING! Detected old style forwarder chain "
                    L"with no bound imports. Currently unhandled.",
                    2);
        WarnForCurrentFile(WarningType::kUnsupported);
      }
    }
    WriteNamedHex(out, L"Name (Raw)", dir.GetNameRaw(), 2);
    try
    {
      // TODO: Detect and handle the case where the string is terminated
      // virtually.

      // Treat anything with unprintable characters as invalid. Mark it as
      // unsupported because unlike export names the import names are actually
      // used, so either the file is invalid or we're not parsing it correctly.
      // TODO: Fix perf for extremely long names. Instead of reading
      // indefinitely and then checking the size after the fact, we should
      // perform a bounded read.
      auto imp_desc_name = dir.GetName();
      if (!IsPrintableClassicLocale(imp_desc_name))
      {
        // TODO: Truncate instead of using an empty name.
        WriteNormal(out,
                    L"WARNING! Detected unprintable import descriptor name. "
                    L"Using empty name instead.",
                    2);
        WarnForCurrentFile(WarningType::kUnsupported);
        imp_desc_name = "";
      }
      // Treat anything longer than 1KB as invalid. Mark it as unsupported
      // because unlike export names the import names are actually used; so
      // either the file is invalid or we're not parsing it correctly.
      else if (imp_desc_name.size() > 1024)
      {
        // TODO: Truncate instead of using an empty name.
        WriteNormal(out,
                    L"WARNING! Import descriptor name is suspiciously long. "
                    L"Using empty name instead.",
                    2);
        WarnForCurrentFile(WarningType::kUnsupported);
        imp_desc_name = "";
      }
      WriteNamedNormal(out, L"Name", imp_desc_name.c_str(), 2);
    }
    catch (std::exception const& /*e*/)
    {
      WriteNormal(out, L"WARNING! Failed to read import dir name.", 2);
      WarnForCurrentFile(WarningType::kUnsupported);
    }
    WriteNamedHex(out, L"FirstThunk", dir.GetFirstThunk(), 2);

    // TODO: Parse the IAT and ILT in parallel, in order to easily detect when
    // imports are bound in-memory. This will also mean we no longer need to
    // count the length of the ILT in order to terminate the IAT pass early.

    if (ilt_empty)
    {
      // Has to be the ILT if we get here because we did a check for an
      // empty/invalid IAT earlier on.

      if (!ilt_valid)
      {
        WriteNewline(out);
        WriteNormal(out, L"WARNING! ILT is invalid.", 2);
        WarnForCurrentFile(WarningType::kSuspicious);
      }
      else
      {
        WriteNewline(out);
        WriteNormal(out, L"WARNING! ILT is empty.", 2);
        WarnForCurrentFile(WarningType::kUnsupported);
      }
    }
    else
    {
      WriteNewline(out);
      WriteNormal(out,
                  L"Import Thunks (" + std::wstring(use_ilt ? L"ILT" : L"IAT") +
                    L")",
                  2);
    }

    // TODO: Distinguish between new and old binding styles and handle
    // appropriately.
    // TODO: Detect when the import dir says it is bound with the new style, but
    // the file does not have a valid bound import dir. In this case it seems to
    // be ignored by the loader. We should warn for this, but we probably can't
    // change the way we interpret the data, because just because there's no
    // bound import dir doesn't mean the IAT contains legitimate un-bound data,
    // it could just be complete garbage. Need to confirm this though...
    bool const is_bound = !!dir.GetTimeDateStamp();
    // Assume that any PE files mapped as images in memory have had their
    // imports resolved.
    bool const is_memory_bound =
      (pe_file.GetType() == hadesmem::PeFileType::Image) && !use_ilt;
    bool const is_ilt_bound = (is_bound && !use_ilt) || is_memory_bound;
    bool const is_iat_bound =
      is_bound || (pe_file.GetType() == hadesmem::PeFileType::Image);
    std::size_t count = 0U;
    for (auto const& thunk : ilt_thunks)
    {
      // TODO: Come up with a better solution to this.
      if (count++ == 1000)
      {
        WriteNewline(out);
        WriteNormal(
          out,
          L"WARNING! Processed 1000 import thunks. Stopping early to "
          L"avoid resource exhaustion attacks. Check PE file for TLS AOI "
          L"trick, virtual terminator trick, or other similar attacks.",
          2);
        WarnForCurrentFile(WarningType::kUnsupported);
        break;
      }

      // TODO: Should probably revert to using 'is_ilt_bound' instead of
      // hardcoding false, but is it even legal to have a module that uses old
      // style bindings with no ILT? Need to investigate, because it seems
      // you're allowed to have modules like that when they're not actually
      // bound, and the loader simply detects that the TimeDateStamp doesn't
      // match and so treats the IAT as unbound? Investigate this further.
      (void)is_ilt_bound;
      // In the case where we have an already mapped image with no ILT, the
      // original name/ordinal inforamtion is long gone so all we have to work
      // from in the IAT (which is bound).
      bool const is_image_iat =
        (pe_file.GetType() == hadesmem::PeFileType::Image && !use_ilt);
      DumpImportThunk(thunk, is_image_iat);
    }

    // Windows will load PE files that have an invalid RVA for the ILT (lies
    // outside of the virtual space), and will fall back to the IAT in this
    // case.
    if (use_ilt && iat)
    {
      hadesmem::ImportThunkList iat_thunks(
        process, pe_file, dir.GetFirstThunk());
      if (std::begin(iat_thunks) != std::end(iat_thunks))
      {
        WriteNewline(out);
        WriteNormal(out, L"Import Thunks (IAT)", 2);
      }
      for (auto const& thunk : iat_thunks)
      {
        if (ilt_valid && !count--)
        {
          WriteNewline(out);
          WriteNormal(out,
                      L"WARNING! IAT size does not match ILT size. Stopping "
                      L"IAT enumeration early.",
                      2);
          WarnForCurrentFile(WarningType::kSuspicious);
          break;
        }

        // If the ILT is not empty (empty includes invalid) we simply treat the
        // IAT as bound, regardless of whether it actually is. This is because
        // apparently as long as you have a valid ILT you can put whatever the
        // hell you want in the IAT, because it's going to be overwitten anyway.
        // See tinynet.exe from the Corkami PE corpus for an example.
        // Furthermore, we only treat the IAT as bound if the ILT is also valid.
        // Not sure if this is correct, but apparently it's possible to have a
        // module with the TimeDateStamp set, indicating that the module is
        // bound, even though it actually isn't (and XP will apparently load
        // such a module). See tinygui.exe from the Corkami PE corpus for an
        // example.
        // TODO: Confirm this is correct.
        DumpImportThunk(thunk, (is_iat_bound && ilt_valid) || !ilt_empty);
      }
    }
  }
}

void DumpBoundImports(hadesmem::Process const& process,
                      hadesmem::PeFile const& pe_file,
                      bool has_new_bound_imports_any)
{
  std::wostream& out = std::wcout;

  // TODO: Add similar checks elsewhere to reduce unnecessary warnings?
  if (!HasBoundImportDir(process, pe_file))
  {
    // Not sure if this is allowed or not. Mark as unsupported in order to
    // quickly identify potential samples.
    if (has_new_bound_imports_any)
    {
      WriteNewline(out);
      WriteNormal(
        out,
        L"WARNING! No bound import directory on file with an import dir "
        L"indicating the presence of a bound import dir.",
        1);
      WarnForCurrentFile(WarningType::kUnsupported);
    }

    return;
  }

  if (!has_new_bound_imports_any)
  {
    WriteNewline(out);
    WriteNormal(
      out,
      L"WARNING! Seemingly valid bound import directory on file with an "
      L"import dir indicating no new bound import dir.",
      1);
    WarnForCurrentFile(WarningType::kSuspicious);
    return;
  }

  hadesmem::BoundImportDirList bound_import_dirs(process, pe_file);

  if (std::begin(bound_import_dirs) != std::end(bound_import_dirs))
  {
    WriteNewline(out);
    WriteNormal(out, L"Bound Import Dirs:", 1);
  }
  else
  {
    WriteNewline(out);
    WriteNormal(out, L"WARNING! Empty or invalid bound import directory.", 1);
    WarnForCurrentFile(WarningType::kSuspicious);
  }

  // TODO: Warn and bail after processing N entries (similar to imports).
  for (auto const& dir : bound_import_dirs)
  {
    WriteNewline(out);

    DWORD const time_date_stamp = dir.GetTimeDateStamp();
    std::wstring time_date_stamp_str;
    if (!ConvertTimeStamp(time_date_stamp, time_date_stamp_str))
    {
      WriteNormal(out, L"WARNING! Invalid timestamp.", 2);
      WarnForCurrentFile(WarningType::kSuspicious);
    }
    WriteNamedHexSuffix(
      out, L"TimeDateStamp", time_date_stamp, time_date_stamp_str, 2);
    WriteNamedHex(out, L"OffsetModuleName", dir.GetOffsetModuleName(), 2);
    WriteNamedNormal(out, L"ModuleName", dir.GetModuleName().c_str(), 2);
    WriteNamedHex(out,
                  L"NumberOfModuleForwarderRefs",
                  dir.GetNumberOfModuleForwarderRefs(),
                  2);
    // TODO: Add a proper API for this.
    auto const forwarder_refs = dir.GetModuleForwarderRefs();
    if (std::begin(forwarder_refs) != std::end(forwarder_refs))
    {
      WriteNormal(out, L"Module Forwarder Refs:", 2);
    }
    for (auto const& forwarder : forwarder_refs)
    {
      DWORD const fwd_time_date_stamp = forwarder.TimeDateStamp;
      std::wstring fwd_time_date_stamp_str;
      if (!ConvertTimeStamp(fwd_time_date_stamp, fwd_time_date_stamp_str))
      {
        WriteNormal(out, L"WARNING! Invalid timestamp.", 2);
        WarnForCurrentFile(WarningType::kSuspicious);
      }
      WriteNamedHexSuffix(
        out, L"TimeDateStamp", fwd_time_date_stamp, fwd_time_date_stamp_str, 2);
      WriteNamedHex(out, L"OffsetModuleName", forwarder.OffsetModuleName, 2);
      WriteNamedNormal(out,
                       L"ModuleName",
                       dir.GetNameForModuleForwarderRef(forwarder).c_str(),
                       2);
      WriteNamedHex(out, L"Reserved", forwarder.Reserved, 2);
    }
  }
}
