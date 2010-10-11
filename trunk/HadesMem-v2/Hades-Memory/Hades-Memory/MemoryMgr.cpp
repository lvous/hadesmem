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

// Hades
#include "MemoryMgr.h"

namespace Hades
{
  namespace Memory
  {
    // RAII class for AsmJit
    class EnsureAsmJitFree : private boost::noncopyable
    {
    public:
      // Constructor
      EnsureAsmJitFree(PVOID Address) 
        : m_Address(Address)
      { }

      // Destructor
      ~EnsureAsmJitFree()
      {
        // Free memory if necessary
        if (m_Address)
        {
          AsmJit::MemoryManager::getGlobal()->free(m_Address);
        }
      }

      // Get address
      PVOID Get() const 
      {
        return m_Address;
      }

    private:
      // Address
      PVOID m_Address;
    };

    // Open process from process ID
    MemoryMgr::MemoryMgr(DWORD ProcID) 
      : m_Process(ProcID) 
    { }

    // Open process from process name
    MemoryMgr::MemoryMgr(std::wstring const& ProcName) 
      : m_Process(ProcName) 
    { }

    // Open process from window name and class
    MemoryMgr::MemoryMgr(std::wstring const& WindowName, 
      std::wstring const& ClassName) 
      : m_Process(WindowName, ClassName) 
    { }

    // Move constructor
    MemoryMgr::MemoryMgr(MemoryMgr&& MyMemoryMgr) 
      : m_Process(std::move(MyMemoryMgr.m_Process))
    { }

    // Move assignment operator
    MemoryMgr& MemoryMgr::operator=(MemoryMgr&& MyMemoryMgr) 
    {
      m_Process = std::move(MyMemoryMgr.m_Process);

      return *this;
    }

    // Call remote function
    DWORD_PTR MemoryMgr::Call(PVOID Address, std::vector<PVOID> const& Args, 
      CallConv MyCallConv) const 
    {
      // Get number of arguments
      std::size_t NumArgs = Args.size();

      // Create Assembler.
      AsmJit::Assembler MyJitFunc;

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

      // Get address to write return value to and store for later
      MyJitFunc.push(AsmJit::rcx);

      // Allocate ghost space
      MyJitFunc.sub(AsmJit::rsp, AsmJit::Imm(0x20));

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
        std::for_each(Args.rbegin(), Args.rend() - 4, 
          [&MyJitFunc] (PVOID Arg)
        {
          MyJitFunc.mov(AsmJit::rax, reinterpret_cast<DWORD_PTR>(Arg));
          MyJitFunc.push(AsmJit::rax);
        });
      }

      // Call target
      MyJitFunc.mov(AsmJit::rax, reinterpret_cast<DWORD_PTR>(Address));
      MyJitFunc.call(AsmJit::rax);

      // Cleanup ghost space
      MyJitFunc.add(AsmJit::rsp, AsmJit::Imm(0x20));

      // Clean up remaining stack space
      std::size_t StackArgs = NumArgs > 4 ? NumArgs - 4 : 0;
      while (StackArgs--)
      {
        MyJitFunc.add(AsmJit::rsp, AsmJit::Imm(0x8));
      }

      // Write return value to memory
      MyJitFunc.pop(AsmJit::rcx);
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

      // Get address to write return value to and store for later
      MyJitFunc.mov(AsmJit::eax, AsmJit::dword_ptr(AsmJit::ebp, 8));
      MyJitFunc.push(AsmJit::eax);

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
        std::for_each(Args.rbegin(), Args.rend() - StackArgOffs, 
          [&] (PVOID Arg)
        {
          MyJitFunc.mov(AsmJit::eax, reinterpret_cast<DWORD_PTR>(Arg));
          MyJitFunc.push(AsmJit::eax);
        });
      }

      // Call target
      MyJitFunc.mov(AsmJit::eax, reinterpret_cast<DWORD_PTR>(Address));
      MyJitFunc.call(AsmJit::eax);

      // Clean up stack if necessary
      if (MyCallConv == CallConv_CDECL)
      {
        MyJitFunc.add(AsmJit::esp, AsmJit::Imm(NumArgs * sizeof(PVOID)));
      }

      // Write return value to memory
      MyJitFunc.pop(AsmJit::ecx);
      MyJitFunc.mov(AsmJit::dword_ptr(AsmJit::ecx), AsmJit::eax);

      // Epilogue
      MyJitFunc.mov(AsmJit::esp, AsmJit::ebp);
      MyJitFunc.pop(AsmJit::ebp);

      // Return
      MyJitFunc.ret(AsmJit::Imm(0x4));
#else 
#error "Unsupported architecture."
#endif

      // Get stub size
      DWORD_PTR const StubSize = MyJitFunc.getCodeSize();
      DWORD_PTR const StubSizeFull = MyJitFunc.getCodeSize() + 
        sizeof(DWORD_PTR);

      // Allocate memory for stub buffer
      AllocAndFree const StubMemRemote(*this, StubSizeFull);
      PBYTE pRemoteStub = static_cast<PBYTE>(StubMemRemote.GetAddress());
      PBYTE pRemoteStubCode = static_cast<PBYTE>(StubMemRemote.GetAddress()) + 
        sizeof(DWORD_PTR);

      // Create buffer to hold relocated code plues the return value address
      std::vector<BYTE> CodeReal(StubSize);

      // Generate code
      MyJitFunc.relocCode(&CodeReal[0], reinterpret_cast<DWORD_PTR>(
        pRemoteStubCode));

