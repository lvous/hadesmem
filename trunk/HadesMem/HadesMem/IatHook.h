/*
This file is part of HadesMem.
Copyright © 2010 Cypherjb (aka Chazwazza, aka Cypher). 
<http://www.cypherjb.com/> <cypher.jb@gmail.com>

HadesMem is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

HadesMem is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

// C++ Standard Library
#include <string>

// Boost
#pragma warning(push, 1)
#include <boost/format.hpp>
#include <boost/noncopyable.hpp>
#pragma warning(pop)

// Windows API
#include <Windows.h>

// Hades
#include "I18n.h"
#include "Module.h"
#include "Memory.h"

namespace Hades
{
  namespace Memory
  {
    // IatHook exception type
    class IatHookError : public virtual HadesMemError 
    { };

    class IatHook : private boost::noncopyable
    {
    public:
      // Constructor
      inline IatHook(MemoryMgr const& MyMemory, std::wstring const& ModuleName, 
        std::wstring const& FunctionName, PVOID Hook, HMODULE Excluded = 
        nullptr);

      // Destructor
      inline ~IatHook();

      // Perform IAT hook for single module
      inline void IatHookSingle(HMODULE TargetMod, 
        std::wstring const& ModuleName, PVOID Orig, PVOID Hook);

      // Perform IAT hook for all modules
      inline void IatHookAll(std::wstring const& ModuleName, PVOID Orig, 
        PVOID Hook);

      // Get original function
      template <typename T>
      T GetOrig() const 
      {
        static_assert(sizeof(T) == sizeof(PVOID), "Invalid type. Size "
          "mismatch.");
        return reinterpret_cast<T>(m_Orig);
      }

    private:
      MemoryMgr const& m_Memory;
      std::wstring m_ModuleName;
      std::wstring m_FunctionName;
      PVOID m_Hook;
      PVOID m_Orig;
      HMODULE m_Excluded;
    };

    IatHook::IatHook(MemoryMgr const& MyMemory, std::wstring const& ModuleName, 
      std::wstring const& FunctionName, PVOID Hook, HMODULE Excluded) 
      : m_Memory(MyMemory), 
      m_ModuleName(Util::ToLower(ModuleName)), 
      m_FunctionName(FunctionName), 
      m_Hook(Hook), 
      m_Orig(0), 
      m_Excluded(Excluded)
    {
      // Get handle to target module
      Module TargetMod(ModuleName, MyMemory);
      if (!TargetMod.Found())
      {
        BOOST_THROW_EXCEPTION(IatHookError() << 
          ErrorFunction("IatHook::IatHook") << 
          ErrorString("Could not find target module."));
      }

      // Find function in target module
      m_Orig = m_Memory.GetRemoteProcAddress(TargetMod.GetBase(), ModuleName, 
        Util::ConvertStr(FunctionName).c_str());
      if (!m_Orig)
      {
        BOOST_THROW_EXCEPTION(IatHookError() << 
          ErrorFunction("IatHook::IatHook") << 
          ErrorString("Could not find target function."));
      }

      // Perform hook
      IatHookAll(m_ModuleName, m_Orig, m_Hook);
    }
    
    // Destructor
    IatHook::~IatHook()
    {
      // Reverse hook
      IatHookAll(m_ModuleName, m_Hook, m_Orig);
    }

    // Perform IAT hook for single module
    void IatHook::IatHookSingle(HMODULE TargetMod, 
      std::wstring const& ModuleName, PVOID Orig, PVOID Hook)
    {
      // Ensure target is a valid PE file
      auto pBase = reinterpret_cast<PBYTE>(TargetMod);
      auto DosHeader = m_Memory.Read<IMAGE_DOS_HEADER>(pBase);
      if (DosHeader.e_magic != IMAGE_DOS_SIGNATURE)
      {
        return;
      }
      auto NtHeader = m_Memory.Read<IMAGE_NT_HEADERS>(pBase + DosHeader.
        e_lfanew);
      if (NtHeader.Signature != IMAGE_NT_SIGNATURE)
      {
        return;
      }

      // Ensure target has an IAT
      if (!NtHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].
        Size)
      {
        return;
      }

      // Get IAT descriptor pointer
      auto pImportDirDesc = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(
        pBase + NtHeader.OptionalHeader.DataDirectory
        [IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

      // Loop over all modules
      for (auto ImportDirDesc(m_Memory.Read<IMAGE_IMPORT_DESCRIPTOR>(
        pImportDirDesc)); ImportDirDesc.Name; ImportDirDesc = m_Memory.
        Read<IMAGE_IMPORT_DESCRIPTOR>(++pImportDirDesc)) 
      {
        // Get module name
        std::string const CurModName(Util::ToLower(m_Memory.Read<std::string>(
          pBase + ImportDirDesc.Name)));

        // Get target module name (lowercase and converted to narrow string)
        std::string const ModNameLower(boost::lexical_cast<std::string>(
          Util::ToLower(ModuleName)));

        // Check if we found target module
        if (CurModName == ModNameLower)
        {
          // Get thunk data for module
          auto pThunk = reinterpret_cast<PIMAGE_THUNK_DATA>(pBase + 
            ImportDirDesc.FirstThunk);

          // Replace current function address with new function address
          for (auto Thunk(m_Memory.Read<IMAGE_THUNK_DATA>(pThunk)); 
            Thunk.u1.Function; Thunk = m_Memory.Read<IMAGE_THUNK_DATA>(
            ++pThunk))
          {
            // Get the address of the function address
            auto pCurOrig = reinterpret_cast<PROC*>(reinterpret_cast<PBYTE>(
              pThunk) + offsetof(IMAGE_THUNK_DATA, u1.Function));
            auto const CurOrig = m_Memory.Read<PROC>(pCurOrig);

            // Check whether we've found the target
            if (CurOrig == Orig) 
            {
              m_Memory.Write<PVOID>(pCurOrig, Hook);
              return;
            }
          }
        }
      }
    }

    // Perform IAT hook for all modules
    void IatHook::IatHookAll(std::wstring const& ModuleName, PVOID Orig, 
      PVOID Hook)
    {
      // Get all modules
      auto const ModuleList(GetModuleList(m_Memory));

      // Hook all modules (except excluded module)
      std::for_each(ModuleList.begin(), ModuleList.end(), 
        [&] (std::shared_ptr<Module> Current) 
      {
        //if (Current->GetBase() != m_Excluded)
        {
          IatHookSingle(Current->GetBase(), ModuleName, Orig, Hook);
        }
      });
    }
  }
}
