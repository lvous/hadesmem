// Copyright (C) 2010-2013 Joshua Boyce.
// See the file COPYING for copying permission.

#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <hadesmem/detail/warning_disable_prefix.hpp>
#include <boost/program_options.hpp>
#include <hadesmem/detail/warning_disable_suffix.hpp>

#include <windows.h>

#include <hadesmem/call.hpp>
#include <hadesmem/config.hpp>
#include <hadesmem/debug_privilege.hpp>
#include <hadesmem/detail/filesystem.hpp>
#include <hadesmem/detail/self_path.hpp>
#include <hadesmem/detail/to_upper_ordinal.hpp>
#include <hadesmem/error.hpp>
#include <hadesmem/injector.hpp>
#include <hadesmem/module.hpp>
#include <hadesmem/process.hpp>
#include <hadesmem/process_entry.hpp>
#include <hadesmem/process_list.hpp>

namespace
{

std::wstring PtrToString(void const* const ptr)
{
  std::wostringstream str;
  str.imbue(std::locale::classic());
  str << std::hex << reinterpret_cast<DWORD_PTR>(ptr);
  return str.str();
}

boost::program_options::options_description GetOptionsDesc()
{
  boost::program_options::options_description opts_desc("General options");
  opts_desc.add_options()("help", "produce help message")(
    "pid", boost::program_options::value<DWORD>(), "target process id")(
    "name",
    boost::program_options::wvalue<std::wstring>(),
    "target process name (running instance)")(
    "name-forced",
    "default to first matched process name "
    "(no warnings about multiple instances)")(
    "module", boost::program_options::wvalue<std::wstring>(), "module path")(
    "path-resolution",
    "perform (local) path resolution on "
    "module path")("add-path", "add module dir to (remote) search order")(
    "export",
    boost::program_options::value<std::string>(),
    "module export name (DWORD_PTR (*) ())")("inject", "inject module")(
    "free", "free module")("run",
                           boost::program_options::wvalue<std::wstring>(),
                           "target process path (new instance)")(
    "arg",
    boost::program_options::wvalue<std::vector<std::wstring>>(),
    "target process args (use once for each arg)")(
    "work-dir",
    boost::program_options::wvalue<std::wstring>(),
    "target process working directory");

  return opts_desc;
}

boost::program_options::variables_map
  ParseOptions(boost::program_options::options_description const& opts_desc)
{
  std::vector<std::wstring> const args =
    boost::program_options::split_winmain(GetCommandLine());
  boost::program_options::variables_map var_map;
  boost::program_options::store(
    boost::program_options::wcommand_line_parser(args).options(opts_desc).run(),
    var_map);
  boost::program_options::notify(var_map);
  return var_map;
}

bool IsOptionSet(std::string const& name,
                 boost::program_options::variables_map const& var_map)
{
  return var_map.count(name) != 0;
}

template <typename T>
T GetOptionValue(std::string const& name,
                 boost::program_options::variables_map const& var_map)
{
  return var_map[name].as<T>();
}

DWORD FindProc(std::wstring const& proc_name, bool name_forced)
{
  std::wstring const proc_name_upper =
    hadesmem::detail::ToUpperOrdinal(proc_name);
  hadesmem::ProcessList proc_list;
  if (name_forced)
  {
    auto const proc_iter =
      std::find_if(std::begin(proc_list),
                   std::end(proc_list),
                   [&](hadesmem::ProcessEntry const& proc_entry)
    {
        return hadesmem::detail::ToUpperOrdinal(proc_entry.GetName()) ==
               proc_name_upper;
      });
    if (proc_iter == std::end(proc_list))
    {
      throw std::runtime_error("Failed to find process.");
    }

    return proc_iter->GetId();
  }
  else
  {
    std::vector<hadesmem::ProcessEntry> found_procs;
    std::copy_if(std::begin(proc_list),
                 std::end(proc_list),
                 std::back_inserter(found_procs),
                 [&](hadesmem::ProcessEntry const& proc_entry)
    {
      return hadesmem::detail::ToUpperOrdinal(proc_entry.GetName()) ==
             proc_name_upper;
    });

    if (found_procs.empty())
    {
      throw std::runtime_error("Failed to find process.");
    }

    if (found_procs.size() > 1)
    {
      throw std::runtime_error(
        "Process name search found multiple "
        "matches. Please specify a PID or use --name-forced.");
    }

    return found_procs.front().GetId();
  }
}
}

