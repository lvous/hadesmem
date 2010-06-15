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
#include "GuiMgr.h"
#include "D3D9Mgr.h"
#include "Hades-Kernel/Kernel.h"
#include "Hades-Input/InputMgr.h"
#include "Hades-Common/Filesystem.h"
#include "Hades-Common/StringBuffer.h"

namespace Hades
{
  // Constructor
  GuiMgr::GuiMgr(Kernel* pKernel) 
    : m_pKernel(pKernel), 
    m_pDevice(nullptr), 
    m_CursorX(0), 
    m_CursorY(0)
  {
    // Register for D3D events
    D3D9Mgr::RegisterOnInitialize(std::bind(&GuiMgr::OnInitialize, this, 
      std::placeholders::_1, std::placeholders::_2));
    D3D9Mgr::RegisterOnFrame(std::bind(&GuiMgr::OnFrame, this, 
      std::placeholders::_1, std::placeholders::_2));
    D3D9Mgr::RegisterOnLostDevice(std::bind(&GuiMgr::OnLostDevice, this, 
      std::placeholders::_1, std::placeholders::_2));
    D3D9Mgr::RegisterOnResetDevice(std::bind(&GuiMgr::OnResetDevice, this, 
      std::placeholders::_1, std::placeholders::_2));
    D3D9Mgr::RegisterOnRelease(std::bind(&GuiMgr::OnRelease, this, 
      std::placeholders::_1, std::placeholders::_2));

    // Register for input events
    pKernel->GetInputMgr()->RegisterOnWindowMessage(std::bind(
      &GuiMgr::OnInputMsg, this, std::placeholders::_1, std::placeholders::_2, 
      std::placeholders::_3, std::placeholders::_4));
    pKernel->GetInputMgr()->RegisterOnSetCursor(std::bind(&GuiMgr::OnSetCursor, 
      this, std::placeholders::_1));
    pKernel->GetInputMgr()->RegisterOnGetCursorPos(std::bind(
      &GuiMgr::OnGetCursorPos, this, std::placeholders::_1));
    pKernel->GetInputMgr()->RegisterOnSetCursorPos(std::bind(
      &GuiMgr::OnSetCursorPos, this, std::placeholders::_1, 
      std::placeholders::_2));
  }

  // Initialize GUI from device
  void GuiMgr::OnInitialize(IDirect3DDevice9* pDevice, 
    D3D9HelperPtr /*pHelper*/)
  {
    // Delete GUI instance if it already exists
    if (gpGui)
    {
      delete gpGui;
      gpGui = nullptr;
    }

    // Set device pointer
    m_pDevice = pDevice;

    // Create new GUI instance
    gpGui = new CGUI(pDevice);

    // Get current working directory
    std::wstring CurDir;
    if (!GetCurrentDirectory(MAX_PATH, Util::MakeStringBuffer(CurDir, 
      MAX_PATH)))
    {
      DWORD LastError = GetLastError();
      BOOST_THROW_EXCEPTION(GuiMgrError() << 
        ErrorFunction("GuiMgr::OnInitialize") << 
        ErrorString("Could not get current directory.") << 
        ErrorCodeWin(LastError));
    }

    // Set new working directory to GUI library folder
    std::wstring GuiPath((Windows::GetSelfDirPath() / L"/Gui/").file_string());
    if (!SetCurrentDirectory(GuiPath.c_str()))
    {
      DWORD LastError = GetLastError();
      BOOST_THROW_EXCEPTION(GuiMgrError() << 
        ErrorFunction("GuiMgr::OnInitialize") << 
        ErrorString("Could not set current directory.") << 
        ErrorCodeWin(LastError));
    }

    // Load GUI
    gpGui->LoadInterfaceFromFile("ColorThemes.xml");
    gpGui->LoadInterfaceFromFile("Console.xml");

    // Set callbacks
    auto pConsole = gpGui->GetWindowByString("HADES_CONSOLE_WINDOW", 1);
    if (pConsole)
    {
      auto pInBox = pConsole->GetElementByString("HADES_CONSOLE_INPUT", 1);
      if (pInBox)
      {
        pInBox->SetCallback(&GuiMgr::OnConsoleInput);
      }
      else
      {
        std::wcout << "GuiMgr::OnInitialize: Error! Could not find console "
          "input box." << std::endl;
      }
    }
    else
    {
      std::wcout << "GuiMgr::OnInitialize: Error! Could not find console "
        "window." << std::endl;
    }

    // Show GUI
    gpGui->SetVisible(true);

    // Restore old working directory
    if (!SetCurrentDirectory(CurDir.c_str()))
    {
      DWORD LastError = GetLastError();
      BOOST_THROW_EXCEPTION(GuiMgrError() << 
        ErrorFunction("GuiMgr::OnInitialize") << 
        ErrorString("Could not restore current directory.") << 
        ErrorCodeWin(LastError));
    }
  }

  // D3D9Mgr OnFrame callback
  void GuiMgr::OnFrame(IDirect3DDevice9* pDevice, D3D9HelperPtr pHelper)
  {
    // Ensure GUI is valid
    if (!gpGui)
    {
      return;
    }

    // Draw GUI
    gpGui->Draw();

    // Get viewport
    D3DVIEWPORT9 Viewport;
    pDevice->GetViewport(&Viewport);

    // Draw box on viewport border
    Hades::Math::Vec2f const TopLeft(0,0);
    Hades::Math::Vec2f const BottomRight(static_cast<float>(Viewport.Width), 
      static_cast<float>(Viewport.Height));
    pHelper->DrawBox(TopLeft, BottomRight, 2, D3DCOLOR_ARGB(255, 0, 255, 0));

    // Draw test string
    CColor MyColor(255, 0, 0, 255);
    gpGui->GetFont()->DrawString(10, 10, 0, &MyColor, "Hades");
  }

