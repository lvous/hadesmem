// Copyright (C) 2010-2013 Joshua Boyce.
// See the file COPYING for copying permission.

#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include <windows.h>

#include <hadesmem/alloc.hpp>
#include <hadesmem/call.hpp>
#include <hadesmem/config.hpp>
#include <hadesmem/detail/argv_quote.hpp>
#include <hadesmem/detail/assert.hpp>
#include <hadesmem/detail/filesystem.hpp>
#include <hadesmem/detail/force_initialize.hpp>
#include <hadesmem/detail/self_path.hpp>
#include <hadesmem/detail/static_assert.hpp>
#include <hadesmem/detail/smart_handle.hpp>
#include <hadesmem/detail/trace.hpp>
#include <hadesmem/error.hpp>
#include <hadesmem/find_procedure.hpp>
#include <hadesmem/module.hpp>
#include <hadesmem/process.hpp>
#include <hadesmem/write.hpp>

// TODO: .NET injection (without DLL dependency if possible).

// TODO: Cross-session injection (also cross-winsta and cross-desktop 
// injection). Easiest solution is to use a broker process via a service 
// and CreateProcessAsUser. Potentially 'better' solution would be to use 
// NtCreateThread/RtlCreateUserThread.

// TODO: Support injection into CSRSS. CreateRemoteThread can't be used on 
// CSRSS because when the thread is initialized it tries to notify CSRSS os 
// the thread creation and gets confused. Potential workaround is to use 
// NtCreateThread/RtlCreateUserThread.

// TODO: Support using NtCreateThread/RtlCreateUserThread. Does not create an 
// activation context, so it will need special work done to get cases like 
// .NET working.

// TODO: WoW64 process native DLL injection.

// TODO: Support injection using only NT APIs (for smss.exe etc).

// TODO: IAT injection (to allow execution of code before Dllmain of other 
// modules are executed). Include support for .NET target processes.

// TODO: Support injection into unitialized processes, native processes, 
// CSRSS, etc.

// TODO: Add a way to easily resume targets created with the kKeepSuspended 
// flag.

// TODO: Add a 'thumbprint' to all memory allocations so the blocks can be 
// easily identified in a debugger.

// TODO: Consolidate memory allocations where possible.

// TODO: Injected code should restrict itself to the NT API only where 
// possible.

