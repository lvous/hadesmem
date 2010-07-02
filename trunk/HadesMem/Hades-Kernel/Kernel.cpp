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
#include <boost/bind.hpp>
#include <boost/format.hpp>

// Windows API
#include <Windows.h>

// Hades
#include "Kernel.h"
#include "Loader.h"
#include "DotNet.h"
#include "Wrappers.h"
#include "Scripting.h"
#include "Hades-D3D9/GuiMgr.h"
#include "Hades-Common/I18n.h"
#include "Hades-D3D9/D3D9Mgr.h"
#include "Hades-Input/InputMgr.h"
#include "Hades-Common/Filesystem.h"

namespace Hades
{
  namespace Kernel
  {
    // Constructor
    Kernel::Kernel() 
      : m_Memory(new Hades::Memory::MemoryMgr(GetCurrentProcessId())), 
      m_PathToSelfDir(Hades::Windows::GetSelfDirPath().file_string()), 
      m_pInputMgr(nullptr), 
      m_pD3D9Mgr(nullptr), 
      m_pGuiMgr(nullptr), 
      m_LuaMgr(), 
      m_pDotNetMgr(nullptr), 
      m_SessionId(0)
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
      std::wcout << boost::wformat(L"Kernel::Initialize: Path to current "
        L"binary = \"%ls\".") %BinPath << std::endl;

      // Path to self
      auto const PathToSelf(Hades::Windows::GetSelfPath().file_string());

      // Debug output
      std::wcout << boost::wformat(L"Kernel::Initialize: Path to Self "
        L"(Full): = \"%ls\", Path To Self (Dir): = \"%ls\".") %PathToSelf 
        %m_PathToSelfDir << std::endl;

      // Initialize Loader
      Loader::Initialize(this);
      Loader::LoadConfig(m_PathToSelfDir + L"/Config/Loader.xml");

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
      LoadModule(m_PathToSelfDir + L"\\" + InputModName);
      LoadModule(m_PathToSelfDir + L"\\" + D3D9ModName);

      // Initialize .NET
      m_pDotNetMgr.reset(new DotNetMgr(this, m_PathToSelfDir + 
        L"/Config/DotNet.xml"));

      // Expose Hades API
      luabind::module(m_LuaMgr.GetState(), "Hades")
      [
        luabind::def("WriteLn", luabind::tag_function<void (
          std::string const&)>(Wrappers::WriteLn(this)))
        ,luabind::def("LoadExt", luabind::tag_function<void (
          std::string const&)>(Wrappers::LoadExt(this)))
        ,luabind::def("DotNet", luabind::tag_function<void (std::string const&, 
          std::string const&, std::string const&)>(Wrappers::DotNet(
          &*m_pDotNetMgr)))
        ,luabind::def("Exit", luabind::tag_function<void ()>(Wrappers::Exit()))
        ,luabind::def("GetSessionId", luabind::tag_function<unsigned int ()>(
          Wrappers::SessionId(this)))
        ,luabind::def("SetSessionId", luabind::tag_function<void 
          (unsigned int)>(Wrappers::SessionId(this)))
      ];

      // Debug output
      std::wcout << "Kernel::Initialize: Hades-Kernel initialized." 
        << std::endl;
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
      typedef void (__stdcall* tInitialize)(Kernel* pKernel);
      auto const pInitialize = reinterpret_cast<tInitialize>(GetProcAddress(
        MyModule, "_Initialize@4"));
      if (!pInitialize)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(KernelError() << 
          ErrorFunction("Kernel::LoadModule") << 
          ErrorString("Could not find '_Initialize@4' in module \"" + 
          boost::lexical_cast<std::string>(Module) + "\".") << 
          ErrorCodeWin(LastError));
      }

