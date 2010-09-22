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
#include <iterator>
#include <algorithm>

// Boost
#pragma warning(push, 1)
#pragma warning(disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/lexical_cast.hpp>
#pragma warning(pop)

// Windows API
#include <Windows.h>

// HadesMem
#include "Types.h"
#include "Scanner.h"
#include "MemoryMgr.h"
#include "Hades-Common/I18n.h"

// Wrapper function generator for MyScanner::Find
#define HADESMEM_SCRIPTING_GEN_FIND(x) \
DWORD_PTR Find##x(Types::x Data)\
{\
  return reinterpret_cast<DWORD_PTR>(Scanner::Find(Data));\
}

// Wrapper function generator for MyScanner::Find
#define HADESMEM_SCRIPTING_GEN_FIND_ALL(x) \
DwordPtrList FindAll##x(Types::x Data)\
{\
  std::vector<PVOID> AddrList(Scanner::FindAll(Data));\
  DwordPtrList NewList;\
  NewList.List.reserve(AddrList.size());\
  std::transform(AddrList.cbegin(), AddrList.cend(), std::back_inserter(\
  NewList.List), \
  [] (PVOID Current)\
  {\
    return reinterpret_cast<DWORD_PTR>(Current);\
  });\
  return NewList;\
}

namespace Hades
{
  namespace Memory
  {
    namespace Wrappers
    {
      class ScannerWrappers : public Scanner
      {
      public:
        explicit ScannerWrappers(MemoryMgr& MyMemory) 
          : Scanner(MyMemory)
        { }

        ScannerWrappers(MemoryMgr& MyMemory, DWORD_PTR Module) 
          : Scanner(MyMemory, reinterpret_cast<HMODULE>(Module))
        { }

        ScannerWrappers(MemoryMgr& MyMemory, DWORD_PTR Start, 
          DWORD_PTR End) 
          : Scanner(MyMemory, reinterpret_cast<PVOID>(Start), 
          reinterpret_cast<PVOID>(End))
        { }

        // Scanner::LoadFromXML wrapper
        void LoadFromXML(std::string const& Path)
        {
          Scanner::LoadFromXML(boost::lexical_cast<std::wstring>(Path));
        }

        // Scanner::operator[] wrapper
        DWORD_PTR GetAddress(std::string const& Name)
        {
          return reinterpret_cast<DWORD_PTR>(Scanner::operator[](
            boost::lexical_cast<std::wstring>(Name)));
        }

        // Scanner::Find<T> wrappers
        HADESMEM_SCRIPTING_GEN_FIND(Int8)
        HADESMEM_SCRIPTING_GEN_FIND(UInt8)
        HADESMEM_SCRIPTING_GEN_FIND(Int16)
        HADESMEM_SCRIPTING_GEN_FIND(UInt16)
        HADESMEM_SCRIPTING_GEN_FIND(Int32)
        HADESMEM_SCRIPTING_GEN_FIND(UInt32)
        HADESMEM_SCRIPTING_GEN_FIND(Int64)
        HADESMEM_SCRIPTING_GEN_FIND(UInt64)
        HADESMEM_SCRIPTING_GEN_FIND(Float)
        HADESMEM_SCRIPTING_GEN_FIND(Double)
        HADESMEM_SCRIPTING_GEN_FIND(StrNarrow)

        // Wrapper function for Scanner::FindCharNarrow
        DWORD_PTR FindCharNarrow(std::string const& Data)
        {
          if (Data.size() != 1)
          {
            BOOST_THROW_EXCEPTION(HadesMemError() << 
              ErrorFunction("Scanner_FindCharNarrow") << 
              ErrorString("Value invalid (must be a single character)."));
          }
          return reinterpret_cast<DWORD_PTR>(Scanner::Find(Data[0]));
        }

        // Wrapper function for Scanner::FindCharWide
        DWORD_PTR FindCharWide(std::string const& Data)
        {
          if (Data.size() != 1)
          {
            BOOST_THROW_EXCEPTION(HadesMemError() << 
              ErrorFunction("Scanner_FindCharNarrow") << 
              ErrorString("Value invalid (must be a single character)."));
          }
          return reinterpret_cast<DWORD_PTR>(Scanner::Find(
            boost::lexical_cast<Types::StrWide>(Data[0])));
        }

        // Wrapper function for Scanner::FindStrWide
        DWORD_PTR FindStrWide(std::string const& Data)
        {
          return reinterpret_cast<DWORD_PTR>(Scanner::Find(
            boost::lexical_cast<Types::StrWide>(Data)));
        }

