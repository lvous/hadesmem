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
  InputMgr::OnSetCursorCallbacks InputMgr::m_CallsOnSetCursor;

  // SetCursor hook
  std::shared_ptr<Memory::PatchDetour> InputMgr::m_pSetCursorHk;

  // Initialize input subsystem
  void InputMgr::Startup(Kernel* pKernel)
  {
    // Set HadesMgr pointer
    m_pKernel = pKernel;

    // Load User32
    // Todo: Defer hooking until game loads User32 (e.g. via LoadLibrary 
    // hook).
    HMODULE User32Mod = LoadLibrary(L"User32.dll");
    if (!User32Mod)
    {
      DWORD LastError = GetLastError();
      BOOST_THROW_EXCEPTION(InputMgrError() << 
        ErrorFunction("InputMgr::Startup") << 
        ErrorString("Could not load User32.") << 
        ErrorCodeWin(LastError));
    }

    // Get address of SetCursor
    FARPROC pSetCursor = GetProcAddress(User32Mod, "SetCursor");
    if (!pSetCursor)
    {
      DWORD LastError = GetLastError();
      BOOST_THROW_EXCEPTION(InputMgrError() << 
        ErrorFunction("InputMgr::Startup") << 
        ErrorString("Could not get address of SetCursor.") << 
        ErrorCodeWin(LastError));
    }

    // Target and detour pointer
    PBYTE Target = reinterpret_cast<PBYTE>(pSetCursor);
    PBYTE Detour = reinterpret_cast<PBYTE>(&SetCursor_Hook);

    // Debug output
    std::wcout << "InputMgr::Startup: Hooking user32.dll!SetCursor." << 
      std::endl;
    std::wcout << boost::wformat(L"InputMgr::Startup: Target = %p, "
      L"Detour = %p.") %Target %Detour << std::endl;

    // Hook user32.dll!SetCursor
    m_pSetCursorHk.reset(new Hades::Memory::PatchDetour(*pKernel->
      GetMemoryMgr(), Target, Detour));
    m_pSetCursorHk->Apply();
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

  boost::signals2::connection InputMgr::RegisterOnSetCursor(
    OnSetCursorCallbacks::slot_type const& Subscriber)
  {
    // Register callback and return connection
    return m_CallsOnSetCursor.connect(Subscriber);
  }

  HWND InputMgr::GetTargetWindow()
  {
    return m_TargetWindow;
  }

  HCURSOR WINAPI InputMgr::SetCursor_Hook(HCURSOR Cursor)
  {
    // Get trampoline6
    typedef HCURSOR (WINAPI* tSetCursor)(HCURSOR Cursor);
    auto pSetCursor = reinterpret_cast<tSetCursor>(m_pSetCursorHk->
      GetTrampoline());

    return *m_CallsOnSetCursor(Cursor) ? pSetCursor(Cursor) : nullptr;
  }
}
