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

// Windows API
#include <Windows.h>
#include <atlbase.h>
#include <atlwin.h>

// Boost
#pragma warning(push, 1)
#pragma warning(disable: 4267)
#include <boost/signals2.hpp>
#pragma warning(pop)

// Hades
#include "Hades-Common/Error.h"
#include "Hades-Memory/Patcher.h"

namespace Hades
{
  // InputMgr exception type
  class InputMgrError : public virtual HadesError 
  { };

  // Input managing class
  class InputMgr
  {
  public:
    // Initialize input subsystem
    static void Startup(class Kernel* pKernel);

    // Hook window for input
    static void HookWindow(HWND Window);

    // Callback types
    typedef boost::signals2::signal<bool (HWND hwnd, UINT uMsg, WPARAM 
      wParam, LPARAM lParam)> OnWindowMessageCallbacks;

    // Register callback for OnWindowMsg event
    static boost::signals2::connection RegisterOnWindowMessage(
      OnWindowMessageCallbacks::slot_type const& Subscriber);

    // Get target window
    static HWND GetTargetWindow();

  private:
    // Window hook procedure
    static LRESULT CALLBACK MyWindowProc(
      HWND hwnd,
      UINT uMsg,
      WPARAM wParam,
      LPARAM lParam);

    // Hades manager
    static class Kernel* m_pKernel;

    // Target window
    static HWND m_TargetWindow;
    
    // Previous window procedure
    static WNDPROC m_OrigProc;

    // Callback managers
    static OnWindowMessageCallbacks m_CallsOnWndMsg;
  };

  // Input managing class wrapper
  class InputMgrWrapper
  {
  public:
    virtual void Startup(class Kernel* pHades)
    {
      return InputMgr::Startup(pHades);
    }

    virtual void HookWindow(HWND Window)
    {
      return InputMgr::HookWindow(Window);
    }

    virtual boost::signals2::connection RegisterOnWindowMessage(
      const InputMgr::OnWindowMessageCallbacks::slot_type& Subscriber)
    {
      return InputMgr::RegisterOnWindowMessage(Subscriber);
    }

    virtual HWND GetTargetWindow()
    {
      return InputMgr::GetTargetWindow();
    }
  };
}
