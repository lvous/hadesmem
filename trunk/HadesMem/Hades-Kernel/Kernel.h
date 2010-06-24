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

#pragma once

// C++ Standard Library
#include <string>
#include <memory>

// Boost
#include <boost/noncopyable.hpp>

// Hades
#include "Scripting.h"
#include "Hades-Memory/Memory.h"

namespace Hades
{
  // Kernel exception type
  class KernelError : public virtual HadesError 
  { };

  // Hades kernel
  class Kernel : private boost::noncopyable
  {
  public:
    // Constructor
    Kernel();

    // Initialize kernel
    virtual void Initialize();

    // Get memory manager
    virtual std::shared_ptr<Memory::MemoryMgr> GetMemoryMgr();

    // Load and initialize a Hades helper module
    virtual void LoadModule(std::wstring const& Module);

    // Load and initialize a Hades extension
    virtual void LoadExtension(std::wstring const& Module);

    // Get D3D9 manager wrapper
    virtual class D3D9MgrWrapper* GetD3D9Mgr();

    // Set D3D9 manager wrapper
    virtual void SetD3D9Mgr(class D3D9MgrWrapper* pD3D9Mgr);

    // Get input manager wrapper
    virtual class InputMgrWrapper* GetInputMgr();

    // Set input manager wrapper
    virtual void SetInputMgr(class InputMgrWrapper* pD3D9Mgr);

    // Set GUI manager
    virtual void SetGuiMgr(class GuiMgr* pGuiMgr);

    // Get GUI manager
    virtual class GuiMgr* GetGuiMgr();

    // GUI manager OnConsoleInput callback
    virtual void OnConsoleInput(std::string const& Input);

  private:
    // Memory manager
    std::shared_ptr<Memory::MemoryMgr> m_Memory;

    // Path to self dir
    std::wstring const m_PathToSelfDir;

    // D3D9 manager wrapper
    class D3D9MgrWrapper* m_pD3D9Mgr;

    // Input manager wrapper
    class InputMgrWrapper* m_pInputMgr;

    // GUI manager
    class GuiMgr* m_pGuiMgr;

    // Lua manager
    LuaMgr m_LuaMgr;

    // DotNet manager
    std::shared_ptr<DotNetMgr> m_pDotNetMgr;
  };
}
