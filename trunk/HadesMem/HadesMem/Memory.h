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

      // MemoryMgr::Call trait class. Ensures the function arity is valid.
      template <typename T, std::size_t Arity>
      struct IsArity
      {
        static const bool value = typename boost::function_traits<T>::arity == 
          Arity;
      };

      // Call remote function
      template <typename T>
      typename boost::function_traits<T>::result_type Call(PVOID Address, 
        typename boost::function_traits<T>::arg1_type const& Arg1,  
        typename boost::enable_if_c<MemoryMgr::IsArity<T, 1>::value>::type* 
        Dummy = 0) const;

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

      // Search memory (POD types)
      template <typename T>
      PVOID Find(T const& Data, PVOID Start, PVOID End, typename 
        boost::enable_if<std::is_pod<T>>::type* Dummy = 0) const;

      // Search memory (string types)
      template <typename T>
      PVOID Find(T const& Data, PVOID Start, PVOID End, typename boost::
        enable_if<std::is_same<T, std::basic_string<typename T::value_type>>>::
        type* Dummy = 0) const;

      // Search memory (vector types)
      template <typename T>
      PVOID Find(T const& Data, PVOID Start, PVOID End, std::wstring const& 
        Mask = L"", typename boost::enable_if<std::is_same<T, std::vector<
        typename T::value_type>>>::type* Dummy1 = 0, typename boost::enable_if<
        std::is_pod<typename T::value_type>>::type* Dummy2 = 0) const;

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
    template <typename T>
    typename boost::function_traits<T>::result_type MemoryMgr::Call(
      PVOID Address, typename boost::function_traits<T>::arg1_type const& Arg1, 
      typename boost::enable_if_c<MemoryMgr::IsArity<T, 1>::value>::type*) 
      const 
    {
      // Ensure argument is the same size (or smaller) as the native 
      // size for the architecture. Larger sized objects must be passed by 
      // pointer.
      static_assert(sizeof(Arg1) <= sizeof(PVOID), "Size of argument is "
        "invalid.");

      // Ensure specified return type is of a valid size and type
      typedef typename boost::function_traits<T>::result_type RetType;
      static_assert(std::is_scalar<RetType>::value, "Return type is "
        "invalid.");
      static_assert(sizeof(RetType) <= sizeof(DWORD), "Size of return type "
        "is invalid.");

      // Allocate and write argument to process
      AllocAndFree MyRemoteMem(*this, sizeof(Arg1));
      Write(MyRemoteMem.GetAddress(), Arg1);

      // Call function via creating a remote thread in the target.
      // Todo: Robust implementation via ASM Jit and SEH.
      // Todo: Support parameters, calling conventions, etc.
      // Todo: Pass arguments via value not pointer.
      EnsureCloseHandle MyThread = CreateRemoteThread(m_Process.GetHandle(), 
        nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(Address), 
        MyRemoteMem.GetAddress(), 0, nullptr);
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
      return RetType(ExitCode);
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

    // Search memory (POD types)
    template <typename T>
    PVOID MemoryMgr::Find(T const& Data, PVOID Start, PVOID End, typename 
      boost::enable_if<std::is_pod<T>>:: type* /*Dummy*/) const
    {
      // Note: Assumes data is aligned to the size of T.
      // Todo: Unaligned version, and aligned to size of pointer version.

      // Calculate number of elements in address range
      DWORD_PTR NumElems = (reinterpret_cast<DWORD_PTR>(End) - 
        reinterpret_cast<DWORD_PTR>(Start)) / sizeof(T);
      // Read vector of Ts into cache
      auto Buffer(Read<std::vector<T>>(Start, NumElems));
      // Search cache for supplied value
      auto Iter = std::find(Buffer.begin(), Buffer.end(), Data);
      // If value was found return its address, or null otherwise
      return Iter != Buffer.end() ? reinterpret_cast<PBYTE>(Start) + 
        std::distance(Buffer.begin(), Iter) * sizeof(T) : nullptr;
    }

    // Search memory (string types)
    template <typename T>
    PVOID MemoryMgr::Find(T const& Data, PVOID Start, PVOID End, typename 
      boost::enable_if<std::is_same<T, std::basic_string<typename T::
      value_type>>>::type* /*Dummy*/) const
    {
      BOOST_THROW_EXCEPTION(MemoryError() << 
        ErrorFunction("MemoryMgr::Find") << 
        ErrorString("Operation not yet supported."));
    }

    // Search memory (vector types)
    template <typename T>
    PVOID MemoryMgr::Find(T const& Data, PVOID Start, PVOID End, 
      std::wstring const& Mask, typename boost::enable_if<std::is_same<T, 
      std::vector<typename T::value_type>>>::type* /*Dummy1*/, typename boost::
      enable_if<std::is_pod<typename T::value_type>>::type* /*Dummy2*/) const
    {
      // Ensure there is data to process
      if (Data.empty())
      {
        BOOST_THROW_EXCEPTION(MemoryError() << 
          ErrorFunction("MemoryMgr::Find") << 
          ErrorString("Mask does not match data."));
      }

      // Ensure mask matches data
      if (!Mask.empty() && Mask.size() != Data.size())
      {
        BOOST_THROW_EXCEPTION(MemoryError() << 
          ErrorFunction("MemoryMgr::Find") << 
          ErrorString("Mask does not match data."));
      }

      // Rather than performing a read for each address we instead perform 
      // caching.
      std::shared_ptr<std::vector<BYTE>> MyBuffer;

      // Loop over entire memory region
      for (auto Address = static_cast<PBYTE>(Start); 
        Address != static_cast<PBYTE>(End) - Data.size(); 
        ++Address) 
      {
        // Read 0x5000 bytes at a time
        DWORD_PTR ChunkSize = 0x5000;
        // Calculate current chunk offset
        DWORD_PTR Offset = reinterpret_cast<DWORD_PTR>(Address) % ChunkSize;
        // Whenever we reach the chunk size we need to re-cache
        if (Offset == 0 || !MyBuffer)
        {
          MyBuffer.reset(new std::vector<BYTE>(Read<std::vector<BYTE>>(Address, 
            ChunkSize + Data.size())));
        }

        // Check if current address matches buffer
        bool Found = true;
        for (T::size_type i = 0; i != Data.size(); ++i)
        {
          T::value_type* TempAddr = reinterpret_cast<T::value_type*>(
            reinterpret_cast<PBYTE>(&(*MyBuffer)[0]) + Offset);
          if (Mask.empty() || Mask[i] == 'x')
          {
            if (TempAddr[i] != Data[i])
            {
              Found = false;
              break;
            }
          }
        }
        
        // If the buffer matched return the current address
        if (Found)
        {
          return Address;
        }
      }

      // Nothing found, return null
      return nullptr;
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
