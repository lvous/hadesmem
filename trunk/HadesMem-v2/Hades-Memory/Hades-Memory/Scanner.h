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

#pragma once

// Windows API
#include <tchar.h>
#include <Windows.h>

// C++ Standard Library
#include <map>
#include <string>
#include <vector>

// Boost
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/filesystem.hpp>
#include <boost/noncopyable.hpp>
#pragma warning(pop)

// Hades
#include "MemoryMgr.h"

namespace Hades
{
  namespace Memory
  {
    // Memory searching class
    class Scanner : private boost::noncopyable
    {
    public:
      // Scanner exception type
      class Error : public virtual HadesMemError 
      { };

      // Constructor
      explicit Scanner(MemoryMgr& MyMemory);
      Scanner(MemoryMgr& MyMemory, HMODULE Module);
      Scanner(MemoryMgr& MyMemory, PVOID Start, PVOID End);

      // Search memory (POD types)
      template <typename T>
      PVOID Find(T const& Data, typename boost::enable_if<std::is_pod<T>>::
        type* Dummy = 0) const;

      // Search memory (string types)
      template <typename T>
      PVOID Find(T const& Data, typename boost::enable_if<std::is_same<T, std::
        basic_string<typename T::value_type>>>::type* Dummy = 0) const;

      // Search memory (vector types)
      template <typename T>
      PVOID Find(T const& Data, std::basic_string<TCHAR> const& Mask = _T(""), 
        typename boost::enable_if<std::is_same<T, std::vector<typename T::
        value_type>>>::type* Dummy1 = 0, typename boost::enable_if<std::
        is_pod<typename T::value_type>>::type* Dummy2 = 0) const;

      // Search memory (POD types)
      template <typename T>
      std::vector<PVOID> FindAll(T const& data, typename boost::enable_if<std::
        is_pod<T>>::type* Dummy = 0) const;

      // Search memory (string types)
      template <typename T>
      std::vector<PVOID> FindAll(T const& Data, typename boost::enable_if<std::
        is_same<T, std::basic_string<typename T::value_type>>>::type* Dummy 
        = 0) const;

      // Search memory (vector types)
      template <typename T>
      std::vector<PVOID> FindAll(T const& Data, 
        std::basic_string<TCHAR> const& Mask = _T(""), 
        typename boost::enable_if<std::is_same<T, std::vector<typename 
        T::value_type>>>::type* Dummy1 = 0, typename boost::enable_if<std::
        is_pod<typename T::value_type>>::type* Dummy2 = 0) const;

      // Load patterns from XML file
      void LoadFromXML(boost::filesystem::path const& Path);

      // Get address map
      std::map<std::basic_string<TCHAR>, PVOID> GetAddresses() const;

      // Operator[] overload to allow retrieving addresses by name
      PVOID operator[](std::basic_string<TCHAR> const& Name) const;

    private:
      // Memory manager instance
      MemoryMgr* m_pMemory;

      // Start and end addresses of search region
      PBYTE m_Start;
      PBYTE m_End;

      // Map to hold addresses
      std::map<std::basic_string<TCHAR>, PVOID> m_Addresses;
    };

    // Search memory (POD types)
    template <typename T>
    PVOID Scanner::Find(T const& Data, typename boost::enable_if<std::is_pod<
      T>>:: type* /*Dummy*/) const
    {
      // Put data in container
      std::vector<T> Buffer;
      Buffer.push_back(Data);
      // Use vector specialization of FindAll
      return Find(Buffer);
    }

    // Search memory (POD types)
    template <typename T>
    std::vector<PVOID> Scanner::FindAll(T const& Data, typename boost::
      enable_if<std::is_pod<T>>::type* /*Dummy*/) const
    {
      // Put data in container
      std::vector<T> Buffer;
      Buffer.push_back(Data);
      // Use vector specialization of FindAll
      return FindAll(Buffer);
    }

    // Search memory (string types)
    template <typename T>
    PVOID Scanner::Find(T const& Data, typename boost::enable_if<std::is_same<
      T, std::basic_string<typename T::value_type>>>::type* /*Dummy*/) const
    {
      // Convert string to character buffer
      std::vector<T::value_type> const MyBuffer(Data.cbegin(), Data.cend());
      // Use vector specialization of find
      return Find(MyBuffer);
    }

    template <typename T>
    std::vector<PVOID> Scanner::FindAll(T const& Data, typename boost::
      enable_if<std::is_same<T, std::basic_string<typename T::value_type>>>::
      type* /*Dummy*/) const
    {
      // Convert string to character buffer
      std::vector<T::value_type> const MyBuffer(Data.cbegin(), Data.cend());
      // Use vector specialization of find all
      return FindAll(MyBuffer);
    }

