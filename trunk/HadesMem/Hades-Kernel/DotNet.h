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
#include <string>

// Boost
#pragma warning(push, 1)
#pragma warning(disable: 4267)
#include <boost/signals2.hpp>
#pragma warning(pop)

// Windows API
#include <Windows.h>
#include <atlbase.h>
#include <metahost.h>
#include <CorError.h>

// DirectX
#include <d3d9.h>
#include <d3dx9.h>

// Hades
#include "Hades-Common/Error.h"
#include "Hades-D3D9/D3D9Helper.h"


namespace Hades
{
  // DotNetMgr exception type
  class DotNetMgrError : public virtual HadesError 
  { };

  class DotNetMgr
  {
  public:
    DotNetMgr(class Kernel* pKernel, std::wstring const& ConfigPath);

    ~DotNetMgr();

    void LoadAssembly(std::wstring const& Assembly, 
      std::wstring const& Type, std::wstring const& Method, 
      std::wstring const& Parameters);

  private:
    void LoadAssemblyReal(class Kernel* pKernel, 
      std::wstring Assembly, 
      std::wstring Type, 
      std::wstring Method, 
      std::wstring Parameters);

    class Kernel* m_pKernel;

    CComPtr<ICLRMetaHost> m_pMetaHost;
    CComPtr<ICLRRuntimeInfo> m_pRuntimeInfo;
    CComPtr<ICLRRuntimeHost> m_pClrHost;

    bool m_ClrStarted;
  };
}
