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
  GuiMgr::GuiMgr(Kernel* pKernel) 
  {
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

    pKernel->GetInputMgr()->RegisterOnMessage(std::bind(&GuiMgr::OnInputMsg, 
      this, std::placeholders::_1, std::placeholders::_2, 
      std::placeholders::_3, std::placeholders::_4));
  }

  void GuiMgr::OnInitialize(IDirect3DDevice9* pDevice, 
    D3D9HelperPtr /*pHelper*/)
  {
    if (gpGui)
    {
      delete gpGui;
      gpGui = nullptr;
    }

    gpGui = new CGUI(pDevice);

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

    std::wstring GuiPath((Windows::GetSelfDirPath() / L"/Gui/").file_string());
    if (!SetCurrentDirectory(GuiPath.c_str()))
    {
      DWORD LastError = GetLastError();
      BOOST_THROW_EXCEPTION(GuiMgrError() << 
        ErrorFunction("GuiMgr::OnInitialize") << 
        ErrorString("Could not set current directory.") << 
        ErrorCodeWin(LastError));
    }

    gpGui->LoadInterfaceFromFile("ColorThemes.xml");
    gpGui->LoadInterfaceFromFile("GuiTest_UI.xml");

    gpGui->SetVisible(true);

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
    if (uMsg == WM_KEYDOWN && wParam == VK_F12)
    {
      gpGui->SetVisible(!gpGui->IsVisible());
    }

    gpGui->GetMouse().HandleMessage(uMsg, wParam, lParam);
    gpGui->GetKeyboard()->HandleMessage(uMsg, wParam, lParam);

    return false;
//     return !gpGui->IsVisible();
  }
}
