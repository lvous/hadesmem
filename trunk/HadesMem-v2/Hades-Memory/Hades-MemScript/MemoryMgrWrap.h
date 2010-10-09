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

// C++ Standard Library
#include <string>
#include <vector>
#include <algorithm>

// Boost
#pragma warning(push, 1)
#pragma warning(disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#pragma warning(pop)

// Windows API
#include <Windows.h>

// Hades
#include "Hades-Common/I18n.h"
#include "Hades-Memory/Types.h"
#include "Hades-Memory/MemoryMgr.h"

// Wrapper function generator for MemoryMgr::Read
#define HADESMEM_SCRIPTING_GEN_READ(x) \
  Types::x Read##x(DWORD_PTR Address)\
{\
  return MemoryMgr::Read<Types::x>(reinterpret_cast<PVOID>(Address));\
}\


// Wrapper function generator for MemoryMgr::Write
#define HADESMEM_SCRIPTING_GEN_WRITE(x) \
  void Write##x(DWORD_PTR Address, Types::x Data)\
{\
  MemoryMgr::Write<Types::x>(reinterpret_cast<PVOID>(Address), Data);\
}\

namespace Hades
{
  namespace Memory
  {
    namespace Wrappers
    {
      class MemoryMgrWrappers : public MemoryMgr
      {
      public:
        explicit MemoryMgrWrappers(DWORD ProcID)
          : MemoryMgr(ProcID)
        { }

        explicit MemoryMgrWrappers(std::string const& ProcName) 
          : MemoryMgr(boost::lexical_cast<std::wstring>(ProcName))
        { }

        explicit MemoryMgrWrappers(std::string const& WindowName, 
          std::string const& ClassName)
          : MemoryMgr(boost::lexical_cast<std::wstring>(WindowName), 
          boost::lexical_cast<std::wstring>(ClassName))
        { }

        // MemoryMgr::CanRead wrapper
        bool CanRead(DWORD_PTR Address)
        {
          return MemoryMgr::CanRead(reinterpret_cast<PVOID>(Address));
        }

        // MemoryMgr::CanWrite wrapper
        bool CanWrite(DWORD_PTR Address)
        {
          return MemoryMgr::CanWrite(reinterpret_cast<PVOID>(Address));
        }

        // MemoryMgr::Call wrapper
        DWORD Call(DWORD_PTR Address, std::vector<DWORD_PTR> const& Args, 
          CallConv MyCallConv)
        {
          std::vector<PVOID> ArgsNew;
          std::transform(Args.cbegin(), Args.cend(), std::back_inserter(
            ArgsNew), 
            [] (DWORD_PTR Arg)
          {
            return reinterpret_cast<PVOID>(Arg);
          });

          return MemoryMgr::Call(reinterpret_cast<PVOID>(Address), ArgsNew, 
            MyCallConv);
        }

        // MemoryMgr::Alloc wrapper
        DWORD_PTR Alloc(SIZE_T Size)
        {
          return reinterpret_cast<DWORD_PTR>(MemoryMgr::Alloc(Size));
        }

        // MemoryMgr::Free wrapper
        void Free(DWORD_PTR Address)
        {
          return MemoryMgr::Free(reinterpret_cast<PVOID>(Address));
        }

        // MemoryMgr::GetProcessHandle wrapper
        DWORD_PTR GetProcessHandle()
        {
          return reinterpret_cast<DWORD_PTR>(MemoryMgr::GetProcessHandle());
        }

        // MemoryMgr::GetRemoteProcAddress wrapper
        DWORD_PTR GetRemoteProcAddressByOrdinal(DWORD_PTR RemoteMod, 
          std::string const& Module, WORD Ordinal)
        {
          return reinterpret_cast<DWORD_PTR>(MemoryMgr::GetRemoteProcAddress(
            reinterpret_cast<HMODULE>(RemoteMod), 
            boost::lexical_cast<std::wstring>(Module), 
            reinterpret_cast<LPCSTR>(MAKELONG(Ordinal, 0))));
        }

        // MemoryMgr::GetRemoteProcAddress wrapper
        DWORD_PTR GetRemoteProcAddressByName(DWORD_PTR RemoteMod, 
          std::string const& Module, std::string const& Function)
        {
          return reinterpret_cast<DWORD_PTR>(MemoryMgr::GetRemoteProcAddress(
            reinterpret_cast<HMODULE>(RemoteMod), 
            boost::lexical_cast<std::wstring>(Module), 
            Function.c_str()));
        }

        // MemoryMgr::FlushCache wrapper
        void FlushCache(DWORD_PTR Address, SIZE_T Size)
        {
          return MemoryMgr::FlushCache(reinterpret_cast<LPVOID>(Address), 
            Size);
        }

