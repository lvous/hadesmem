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
#include <iostream>

// Hades
#include "InputMgr.h"
#include "Hades-Kernel/Kernel.h"

namespace Hades
{
  // Hades manager
  Kernel* InputMgr::m_pKernel = nullptr;

  // Target window
  HWND InputMgr::m_TargetWindow = nullptr;
  
  // Previous window procedure
  WNDPROC InputMgr::m_OrigProc = nullptr;

  // Callback managers
  InputMgr::OnWindowMessageCallbacks InputMgr::m_CallsOnWndMsg;

  // Initialize input subsystem
  void InputMgr::Startup(Kernel* pKernel)
  {
    // Set HadesMgr pointer
    m_pKernel = pKernel;
  }

  // Window hook procedure
  LRESULT CALLBACK InputMgr::MyWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, 
    LPARAM lParam)
  {
    // Call registered callbacks and block input if requested
    return *m_CallsOnWndMsg(hwnd, uMsg, wParam, lParam) ? CallWindowProc(
      m_OrigProc, hwnd, uMsg, wParam, lParam) : 0;
  }

  // Hook target window
  void InputMgr::HookWindow(HWND Window)
  {
    try
    {
      m_TargetWindow = Window;
      m_OrigProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(Window, 
        GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&MyWindowProc)));
      if (!m_OrigProc)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(InputMgrError() << 
          ErrorFunction("InputMgr::HookWindow") << 
          ErrorString("Could not set new window procedure.") << 
          ErrorCodeWin(LastError));
      }
    }
    catch (boost::exception const& e)
    {
      // Debug output
      std::cout << boost::format("InputMgr::HookWindow: Error! %s.") 
        %boost::diagnostic_information(e) << std::endl;
    }
    catch (std::exception const& e)
    {
      // Debug output
      std::cout << boost::format("InputMgr::HookWindow: Error! %s.") 
        %e.what() << std::endl;
    }
  }

  boost::signals2::connection InputMgr::RegisterOnWindowMessage(
    OnWindowMessageCallbacks::slot_type const& Subscriber)
  {
    // Register callback and return connection
    return m_CallsOnWndMsg.connect(Subscriber);
  }

  HWND InputMgr::GetTargetWindow()
  {
    return m_TargetWindow;
  }
}
