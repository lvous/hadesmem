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
    pKernel->GetInputMgr()->RegisterOnMessage(std::bind(&GuiMgr::OnInputMsg, 
      this, std::placeholders::_1, std::placeholders::_2, 
      std::placeholders::_3, std::placeholders::_4));
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
    gpGui->LoadInterfaceFromFile("GuiTest_UI.xml");

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

  void GuiMgr::OnFrame(IDirect3DDevice9* pDevice, D3D9HelperPtr pHelper)
  {
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

  void GuiMgr::OnLostDevice(IDirect3DDevice9* /*pDevice*/, 
    D3D9HelperPtr /*pHelper*/)
  {
    gpGui->OnLostDevice();
  }

  void GuiMgr::OnResetDevice(IDirect3DDevice9* pDevice, 
    D3D9HelperPtr /*pHelper*/)
  {
    gpGui->OnResetDevice(pDevice);
  }

  void GuiMgr::OnRelease(IDirect3DDevice9* /*pDevice*/, 
    D3D9HelperPtr /*pHelper*/)
  {
  }

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
      gpGui->SetVisible(!gpGui->IsVisible());
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
}