        // MemoryMgr::Read<T> wrappers
        HADESMEM_SCRIPTING_GEN_READ(Int8)
        HADESMEM_SCRIPTING_GEN_READ(UInt8)
        HADESMEM_SCRIPTING_GEN_READ(Int16)
        HADESMEM_SCRIPTING_GEN_READ(UInt16)
        HADESMEM_SCRIPTING_GEN_READ(Int32)
        HADESMEM_SCRIPTING_GEN_READ(UInt32)
        HADESMEM_SCRIPTING_GEN_READ(Int64)
        HADESMEM_SCRIPTING_GEN_READ(UInt64)
        HADESMEM_SCRIPTING_GEN_READ(Float)
        HADESMEM_SCRIPTING_GEN_READ(Double)
        HADESMEM_SCRIPTING_GEN_READ(StringA)

        // Wrapper function generator for MemoryMgr::ReadCharNarrow
        std::string ReadCharA(DWORD_PTR Address)
        {
          std::string MyStr;
          MyStr += MemoryMgr::Read<Types::CharA>(reinterpret_cast<PVOID>(
            Address));
          return MyStr;
        }

        // Wrapper function generator for MemoryMgr::ReadCharWide
        std::string ReadCharW(DWORD_PTR Address)
        {
          std::wstring MyStr;
          MyStr += MemoryMgr::Read<Types::CharW>(reinterpret_cast<PVOID>(
            Address));
          return boost::lexical_cast<std::string>(MyStr);
        }

        // Wrapper function generator for MemoryMgr::ReadStrWide
        std::string ReadStringW(DWORD_PTR Address)
        {
          return boost::lexical_cast<std::string>(MemoryMgr::Read<Types::
            StringW>(reinterpret_cast<PVOID>(Address)));
        }

        // Wrapper function generator for MemoryMgr::ReadPointer
        DWORD_PTR ReadPointer(DWORD_PTR Address)
        {
          return reinterpret_cast<DWORD_PTR>(MemoryMgr::Read<Types::Pointer>(
            reinterpret_cast<PVOID>(Address)));
        }

        // MemoryMgr::Write<T> wrappers
        HADESMEM_SCRIPTING_GEN_WRITE(Int8)
        HADESMEM_SCRIPTING_GEN_WRITE(UInt8)
        HADESMEM_SCRIPTING_GEN_WRITE(Int16)
        HADESMEM_SCRIPTING_GEN_WRITE(UInt16)
        HADESMEM_SCRIPTING_GEN_WRITE(Int32)
        HADESMEM_SCRIPTING_GEN_WRITE(UInt32)
        HADESMEM_SCRIPTING_GEN_WRITE(Int64)
        HADESMEM_SCRIPTING_GEN_WRITE(UInt64)
        HADESMEM_SCRIPTING_GEN_WRITE(Float)
        HADESMEM_SCRIPTING_GEN_WRITE(Double)
        HADESMEM_SCRIPTING_GEN_WRITE(StringA)

        // Wrapper function generator for MemoryMgr::WriteCharNarrow
        void WriteCharA(DWORD_PTR Address, std::string const& Value)
        {
          if (Value.size() != 1)
          {
            BOOST_THROW_EXCEPTION(HadesMemError() << 
              ErrorFunction("Memory_WriteCharNarrow") << 
              ErrorString("Value invalid (must be a single character)."));
          }

          MemoryMgr::Write(reinterpret_cast<PVOID>(Address), Value[0]);
        }

        // Wrapper function generator for MemoryMgr::WriteCharWide
        void WriteCharW(DWORD_PTR Address, std::string const& Value)
        {
          if (Value.size() != 1)
          {
            BOOST_THROW_EXCEPTION(HadesMemError() << 
              ErrorFunction("Memory_WriteCharWide") << 
              ErrorString("Value invalid (must be a single character)."));
          }

          MemoryMgr::Write(reinterpret_cast<PVOID>(Address), 
            boost::lexical_cast<std::wstring>(Value)[0]);
        }

        // Wrapper function generator for MemoryMgr::ReadStrWide
        void WriteStringW(DWORD_PTR Address, std::string const& Value)
        {
          MemoryMgr::Write(reinterpret_cast<PVOID>(Address), 
            boost::lexical_cast<std::wstring>(Value));
        }

        // Wrapper function generator for MemoryMgr::ReadPointer
        void WritePointer(DWORD_PTR Address, DWORD_PTR Value)
        {
          MemoryMgr::Write(reinterpret_cast<PVOID>(Address), 
            reinterpret_cast<Types::Pointer>(Value));
        }
      };
    }
  }
}

#undef HADESMEM_SCRIPTING_GEN_READ
#undef HADESMEM_SCRIPTING_GEN_WRITE