      // Write stub buffer to process
      Write(pRemoteStubCode, CodeReal);

      // Call stub via creating a remote thread in the target.
      Windows::EnsureCloseHandle const MyThread(CreateRemoteThread(m_Process.
        GetHandle(), nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(
        pRemoteStubCode), pRemoteStub, 0, nullptr));
      if (!MyThread)
      {
        DWORD const LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("MemoryMgr::Call") << 
          ErrorString("Could not create remote thread.") << 
          ErrorCodeWin(LastError));
      }

      // Wait for the remote thread to terminate
      if (WaitForSingleObject(MyThread, INFINITE) != WAIT_OBJECT_0)
      {
        DWORD const LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("MemoryMgr::Call") << 
          ErrorString("Could not wait for remote thread.") << 
          ErrorCodeWin(LastError));
      }

      // Forward return value remote thread
      DWORD_PTR RetVal = Read<DWORD_PTR>(pRemoteStub);
      return RetVal;
    }

    // Whether an address is currently readable
    bool MemoryMgr::CanRead(PVOID Address) const
    {
      // Query page protections
      MEMORY_BASIC_INFORMATION MyMemInfo = { 0 };
      if (!VirtualQueryEx(m_Process.GetHandle(), Address, &MyMemInfo, 
        sizeof(MyMemInfo)))
      {
        DWORD const LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("MemoryMgr::CanRead") << 
          ErrorString("Could not read process memory protection.") << 
          ErrorCodeWin(LastError));
      }

      // Whether memory is currently readable
      return MyMemInfo.Protect == PAGE_EXECUTE_READ || 
        MyMemInfo.Protect == PAGE_EXECUTE_READWRITE || 
        MyMemInfo.Protect == PAGE_EXECUTE_WRITECOPY || 
        MyMemInfo.Protect == PAGE_READONLY || 
        MyMemInfo.Protect == PAGE_READWRITE || 
        MyMemInfo.Protect == PAGE_WRITECOPY;
    }

    // Whether an address is currently writable
    bool MemoryMgr::CanWrite(PVOID Address) const
    {
      // Query page protections
      MEMORY_BASIC_INFORMATION MyMemInfo = { 0 };
      if (!VirtualQueryEx(m_Process.GetHandle(), Address, &MyMemInfo, 
        sizeof(MyMemInfo)))
      {
        DWORD const LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("MemoryMgr::Write") << 
          ErrorString("Could not read process memory protection.") << 
          ErrorCodeWin(LastError));
      }

      // Whether memory is currently writable
      return MyMemInfo.Protect == PAGE_EXECUTE_READWRITE || 
        MyMemInfo.Protect == PAGE_EXECUTE_WRITECOPY || 
        MyMemInfo.Protect == PAGE_READWRITE || 
        MyMemInfo.Protect == PAGE_WRITECOPY;
    }

    // Allocate memory
    PVOID MemoryMgr::Alloc(SIZE_T Size) const
    {
      PVOID Address = VirtualAllocEx(m_Process.GetHandle(), nullptr, Size, 
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
      if (!Address)
      {
        DWORD const LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("MemoryMgr::Alloc") << 
          ErrorString("Could not allocate memory.") << 
          ErrorCodeWin(LastError));
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
          ErrorCodeWin(LastError));
      }
    }

    // Get process ID of target
    DWORD MemoryMgr::GetProcessID() const
    {
      return m_Process.GetID();
    }

    // Get process handle of target
    HANDLE MemoryMgr::GetProcessHandle() const
    {
      return m_Process.GetHandle();
    }

    // Get address of export in remote process
    FARPROC MemoryMgr::GetRemoteProcAddress(HMODULE RemoteMod, 
      boost::filesystem::path const& ModulePath, std::string const& Function) 
      const
    {
      // Load module as data so we can read the EAT locally
      Windows::EnsureFreeLibrary const LocalMod(LoadLibraryEx(
        ModulePath.c_str(), NULL, DONT_RESOLVE_DLL_REFERENCES));
      if (!LocalMod)
      {
        DWORD const LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("MemoryMgr::GetRemoteProcAddress") << 
          ErrorString("Could not load module locally.") << 
          ErrorCodeWin(LastError));
      }

      // Find target function in module
      FARPROC const LocalFunc = GetProcAddress(LocalMod, Function.c_str());
      if (!LocalFunc)
      {
        return nullptr;
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
      boost::filesystem::path const& ModulePath, WORD Ordinal) const
    {
      // Load module as data so we can read the EAT locally
      Windows::EnsureFreeLibrary const LocalMod(LoadLibraryExW(
        ModulePath.c_str(), NULL, DONT_RESOLVE_DLL_REFERENCES));
      if (!LocalMod)
      {
        DWORD const LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("MemoryMgr::GetRemoteProcAddress") << 
          ErrorString("Could not load module locally.") << 
          ErrorCodeWin(LastError));
      }

      // Find target function in module
      FARPROC const LocalFunc = GetProcAddress(LocalMod, MAKEINTRESOURCEA(
        Ordinal));
      if (!LocalFunc)
      {
        return nullptr;
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
    void MemoryMgr::FlushCache(PVOID Address, SIZE_T Size) const
    {
      if (!FlushInstructionCache(m_Process.GetHandle(), Address, Size))
      {
        DWORD const LastError = GetLastError();
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("MemoryMgr::FlushInstructionCache") << 
          ErrorString("Could not flush instruction cache.") << 
          ErrorCodeWin(LastError));
      }
    }
  }
}
