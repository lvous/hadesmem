#include <Windows.h>
#include <process.h>
#include <metahost.h>
#include <corerror.h>

#include <string>

#pragma comment( lib, "mscoree" )

#define LOAD_DLL_PATH L"C:\\Users\\Josh\\Documents\\visual studio 2010\\"
L"Projects\\DotNetTest\\DotNetTest\\bin\\Debug\\DotNetTest.exe"
#define LOAD_DLL_FILE_NAME L"DotNetTest.exe"
#define NAMESPACE_AND_CLASS L"DotNetTest.Program"
#define MAIN_METHOD L"Launch"
#define MAIN_METHOD_ARGS L""

// WARNING: This code is purely for internal testing. It breaks numerous 
// 'good practice' guidelines and is not production quality. Do not attempt 
// to run.

ICLRMetaHostPolicy* pMetaHost = NULL;
ICLRRuntimeInfo* pRuntimeInfo = NULL;
ICLRRuntimeHost* clrHost = NULL;
HANDLE hThread = NULL;

#define MB(s) MessageBox(NULL, s, NULL, MB_OK);

unsigned __stdcall ThreadMain( void* )
{
  if (IsDebuggerPresent())
  {
    DebugBreak();
  }
  
	HRESULT hr = CLRCreateInstance(CLSID_CLRMetaHostPolicy, 
	  IID_ICLRMetaHostPolicy, 
	  (LPVOID*)&pMetaHost);
	if (FAILED(hr))
	{
		MB(L"Could not create instance of ICLRMetaHost");
		return 1;
	}
  
	DWORD pcchVersion;
	DWORD dwConfigFlags;
	hr = pMetaHost->GetRequestedRuntime(METAHOST_POLICY_HIGHCOMPAT,
										LOAD_DLL_PATH, 
										NULL,
										NULL, 
										&pcchVersion,
										NULL, 
										NULL,
										&dwConfigFlags,
										IID_ICLRRuntimeInfo,
										(LPVOID*)&pRuntimeInfo);
	if (FAILED(hr))
	{
		MB(L"Could not get an instance of ICLRRuntimeInfo");
		return 1;
	}

	// We need this if we have old .NET 3.5 mixed-mode DLLs
	hr = pRuntimeInfo->BindAsLegacyV2Runtime();
	if (FAILED(hr))
	{
		MB(L"Failed to bind as legacy v2 runtime! (.NET 3.5 Mixed-Mode Support)");
		return 1;
	}

	hr = pRuntimeInfo->GetInterface(CLSID_CLRRuntimeHost, 
	  IID_ICLRRuntimeHost, 
	  (LPVOID*)&clrHost);
	if (FAILED(hr))
	{
		MB(L"Could not get an instance of ICLRRuntimeHost!");
		return 1;
	}

	hr = clrHost->Start();
	if (FAILED(hr))
	{
		MB(L"Failed to start the CLR!");
		return 1;
	}
	
	// Note: Blocking call.
	DWORD dwRet = 0;
	hr = clrHost->ExecuteInDefaultAppDomain(LOAD_DLL_PATH, 
	  NAMESPACE_AND_CLASS, 
	  MAIN_METHOD, 
	  MAIN_METHOD_ARGS, 
	  &dwRet);
	if (FAILED(hr))
	{
		MB(L"Failed to execute in the default app domain!");
		return 1;
	}

	return 0;
}

void LoadClr()
{
	hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadMain, NULL, 0, NULL);
}

BOOL WINAPI DllMain(HMODULE hDll, DWORD dwReason, LPVOID)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		LoadClr();
	}
	
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		if (clrHost)
		{
			// We eventually 'die' so we make sure we stop the CLR.
			clrHost->Stop();
			// And release it.
			clrHost->Release();
		}

		// Yes yes, I know. I should be using _endthread(ex)
		// however, I can't. Since we don't want the thread killed until we exit.
		if (hThread)
		{
			TerminateThread( hThread, 0 );
			CloseHandle(hThread);
		}
	}

	return TRUE;
}
