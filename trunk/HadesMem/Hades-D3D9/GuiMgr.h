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

// Hades
#include "D3D9Helper.h"
#include "Hades-GUI/CGUI.h"
#include "Hades-Common/Error.h"

namespace Hades
{
  // GuiMgr exception type
  class GuiMgrError : public virtual HadesError 
  { };

  // GUI managing class
  class GuiMgr
  {
  public:
    // Constructor
    GuiMgr(class Kernel* pKernel);

  private:
    // Input callbacks
    bool OnInputMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    bool OnSetCursor(HCURSOR hCursor);

    // D3D9Mgr callbacks
    void OnInitialize(IDirect3DDevice9* pDevice, D3D9HelperPtr pHelper);
    void OnFrame(IDirect3DDevice9* pDevice, D3D9HelperPtr pHelper);
    void OnLostDevice(IDirect3DDevice9* pDevice, D3D9HelperPtr pHelper);
    void OnResetDevice(IDirect3DDevice9* pDevice, D3D9HelperPtr pHelper);
    void OnRelease(IDirect3DDevice9* pDevice, D3D9HelperPtr pHelper);

    // Toggle visibility
    void ToggleVisible();

    // Kernel instance
    class Kernel* m_pKernel;

    // D3D9 device
    IDirect3DDevice9* m_pDevice;
  };
}
