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
#include <memory>

// Windows
#include <Windows.h>
#include <atlbase.h>
#include <atlwin.h>

// DirectX
#include <d3d9.h>
#include <d3dx9.h>

// Hades
#include "Hades-GUI/CGUI.h"
#include "Hades-Common/EnsureCleanup.h"

namespace Hades
{
  namespace Loader
  {
    class LoaderWindow : public CWindowImpl<LoaderWindow>
    {
    public:
      // Initialize D3D
      void InitD3D();

      // Load GUI
      void LoadGUI();

      // Render frame
      void RenderFrame();

      // Specify window class name
      DECLARE_WND_CLASS(L"HadesLoaderWndClass")

      // WM_DESTROY message callback
      LRESULT OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam, 
        BOOL& bHandled);

      // WM_CREATE message callback
      LRESULT OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, 
        BOOL& bHandled);

      // Custom window procedure
      static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, 
        LPARAM lParam);

      // GetWindowProc override to allow use of custom window procedure
      WNDPROC GetWindowProc();

      BEGIN_MSG_MAP(LoaderWindow)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
      END_MSG_MAP()

    private:
      // D3D instance
      static CComPtr<IDirect3D9Ex> m_pD3D;
      // D3D device
      static CComPtr<IDirect3DDevice9Ex> m_pD3DDevice;

      // Hades GUI
      static std::shared_ptr<Hades::GUI::GUI> m_pGui;

      // Hades icon
      static Hades::Windows::EnsureDestroyIcon m_HadesIcon;
    };
  }
}
