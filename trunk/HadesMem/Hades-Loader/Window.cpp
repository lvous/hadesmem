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
#include "Window.h"
#include "Resource.h"
#include "Hades-Common/Error.h"
#include "Hades-Common/Filesystem.h"
#include "Hades-Common/StringBuffer.h"

namespace Hades
{
  namespace Loader
  {
    // D3D instance
    CComPtr<IDirect3D9Ex> LoaderWindow::m_pD3D;
    // D3D device
    CComPtr<IDirect3DDevice9Ex> LoaderWindow::m_pD3DDevice;
    // Hades GUI
    std::shared_ptr<GUI::GUI> LoaderWindow::m_pGui;
    // Hades icon
    Windows::EnsureDestroyIcon LoaderWindow::m_HadesIcon;

    // WM_DESTROY message callback
    LRESULT LoaderWindow::OnDestroy( UINT /*nMsg*/, WPARAM /*wParam*/, 
      LPARAM /*lParam*/, BOOL& /*bHandled*/ )
    {
      PostQuitMessage(0);
      return 0;
    }

    // WM_CREATE message callback
    LRESULT LoaderWindow::OnCreate( UINT /*nMsg*/, WPARAM /*wParam*/, 
      LPARAM /*lParam*/, BOOL& /*bHandled*/ )
    {
      // Load Hades icon from resources
      m_HadesIcon = static_cast<HICON>(LoadImage(GetModuleHandle(NULL), 
        MAKEINTRESOURCE(IDI_ICON_LOGO), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));
      if (!m_HadesIcon)
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(HadesError() << 
          ErrorFunction("wWinMain") << 
          ErrorString("Could not load Hades icon.") << 
          ErrorCodeWin(LastError));
      }

      // Set window icon as Hades icon
      SetIcon(m_HadesIcon, TRUE);
      SetIcon(m_HadesIcon, FALSE);

      return 0;
    }

    // Custom window procedure
    LRESULT CALLBACK LoaderWindow::WindowProc(HWND hWnd, UINT uMsg, 
      WPARAM wParam, LPARAM lParam)
    {
      // Notify GUI library of message
      if(m_pGui && (m_pGui->GetMouse().HandleMessage(uMsg, wParam, lParam) || 
        m_pGui->GetKeyboard().HandleMessage(uMsg, wParam, lParam)))
      {
        return 0;
      }

      // Call 'default' window procedure
      return CWindowImpl<LoaderWindow>::WindowProc(hWnd, uMsg, wParam, lParam);
    }

    // GetWindowProc override to allow use of custom window procedure
    WNDPROC LoaderWindow::GetWindowProc()
    {
      return &LoaderWindow::WindowProc;
    }

    // Initialize D3D
    void LoaderWindow::InitD3D()
    {
      // Create D3D9
      HRESULT Result = Direct3DCreate9Ex(D3D_SDK_VERSION, &m_pD3D.p);
      if (FAILED(Result))
      {
        BOOST_THROW_EXCEPTION(HadesError() << 
          ErrorFunction("LoaderWindow::InitD3D") << 
          ErrorString("Could not create D3D9.") << 
          ErrorCodeWin(Result));
      }

      // Set up device presentation parameters
      D3DPRESENT_PARAMETERS PresentParams = { 0 };
      PresentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
      PresentParams.hDeviceWindow = *this;
      PresentParams.Windowed = TRUE;

      // Create D3D9 device
      Result = m_pD3D->CreateDeviceEx(D3DADAPTER_DEFAULT, 
        D3DDEVTYPE_HAL, 
        *this, 
        D3DCREATE_SOFTWARE_VERTEXPROCESSING, 
        &PresentParams, 
        NULL, 
        &m_pD3DDevice.p);
      if (FAILED(Result))
      {
        BOOST_THROW_EXCEPTION(HadesError() << 
          ErrorFunction("LoaderWindow::InitD3D") << 
          ErrorString("Could not create D3D9 device.") << 
          ErrorCodeWin(Result));
      }
    }

    // Load GUI
    void LoaderWindow::LoadGUI()
    {
      // Create GUI
      m_pGui.reset(new GUI::GUI(m_pD3DDevice));

      // Get current working directory
      std::wstring CurDir;
      if (!GetCurrentDirectory(MAX_PATH, Util::MakeStringBuffer(CurDir, 
        MAX_PATH)))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(HadesError() << 
          ErrorFunction("LoaderWindow::LoadGUI") << 
          ErrorString("Could not get current directory.") << 
          ErrorCodeWin(LastError));
      }

      // Set new working directory to GUI library folder
      std::wstring GuiPath((Windows::GetSelfDirPath() / L"/Gui/").
        file_string());
      if (!SetCurrentDirectory(GuiPath.c_str()))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(HadesError() << 
          ErrorFunction("LoaderWindow::LoadGUI") << 
          ErrorString("Could not set current directory.") << 
          ErrorCodeWin(LastError));
      }

      // Load interface data
      m_pGui->LoadInterfaceFromFile("ColorThemes.xml");
      m_pGui->LoadInterfaceFromFile("GuiTest_UI.xml");

      // Restore old working directory
      if (!SetCurrentDirectory(CurDir.c_str()))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(HadesError() << 
          ErrorFunction("LoaderWindow::LoadGUI") << 
          ErrorString("Could not restore current directory.") << 
          ErrorCodeWin(LastError));
      }

      // Set GUI as visible
      m_pGui->SetVisible(true);
    }

    // Render frame
    void LoaderWindow::RenderFrame()
    {
      // Clear back buffer
      HRESULT Result = m_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, 
        D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);
      if (FAILED(Result))
      {
        BOOST_THROW_EXCEPTION(HadesError() << 
          ErrorFunction("LoaderWindow::RenderFrame") << 
          ErrorString("Could not clear back buffer.") << 
          ErrorCodeWin(Result));
      }

      // Begin scene
      Result = m_pD3DDevice->BeginScene();
      if (FAILED(Result))
      {
        BOOST_THROW_EXCEPTION(HadesError() << 
          ErrorFunction("LoaderWindow::RenderFrame") << 
          ErrorString("Could not begin scene.") << 
          ErrorCodeWin(Result));
      }

      // Render GUI if available
      if (m_pGui)
      {
        m_pGui->Draw();
      }

      // End scene
      Result = m_pD3DDevice->EndScene();
      if (FAILED(Result))
      {
        BOOST_THROW_EXCEPTION(HadesError() << 
          ErrorFunction("LoaderWindow::RenderFrame") << 
          ErrorString("Could not end scene.") << 
          ErrorCodeWin(Result));
      }

      // Present back buffer
      Result = m_pD3DDevice->Present(NULL, NULL, NULL, NULL);
      if (FAILED(Result))
      {
        BOOST_THROW_EXCEPTION(HadesError() << 
          ErrorFunction("LoaderWindow::RenderFrame") << 
          ErrorString("Could not present scene.") << 
          ErrorCodeWin(Result));
      }
    }
  }
}
