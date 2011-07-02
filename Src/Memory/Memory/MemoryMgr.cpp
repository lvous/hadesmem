/*
This file is part of HadesMem.
Copyright (C) 2011 Joshua Boyce (a.k.a. RaptorFactor).
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

// Hades
#include <HadesMemory/MemoryMgr.hpp>
#include <HadesMemory/Detail/Config.hpp>
#include <HadesMemory/Detail/EnsureCleanup.hpp>

// Boost
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

// AsmJit
#ifdef HADES_MSVC
#pragma warning(push, 1)
#endif
#ifdef HADES_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#pragma GCC diagnostic ignored "-Wattributes"
#pragma GCC diagnostic ignored "-Wparentheses"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <AsmJit/AsmJit.h>
#ifdef HADES_MSVC
#pragma warning(pop)
#endif
#ifdef HADES_GCC
#pragma GCC diagnostic pop
#endif

// Windows API
#include <Windows.h>
#include <TlHelp32.h>

namespace HadesMem
{
  // Open process from process ID
  MemoryMgr::MemoryMgr(DWORD ProcID) 
    : m_Process(ProcID) 
  { }
  
  namespace
  {
    // Function to find Kernel32.dll
    HMODULE GetKernel32(MemoryMgr const& MyMemory)
    {
      // Get module snapshot
      Detail::EnsureCloseSnap Snap = CreateToolhelp32Snapshot(
        TH32CS_SNAPMODULE, MyMemory.GetProcessId());
      if (Snap == INVALID_HANDLE_VALUE)
      {
        DWORD const LastError = GetLastError();
        BOOST_THROW_EXCEPTION(MemoryMgr::Error() <<  
          ErrorFunction("MemoryMgr::Call") << 
          ErrorString("Could not get module snapshot.") << 
          ErrorCodeWinLast(LastError));
      }
      
      // Start module enumeration
      MODULEENTRY32 ModEntry;
      ZeroMemory(&ModEntry, sizeof(ModEntry));
      ModEntry.dwSize = sizeof(ModEntry);
      if (!Module32First(Snap, &ModEntry))
      {
        DWORD const LastError = GetLastError();
        BOOST_THROW_EXCEPTION(MemoryMgr::Error() <<  
          ErrorFunction("MemoryMgr::Call") << 
          ErrorString("Could not get first module in list.") << 
          ErrorCodeWinLast(LastError));
      }
      
      // Enumerate module list
      do
      {
        // Find Kernel32.dll
        if (boost::to_lower_copy(static_cast<std::wstring>(
          ModEntry.szModule)) == L"kernel32.dll")
        {
          return ModEntry.hModule;
        }
      } while(Module32Next(Snap, &ModEntry));
      
      // If module enumeration stopped for a reason other than EOL return 
      // an error.
      DWORD const LastError = GetLastError();
      if (LastError != ERROR_NO_MORE_FILES)
      {
        BOOST_THROW_EXCEPTION(MemoryMgr::Error() <<  
          ErrorFunction("MemoryMgr::Call") << 
          ErrorString("Module enumeration failed.") << 
          ErrorCodeWinLast(LastError));
      }
      
      // If we reached EOL then return a different error
      BOOST_THROW_EXCEPTION(MemoryMgr::Error() << 
        ErrorFunction("MemoryMgr::Call") << 
        ErrorString("Could not find Kernel32.dll."));
    }
  }

  // Call remote function
  MemoryMgr::RemoteFunctionRet MemoryMgr::Call(LPCVOID Address, 
    CallConv MyCallConv, std::vector<PVOID> const& Args) const 
  {
    // Get number of arguments
    std::size_t const NumArgs = Args.size();

    // Create Assembler.
    AsmJit::Assembler MyJitFunc;

    // Allocate memory for return value
    AllocAndFree const ReturnValueRemote(*this, sizeof(DWORD_PTR));

    // Allocate memory for 64-bit return value
    AllocAndFree const ReturnValue64Remote(*this, sizeof(DWORD64));

    // Allocate memory for floating point return value
    AllocAndFree const ReturnValueFloatRemote(*this, sizeof(double));

    // Allocate memory for thread's last-error code
    AllocAndFree const LastErrorRemote(*this, sizeof(DWORD));

    // Get address of Kernel32.dll
    HMODULE const K32Mod = GetKernel32(*this);
    // Get address of kernel32.dll!GetLastError and 
    // kernel32.dll!SetLastError
    DWORD_PTR const pGetLastError = reinterpret_cast<DWORD_PTR>(
      GetRemoteProcAddress(K32Mod, L"kernel32.dll", "GetLastError"));
    DWORD_PTR const pSetLastError = reinterpret_cast<DWORD_PTR>(
      GetRemoteProcAddress(K32Mod, L"kernel32.dll", "SetLastError"));

#if defined(_M_AMD64) 
    // Check calling convention
    if (MyCallConv != CallConv_X64 && MyCallConv != CallConv_Default)
    {
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("MemoryMgr::Call") << 
        ErrorString("Invalid calling convention."));
    }

    // Prologue
    MyJitFunc.push(AsmJit::rbp);
    MyJitFunc.mov(AsmJit::rbp, AsmJit::rsp);

    // Allocate ghost space
    MyJitFunc.sub(AsmJit::rsp, AsmJit::Imm(0x20));

    // Call kernel32.dll!SetLastError
    MyJitFunc.mov(AsmJit::rcx, 0);
    MyJitFunc.mov(AsmJit::rax, pSetLastError);
    MyJitFunc.call(AsmJit::rax);

    // Cleanup ghost space
    MyJitFunc.add(AsmJit::rsp, AsmJit::Imm(0x20));

    // Set up first 4 parameters
    MyJitFunc.mov(AsmJit::rcx, NumArgs > 0 ? reinterpret_cast<DWORD_PTR>(
      Args[0]) : 0);
    MyJitFunc.mov(AsmJit::rdx, NumArgs > 1 ? reinterpret_cast<DWORD_PTR>(
      Args[1]) : 0);
    MyJitFunc.mov(AsmJit::r8, NumArgs > 2 ? reinterpret_cast<DWORD_PTR>(
      Args[2]) : 0);
    MyJitFunc.mov(AsmJit::r9, NumArgs > 3 ? reinterpret_cast<DWORD_PTR>(
      Args[3]) : 0);

    // Handle remaining parameters (if any)
    if (NumArgs > 4)
    {
      std::for_each(Args.crbegin(), Args.crend() - 4, 
        [&MyJitFunc] (PVOID Arg)
      {
        MyJitFunc.mov(AsmJit::rax, reinterpret_cast<DWORD_PTR>(Arg));
        MyJitFunc.push(AsmJit::rax);
      });
    }

    // Allocate ghost space
    MyJitFunc.sub(AsmJit::rsp, AsmJit::Imm(0x20));

    // Call target
    MyJitFunc.mov(AsmJit::rax, reinterpret_cast<DWORD_PTR>(Address));
    MyJitFunc.call(AsmJit::rax);
    
    // Cleanup ghost space
    MyJitFunc.add(AsmJit::rsp, AsmJit::Imm(0x20));

    // Clean up remaining stack space
    MyJitFunc.add(AsmJit::rsp, 0x8 * (NumArgs - 4));

    // Write return value to memory
    MyJitFunc.mov(AsmJit::rcx, reinterpret_cast<DWORD_PTR>(
      ReturnValueRemote.GetBase()));
    MyJitFunc.mov(AsmJit::dword_ptr(AsmJit::rcx), AsmJit::rax);

    // Write 64-bit return value to memory
    MyJitFunc.mov(AsmJit::rcx, reinterpret_cast<DWORD_PTR>(
      ReturnValue64Remote.GetBase()));
    MyJitFunc.mov(AsmJit::dword_ptr(AsmJit::rcx), AsmJit::rax);

    // Write floating point return value to memory
    MyJitFunc.mov(AsmJit::rcx, reinterpret_cast<DWORD_PTR>(
      ReturnValueFloatRemote.GetBase()));
    MyJitFunc.movq(AsmJit::dword_ptr(AsmJit::rcx), AsmJit::xmm0);

    // Call kernel32.dll!GetLastError
    MyJitFunc.mov(AsmJit::rax, pGetLastError);
    MyJitFunc.call(AsmJit::rax);
    
    // Write error code to memory
    MyJitFunc.mov(AsmJit::rcx, reinterpret_cast<DWORD_PTR>(
      LastErrorRemote.GetBase()));
    MyJitFunc.mov(AsmJit::dword_ptr(AsmJit::rcx), AsmJit::rax);

    // Epilogue
    MyJitFunc.mov(AsmJit::rsp, AsmJit::rbp);
    MyJitFunc.pop(AsmJit::rbp);

    // Return
    MyJitFunc.ret();
#elif defined(_M_IX86) 
    // Prologue
    MyJitFunc.push(AsmJit::ebp);
    MyJitFunc.mov(AsmJit::ebp, AsmJit::esp);

    // Call kernel32.dll!SetLastError
    MyJitFunc.push(AsmJit::Imm(0x0));
    MyJitFunc.mov(AsmJit::eax, pSetLastError);
    MyJitFunc.call(AsmJit::eax);

    // Get stack arguments offset
    std::size_t StackArgOffs = 0;
    switch (MyCallConv)
    {
    case CallConv_THISCALL:
      StackArgOffs = 1;
      break;

    case CallConv_FASTCALL:
      StackArgOffs = 2;
      break;

    case CallConv_CDECL:
    case CallConv_STDCALL:
    case CallConv_Default:
      StackArgOffs = 0;
      break;

    default:
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("MemoryMgr::Call") << 
        ErrorString("Invalid calling convention."));
    }

    // Pass first arg in through ECX if 'thiscall' is specified
    if (MyCallConv == CallConv_THISCALL)
    {
      MyJitFunc.mov(AsmJit::ecx, NumArgs ? reinterpret_cast<DWORD_PTR>(
        Args[0]) : 0);
    }

    // Pass first two args in through ECX and EDX if 'fastcall' is specified
    if (MyCallConv == CallConv_FASTCALL)
    {
      MyJitFunc.mov(AsmJit::ecx, NumArgs ? reinterpret_cast<DWORD_PTR>(
        Args[0]) : 0);
      MyJitFunc.mov(AsmJit::edx, NumArgs > 1 ? reinterpret_cast<DWORD_PTR>(
        Args[1]) : 0);
    }

    // Pass all remaining args on stack if there are any left to process.
    if (NumArgs > StackArgOffs)
    {
      std::for_each(Args.crbegin(), Args.crend() - StackArgOffs, 
        [&] (PVOID Arg)
      {
        MyJitFunc.mov(AsmJit::eax, reinterpret_cast<DWORD_PTR>(Arg));
        MyJitFunc.push(AsmJit::eax);
      });
    }
    
    // Call target
    MyJitFunc.mov(AsmJit::eax, reinterpret_cast<DWORD_PTR>(Address));
    MyJitFunc.call(AsmJit::eax);
    
    // Write return value to memory
    MyJitFunc.mov(AsmJit::ecx, reinterpret_cast<DWORD_PTR>(
      ReturnValueRemote.GetBase()));
    MyJitFunc.mov(AsmJit::dword_ptr(AsmJit::ecx), AsmJit::eax);
    
    // Write 64-bit return value to memory
    MyJitFunc.mov(AsmJit::ecx, reinterpret_cast<DWORD_PTR>(
      ReturnValue64Remote.GetBase()));
    MyJitFunc.mov(AsmJit::dword_ptr(AsmJit::ecx), AsmJit::eax);
    MyJitFunc.mov(AsmJit::dword_ptr(AsmJit::ecx, 4), AsmJit::edx);
    
    // Write floating point return value to memory
    MyJitFunc.mov(AsmJit::ecx, reinterpret_cast<DWORD_PTR>(
      ReturnValueFloatRemote.GetBase()));
    MyJitFunc.fst(AsmJit::qword_ptr(AsmJit::ecx));
    
    // Call kernel32.dll!GetLastError
    MyJitFunc.mov(AsmJit::eax, pGetLastError);
    MyJitFunc.call(AsmJit::eax);
    
    // Write return value to memory
    MyJitFunc.mov(AsmJit::ecx, reinterpret_cast<DWORD_PTR>(
      LastErrorRemote.GetBase()));
    MyJitFunc.mov(AsmJit::dword_ptr(AsmJit::ecx), AsmJit::eax);
    
    // Clean up stack if necessary
    if (MyCallConv == CallConv_CDECL)
    {
      MyJitFunc.add(AsmJit::esp, AsmJit::Imm(NumArgs * sizeof(PVOID)));
    }

    // Epilogue
    MyJitFunc.mov(AsmJit::esp, AsmJit::ebp);
    MyJitFunc.pop(AsmJit::ebp);

    // Return
    MyJitFunc.ret(AsmJit::Imm(0x4));
#else 
#error "[HadesMem] Unsupported architecture."
#endif

    // Get stub size
    DWORD_PTR const StubSize = MyJitFunc.getCodeSize();

    // Allocate memory for stub buffer
    AllocAndFree const StubMemRemote(*this, StubSize);
    PBYTE const pRemoteStub = static_cast<PBYTE>(StubMemRemote.GetBase());
    DWORD_PTR const pRemoteStubTemp = reinterpret_cast<DWORD_PTR>(
      pRemoteStub);

    // Create buffer to hold relocated code plus the return value address
    std::vector<BYTE> CodeReal(StubSize);

    // Generate code
    MyJitFunc.relocCode(CodeReal.data(), reinterpret_cast<DWORD_PTR>(
      pRemoteStub));

    // Write stub buffer to process
    Write(pRemoteStub, CodeReal);

    // Call stub via creating a remote thread in the target.
    Detail::EnsureCloseHandle const MyThread(CreateRemoteThread(m_Process.
      GetHandle(), nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(
      pRemoteStubTemp), nullptr, 0, nullptr));
    if (!MyThread)
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("MemoryMgr::Call") << 
        ErrorString("Could not create remote thread.") << 
        ErrorCodeWinLast(LastError));
    }

    // Wait for the remote thread to terminate
    if (WaitForSingleObject(MyThread, INFINITE) != WAIT_OBJECT_0)
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("MemoryMgr::Call") << 
        ErrorString("Could not wait for remote thread.") << 
        ErrorCodeWinLast(LastError));
    }

    // Forward return value from remote thread
    DWORD_PTR const RetVal = Read<DWORD_PTR>(ReturnValueRemote.GetBase());
    DWORD const ErrorCode = Read<DWORD>(LastErrorRemote.GetBase());
    DWORD64 const RetVal64 = Read<DWORD64>(ReturnValue64Remote.GetBase());
    double const RetValFloat = Read<double>(ReturnValueFloatRemote.GetBase());
    return RemoteFunctionRet(RetVal, RetVal64, RetValFloat, ErrorCode);
  }

  // Whether an address is currently readable
  bool MemoryMgr::CanRead(LPCVOID Address) const
  {
    // Query page protections
    MEMORY_BASIC_INFORMATION MyMbi;
    ZeroMemory(&MyMbi, sizeof(MyMbi));
    if (VirtualQueryEx(m_Process.GetHandle(), Address, &MyMbi, 
      sizeof(MyMbi)) != sizeof(MyMbi))
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("MemoryMgr::CanRead") << 
        ErrorString("Could not read process memory protection.") << 
        ErrorCodeWinLast(LastError));
    }

    // Whether memory is currently readable
    return 
      (MyMbi.Protect & PAGE_EXECUTE_READ) == PAGE_EXECUTE_READ || 
      (MyMbi.Protect & PAGE_EXECUTE_READWRITE) == PAGE_EXECUTE_READWRITE || 
      (MyMbi.Protect & PAGE_EXECUTE_WRITECOPY) == PAGE_EXECUTE_WRITECOPY || 
      (MyMbi.Protect & PAGE_READONLY) == PAGE_READONLY || 
      (MyMbi.Protect & PAGE_READWRITE) == PAGE_READWRITE || 
      (MyMbi.Protect & PAGE_WRITECOPY) == PAGE_WRITECOPY;
  }

  // Whether an address is currently writable
  bool MemoryMgr::CanWrite(LPCVOID Address) const
  {
    // Query page protections
    MEMORY_BASIC_INFORMATION MyMbi;
    ZeroMemory(&MyMbi, sizeof(MyMbi));
    if (VirtualQueryEx(m_Process.GetHandle(), Address, &MyMbi, 
      sizeof(MyMbi)) != sizeof(MyMbi))
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("MemoryMgr::Write") << 
        ErrorString("Could not read process memory protection.") << 
        ErrorCodeWinLast(LastError));
    }

    // Whether memory is currently writable
    return 
      (MyMbi.Protect & PAGE_EXECUTE_READWRITE) == PAGE_EXECUTE_READWRITE || 
      (MyMbi.Protect & PAGE_EXECUTE_WRITECOPY) == PAGE_EXECUTE_WRITECOPY || 
      (MyMbi.Protect & PAGE_READWRITE) == PAGE_READWRITE || 
      (MyMbi.Protect & PAGE_WRITECOPY) == PAGE_WRITECOPY;
  }

  // Whether an address is contained within a guard page
  bool MemoryMgr::IsGuard(LPCVOID Address) const
  {
    // Query page protections
    MEMORY_BASIC_INFORMATION MyMbi;
    ZeroMemory(&MyMbi, sizeof(MyMbi));
    if (VirtualQueryEx(m_Process.GetHandle(), Address, &MyMbi, 
      sizeof(MyMbi)) != sizeof(MyMbi))
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("MemoryMgr::IsGuard") << 
        ErrorString("Could not read process memory protection.") << 
        ErrorCodeWinLast(LastError));
    }

    // Whether address is in a guard page
    return (MyMbi.Protect & PAGE_GUARD) == PAGE_GUARD;
  }
  
  // Protect a memory region
  DWORD MemoryMgr::ProtectRegion(LPVOID Address, DWORD Protect) const
  {
    // Query page protections
    MEMORY_BASIC_INFORMATION MyMbi;
    ZeroMemory(&MyMbi, sizeof(MyMbi));
    if (VirtualQueryEx(m_Process.GetHandle(), Address, &MyMbi, 
      sizeof(MyMbi)) != sizeof(MyMbi))
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("MemoryMgr::ProtectRegion") << 
        ErrorString("Could not read process memory protection.") << 
        ErrorCodeWinLast(LastError));
    }
    
    // Protect memory region
    DWORD OldProtect = 0;
    if (!VirtualProtectEx(m_Process.GetHandle(), MyMbi.BaseAddress, 
      MyMbi.RegionSize, Protect, &OldProtect))
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("MemoryMgr::ProtectRegion") << 
        ErrorString("Could not change process memory protection.") << 
        ErrorCodeWinLast(LastError));
    }
    
    // Return previous protection
    return OldProtect;
  }

  // Allocate memory
  PVOID MemoryMgr::Alloc(SIZE_T Size) const
  {
    PVOID const Address = VirtualAllocEx(m_Process.GetHandle(), nullptr, 
      Size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!Address)
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("MemoryMgr::Alloc") << 
        ErrorString("Could not allocate memory.") << 
        ErrorCodeWinLast(LastError));
    }

    return Address;
  }

  // Free memory
  void MemoryMgr::Free(PVOID Address) const
  {
    if (!VirtualFreeEx(m_Process.GetHandle(), Address, 0, MEM_RELEASE))
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("MemoryMgr::Free") << 
        ErrorString("Could not free memory.") << 
        ErrorCodeWinLast(LastError));
    }
  }

  // Get address of export in remote process
  FARPROC MemoryMgr::GetRemoteProcAddress(HMODULE RemoteMod, 
    std::wstring const& ModulePath, std::string const& Function) 
    const
  {
    // Load module as data so we can read the EAT locally
    Detail::EnsureFreeLibrary const LocalMod(LoadLibraryEx(
      ModulePath.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES));
    if (!LocalMod)
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("MemoryMgr::GetRemoteProcAddress") << 
        ErrorString("Could not load module locally.") << 
        ErrorCodeWinLast(LastError));
    }

    // Find target function in module
    FARPROC const LocalFunc = GetProcAddress(LocalMod, Function.c_str());
    if (!LocalFunc)
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("MemoryMgr::GetRemoteProcAddress") << 
        ErrorString("Could not find target function.") << 
        ErrorCodeWinLast(LastError));
    }

    // Calculate function delta
    LONG_PTR const FuncDelta = reinterpret_cast<DWORD_PTR>(LocalFunc) - 
      reinterpret_cast<DWORD_PTR>(static_cast<HMODULE>(LocalMod));

    // Calculate function location in remote process
    FARPROC const RemoteFunc = reinterpret_cast<FARPROC>(
      reinterpret_cast<DWORD_PTR>(RemoteMod) + FuncDelta);

    // Return remote function location
    return RemoteFunc;
  }

  // Get address of export in remote process
  FARPROC MemoryMgr::GetRemoteProcAddress(HMODULE RemoteMod, 
    std::wstring const& ModulePath, WORD Ordinal) const
  {
    // Load module as data so we can read the EAT locally
    Detail::EnsureFreeLibrary const LocalMod(LoadLibraryEx(
      ModulePath.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES));
    if (!LocalMod)
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("MemoryMgr::GetRemoteProcAddress") << 
        ErrorString("Could not load module locally.") << 
        ErrorCodeWinLast(LastError));
    }

    // Find target function in module
    FARPROC const LocalFunc = GetProcAddress(LocalMod, MAKEINTRESOURCEA(
      Ordinal));
    if (!LocalFunc)
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("MemoryMgr::GetRemoteProcAddress") << 
        ErrorString("Could not find target function.") << 
        ErrorCodeWinLast(LastError));
    }

    // Calculate function delta
    DWORD_PTR const FuncDelta = reinterpret_cast<DWORD_PTR>(LocalFunc) - 
      reinterpret_cast<DWORD_PTR>(static_cast<HMODULE>(LocalMod));

    // Calculate function location in remote process
    FARPROC const RemoteFunc = reinterpret_cast<FARPROC>(
      reinterpret_cast<DWORD_PTR>(RemoteMod) + FuncDelta);

    // Return remote function location
    return RemoteFunc;
  }

  // Flush instruction cache
  void MemoryMgr::FlushCache(LPCVOID Address, SIZE_T Size) const
  {
    if (!FlushInstructionCache(m_Process.GetHandle(), Address, Size))
    {
      DWORD const LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Error() << 
        ErrorFunction("MemoryMgr::FlushInstructionCache") << 
        ErrorString("Could not flush instruction cache.") << 
        ErrorCodeWinLast(LastError));
    }
  }

  // Get process handle of target
  HANDLE MemoryMgr::GetProcessHandle() const
  {
    return m_Process.GetHandle();
  }

  // Get process ID of target
  DWORD MemoryMgr::GetProcessId() const
  {
    return m_Process.GetID();
  }
    
  // Get process path
  std::wstring MemoryMgr::GetProcessPath() const
  {
    return m_Process.GetPath();
  }

  // Is WoW64 process
  bool MemoryMgr::IsWoW64Process() const 
  {
    return m_Process.IsWoW64();
  }
}