namespace hadesmem
{

struct InjectFlags
{
  enum
  {
    kNone = 0, 
    kPathResolution = 1 << 0, 
    kAddToSearchOrder = 1 << 1, 
    kKeepSuspended = 1 << 2, 
    kInvalidFlagMaxValue = 1 << 3
  };
};

inline HMODULE InjectDll(
  Process const& process, 
  std::wstring const& path, 
  std::uint32_t flags)
{
  HADESMEM_DETAIL_ASSERT(!(flags & 
    ~(InjectFlags::kInvalidFlagMaxValue - 1UL)));

  std::wstring path_real(path);

  bool const path_resolution = !!(flags & InjectFlags::kPathResolution);

  if (path_resolution && detail::IsPathRelative(path_real))
  {
    path_real = detail::CombinePath(detail::GetSelfDirPath(), path_real);
  }

  bool const add_path = !!(flags & InjectFlags::kAddToSearchOrder);
  if (add_path && detail::IsPathRelative(path_real))
  {
    HADESMEM_DETAIL_THROW_EXCEPTION(Error() << 
      ErrorString("Cannot modify search order unless an absolute path "
      "or path resolution is used."));
  }

  // Note: Only performing this check when path resolution is enabled, 
  // because otherwise we would need to perform the check in the context 
  // of the remote process, which is not possible to do without 
  // introducing race conditions and other potential problems. So we just 
  // let LoadLibraryExW do the check for us.
  if (path_resolution && !detail::DoesFileExist(path_real))
  {
    HADESMEM_DETAIL_THROW_EXCEPTION(Error() << 
      ErrorString("Could not find module file."));
  }
  
  HADESMEM_DETAIL_TRACE_A("Calling ForceLdrInitializeThunk.\n");

  detail::ForceLdrInitializeThunk(process.GetId());

  HADESMEM_DETAIL_TRACE_FORMAT_W(L"Module path is \"%s\".", path_real.c_str());

  std::size_t const path_buf_size = (path_real.size() + 1) * sizeof(wchar_t);
  
  HADESMEM_DETAIL_TRACE_A("Allocating memory for module path.");

  Allocator const lib_file_remote(process, path_buf_size);
  
  HADESMEM_DETAIL_TRACE_A("Writing memory for module path.");

  WriteString(process, lib_file_remote.GetBase(), path_real);
  
  HADESMEM_DETAIL_TRACE_A("Finding LoadLibraryExW.");

  Module const kernel32_mod(process, L"kernel32.dll");
  auto const load_library = FindProcedure(
    process, 
    kernel32_mod, 
    "LoadLibraryExW");
  
  HADESMEM_DETAIL_TRACE_A("Calling LoadLibraryExW.");

  typedef HMODULE (LoadLibraryExFuncT)(LPCWSTR lpFileName, HANDLE hFile, 
    DWORD dwFlags);
  auto const load_library_ret = Call<LoadLibraryExFuncT>(
    process, 
    reinterpret_cast<FnPtr>(load_library), 
    CallConv::kWinApi, 
    static_cast<LPCWSTR>(lib_file_remote.GetBase()), 
    nullptr, 
    add_path ? LOAD_WITH_ALTERED_SEARCH_PATH : 0UL);
  if (!load_library_ret.GetReturnValue())
  {
    HADESMEM_DETAIL_THROW_EXCEPTION(Error() << 
      ErrorString("LoadLibraryExW failed.") << 
      ErrorCodeWinLast(load_library_ret.GetLastError()));
  }

  return load_library_ret.GetReturnValue();
}

inline void FreeDll(Process const& process, HMODULE module)
{
  Module const kernel32_mod(process, L"kernel32.dll");
  auto const free_library = FindProcedure(
    process, 
    kernel32_mod, 
    "FreeLibrary");

  typedef BOOL (FreeLibraryFuncT)(HMODULE hModule);
  auto const free_library_ret = 
    Call<FreeLibraryFuncT>(
    process, 
    reinterpret_cast<FnPtr>(free_library), 
    CallConv::kWinApi, 
    module);
  if (!free_library_ret.GetReturnValue())
  {
    HADESMEM_DETAIL_THROW_EXCEPTION(Error() << 
      ErrorString("FreeLibrary failed.") << 
      ErrorCodeWinLast(free_library_ret.GetLastError()));
  }
}

// TODO: Configurable timeout. This will complicate resource management 
// however, as we will need to extend the lifetime of the remote memory 
// in case it executes after we time out. Also, if it times out there 
// is no way to try again in the future... Should we just leak the memory 
// on timeout? Return a 'future' object? Some sort of combination? Requires 
// more investigation...
inline CallResult<DWORD_PTR> CallExport(
  Process const& process, 
  HMODULE module, 
  std::string const& export_name)
{
  Module const module_remote(process, module);
  auto const export_ptr = FindProcedure(process, module_remote, export_name);

  return Call<DWORD_PTR()>(process, reinterpret_cast<FnPtr>(export_ptr), 
    CallConv::kDefault);
}

class CreateAndInjectData
{
public:
  explicit CreateAndInjectData(
    Process const& process, 
    HMODULE module, 
    DWORD_PTR export_ret, 
    DWORD export_last_error)
    : process_(process), 
    module_(module), 
    export_ret_(export_ret), 
    export_last_error_(export_last_error)
  { }

#if !defined(HADESMEM_DETAIL_NO_DEFAULTED_FUNCTIONS)

  CreateAndInjectData(CreateAndInjectData const&) 
    HADESMEM_DETAIL_DEFAULTED_FUNCTION;

  CreateAndInjectData& operator=(CreateAndInjectData const&) 
    HADESMEM_DETAIL_DEFAULTED_FUNCTION;

  CreateAndInjectData(CreateAndInjectData&&) 
    HADESMEM_DETAIL_DEFAULTED_FUNCTION;

  CreateAndInjectData& operator=(CreateAndInjectData&&) 
    HADESMEM_DETAIL_DEFAULTED_FUNCTION;

#else // #if !defined(HADESMEM_DETAIL_NO_DEFAULTED_FUNCTIONS)

  CreateAndInjectData(CreateAndInjectData const& other)
    : process_(other.process_), 
    module_(other.module_), 
    export_ret_(other.export_ret_), 
    export_last_error_(other.export_last_error_)
  { }

  CreateAndInjectData& operator=(CreateAndInjectData const& other)
  {
    CreateAndInjectData tmp(other);
    *this = std::move(tmp);

    return *this;
  }

  CreateAndInjectData(CreateAndInjectData&& other) HADESMEM_DETAIL_NOEXCEPT
    : process_(std::move(other.process_)), 
    module_(other.module_), 
    export_ret_(other.export_ret_), 
    export_last_error_(other.export_last_error_)
  { }

  CreateAndInjectData& operator=(CreateAndInjectData&& other) 
    HADESMEM_DETAIL_NOEXCEPT
  {
    process_ = std::move(other.process_);
    module_ = other.module_;
    export_ret_ = other.export_ret_;
    export_last_error_ = other.export_last_error_;

    return *this;
  }

#endif // #if !defined(HADESMEM_DETAIL_NO_DEFAULTED_FUNCTIONS)

