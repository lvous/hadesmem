#pragma once

// Windows API
#include <Windows.h>

// C++ Standard Library
#include <memory>
#include <string>
#include <vector>
#include <type_traits>

// Boost
#pragma warning(push, 1)
#pragma warning(disable: 4706)
#include <boost/noncopyable.hpp>
#include <boost/type_traits.hpp>
#pragma warning(pop)

// AsmJit
#pragma warning(push, 1)
#include "AsmJit/AsmJit.h"
#pragma warning(pop)

// HadesMem
#include "Error.h"
#include "Process.h"

namespace Hades
{
  namespace Memory
  {
    // Memory exception type
    class MemoryError : public virtual HadesMemError 
    { };

    // Memory managing class
    class MemoryMgr : private boost::noncopyable
    {
    public:
      // Open process from process ID
      inline explicit MemoryMgr(DWORD ProcID);
      // Open process from process name
      inline explicit MemoryMgr(std::wstring const& ProcName);
      // Open process from window name (and optionally class)
      inline MemoryMgr(std::wstring const& WindowName, 
        std::wstring const* const ClassName);

      // Call remote function
      DWORD Call(PVOID Address, std::vector<PVOID> const& Args) const;

      // Read memory (POD types)
      template <typename T>
      T Read(PVOID Address, typename boost::enable_if<std::is_pod<T>>::type* 
        Dummy = 0) const;

      // Read memory (string types)
      template <typename T>
      T Read(PVOID Address, typename boost::enable_if<std::is_same<T, 
        std::basic_string<typename T::value_type>>>::type* Dummy = 0) const;

      // Read memory (vector types)
      template <typename T>
      T Read(PVOID Address, typename std::vector<typename T::value_type>::
        size_type Size, typename boost::enable_if<std::is_same<T, std::vector<
        typename T::value_type>>>::type* Dummy1 = 0, typename boost::enable_if<
        std::is_pod<typename T::value_type>>::type* Dummy2 = 0) const;

      // Write memory (POD types)
      template <typename T>
      void Write(PVOID Address, T const& Data, typename boost::enable_if<
        std::is_pod<T>>::type* Dummy = 0) const;

      // Write memory (string types)
      template <typename T>
      void Write(PVOID Address, T const& Data, typename boost::enable_if<
        std::is_same<T, std::basic_string<typename T::value_type>>>::type* 
        Dummy = 0) const;

      // Write memory (vector types)
      template <typename T>
      void Write(PVOID Address, T const& Data, typename boost::enable_if<
        std::is_same<T, std::vector<typename T::value_type>>>::type* 
        Dummy = 0) const;

      // Whether an address is currently readable
      inline bool CanRead(PVOID Address) const;

      // Whether an address is currently writable
      inline bool CanWrite(PVOID Address) const;

      // Allocate memory
      inline PVOID Alloc(SIZE_T Size) const;

      // Free memory
      inline void Free(PVOID Address) const;

      // Get process ID of target
      inline DWORD GetProcessID() const;

      // Get process handle of target
      inline HANDLE GetProcessHandle() const;

    private:
      // Target process
      Process m_Process;
    };

    class AllocAndFree : private boost::noncopyable
    {
    public:
      AllocAndFree(MemoryMgr const& MyMemoryMgr, SIZE_T Size) 
        : m_Memory(MyMemoryMgr), m_Address(m_Memory.Alloc(Size)) 
      { }

      ~AllocAndFree()
      {
        m_Memory.Free(m_Address);
      }

      PVOID GetAddress() const 
      {
        return m_Address;
      }

    private:
      MemoryMgr const& m_Memory;
      PVOID m_Address;
    };

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
          AsmJit::MemoryManager::global()->free(m_Address);
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

    // Open process from window name (and optionally class)
    MemoryMgr::MemoryMgr(std::wstring const& WindowName, 
      std::wstring const* const ClassName) 
      : m_Process(WindowName, ClassName) 
    { }