int main(int argc, char * /*argv*/ [])
{
  try
  {
    std::cout << "HadesMem Injector [" << HADESMEM_VERSION_STRING << "]\n";

    boost::program_options::options_description const opts_desc =
      GetOptionsDesc();
    boost::program_options::variables_map const var_map =
      ParseOptions(opts_desc);

    if (IsOptionSet("help", var_map) || argc == 1)
    {
      std::cout << '\n' << opts_desc << '\n';
      return 1;
    }

    if (!IsOptionSet("module", var_map))
    {
      throw std::runtime_error("Module path must be specified");
    }

    bool const has_pid = IsOptionSet("pid", var_map);
    bool const create_proc = IsOptionSet("run", var_map);
    bool const has_name = IsOptionSet("name", var_map);
    if ((has_pid && create_proc) || (has_pid && has_name) ||
        (create_proc && has_name))
    {
      throw std::runtime_error(
        "A process ID, process name, or "
        "executable path must be specified, not a combination.");
    }

    if (!has_pid && !has_name && !create_proc)
    {
      throw std::runtime_error("A process ID, process name, or "
                               "executable path must be specified.");
    }

    bool const inject = IsOptionSet("inject", var_map);
    bool const free = IsOptionSet("free", var_map);
    bool const call_export = IsOptionSet("export", var_map);

    if (inject && free)
    {
      throw std::runtime_error("Please specify inject or free, not "
                               "both.");
    }

    if (free && create_proc)
    {
      throw std::runtime_error("Modules can only be unloaded from "
                               "running targets.");
    }

    if (!inject && create_proc)
    {
      throw std::runtime_error(
        "Exports can only be called without "
        "injection on running targets. Did you mean to use "
        "--inject?");
    }

    if (!inject && !free && !call_export)
    {
      throw std::runtime_error("Please choose action(s) to perform on "
                               "the process (inject, free, export).");
    }

    auto const module_path = GetOptionValue<std::wstring>("module", var_map);
    bool const path_resolution = IsOptionSet("path-resolution", var_map);
    bool const add_path = IsOptionSet("add-path", var_map);

    std::uint32_t flags = hadesmem::InjectFlags::kNone;
    if (path_resolution)
    {
      flags |= hadesmem::InjectFlags::kPathResolution;
    }
    if (add_path)
    {
      flags |= hadesmem::InjectFlags::kAddToSearchOrder;
    }

    if (has_pid || has_name)
    {
      try
      {
        hadesmem::GetSeDebugPrivilege();

        std::wcout << "\nAcquired SeDebugPrivilege.\n";
      }
      catch (std::exception const& /*e*/)
      {
        std::wcout << "\nFailed to acquire SeDebugPrivilege.\n";
      }

      std::unique_ptr<hadesmem::Process> process;

      if (has_pid)
      {
        DWORD const pid = GetOptionValue<DWORD>("pid", var_map);
        process = std::make_unique<hadesmem::Process>(pid);
      }
      else
      {
        auto const proc_name = GetOptionValue<std::wstring>("name", var_map);
        bool const name_forced = IsOptionSet("name-forced", var_map);

        // Guard against potential PID reuse race condition. Unlikely
        // to ever happen in practice, but better safe than sorry.
        DWORD proc_pid = 0;
        DWORD proc_pid_2 = 0;
        DWORD retries = 3;
        do
        {
          proc_pid = FindProc(proc_name, name_forced);
          process = std::make_unique<hadesmem::Process>(proc_pid);

          proc_pid_2 = FindProc(proc_name, name_forced);
          hadesmem::Process process_2(proc_pid_2);
        } while (proc_pid != proc_pid_2 && retries--);

        if (proc_pid != proc_pid_2)
        {
          throw std::runtime_error("Could not get handle to target "
                                   "process (PID reuse race).");
        }
      }

      HMODULE module = nullptr;

      if (inject)
      {
        module = hadesmem::InjectDll(*process, module_path, flags);

        std::wcout << "\nSuccessfully injected module at base "
                      "address " << PtrToString(module) << ".\n";
      }
      else
      {
        std::wstring path_real(module_path);
        if (path_resolution && hadesmem::detail::IsPathRelative(path_real))
        {
          path_real = hadesmem::detail::CombinePath(
            hadesmem::detail::GetSelfDirPath(), path_real);
        }

        hadesmem::Module const remote_module(*process, path_real);
        module = remote_module.GetHandle();
      }

      if (call_export)
      {
        auto const export_name = GetOptionValue<std::string>("export", var_map);
        hadesmem::CallResult<DWORD_PTR> const export_ret =
          hadesmem::CallExport(*process, module, export_name);

        std::wcout << "\nSuccessfully called module export.\n";
        std::wcout << "Return: " << export_ret.GetReturnValue() << ".\n";
        std::wcout << "LastError: " << export_ret.GetLastError() << ".\n";
      }

      if (free)
      {
        hadesmem::FreeDll(*process, module);

        std::wcout << "\nSuccessfully freed module at base address "
                   << PtrToString(module) << ".\n";
      }
    }
    else
    {
      auto const exe_path = GetOptionValue<std::wstring>("run", var_map);
      auto const exe_args =
        IsOptionSet("arg", var_map)
          ? GetOptionValue<std::vector<std::wstring>>("arg", var_map)
          : std::vector<std::wstring>();
      auto const export_name =
        IsOptionSet("export", var_map)
          ? GetOptionValue<std::string>("export", var_map)
          : "";
      auto const work_dir =
        IsOptionSet("work-dir", var_map)
          ? GetOptionValue<std::wstring>("work-dir", var_map)
          : L"";
      hadesmem::CreateAndInjectData const inject_data =
        hadesmem::CreateAndInject(exe_path,
                                  work_dir,
                                  std::begin(exe_args),
                                  std::end(exe_args),
                                  module_path,
                                  export_name,
                                  flags);

      std::wcout << "\nSuccessfully created target.\n";
      std::wcout << "Process ID: " << inject_data.GetProcess() << ".\n";
      std::wcout << "Module Base: " << PtrToString(inject_data.GetModule())
                 << ".\n";
      std::wcout << "Export Return: " << inject_data.GetExportRet() << ".\n";
      std::wcout << "Export LastError: " << inject_data.GetExportLastError()
                 << ".\n";
    }

    return 0;
  }
  catch (...)
  {
    std::cerr << "\nError!\n";
    std::cerr << boost::current_exception_diagnostic_information() << '\n';

    return 1;
  }
}