  Process GetProcess() const
  {
    return process_;
  }

  HMODULE GetModule() const HADESMEM_DETAIL_NOEXCEPT
  {
    return module_;
  }

  DWORD_PTR GetExportRet() const HADESMEM_DETAIL_NOEXCEPT
  {
    return export_ret_;
  }

  DWORD GetExportLastError() const HADESMEM_DETAIL_NOEXCEPT
  {
    return export_last_error_;
  }

private:
  Process process_;
  HMODULE module_;
  DWORD_PTR export_ret_;
  DWORD export_last_error_;
};

template <typename ArgsIter>
inline CreateAndInjectData CreateAndInject(
  std::wstring const& path, 
  std::wstring const& work_dir, 
  ArgsIter args_beg, 
  ArgsIter args_end, 
  std::wstring const& module, 
  std::string const& export_name, 
  std::uint32_t flags)
{
  HADESMEM_DETAIL_STATIC_ASSERT(std::is_same<std::wstring, 
    typename std::iterator_traits<ArgsIter>::value_type>::value);

  std::wstring command_line;
  detail::ArgvQuote(&command_line, path, false);
  std::for_each(args_beg, args_end, 
    [&] (std::wstring const& arg) 
  {
    command_line += L' ';
    detail::ArgvQuote(&command_line, arg, false);
  });
  std::vector<wchar_t> proc_args(
    std::begin(command_line), 
    std::end(command_line));
  proc_args.push_back(L'\0');

  std::wstring work_dir_real(work_dir);
  if (work_dir_real.empty() && 
    !path.empty() && 
    !detail::IsPathRelative(path))
  {
    std::size_t const separator = path.find_last_of(L"\\/");
    if (separator != std::wstring::npos && 
      separator != path.size() - 1)
    {
      work_dir_real = path.substr(0, separator + 1);
    }
  }

  STARTUPINFO start_info;
  ::ZeroMemory(&start_info, sizeof(start_info));
  start_info.cb = static_cast<DWORD>(sizeof(start_info));
  PROCESS_INFORMATION proc_info;
  ::ZeroMemory(&proc_info, sizeof(proc_info));
  if (!::CreateProcessW(
    path.c_str(), 
    proc_args.data(), 
    nullptr, 
    nullptr, 
    FALSE, 
    CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT, 
    nullptr, 
    work_dir_real.empty() ? nullptr : work_dir_real.c_str(), 
    &start_info, 
    &proc_info))
  {
    DWORD const last_error = ::GetLastError();
    HADESMEM_DETAIL_THROW_EXCEPTION(Error() << 
      ErrorString("CreateProcess failed.") << 
      ErrorCodeWinLast(last_error));
  }

  detail::SmartHandle const proc_handle(proc_info.hProcess);
  detail::SmartHandle const thread_handle(proc_info.hThread);

  try
  {
    Process const process(proc_info.dwProcessId);

    HMODULE const remote_module = InjectDll(process, module, flags);

    CallResult<DWORD_PTR> export_ret(0, 0);
    if (!export_name.empty())
    {
      // TODO: Configurable timeout. This will complicate resource management 
      // however, as we will need to extend the lifetime of the remote memory 
      // in case it executes after we time out. Also, if it times out there 
      // is no way to try again in the future... Should we just leak the memory 
      // on timeout? Return a 'future' object? Some sort of combination? Requires 
      // more investigation...
      export_ret = CallExport(process, remote_module, export_name);
    }

    if (!(flags & InjectFlags::kKeepSuspended))
    {
      if (::ResumeThread(thread_handle.GetHandle()) == static_cast<DWORD>(-1))
      {
        DWORD const last_error = ::GetLastError();
        HADESMEM_DETAIL_THROW_EXCEPTION(Error() << 
          ErrorString("ResumeThread failed.") << 
          ErrorCodeWinLast(last_error) << 
          ErrorCodeWinRet(export_ret.GetReturnValue()) << 
          ErrorCodeWinOther(export_ret.GetLastError()));
      }
    }

    return CreateAndInjectData(
      process, 
      remote_module, 
      export_ret.GetReturnValue(), 
      export_ret.GetLastError());
  }
  catch (std::exception const& /*e*/)
  {
    // Terminate process if injection failed, otherwise the 'zombie' process 
    // would be leaked.
    BOOL const terminated = ::TerminateProcess(proc_handle.GetHandle(), 0);
    (void)terminated;
    HADESMEM_DETAIL_ASSERT(terminated != FALSE);

    throw;
  }
}

}