  // D3D9Mgr OnLostDevice callback
  void GuiMgr::OnLostDevice(IDirect3DDevice9* /*pDevice*/, 
    D3D9HelperPtr /*pHelper*/)
  {
    if (!gpGui)
    {
      return;
    }

    gpGui->OnLostDevice();
  }

  // D3D9Mgr OnResetDevice callback
  void GuiMgr::OnResetDevice(IDirect3DDevice9* pDevice, 
    D3D9HelperPtr /*pHelper*/)
  {
    if (!gpGui)
    {
      return;
    }

    gpGui->OnResetDevice(pDevice);
  }

  // D3D9Mgr OnRelease callback
  void GuiMgr::OnRelease(IDirect3DDevice9* /*pDevice*/, 
    D3D9HelperPtr /*pHelper*/)
  {
  }

  // InputMgr OnInputMsg callback
  bool GuiMgr::OnInputMsg(HWND /*hwnd*/, UINT uMsg, WPARAM wParam, 
    LPARAM lParam)
  {
    // Nothing to do if there is no current GUI instance
    if (!gpGui)
    {
      return true;
    }

    // Toggle GUI on F12
    if (uMsg == WM_KEYDOWN && wParam == VK_F12)
    {
      ToggleVisible();
    }

    // Notify GUI of input events
    gpGui->GetMouse().HandleMessage(uMsg, wParam, lParam);
    gpGui->GetKeyboard()->HandleMessage(uMsg, wParam, lParam);

    // Block input when GUI is visible
    return !(gpGui->IsVisible() && 
      (uMsg == WM_CHAR || 
      uMsg == WM_KEYDOWN || 
      uMsg == WM_KEYUP || 
      uMsg == WM_MOUSEMOVE || 
      uMsg == WM_LBUTTONDOWN || 
      uMsg == WM_RBUTTONDOWN || 
      uMsg == WM_MBUTTONDOWN || 
      uMsg == WM_LBUTTONUP || 
      uMsg == WM_RBUTTONUP || 
      uMsg == WM_MBUTTONUP || 
      uMsg == WM_RBUTTONDBLCLK || 
      uMsg == WM_LBUTTONDBLCLK || 
      uMsg == WM_MBUTTONDBLCLK || 
      uMsg == WM_MOUSEWHEEL));
  }

  // InputMgr OnSetCursor callback
  bool GuiMgr::OnSetCursor(HCURSOR hCursor)
  {
    // Only allow cursor to be modified when it's not being removed entirely 
    // and the GUI is not visible
    return !(hCursor == NULL && gpGui && gpGui->IsVisible());
  }

  // Toggle GUI's visibility
  void GuiMgr::ToggleVisible()
  {
    // Previous cursor state
    static HCURSOR PrevCursor = NULL;

    // If GUI is visible
    if (gpGui->IsVisible()) 
    {
      // Hide GUI 
      gpGui->SetVisible(false);

      // Restore previous cursor
      PrevCursor = SetCursor(PrevCursor);
    }
    else
    {
      // Hide GUI 
      gpGui->SetVisible(true);

      // Load default cursor
      HCURSOR DefArrow = LoadCursor(NULL, IDC_ARROW);
      // Set cursor
      PrevCursor = SetCursor(DefArrow);
    }
  }

  // InputMgr OnGetCursorPos callback
  bool GuiMgr::OnGetCursorPos(LPPOINT lpPoint)
  {
    // Use cached values and block call if GUI is currently visible
    if (gpGui && gpGui->IsVisible() && (m_CursorX != 0 || m_CursorY != 0))
    {
      lpPoint->x = m_CursorX;
      lpPoint->y = m_CursorY;

      return false;
    }

    return true;
  }

  // InputMgr OnSetCursorPos callback
  bool GuiMgr::OnSetCursorPos(int X, int Y)
  {
    // Backup current values and block call if GUI is currently visible
    if (gpGui && gpGui->IsVisible())
    {
      m_CursorX = X;
      m_CursorY = Y;

      return false;
    }

    return true;
  }

  // GUI library callback for console input
  std::string __cdecl GuiMgr::OnConsoleInput(char const* pszArgs, 
    CElement* pElement)
  {
    // Print input to console
    Print(pszArgs);

    // Clear input box and refocus
    auto pEditBox = dynamic_cast<CEditBox*>(pElement);
    pEditBox->SetString("");
    pEditBox->SetStart(0);
    pEditBox->SetIndex(0);
    pEditBox->GetParent()->SetFocussedElement(pElement);

    // Forced return value
    return std::string();
  }

  // Print output to console
  void GuiMgr::Print( std::string const& Output )
  {
    // Get console window
    auto pConsole = gpGui->GetWindowByString("HADES_CONSOLE_WINDOW", 1);
    if (!pConsole)
    {
      std::wcout << "GuiMgr::Print: Warning! Could not find console window." << 
        std::endl;
      return;
    }

    // Get input box
    auto pOutBox = pConsole->GetElementByString("HADES_CONSOLE_OUTPUT", 1);
    if (!pOutBox)
    {
      std::wcout << "GuiMgr::Print: Warning! Could not find console output "
        "box." << std::endl;
      return;
    }

    // Add output
    auto pOutBoxReal = dynamic_cast<CTextBox*>(pOutBox);
    pOutBoxReal->AddString(Output);
  }
}