    // Call remote function
    DWORD MemoryMgr::Call(PVOID Address, std::vector<PVOID> const& Args) const 
    {
      // Create Assembler.
      AsmJit::Assembler MyJitFunc;
      
      #if defined(_M_AMD64) 
      // Prologue
      MyJitFunc.push(AsmJit::rbp);
      MyJitFunc.mov(AsmJit::rbp, AsmJit::rsp);

      // Function calling code
      MyJitFunc.push(AsmJit::Immediate(0));
      MyJitFunc.push(AsmJit::Immediate(0));
      MyJitFunc.push(AsmJit::Immediate(0));
      MyJitFunc.push(AsmJit::Immediate(0));
      MyJitFunc.mov(AsmJit::rcx, Args.size() > 0 ? reinterpret_cast<DWORD_PTR>(
        Args[0]) : 0);
      MyJitFunc.mov(AsmJit::rdx, Args.size() > 1 ? reinterpret_cast<DWORD_PTR>(
        Args[1]) : 0);
      MyJitFunc.mov(AsmJit::r8, Args.size() > 2 ? reinterpret_cast<DWORD_PTR>(
        Args[2]) : 0);
      MyJitFunc.mov(AsmJit::r9, Args.size() > 3 ? reinterpret_cast<DWORD_PTR>(
        Args[3]) : 0);
      if (Args.size() > 4)
      {
        std::for_each(Args.rbegin(), Args.rend() - 4, 
          [&MyJitFunc] (PVOID Arg)
        {
          MyJitFunc.mov(AsmJit::rax, reinterpret_cast<DWORD_PTR>(Arg));
          MyJitFunc.push(AsmJit::rax);
        });
      }
      MyJitFunc.mov(AsmJit::rax, reinterpret_cast<DWORD_PTR>(Address));
      MyJitFunc.call(AsmJit::rax);
      
      // Cleanup ghost space
      MyJitFunc.pop(AsmJit::rdx);
      MyJitFunc.pop(AsmJit::rdx);
      MyJitFunc.pop(AsmJit::rdx);
      MyJitFunc.pop(AsmJit::rdx);

      // Clean up remaining stack space
      std::size_t StackArgs(Args.size() > 4 ? Args.size() - 4 : 0);
      while (StackArgs--)
      {
        MyJitFunc.pop(AsmJit::rdx);
      }

      // Epilogue
      MyJitFunc.mov(AsmJit::rsp, AsmJit::rbp);
      MyJitFunc.pop(AsmJit::rbp);

      // Return
      MyJitFunc.ret();
      #elif defined(_M_IX86) 
      // Prologue
      MyJitFunc.push(AsmJit::ebp);
      MyJitFunc.mov(AsmJit::ebp, AsmJit::esp);

      // Function calling code
      std::for_each(Args.rbegin(), Args.rend(), 
        [&MyJitFunc] (PVOID Arg)
      {
        MyJitFunc.mov(AsmJit::eax, reinterpret_cast<DWORD_PTR>(Arg));
        MyJitFunc.push(AsmJit::eax);
      });
      MyJitFunc.mov(AsmJit::eax, reinterpret_cast<DWORD_PTR>(Address));
      MyJitFunc.call(AsmJit::eax);

      // Epilogue
      MyJitFunc.mov(AsmJit::esp, AsmJit::ebp);
      MyJitFunc.pop(AsmJit::ebp);

      // Return
      MyJitFunc.ret();
      #else 
        #error "Unsupported architecture."
      #endif
      
      // Make JIT function.
      EnsureAsmJitFree LoaderStub(MyJitFunc.make());

      // Ensure function creation succeeded
      if (!LoaderStub.Get())
      {
        BOOST_THROW_EXCEPTION(MemoryError() << 
          ErrorFunction("MemoryMgr::Call") << 
          ErrorString("Error JIT'ing loader stub."));
      }

      // Get stub size
      DWORD_PTR StubSize = MyJitFunc.codeSize();

      // Allocate memory for stub buffer
      AllocAndFree EpCallerMem(*this, StubSize);
      // Copy loader stub to stub buffer
      std::vector<BYTE> EpCallerBuf(reinterpret_cast<PBYTE>(LoaderStub.Get()), 
        reinterpret_cast<PBYTE>(LoaderStub.Get()) + StubSize);
      // Write stub buffer to process
      Write(EpCallerMem.GetAddress(), EpCallerBuf);

      // Call stub via creating a remote thread in the target.
      // Todo: Robust implementation via ASM Jit and SEH.
      // Todo: Support parameters, calling conventions, etc.
      // Todo: Pass arguments via value not pointer.
      EnsureCloseHandle MyThread = CreateRemoteThread(m_Process.GetHandle(), 
        nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(Address), 
        EpCallerMem.GetAddress(), 0, nullptr);
      if (!MyThread)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(MemoryError() << 
          ErrorFunction("MemoryMgr::Call") << 
          ErrorString("Could not create remote thread.") << 
          ErrorCodeWin(LastError));
      }