        // Wrapper function for Scanner::FindPointer
        DWORD_PTR FindPointer(DWORD_PTR Data)
        {
          return reinterpret_cast<DWORD_PTR>(Scanner::Find(
            reinterpret_cast<Types::Pointer>(Data)));
        }

        // Dword pointer list wrapper
        struct DwordPtrList
        {
          std::vector<DWORD_PTR> List;
        };

        // Scanner::FindAll<T> wrappers
        HADESMEM_SCRIPTING_GEN_FIND_ALL(Int8)
        HADESMEM_SCRIPTING_GEN_FIND_ALL(UInt8)
        HADESMEM_SCRIPTING_GEN_FIND_ALL(Int16)
        HADESMEM_SCRIPTING_GEN_FIND_ALL(UInt16)
        HADESMEM_SCRIPTING_GEN_FIND_ALL(Int32)
        HADESMEM_SCRIPTING_GEN_FIND_ALL(UInt32)
        HADESMEM_SCRIPTING_GEN_FIND_ALL(Int64)
        HADESMEM_SCRIPTING_GEN_FIND_ALL(UInt64)
        HADESMEM_SCRIPTING_GEN_FIND_ALL(Float)
        HADESMEM_SCRIPTING_GEN_FIND_ALL(Double)
        HADESMEM_SCRIPTING_GEN_FIND_ALL(StrNarrow)

        // Wrapper function for Scanner::FindAllCharNarrow
        DwordPtrList FindAllCharNarrow(std::string const& Data)
        {
          if (Data.size() != 1)
          {
            BOOST_THROW_EXCEPTION(HadesMemError() << 
              ErrorFunction("Scanner_FindAllCharNarrow") << 
              ErrorString("Value invalid (must be a single character)."));
          }
          std::vector<PVOID> AddrList(Scanner::FindAll(Data[0]));
          DwordPtrList NewList;
          NewList.List.reserve(AddrList.size());
          std::transform(AddrList.cbegin(), AddrList.cend(), 
            std::back_inserter(NewList.List), 
            [] (PVOID Current)
          {
            return reinterpret_cast<DWORD_PTR>(Current);
          });
          return NewList;
        }

        // Wrapper function for Scanner::FindAllCharWide
        DwordPtrList FindAllCharWide(std::string const& Data)
        {
          if (Data.size() != 1)
          {
            BOOST_THROW_EXCEPTION(HadesMemError() << 
              ErrorFunction("Scanner_FindAllCharWide") << 
              ErrorString("Value invalid (must be a single character)."));
          }
          std::vector<PVOID> AddrList(Scanner::FindAll(
            boost::lexical_cast<Types::StrWide>(Data[0])));
          DwordPtrList NewList;
          NewList.List.reserve(AddrList.size());
          std::transform(AddrList.cbegin(), AddrList.cend(), 
            std::back_inserter(NewList.List), 
            [] (PVOID Current)
          {
            return reinterpret_cast<DWORD_PTR>(Current);
          });
          return NewList;
        }

        // Wrapper function for Scanner::FindAllStrWide
        DwordPtrList FindAllStrWide(std::string const& Data)
        {
          std::vector<PVOID> AddrList(Scanner::FindAll(
            boost::lexical_cast<Types::StrWide>(Data[0])));
          DwordPtrList NewList;
          NewList.List.reserve(AddrList.size());
          std::transform(AddrList.cbegin(), AddrList.cend(), 
            std::back_inserter(NewList.List), 
            [] (PVOID Current)
          {
            return reinterpret_cast<DWORD_PTR>(Current);
          });
          return NewList;
        }

        // Wrapper function for Scanner::FindAllPointer
        DwordPtrList FindAllPointer(DWORD_PTR Data)
        {
          std::vector<PVOID> AddrList(Scanner::FindAll(
            reinterpret_cast<Types::Pointer>(Data)));
          DwordPtrList NewList;
          NewList.List.reserve(AddrList.size());
          std::transform(AddrList.cbegin(), AddrList.cend(), 
            std::back_inserter(NewList.List), 
            [] (PVOID Current)
          {
            return reinterpret_cast<DWORD_PTR>(Current);
          });
          return NewList;
        }
      };
    }
  }
}

#undef HADESMEM_SCRIPTING_GEN_FIND
#undef HADESMEM_SCRIPTING_GEN_FIND_ALL
