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
#include "DotNet.h"
#include "Kernel.h"
#include "CLRHostControl.h"
#include "Hades-Common/I18n.h"

// C++ Standard Library
#include <exception>
#include <stdexcept>
#include <iostream>

// Boost C++ Libraries
#pragma warning(push, 1)
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#pragma warning(pop)

// Hades namespace
namespace Hades
{
  namespace Kernel
  {
    // Static data
    std::vector<DotNetMgr::FrameCallback> DotNetMgr::m_FrameEvents;

    // Constructor
    DotNetMgr::DotNetMgr(Kernel* pKernel) 
      : m_pClrHost(), 
      m_pClrHostControl(nullptr), 
      m_IsDotNetInitialized(false), 
      m_pKernel(pKernel)
    {
#pragma warning(push)
#pragma warning(disable: 4996)
      // Initialize the CLR using CorBindToRuntimeEx. This gets us
      // the ICLRRuntimeHost pointer we'll need to call Start.
      HRESULT BindResult = CorBindToRuntimeEx(L"v2.0.50727", L"wks", 
        STARTUP_CONCURRENT_GC, CLSID_CLRRuntimeHost, IID_ICLRRuntimeHost, 
        reinterpret_cast<PVOID*>(&m_pClrHost));
      if (FAILED(BindResult))
      {
        BOOST_THROW_EXCEPTION(DotNetMgrError() << 
          ErrorFunction("DotNetMgr::DotNetMgr") << 
          ErrorString("Failed to bind .NET framework.") << 
          ErrorCodeWin(BindResult));
      }
#pragma warning(pop)

      m_pClrHostControl = new HadesHostControl();
      m_pClrHost->SetHostControl(static_cast<IHostControl*>(
        m_pClrHostControl));

      // Get a pointer to the ICLRControl interface.
      ICLRControl *pCLRControl = NULL;
      HRESULT GetClrResult = m_pClrHost->GetCLRControl(&pCLRControl);
      if (FAILED(GetClrResult))
      {
        BOOST_THROW_EXCEPTION(DotNetMgrError() << 
          ErrorFunction("DotNetMgr::DotNetMgr") << 
          ErrorString("Could not get CLR control.") << 
          ErrorCodeWin(GetClrResult));
      }

      // Call SetAppDomainManagerType to associate our domain manager with
      // the process.
      std::wstring DomainMgr(L"HadesAD, Version=1.0.0.0, PublicKeyToken="
        L"cd2e409a307a3c42, culture=neutral, processorArchitecture=MSIL");
      std::wstring DomainMgrType(L"HadesAD.HadesVM");
      pCLRControl->SetAppDomainManagerType(DomainMgr.c_str(), 
        DomainMgrType.c_str());

      // Start the CLR.
      HRESULT StartResult = m_pClrHost->Start();
      if (FAILED(StartResult))
      {
        BOOST_THROW_EXCEPTION(DotNetMgrError() << 
          ErrorFunction("DotNetMgr::DotNetMgr") << 
          ErrorString("Could not start CLR.") << 
          ErrorCodeWin(StartResult));
      }

      HadesAD::IHadesVM* pDomainManagerForDefaultDomain = 
        m_pClrHostControl->GetDomainManagerForDefaultDomain();

      if (!pDomainManagerForDefaultDomain)
      {
        BOOST_THROW_EXCEPTION(DotNetMgrError() << 
          ErrorFunction("DotNetMgr::DotNetMgr") << 
          ErrorString("Could not find default domain CLR."));
      }

      // Call into the default application domain to attach the frame event
#pragma warning(push)
#pragma warning(disable: 4244)
      pDomainManagerForDefaultDomain->RegisterOnFrame(
        reinterpret_cast<LONG_PTR>(&DotNetMgr::SubscribeFrameEvent));
#pragma warning(pop)

      m_IsDotNetInitialized = true;

      std::wcout << "DotNetMgr: .NET framework successfully initialized." 
        << std::endl;

      // Register OnFrame event for callback system
      m_pKernel->GetD3D9Mgr()->RegisterOnFrame(std::bind(
        &DotNetMgr::OnFrameEvent, this, std::placeholders::_1, 
        std::placeholders::_2));

      // Debug output
      std::wcout << "DotNetMgr initialized." << std::endl;
    }

    // Load an assembly in the context of the current process
    void DotNetMgr::LoadAssembly(const std::wstring& Assembly, 
      const std::wstring& Parameters, 
      const std::wstring& Domain)
    {
      if (!m_IsDotNetInitialized)
      {
        BOOST_THROW_EXCEPTION(DotNetMgrError() << 
          ErrorFunction("DotNetMgr::DotNetMgr") << 
          ErrorString(".NET is not initialized."));
      }

      HadesAD::IHadesVM* pDomainMgrForDefaultDomain = m_pClrHostControl->
        GetDomainManagerForDefaultDomain();

      if (!pDomainMgrForDefaultDomain)
      {
        BOOST_THROW_EXCEPTION(DotNetMgrError() << 
          ErrorFunction("DotNetMgr::DotNetMgr") << 
          ErrorString("Could not get domain manager for the default domain."));
      }

      pDomainMgrForDefaultDomain->RunAssembly(Domain.c_str(), Assembly.c_str(), 
        Parameters.c_str());
    }

    void __stdcall DotNetMgr::SubscribeFrameEvent(FrameCallback Function)
    {
      m_FrameEvents.push_back(Function);
    }

    void DotNetMgr::OnFrameEvent(IDirect3DDevice9* /*pDevice*/, 
      D3D9::D3D9HelperPtr /*pHelper*/)
    {
      std::for_each(m_FrameEvents.begin(), m_FrameEvents.end(), 
        [] (FrameCallback Current) 
      {
        Current();
      });
    }
  }
}
