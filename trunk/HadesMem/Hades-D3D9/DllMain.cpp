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

// Windows API
#include <crtdbg.h>
#include <Windows.h>

// C++ Standard Library
#include <string>
#include <iostream>
#include <exception>

// Boost
#pragma warning(push, 1)
#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#pragma warning(pop)

// Hades
#include "D3D9Mgr.h"
#include "Hades-Kernel/Kernel.h"
#include "Hades-Common/Logger.h"

void DrawOutline(IDirect3DDevice9* pDevice, Hades::D3D9HelperPtr pHelper)
{
  // Get viewport
  D3DVIEWPORT9 Viewport;
  pDevice->GetViewport(&Viewport);

  // Draw box on viewport border
  Hades::Math::Vec2f const BottomLeft(0,0);
  Hades::Math::Vec2f const TopRight(static_cast<float>(Viewport.Width), 
    static_cast<float>(Viewport.Height));
  pHelper->DrawBox(BottomLeft, TopRight, 5, D3DCOLOR_ARGB(100, 255, 0, 0));
}

extern "C" __declspec(dllexport) DWORD __stdcall Initialize(HMODULE Module, 
  Hades::Kernel* pKernel)
{
  try
  {
    // Break to debugger if present
    if (IsDebuggerPresent())
    {
      DebugBreak();
    }

    // Hades version number
    std::wstring const VerNum(L"TRUNK");

    // Version and copyright output
#if defined(_M_X64)
    std::wcout << "Hades-D3D9 AMD64 [Version " << VerNum << "]" << std::endl;
#elif defined(_M_IX86)
    std::wcout << "Hades-D3D9 IA32 [Version " << VerNum << "]" << std::endl;
#else
#error Unsupported platform!
#endif
    std::wcout << "Copyright (C) 2010 Cypherjb. All rights reserved." << 
      std::endl;
    std::wcout << "Website: http://www.cypherjb.com/, "
      "Email: cypher.jb@gmail.com." << std::endl;
    std::wcout << "Built on " << __DATE__ << " at " << __TIME__ << "." << 
      std::endl << std::endl;

    // Debug output
    std::wcout << boost::wformat(L"Initialize: Module = %p, Kernel = %p.") 
      %Module %pKernel << std::endl;

    // Initialize D3D9 manager
    Hades::D3D9Mgr::Startup(pKernel);

    // Register test callback
    Hades::D3D9Mgr::RegisterOnFrame(&DrawOutline);
  }
  catch (boost::exception const& e)
  {
    // Dump error information
    MessageBoxA(NULL, boost::diagnostic_information(e).c_str(), 
      "Hades-D3D9", MB_OK);
  }
  catch (std::exception const& e)
  {
    // Dump error information
    MessageBoxA(NULL, e.what(), "Hades-D3D9", MB_OK);
  }

  // Success
  return 0;
}

BOOL WINAPI DllMain(HINSTANCE /*hinstDLL*/, DWORD /*fdwReason*/, 
  LPVOID /*lpvReserved*/)
{
  // Attempt to detect memory leaks in debug mode
#ifdef _DEBUG
  int CurrentFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  int NewFlags = (_CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF | 
    _CRTDBG_CHECK_ALWAYS_DF);
  _CrtSetDbgFlag(CurrentFlags | NewFlags);
#endif

  return TRUE;
}