      // Call initialization routine
      pInitialize(this);
    }

    // Load and initialize a Hades extension
    void Kernel::LoadExtension(std::wstring const& Module)
    {
      // Load module from extension directory
      LoadModule(m_PathToSelfDir + L"/Extensions/" + Module);
    }

    // Get D3D9 manager wrapper
    D3D9::D3D9MgrWrapper* Kernel::GetD3D9Mgr()
    {
      return m_pD3D9Mgr;
    }

    // Set D3D9 manager wrapper
    void Kernel::SetD3D9Mgr(D3D9::D3D9MgrWrapper* pD3D9Mgr)
    {
      // Sanity check
      if (m_pD3D9Mgr)
      {
        std::wcout << "Kernel::SetD3D9Mgr: Warning! Attempt to overwrite "
          "existing D3D9Mgr instance." << std::endl;
        return;
      }

      // Set D3D9 manager
      m_pD3D9Mgr = pD3D9Mgr;
    }

    // Get input manager wrapper
    Input::InputMgrWrapper* Kernel::GetInputMgr()
    {
      return m_pInputMgr;
    }

    // Set input manager wrapper
    void Kernel::SetInputMgr(Input::InputMgrWrapper* pInputMgr)
    {
      // Sanity check
      if (m_pInputMgr)
      {
        std::wcout << "Kernel::SetInputMgr: Warning! Attempt to overwrite "
          "existing InputMgr instance." << std::endl;
        return;
      }

      // Set input manager
      m_pInputMgr = pInputMgr;
    }

    // Set GUI manager
    void Kernel::SetGuiMgr(D3D9::GuiMgr* pGuiMgr)
    {
      // Sanity check
      if (m_pGuiMgr)
      {
        std::wcout << "Kernel::SetGuiMgr: Warning! Attempt to overwrite "
          "existing GuiMgr instance." << std::endl;
        return;
      }

      // Set GUI manager
      m_pGuiMgr = pGuiMgr;
      m_pGuiMgr->RegisterOnConsoleInput(std::bind(&Kernel::OnConsoleInput, 
        this, std::placeholders::_1));
    }

    // Get GUI manager
    D3D9::GuiMgr* Kernel::GetGuiMgr()
    {
      return m_pGuiMgr;
    }

    // GUI manager OnConsoleInput callback
    void Kernel::OnConsoleInput(std::string const& Input)
    {
      // Debug output
      std::cout << "Kernel::OnConsoleInput: \"" << Input << "\"." << std::endl;

      try
      {
        // Run lua
        m_LuaMgr.RunString(Input);
      }
      catch (boost::exception const& e)
      {
        // Print error information
        m_pGuiMgr->Print(boost::diagnostic_information(e));
      }
      catch (std::exception const& e)
      {
        // Print error information
        m_pGuiMgr->Print(e.what());
      }
    }

    // Get session ID
    unsigned int Kernel::GetSessionId()
    {
      return m_SessionId;
    }

    // Set session ID
    void Kernel::SetSessionId(unsigned int SessionId)
    {
      // Sanity check
      if (m_SessionId)
      {
        std::wcout << "Kernel::SetSessionId: Warning! Attempt to overwrite "
          "an existing session ID." << std::endl;
        return;
      }

      // Debug output
      std::wcout << "Kernel::SetSessionId: Assigning session ID '" << 
        SessionId << "'." << std::endl;
      
      // Set session ID
      m_SessionId = SessionId;

      // Notify .NET layer of session ID change
      m_pDotNetMgr->SetSessionId(SessionId);
    }

    // Run script
    void Kernel::RunScript(std::string const& Script)
    {
      // Debug output
      std::cout << "Kernel::RunScript: \"" << Script << "\"." << std::endl;

      try
      {
        // Run lua
        m_LuaMgr.RunString(Script);
      }
      catch (boost::exception const& e)
      {
        // Print error information
        if (m_pGuiMgr)
        {
          m_pGuiMgr->Print(boost::diagnostic_information(e));
        }
        else
        {
          std::cout << "Kernel::RunScript: Error! " << 
            boost::diagnostic_information(e) << std::endl;
        }
      }
      catch (std::exception const& e)
      {
        // Print error information
        if (m_pGuiMgr)
        {
          m_pGuiMgr->Print(e.what());
        }
        else
        {
          std::cout << "Kernel::RunScript: Error! " << e.what() << std::endl;
        }
      }
    }

    // Run script file
    void Kernel::RunScriptFile(std::string const& Script)
    {
      // Debug output
      std::cout << "Kernel::RunScriptFile: \"" << Script << "\"." << std::endl;

      try
      {
        // Run lua
        m_LuaMgr.RunFile(Script);
      }
      catch (boost::exception const& e)
      {
        // Print error information
        if (m_pGuiMgr)
        {
          m_pGuiMgr->Print(boost::diagnostic_information(e));
        }
        else
        {
          std::cout << "Kernel::RunScriptFile: Error! " << 
            boost::diagnostic_information(e) << std::endl;
        }
      }
      catch (std::exception const& e)
      {
        // Print error information
        if (m_pGuiMgr)
        {
          m_pGuiMgr->Print(e.what());
        }
        else
        {
          std::cout << "Kernel::RunScriptFile: Error! " << e.what() << 
            std::endl;
        }
      }
    }
  }
}
