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

#pragma once

// C++ Standard Library
#include <memory>

// Windows API
#include <Windows.h>

// Hades
#include "Kernel.hpp"
#include "HadesMemory/Memory.hpp"

namespace Hades
{
  namespace Kernel
  {
    // Hades child process 'loader'
    class Loader
    {
    public:
      // Error type
      class Error : public virtual HadesError 
      { };
      
      // Initialize loader
      static void Initialize(Kernel& MyKernel);
      
      // Hook process creation
      static void Hook();
      
      // Unhook process creation
      static void Unhook();
    
    private:
      // kernel32.dll!CreateProcessInternalW hook implementation
      static BOOL WINAPI CreateProcessInternalW_Hook(
        HANDLE hToken,
        LPCWSTR lpApplicationName,
        LPWSTR lpCommandLine,
        LPSECURITY_ATTRIBUTES lpProcessAttributes,
        LPSECURITY_ATTRIBUTES lpThreadAttributes,
        BOOL bInheritHandles,
        DWORD dwCreationFlags,
        LPVOID lpEnvironment,
        LPCWSTR lpCurrentDirectory,
        LPSTARTUPINFOW lpStartupInfo,
        LPPROCESS_INFORMATION lpProcessInformation,
        PHANDLE hNewToken);

      // Kernel instance
      static Kernel* m_pKernel;
      
      // kernel32.dll!CreateProcessInternalW hook
      static std::shared_ptr<Hades::Memory::PatchDetour> m_pCreateProcessInternalWHk;
    };
  }
}
