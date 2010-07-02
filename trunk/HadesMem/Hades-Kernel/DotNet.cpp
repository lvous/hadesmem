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

// Hades
#include "DotNet.h"
#include "Kernel.h"
#include "Hades-Common/I18n.h"

// C++ Standard Library
#include <fstream>
#include <iostream>
#include <exception>
#include <stdexcept>

// Boost C++ Libraries
#pragma warning(push, 1)
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#pragma warning(pop)

// RapidXML
#include <RapidXML/rapidxml.hpp>

// Hades namespace
namespace Hades
{
  namespace Kernel
  {
    // .NET frame event callback list
    std::vector<DotNetMgr::FrameCallback> DotNetMgr::m_FrameEvents;

    // Constructor
    DotNetMgr::DotNetMgr(Kernel* pKernel, std::wstring const& Config) 
      : m_pClrHost(), 
      m_pClrHostControl(nullptr), 
      m_pKernel(pKernel), 
      m_pDomainMgr(nullptr)
    {
      // Open config file
      std::wifstream ConfigFile(Config.c_str());
      if (!ConfigFile)
      {
        BOOST_THROW_EXCEPTION(DotNetMgrError() << 
          ErrorFunction("DotNetMgr::DotNetMgr") << 
          ErrorString("Could not open config file."));
      }

      // Copy file to buffer
      std::istreambuf_iterator<wchar_t> ConfigFileBeg(ConfigFile);
      std::istreambuf_iterator<wchar_t> ConfigFileEnd;
      std::vector<wchar_t> ConfigFileBuf(ConfigFileBeg, ConfigFileEnd);
      ConfigFileBuf.push_back(L'\0');

      // Open XML document
      rapidxml::xml_document<wchar_t> ConfigDoc;
      ConfigDoc.parse<0>(&ConfigFileBuf[0]);

      // Get runtime information
      auto RuntimeTag = ConfigDoc.first_node(L"Runtime");
      if (!RuntimeTag)
      {
        BOOST_THROW_EXCEPTION(DotNetMgrError() << 
          ErrorFunction("DotNetMgr::DotNetMgr") << 
          ErrorString("Invalid config file. Could not get runtime info."));
      }
      auto RuntimeVerNode = RuntimeTag->first_attribute(L"Version");
      std::wstring const RuntimeVer(RuntimeVerNode ? RuntimeVerNode->value() : 
        L"");
      if (RuntimeVer.empty())
      {
        BOOST_THROW_EXCEPTION(DotNetMgrError() << 
          ErrorFunction("DotNetMgr::DotNetMgr") << 
          ErrorString("Invalid config file. Unspecified runtime version."));
      }

      // Get domain manager information
      auto DomainMgrTag = ConfigDoc.first_node(L"DomainManager");
      if (!DomainMgrTag)
      {
        BOOST_THROW_EXCEPTION(DotNetMgrError() << 
          ErrorFunction("DotNetMgr::DotNetMgr") << 
          ErrorString("Invalid config file. Could not get domain manager "
            "info."));
      }
      auto DomainMgrAssemblyNode = DomainMgrTag->first_attribute(L"Assembly");
      std::wstring const DomainMgrAssembly(DomainMgrAssemblyNode ? 
        DomainMgrAssemblyNode->value() : L"");
      if (DomainMgrAssembly.empty())
      {
        BOOST_THROW_EXCEPTION(DotNetMgrError() << 
          ErrorFunction("DotNetMgr::DotNetMgr") << 
          ErrorString("Invalid config file. Unspecified domain manager "
            "assembly."));
      }
      auto DomainMgrTypeNode = DomainMgrTag->first_attribute(L"Type");
      std::wstring const DomainMgrType(DomainMgrTypeNode ? 
        DomainMgrTypeNode->value() : L"");
      if (DomainMgrType.empty())
      {
        BOOST_THROW_EXCEPTION(DotNetMgrError() << 
          ErrorFunction("DotNetMgr::DotNetMgr") << 
          ErrorString("Invalid config file. Unspecified domain manager "
            "type."));
      }

#pragma warning(push)
#pragma warning(disable: 4996)
      // Initialize the CLR using CorBindToRuntimeEx. This gets us
      // the ICLRRuntimeHost pointer we'll need to call Start.
      HRESULT BindResult = CorBindToRuntimeEx(
        RuntimeVer.c_str(), 
        L"wks", 
        STARTUP_CONCURRENT_GC, 
        CLSID_CLRRuntimeHost, 
        IID_ICLRRuntimeHost, 
        reinterpret_cast<PVOID*>(&m_pClrHost));
      if (FAILED(BindResult))
      {
        BOOST_THROW_EXCEPTION(DotNetMgrError() << 
          ErrorFunction("DotNetMgr::DotNetMgr") << 
          ErrorString("Failed to bind .NET framework.") << 
          ErrorCodeWin(BindResult));
      }
#pragma warning(pop)

      // Set CLR host control
      m_pClrHostControl.reset(new HadesHostControl());
      m_pClrHost->SetHostControl(static_cast<IHostControl*>(
        &*m_pClrHostControl));

      // Get a pointer to the ICLRControl interface
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
      // the process
      pCLRControl->SetAppDomainManagerType(DomainMgrAssembly.c_str(), 
        DomainMgrType.c_str());

      // Start the CLR
      HRESULT StartResult = m_pClrHost->Start();
      if (FAILED(StartResult))
      {
        BOOST_THROW_EXCEPTION(DotNetMgrError() << 
          ErrorFunction("DotNetMgr::DotNetMgr") << 
          ErrorString("Could not start CLR.") << 
          ErrorCodeWin(StartResult));
      }

      // Get domain manager
      m_pDomainMgr = m_pClrHostControl->GetDomainManagerForDefaultDomain();
      if (!m_pDomainMgr)
      {
        BOOST_THROW_EXCEPTION(DotNetMgrError() << 
          ErrorFunction("DotNetMgr::DotNetMgr") << 
          ErrorString("Could not get domain manager instance."));
      }

      // Call into the default application domain to attach the frame event
#pragma warning(push)
#pragma warning(disable: 4244)
      m_pDomainMgr->RegisterOnFrame(reinterpret_cast<LONG_PTR>(
        &DotNetMgr::SubscribeFrameEvent));
#pragma warning(pop)

      // Register OnFrame event for callback system
      m_pKernel->GetD3D9Mgr()->RegisterOnFrame(std::bind(
        &DotNetMgr::OnFrameEvent, this, std::placeholders::_1, 
        std::placeholders::_2));

      // Debug output
      std::wcout << "DotNetMgr::DotNetMgr: initialized." << std::endl;
    }

    // Load an assembly in the context of the current process
    void DotNetMgr::LoadAssembly(const std::wstring& Assembly, 
      const std::wstring& Parameters, 
      const std::wstring& Domain)
    {
      // Run assembly using domain manager
      m_pDomainMgr->RunAssembly(Domain.c_str(), Assembly.c_str(), 
        Parameters.c_str());
    }

    // Subscribe for OnFrame event
    void __stdcall DotNetMgr::SubscribeFrameEvent(FrameCallback Function)
    {
      // Add callback to list
      m_FrameEvents.push_back(Function);
    }

    // Hades OnFrame callback
    void DotNetMgr::OnFrameEvent(IDirect3DDevice9* /*pDevice*/, 
      D3D9::D3D9HelperPtr /*pHelper*/)
    {
      // Run all callbacks
      std::for_each(m_FrameEvents.begin(), m_FrameEvents.end(), 
        [] (FrameCallback Current) 
      {
        Current();
      });
    }
  }
}
