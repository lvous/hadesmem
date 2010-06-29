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
#include <string>
#include <vector>

// Windows API
#include <atlbase.h>
#include <mscoree.h>
#import <mscorlib.tlb> raw_interfaces_only\
  rename("ReportEvent","ReportEventManaged")

// DirectX API
#include <d3d9.h>

// Hades
#include "Hades-D3D9/D3D9Mgr.h"
#include "Hades-Common/Error.h"

// Hades namespace
namespace Hades
{
  // DotNetMgr exception type
  class DotNetMgrError : public virtual HadesError 
  { };

  // DotNet related code
  class DotNetMgr
  {
  public:
    typedef void (__stdcall* FrameCallback)();

    static void __stdcall SubscribeFrameEvent(FrameCallback Function);

    void LoadAssembly(const std::wstring& Assembly, 
      const std::wstring& Parameters, 
      const std::wstring& Domain);

    void OnFrameEvent(IDirect3DDevice9* pDevice, D3D9HelperPtr pHelper);

    DotNetMgr(class Kernel* pKernel);

  private:
    CComPtr<ICLRRuntimeHost> m_pClrHost;
    class HadesHostControl* m_pClrHostControl;
    bool m_IsDotNetInitialized;
    class Kernel* m_pKernel;

    static std::vector<FrameCallback> m_FrameEvents;
  };
}
