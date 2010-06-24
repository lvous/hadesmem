// C++ Standard Library
#include <iostream>

// Boost
#pragma warning(push, 1)
#include <boost/thread.hpp>
#pragma warning(pop)

// Hades
#include "DotNet.h"
#include "Kernel.h"
#include "Hades-D3D9/GuiMgr.h"

namespace Hades
{
  DotNetMgr::DotNetMgr(Kernel* pKernel, std::wstring const& RuntimeVersion) 
    : m_pKernel(pKernel), 
    m_pMetaHost(nullptr), 
    m_pRuntimeInfo(nullptr), 
    m_pClrHost(nullptr), 
    m_ClrStarted(false)
  {
    HRESULT ClrCreateResult = CLRCreateInstance(
      CLSID_CLRMetaHost, 
      IID_ICLRMetaHost, 
      reinterpret_cast<void**>(&m_pMetaHost.p));
    if (FAILED(ClrCreateResult))
    {
      BOOST_THROW_EXCEPTION(DotNetMgrError() << 
        ErrorFunction("DotNetMgr::DotNetMgr") << 
        ErrorString("Could not create CLR Meta Host.") << 
        ErrorCodeWin(ClrCreateResult));
    }

    HRESULT GetRuntimeResult = m_pMetaHost->GetRuntime(
      RuntimeVersion.c_str(), 
      IID_ICLRRuntimeInfo, 
      reinterpret_cast<void**>(&m_pRuntimeInfo.p));
    if (FAILED(GetRuntimeResult))
    {
      BOOST_THROW_EXCEPTION(DotNetMgrError() << 
        ErrorFunction("DotNetMgr::DotNetMgr") << 
        ErrorString("Could not create CLR Runtime Info.") << 
        ErrorCodeWin(GetRuntimeResult));
    }

    HRESULT BindLegacyResult = m_pRuntimeInfo->BindAsLegacyV2Runtime();
    if (FAILED(BindLegacyResult))
    {
      BOOST_THROW_EXCEPTION(DotNetMgrError() << 
        ErrorFunction("DotNetMgr::DotNetMgr") << 
        ErrorString("Could not bind as legacy runtime.") << 
        ErrorCodeWin(BindLegacyResult));
    }

    HRESULT GetInterfaceResult = m_pRuntimeInfo->GetInterface(
      CLSID_CLRRuntimeHost, 
      IID_ICLRRuntimeHost, 
      reinterpret_cast<void**>(&m_pClrHost.p));
    if (FAILED(GetInterfaceResult))
    {
      BOOST_THROW_EXCEPTION(DotNetMgrError() << 
        ErrorFunction("DotNetMgr::DotNetMgr") << 
        ErrorString("Could not create CLR Runtime Host.") << 
        ErrorCodeWin(GetInterfaceResult));
    }

    HRESULT ClrStartResult = m_pClrHost->Start();
    if (FAILED(ClrStartResult))
    {
      BOOST_THROW_EXCEPTION(DotNetMgrError() << 
        ErrorFunction("DotNetMgr::DotNetMgr") << 
        ErrorString("Could not start CLR.") << 
        ErrorCodeWin(ClrStartResult));
    }

    m_ClrStarted = true;
  }

  DotNetMgr::~DotNetMgr()
  {
    if (m_ClrStarted)
    {
      m_pClrHost->Stop();
    }
  }

  void DotNetMgr::LoadAssembly(std::wstring const& Assembly, 
    std::wstring const& Type, std::wstring const& Method, 
    std::wstring const& Parameters)
  {
    boost::thread LoadThread(&DotNetMgr::LoadAssemblyReal, 
      this, m_pKernel, Assembly, Type, Method, Parameters);
    LoadThread;
  }

  void DotNetMgr::LoadAssemblyReal(Kernel* pKernel, 
    std::wstring Assembly, 
    std::wstring Type, 
    std::wstring Method, 
    std::wstring Parameters)
  {
    try
    {
      DWORD AppDomainResult = 0;
      HRESULT Result = m_pClrHost->ExecuteInDefaultAppDomain(
        Assembly.c_str(), 
        Type.c_str(), 
        Method.c_str(), 
        Parameters.c_str(), 
        &AppDomainResult);
      if (FAILED(Result))
      {
        std::wcout << "DotNetMgr::LoadAssembly: Failed to execute in default "
          "app domain." << std::endl;

        BOOST_THROW_EXCEPTION(DotNetMgrError() << 
          ErrorFunction("DotNetMgr::LoadAssembly") << 
          ErrorString("Could not execute in default app domain.") << 
          ErrorCodeWin(Result));
      }
    }
    catch (boost::exception const& e)
    {
      // Print error information
      pKernel->GetGuiMgr()->Print(boost::diagnostic_information(e));
    }
    catch (std::exception const& e)
    {
      // Print error information
      pKernel->GetGuiMgr()->Print(e.what());
    }
  }
}
