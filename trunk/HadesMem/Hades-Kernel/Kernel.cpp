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
along with HadesMem.  If not, see <http://www.gnu.org/licenses/>.
*/

// C++ Standard Library
#include <array>
#include <iostream>

// Boost
#include <boost/format.hpp>

// Windows API
#include <Windows.h>

// Hades
#include "Kernel.h"
#include "Loader.h"
#include "Hades-Common/I18n.h"
#include "Hades-D3D9/DllMain.h"
#include "Hades-Input/DllMain.h"
#include "Hades-Common/Filesystem.h"

namespace Hades
{
  // Constructor
  Kernel::Kernel() 
    : m_Memory(new Hades::Memory::MemoryMgr(GetCurrentProcessId()))
  { }

  // Initialize kernel
  void Kernel::Initialize()
  {
    // Get string to binary we're injected into
    DWORD const BinPathSize = MAX_PATH;
    std::wstring BinPath;
    if (!GetModuleFileName(nullptr, Util::MakeStringBuffer(BinPath, 
      BinPathSize), BinPathSize))
    {
      DWORD LastError = GetLastError();
      BOOST_THROW_EXCEPTION(KernelError() << 
        ErrorFunction("Kernel::Initialize") << 
        ErrorString("Could not get path to current binary.") << 
        ErrorCodeWin(LastError));
    }

    // Debug output
    std::wcout << boost::wformat(L"Kernel::Initialize: Path to current binary "
      L"= \"%ls\".") %BinPath << std::endl;

    // Path to self
    auto const PathToSelf(Hades::Windows::GetSelfPath().file_string());
    auto const PathToSelfDir(Hades::Windows::GetSelfDirPath().file_string());

    // Debug output
    std::wcout << boost::wformat(L"Kernel::Initialize: Path to Self (Full): = "
      L"\"%ls\", Path To Self (Dir): = \"%ls\".") %PathToSelf %PathToSelfDir 
      << std::endl;

    // Initialize Loader
    // Todo: Move this data to external file (XML?)
    Loader::Initialize(this);
    Loader::AddExe(L"WoW.exe", L"Hades-Kernel_IA32.dll");
    Loader::AddExe(L"Launcher.exe", L"Hades-Kernel_IA32.dll");
    Loader::AddExe(L"UT3.exe", L"Hades-Kernel_IA32.dll");
    Loader::AddExe(L"iw4mp.exe", L"Hades-Kernel_IA32.dll");
    Loader::AddExe(L"aion.bin", L"Hades-Kernel_IA32.dll");
    Loader::AddExe(L"hl2.exe", L"Hades-Kernel_IA32.dll");
    Loader::AddExe(L"BFBC2Game.exe", L"Hades-Kernel_IA32.dll");
    Loader::AddExe(L"NCLauncher.exe", L"Hades-Kernel_IA32.dll");
    Loader::AddExe(L"NCWLauncherSetup.exe", L"Hades-Kernel_IA32.dll");
    Loader::AddExe(L"AvP.exe", L"Hades-Kernel_IA32.dll");
    Loader::AddExe(L"AvP_DX11.exe", L"Hades-Kernel_IA32.dll");
    Loader::AddExe(L"SR2_pc.exe", L"Hades-Kernel_IA32.dll");
    Loader::AddExe(L"FEAR2.exe", L"Hades-Kernel_IA32.dll");
    Loader::AddExe(L"Left4Dead.exe", L"Hades-Kernel_IA32.dll");

    // Start aux modules
#if defined(_M_X64)
    std::wstring const D3D9ModName(L"Hades-D3D9_AMD64.dll");
    std::wstring const InputModName(L"Hades-Input_AMD64.dll");
#elif defined(_M_IX86)
    std::wstring const D3D9ModName(L"Hades-D3D9_IA32.dll");
    std::wstring const InputModName(L"Hades-Input_IA32.dll");
#else
#error Unsupported platform!
#endif
    Hades::Modules::Input::Initialize(this);
    Hades::Modules::D3D9::Initialize(this);

    // Debug output
    std::wcout << "Kernel::Initialize: Hades-Kernel initialized." << std::endl;
  }

  // Get memory manager
  std::shared_ptr<Hades::Memory::MemoryMgr> Kernel::GetMemoryMgr() 
  {
    return m_Memory;
  }

  // Load and initialize a Hades helper module
  void Kernel::LoadModule(std::wstring const& Module) 
  {
    // Load module
    HMODULE const MyModule = LoadLibrary(Module.c_str());
    if (!MyModule)
    {
      DWORD LastError = GetLastError();
      BOOST_THROW_EXCEPTION(KernelError() << 
        ErrorFunction("Kernel::LoadModule") << 
        ErrorString("Could not find module \"" + 
        boost::lexical_cast<std::string>(Module) + "\".") << 
        ErrorCodeWin(LastError));
    }

    // Get address of Initialize export. Should be exported by all Hades 
    // extensions and modules.
    typedef void (__stdcall* tInitialize)(HMODULE Module, 
      Kernel* pKernel);
    auto const pInitialize = reinterpret_cast<tInitialize>(GetProcAddress(
      MyModule, "_Initialize@8"));
    if (!pInitialize)
    {
      DWORD LastError = GetLastError();
      BOOST_THROW_EXCEPTION(KernelError() << 
        ErrorFunction("Kernel::LoadModule") << 
        ErrorString("Could not find '_Initialize@8' in module \"" + 
        boost::lexical_cast<std::string>(Module) + "\".") << 
        ErrorCodeWin(LastError));
    }

    // Call initialization routine
    pInitialize(MyModule, this);
  }
}
