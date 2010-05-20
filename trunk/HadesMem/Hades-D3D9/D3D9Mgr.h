/*
This file is part of HadesMem.
Copyright � 2010 Cypherjb (aka Chazwazza, aka Cypher). 
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

// Boost
#pragma warning(push, 1)
#pragma warning(disable: 4267)
#include <boost/signals2.hpp>
#pragma warning(pop)

// Windows API
#include <d3d9.h>

// Hades
#include "D3D9Helper.h"
#include "Hades-Common/Error.h"
#include "Hades-Memory/Patcher.h"

namespace Hades
{
  // D3D9Mgr exception type
  class D3D9MgrError : public virtual HadesError 
  { };

  // D3D9 managing class
  class D3D9Mgr
  {
  public:
    // Hook D3D9
    static void Startup(class Kernel* pHades);

    // Callback type
    typedef boost::signals2::signal<void (IDirect3DDevice9* /*pDevice*/, 
      D3D9HelperPtr /*pHelper*/)> Callbacks;

    // Register callbacks for rendering events
    static boost::signals2::connection RegisterOnFrame(
      const Callbacks::slot_type& Subscriber);
    static boost::signals2::connection RegisterOnLostDevice(
      const Callbacks::slot_type& Subscriber);
    static boost::signals2::connection RegisterOnResetDevice(
      const Callbacks::slot_type& Subscriber);
    static boost::signals2::connection RegisterOnInitialize(
      const Callbacks::slot_type& Subscriber);

  protected:
    // Direct3DCreate9 hook implementation
    static IDirect3D9* WINAPI Direct3DCreate9_Hook(UINT SDKVersion);

    // IDirect3D9::CreateDevice hook implementation
    static HRESULT WINAPI CreateDevice_Hook(IDirect3D9* pThis, 
      UINT Adapter, 
      D3DDEVTYPE DeviceType, 
      HWND hFocusWindow, 
      DWORD BehaviorFlags, 
      D3DPRESENT_PARAMETERS* pPresentationParameters, 
      IDirect3DDevice9** ppReturnedDeviceInterface);

    // IDrect3DDevice9::EndScene hook implementation
    static HRESULT WINAPI EndScene_Hook(IDirect3DDevice9* pThis);

    // IDrect3DDevice9::Reset hook implementation
    static HRESULT WINAPI Reset_Hook(IDirect3DDevice9* pThis, 
      D3DPRESENT_PARAMETERS* pPresentationParameters);

    // Do rendering
    static void Render();

    // Initialize resources such as textures (managed and unmanaged), vertex 
    // buffers, and other D3D resources
    static void Initialize();

    // Release all unmanaged resources
    static void PreReset();

    // Re-initialize all unmanaged resources
    static void PostReset();

  private:
    // D3D9 hooks
    static std::shared_ptr<Memory::PatchDetour> m_pDirect3DCreate9Hk;
    static std::shared_ptr<Memory::PatchDetour> m_pCreateDeviceHk;
    static std::shared_ptr<Memory::PatchDetour> m_pEndSceneHk;
    static std::shared_ptr<Memory::PatchDetour> m_pResetHk;

    // Current device
    static IDirect3DDevice9* m_pDevice;

    // State block
    static IDirect3DStateBlock9*	m_pStateBlock;

    // D3D9 helper
    static D3D9HelperPtr m_pD3D9Helper;

    // Callback managers
    static Callbacks m_CallsOnFrame;
    static Callbacks m_CallsOnLostDevice;
    static Callbacks m_CallsOnResetDevice;
    static Callbacks m_CallsOnInitialize;

    // Hades manager
    static class Kernel* m_pKernel;
  };
}
