// Copyright (C) 2010-2014 Joshua Boyce.
// See the file COPYING for copying permission.

#include "main.hpp"

#include <windows.h>

#include <hadesmem/config.hpp>
#include <hadesmem/detail/self_path.hpp>
#include <hadesmem/detail/region_alloc_size.hpp>
#include <hadesmem/process.hpp>
#include <hadesmem/thread.hpp>
#include <hadesmem/thread_entry.hpp>
#include <hadesmem/thread_helpers.hpp>
#include <hadesmem/thread_list.hpp>

#include "d3d11.hpp"
#include "exception.hpp"
#include "module.hpp"
#include "plugin.hpp"
#include "process.hpp"

// WARNING! Most of this is untested, it's for expository and testing
// purposes only.

namespace
{

// This is a nasty hack to call any APIs which may be called from a static
// destructor. We want to ensure that we call it nice and early, so it's not
// called after we load our plugins, because otherwise it will be destructed
// before the plugin's are automatically unloaded via the static destructor of
// the plugin list, and when plugins try to unregister their callbacks (or
// whatever they're doing) they will go boom. This is a nasty workaround, but
// it's guaranteed by the standard to work, because we always use function local
// statics which are guaranteed to be destructed in a deterministic order.
void UseAllStatics()
{
  // Have to use 'real' callbacks rather than just passing in an empty
  // std::function object because we might not be the only thread running at the
  // moment.
  auto const on_frame_callback = [](IDXGISwapChain* /*swap_chain*/,
                                    ID3D11Device* /*device*/,
                                    ID3D11DeviceContext* /*device_context*/)
  {};
  auto const on_frame_id =
    hadesmem::cerberus::RegisterOnFrameCallback(on_frame_callback);
  hadesmem::cerberus::UnregisterOnFrameCallback(on_frame_id);
}

// Check whether any threads are currently executing code in our module. This
// does not check whether we are on the stack, but that should be handled by the
// ref counting done in all the hooks. This is not foolproof, but it's better
// than nothing and will reduce the potential danger window even further.
bool IsSafeToUnload()
{
  auto const& process = hadesmem::cerberus::GetThisProcess();
  bool safe = true;
  std::size_t retries = 5;
  do
  {
    hadesmem::SuspendedProcess suspend{process.GetId()};
    hadesmem::ThreadList threads{process.GetId()};
    safe = true;
    for (auto const& thread_entry : threads)
    {
      auto const id = thread_entry.GetId();
      if (id == ::GetCurrentThreadId())
      {
        continue;
      }

      hadesmem::Thread const thread{id};
      auto const context = GetThreadContext(thread, CONTEXT_CONTROL);
#if defined(HADESMEM_DETAIL_ARCH_X64)
      auto const ip = reinterpret_cast<void const*>(context.Rip);
#elif defined(HADESMEM_DETAIL_ARCH_X86)
      auto const ip = reinterpret_cast<void const*>(context.Eip);
#else
#error "[HadesMem] Unsupported architecture."
#endif
      HADESMEM_DETAIL_ASSERT(ip);
      auto const this_module =
        reinterpret_cast<std::uint8_t*>(hadesmem::detail::GetHandleToSelf());
      auto const this_module_size = hadesmem::detail::GetRegionAllocSize(
        process, reinterpret_cast<void const*>(this_module));
      if (ip >= this_module && ip < this_module + this_module_size)
      {
        safe = false;
      }
    }
  } while (!safe && retries--);

  return safe;
}
}

namespace hadesmem
{

namespace cerberus
{

Process& GetThisProcess()
{
  static Process process{::GetCurrentProcessId()};
  return process;
}
}
}

extern "C" HADESMEM_DETAIL_DLLEXPORT DWORD_PTR Load() HADESMEM_DETAIL_NOEXCEPT
{
  try
  {
    // Support deferred hooking (via module load notifications).
    hadesmem::cerberus::InitializeD3D11();

    hadesmem::cerberus::DetourNtCreateUserProcess();
    hadesmem::cerberus::DetourNtMapViewOfSection();
    hadesmem::cerberus::DetourNtUnmapViewOfSection();
    hadesmem::cerberus::DetourRtlAddVectoredExceptionHandler();

    hadesmem::cerberus::DetourD3D11(nullptr);
    hadesmem::cerberus::DetourDXGI(nullptr);

    UseAllStatics();

    hadesmem::cerberus::LoadPlugins();

    return 0;
  }
  catch (...)
  {
    HADESMEM_DETAIL_TRACE_A(
      boost::current_exception_diagnostic_information().c_str());
    HADESMEM_DETAIL_ASSERT(false);

    return 1;
  }
}

extern "C" HADESMEM_DETAIL_DLLEXPORT DWORD_PTR Free() HADESMEM_DETAIL_NOEXCEPT
{
  try
  {
    hadesmem::cerberus::UndetourNtCreateUserProcess();
    hadesmem::cerberus::UndetourNtMapViewOfSection();
    hadesmem::cerberus::UndetourNtUnmapViewOfSection();
    hadesmem::cerberus::UndetourRtlAddVectoredExceptionHandler();

    hadesmem::cerberus::UndetourDXGI(true);
    hadesmem::cerberus::UndetourD3D11(true);

    hadesmem::cerberus::UnloadPlugins();

    if (!IsSafeToUnload())
    {
      return 2;
    }

    return 0;
  }
  catch (...)
  {
    HADESMEM_DETAIL_TRACE_A(
      boost::current_exception_diagnostic_information().c_str());
    HADESMEM_DETAIL_ASSERT(false);

    return 1;
  }
}

BOOL WINAPI
  DllMain(HINSTANCE /*instance*/, DWORD /*reason*/, LPVOID /*reserved*/)
  HADESMEM_DETAIL_NOEXCEPT
{
  return TRUE;
}