    // Search memory (vector types)
    // Fixme: This function is extremely inefficient and full of potential 
    // bugs. Perform a thorough review and rewrite.
    // Fixme: Refactor Find and FindAll to factor out duplicated code.
    template <typename T>
    PVOID Scanner::Find(T const& Data, std::basic_string<TCHAR> const& Mask, 
      typename boost::enable_if<std::is_same<T, std::vector<typename T::
      value_type>>>::type* /*Dummy1*/, typename boost::enable_if<std::is_pod<
      typename T::value_type>>::type* /*Dummy2*/) const
    {
      // Ensure there is data to process
      if (Data.empty())
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("MemoryMgr::Find") << 
          ErrorString("Mask does not match data."));
      }

      // Ensure mask matches data
      if (!Mask.empty() && Mask.size() != Data.size())
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("MemoryMgr::Find") << 
          ErrorString("Mask does not match data."));
      }

      // Get system information
      SYSTEM_INFO MySystemInfo = { 0 };
      GetSystemInfo(&MySystemInfo);
      DWORD const PageSize = MySystemInfo.dwPageSize;
      PVOID const MinAddr = MySystemInfo.lpMinimumApplicationAddress;
      PVOID const MaxAddr = MySystemInfo.lpMaximumApplicationAddress;

      // Loop over all memory pages
      for (PBYTE Address = static_cast<PBYTE>(MinAddr); Address < MaxAddr; 
        Address += PageSize)
      {
        // Skip region if out of bounds
        if (Address + PageSize - 1 < m_Start)
        {
          continue;
        }

        // Quit if out of bounds
        if (Address > m_End)
        {
          break;
        }

        // Check for invalid memory
        MEMORY_BASIC_INFORMATION MyMbi1 = { 0 };
        if (!VirtualQueryEx(m_pMemory->GetProcessHandle(), Address, &MyMbi1, 
          sizeof(MyMbi1)) || (MyMbi1.Protect & PAGE_GUARD) == PAGE_GUARD)
        {
          continue;
        }

        // Check for invalid memory
        MEMORY_BASIC_INFORMATION MyMbi2 = { 0 };
        if (!VirtualQueryEx(m_pMemory->GetProcessHandle(), Address + 
          PageSize, &MyMbi2, sizeof(MyMbi2)) || (MyMbi2.Protect & 
          PAGE_GUARD) == PAGE_GUARD)
        {
          continue;
        }

        // Read vector of Ts into cache
        std::vector<BYTE> Buffer;
          
        try
        {
          // Efficiency of assignment should not be an issue here (for 
          // C++0x).
          // 12.8/15 states that compilers can perform RVO if possible.
          // In the case that this is not possible (or the compiler chooses 
          // not to do it), 13.3.3.2/3 states that an rvalue ref binds to an 
          // rvalue better than an lvalue ref so the move constructor will 
          // be chosen.
          // Fixme: Confirm that under the primary implementation being 
          // used currently (MSVC10) that a copy never occurs here.
          // Fixme: If we're reading across a region boundary and we hit 
          // inaccessible memory we should simply read all we can, rather 
          // than skipping the block entirely.
          Buffer = m_pMemory->Read<std::vector<BYTE>>(Address, PageSize + 
            Data.size() * sizeof(T::value_type));
        }
        // Ignore any memory errors, as there's nothing we can do about them
        // Fixme: Detect memory read errors and drop back to a slower but 
        // more reliable implementation.
        catch (MemoryMgr::Error const& /*e*/)
        {
          continue;
        }

        // Loop over entire memory region
        for (PBYTE Current = &Buffer[0]; Current != &Buffer[0] + 
          Buffer.size(); ++Current) 
        {
          // Check if current address matches buffer
          bool Found = true;
          for (std::size_t i = 0; i != Data.size(); ++i)
          {
            T::value_type const* CurrentTemp = 
              reinterpret_cast<T::value_type const*>(Current);
            if ((Mask.empty() || Mask[i] == L'x') && (CurrentTemp[i] != 
              Data[i]))
            {
              Found = false;
              break;
            }
          }

          // If the buffer matched return the current address
          if (Found)
          {
            // If the buffer matched and the address is valid, return the 
            // current address.
            // Fixme: Do this check in the outer loop, and break if possible 
            // rather than continuing.
            PVOID const AddressReal = Address + (Current - &Buffer[0]);
            if (AddressReal >= m_Start && AddressReal <= m_End)
            {
              return AddressReal;
            }
          }
        }
      }

      // Nothing found, return null
      return nullptr;
    }

    // Search memory (vector types)
    // Fixme: This function is extremely inefficient and full of potential 
    // bugs. Perform a thorough review and rewrite.
    // Fixme: Refactor Find and FindAll to factor out duplicated code.
    template <typename T>
    std::vector<PVOID> Scanner::FindAll(T const& Data, 
      std::basic_string<TCHAR> const& Mask, typename boost::enable_if<std::
      is_same<T, std::vector<typename T::value_type>>>::type* /*Dummy1*/, 
      typename boost::enable_if<std::is_pod<typename T::value_type>>::type* 
      /*Dummy2*/) const
    {
      // Ensure there is data to process
      if (Data.empty())
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("MemoryMgr::Find") << 
          ErrorString("Mask does not match data."));
      }

      // Ensure mask matches data
      if (!Mask.empty() && Mask.size() != Data.size())
      {
        BOOST_THROW_EXCEPTION(Error() << 
          ErrorFunction("MemoryMgr::Find") << 
          ErrorString("Mask does not match data."));
      }

      // Addresses of matches
      std::vector<PVOID> Matches;

      // Get system information
      SYSTEM_INFO MySystemInfo = { 0 };
      GetSystemInfo(&MySystemInfo);
      DWORD const PageSize = MySystemInfo.dwPageSize;
      PVOID const MinAddr = MySystemInfo.lpMinimumApplicationAddress;
      PVOID const MaxAddr = MySystemInfo.lpMaximumApplicationAddress;

      // Loop over all memory pages
      for (PBYTE Address = static_cast<PBYTE>(MinAddr); Address < MaxAddr; 
        Address += PageSize)
      {
        // Skip region if out of bounds
        if (Address + PageSize - 1 < m_Start)
        {
          continue;
        }

        // Quit if out of bounds
        if (Address > m_End)
        {
          break;
        }

        // Check for invalid memory
        MEMORY_BASIC_INFORMATION MyMbi1 = { 0 };
        if (!VirtualQueryEx(m_pMemory->GetProcessHandle(), Address, &MyMbi1, 
          sizeof(MyMbi1)) || (MyMbi1.Protect & PAGE_GUARD) == PAGE_GUARD)
        {
          continue;
        }

        // Check for invalid memory
        MEMORY_BASIC_INFORMATION MyMbi2 = { 0 };
        if (!VirtualQueryEx(m_pMemory->GetProcessHandle(), Address + PageSize, 
          &MyMbi2, sizeof(MyMbi2)) || (MyMbi2.Protect & PAGE_GUARD) == 
          PAGE_GUARD)
        {
          continue;
        }

        // Read vector of Ts into cache
        std::vector<BYTE> Buffer;

        try
        {
          // Efficiency of assignment should not be an issue here (for 
          // C++0x).
          // 12.8/15 states that compilers can perform RVO if possible.
          // In the case that this is not possible (or the compiler chooses 
          // not to do it), 13.3.3.2/3 states that an rvalue ref binds to an 
          // rvalue better than an lvalue ref so the move constructor will 
          // be chosen.
          // Fixme: Confirm that under the primary implementation being 
          // used currently (MSVC10) that a copy never occurs here.
          // Fixme: If we're reading across a region boundary and we hit 
          // inaccessible memory we should simply read all we can, rather 
          // than skipping the block entirely.
          Buffer = m_pMemory->Read<std::vector<BYTE>>(Address, PageSize + 
            Data.size() * sizeof(T::value_type));
        }
        // Ignore any memory errors, as there's nothing we can do about them
        // Fixme: Detect memory read errors and drop back to a slower but 
        // more reliable implementation.
        catch (MemoryMgr::Error const& /*e*/)
        {
          continue;
        }

        // Loop over entire memory region
        for (PBYTE Current = &Buffer[0]; Current != &Buffer[0] + 
          Buffer.size(); ++Current) 
        {
          // Check if current address matches buffer
          bool Found = true;
          for (std::size_t i = 0; i != Data.size(); ++i)
          {
            T::value_type const* CurrentTemp = 
              reinterpret_cast<T::value_type const*>(Current);
            if ((Mask.empty() || Mask[i] == L'x') && (CurrentTemp[i] != 
              Data[i]))
            {
              Found = false;
              break;
            }
          }

          // If the buffer matched return the current address
          if (Found)
          {
            // If the buffer matched and the address is valid, return the 
            // current address.
            // Fixme: Do this check in the outer loop, and break if possible 
            // rather than continuing.
            PVOID const AddressReal = Address + (Current - &Buffer[0]);
            if (AddressReal >= m_Start && AddressReal <= m_End)
            {
              Matches.push_back(AddressReal);
            }
          }
        }
      }

      // Return matches
      return Matches;
    }
  }
}