      // Wait for the remote thread to terminate
      if (WaitForSingleObject(MyThread, INFINITE) != WAIT_OBJECT_0)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(MemoryError() << 
          ErrorFunction("MemoryMgr::Call") << 
          ErrorString("Could not wait for remote thread.") << 
          ErrorCodeWin(LastError));
      }

      // Get exit code of remote injection thread
      DWORD ExitCode = 0;
      if (!GetExitCodeThread(MyThread, &ExitCode))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(MemoryError() << 
          ErrorFunction("MemoryMgr::Call") << 
          ErrorString("Could not get remote thread exit code.") << 
          ErrorCodeWin(LastError));
      }

      // Return exit code from thread
      return ExitCode;
    }

    // Read memory (POD types)
    template <typename T>
    T MemoryMgr::Read(PVOID Address, typename boost::enable_if<
      std::is_pod<T>>::type* /*Dummy*/) const 
    {
      // Whether we can read the given address
      bool CanReadMem = CanRead(Address);

      // Set page protection for reading
      DWORD OldProtect;
      if (!CanReadMem)
      {
        if (!VirtualProtectEx(m_Process.GetHandle(), Address, sizeof(T), 
          PAGE_EXECUTE_READWRITE, &OldProtect))
        {
          DWORD LastError = GetLastError();
          BOOST_THROW_EXCEPTION(MemoryError() << 
            ErrorFunction("MemoryMgr::Read") << 
            ErrorString("Could not change process memory protection.") << 
            ErrorCodeWin(LastError));
        }
      }

      // Read data
      T Out = T();
      SIZE_T BytesRead = 0;
      if (!ReadProcessMemory(m_Process.GetHandle(), Address, &Out, sizeof(T), 
        &BytesRead) || BytesRead != sizeof(T))
      {
        if (!CanReadMem)
        {
          // Restore original page protections
          VirtualProtectEx(m_Process.GetHandle(), Address, sizeof(T), 
            OldProtect, &OldProtect);
        }

        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(MemoryError() << 
          ErrorFunction("MemoryMgr::Read") << 
          ErrorString("Could not read process memory.") << 
          ErrorCodeWin(LastError));
      }

      // Restore original page protections
      if (!CanReadMem)
      {
        if (!VirtualProtectEx(m_Process.GetHandle(), Address, sizeof(T), 
          OldProtect, &OldProtect))
        {
          DWORD LastError = GetLastError();
          BOOST_THROW_EXCEPTION(MemoryError() << 
            ErrorFunction("MemoryMgr::Read") << 
            ErrorString("Could not restore process memory protection.") << 
            ErrorCodeWin(LastError));
        }
      }

      return Out;
    }

    // Read memory (string types)
    template <typename T>
    T MemoryMgr::Read(PVOID Address, typename boost::enable_if<std::is_same<T, 
      std::basic_string<typename T::value_type>>>::type* /*Dummy*/) const
    {
      // Character type
      typedef typename T::value_type CharT;

      // Create string and character pointers
      std::basic_string<CharT> Buffer;
      CharT* AddressReal = static_cast<CharT*>(Address);

      // Loop until a null terminator is found
      for (;;)
      {
        // Read current character and add to buffer
        CharT Current = Read<CharT>(AddressReal++);
        Buffer += Current;

        // Break on null terminator
        if (Current == 0)
          break;
      }

      // Return generated string
      return Buffer;
    }

    // Read memory (vector types)
    template <typename T>
    T MemoryMgr::Read(PVOID Address, typename std::vector<typename T::
      value_type>::size_type Size, typename boost::enable_if<std::is_same<T, 
      std::vector<typename T::value_type>>>::type* /*Dummy*/, typename boost::
      enable_if<std::is_pod<typename T::value_type>>::type* /*Dummy2*/) const
    {
      // Create buffer
      T Buffer(Size);
      SIZE_T BufferSize = sizeof(T::value_type) * Size;

      // Whether we can read the given address
      bool CanReadMem = CanRead(Address);

      // Set page protection for reading
      DWORD OldProtect;
      if (!CanReadMem)
      {
        if (!VirtualProtectEx(m_Process.GetHandle(), Address, BufferSize, 
          PAGE_EXECUTE_READWRITE, &OldProtect))
        {
          DWORD LastError = GetLastError();
          BOOST_THROW_EXCEPTION(MemoryError() << 
            ErrorFunction("MemoryMgr::Read") << 
            ErrorString("Could not change process memory protection.") << 
            ErrorCodeWin(LastError));
        }
      }

      // Read data
      SIZE_T BytesRead = 0;
      if (!ReadProcessMemory(m_Process.GetHandle(), Address, &Buffer[0], 
        BufferSize, &BytesRead) || BytesRead != BufferSize)
      {
        if (!CanReadMem)
        {
          // Restore original page protections
          VirtualProtectEx(m_Process.GetHandle(), Address, BufferSize, 
            OldProtect, &OldProtect);
        }

        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(MemoryError() << 
          ErrorFunction("MemoryMgr::Read") << 
          ErrorString("Could not read process memory.") << 
          ErrorCodeWin(LastError));
      }

      // Restore original page protections
      if (!CanReadMem)
      {
        if (!VirtualProtectEx(m_Process.GetHandle(), Address, BufferSize, 
          OldProtect, &OldProtect))
        {
          DWORD LastError = GetLastError();
          BOOST_THROW_EXCEPTION(MemoryError() << 
            ErrorFunction("MemoryMgr::Read") << 
            ErrorString("Could not restore process memory protection.") << 
            ErrorCodeWin(LastError));
        }
      }

      // Return generated buffer
      return Buffer;
    }

    // Write memory (POD types)
    template <typename T>
    void MemoryMgr::Write(PVOID Address, T const& Data, typename 
      boost::enable_if<std::is_pod<T>>::type* /*Dummy*/) const 
    {
      // Set page protections for writing
      DWORD OldProtect;
      if (!VirtualProtectEx(m_Process.GetHandle(), Address, sizeof(T), 
        PAGE_EXECUTE_READWRITE, &OldProtect))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(MemoryError() << 
          ErrorFunction("MemoryMgr::Write") << 
          ErrorString("Could not change process memory protection.") << 
          ErrorCodeWin(LastError));
      }

      // Write data
      SIZE_T BytesWritten = 0;
      if (!WriteProcessMemory(m_Process.GetHandle(), Address, &Data, sizeof(T), 
        &BytesWritten) || BytesWritten != sizeof(T))
      {
        // Restore original page protections
        VirtualProtectEx(m_Process.GetHandle(), Address, sizeof(T), OldProtect, 
          &OldProtect);

        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(MemoryError() << 
          ErrorFunction("MemoryMgr::Write") << 
          ErrorString("Could not write process memory.") << 
          ErrorCodeWin(LastError));
      }

      // Restore original page protections
      if (!VirtualProtectEx(m_Process.GetHandle(), Address, sizeof(T), 
        OldProtect, &OldProtect))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(MemoryError() << 
          ErrorFunction("MemoryMgr::Write") << 
          ErrorString("Could not restore process memory protection.") << 
          ErrorCodeWin(LastError));
      }
    }

    // Write memory (string types)
    template <typename T>
    void MemoryMgr::Write(PVOID Address, T const& Data, 
      typename boost::enable_if<std::is_same<T, std::basic_string<
      typename T::value_type>>>::type* /*Dummy*/) const
    {
      // Character type
      typedef typename T::value_type CharT;

      // Create character pointer
      CharT* AddressReal = reinterpret_cast<CharT*>(Address);

      // Write all characters in string to memory
      std::for_each(Data.begin(), Data.end(), 
        [&AddressReal, this] (CharT Current) 
      {
        // Write current character to memory
        this->Write(AddressReal++, Current);
      });

      // Null terminate string
      Write<CharT>(AddressReal, 0);
    }

    // Write memory (vector types)
    template <typename T>
    void MemoryMgr::Write(PVOID Address, T const& Data, 
      typename boost::enable_if<std::is_same<T, std::vector<
      typename T::value_type>>>::type* /*Dummy*/) const
    {
      // Create value type pointer
      T::value_type* AddressReal = static_cast<T::value_type*>(Address);

      // Write all data in buffer to memory
      std::for_each(Data.begin(), Data.end(), 
        [&AddressReal, this] (T::value_type const& Current) 
      {
        // Write current character to memory
        this->Write(AddressReal++, Current);
      });
    }

    // Whether an address is currently readable
    bool MemoryMgr::CanRead(PVOID Address) const
    {
      // Query page protections
      MEMORY_BASIC_INFORMATION MyMemInfo = { 0 };
      if (!VirtualQueryEx(m_Process.GetHandle(), Address, &MyMemInfo, 
        sizeof(MyMemInfo)))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(MemoryError() << 
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
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(MemoryError() << 
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
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(MemoryError() << 
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
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(MemoryError() << 
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
  }
}
