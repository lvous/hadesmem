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

// C++ Standard Library
#include <string>
#include <memory>
#include <iostream>

// Windows
#include <Windows.h>
#include <atlbase.h>
#include <atlwin.h>

// DirectX
#include <d3d9.h>
#include <d3dx9.h>

// Hades
#include "Window.h"
#include "Hades-Common/Error.h"
#include "Hades-Common/Logger.h"

int CALLBACK wWinMain(
  __in  HINSTANCE /*hInstance*/,
  __in  HINSTANCE /*hPrevInstance*/,
  __in  LPWSTR /*lpCmdLine*/,
  __in  int nCmdShow)
{
  try
  {
    // Initialize COM
    HRESULT CoInitResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(CoInitResult))
    {
      DWORD LastError = GetLastError();
      BOOST_THROW_EXCEPTION(Hades::HadesError() << 
        Hades::ErrorFunction("wWinMain") << 
        Hades::ErrorString("Could not initialize COM.") << 
        Hades::ErrorCodeWin(LastError));
    }
    Hades::Windows::EnsureCoUninitialize MyEnsureCoUninitalize;

    // Initialize logger
    Hades::Util::InitLogger(L"Hades-Loader-Log", L"Hades-Loader-Debug");

    // Hades version number
    std::wstring const VerNum(L"TRUNK");

    // Version and copyright output
#if defined(_M_X64)
    std::wcout << "Hades-Loader AMD64 [Version " << VerNum << "]" << std::endl;
#elif defined(_M_IX86)
    std::wcout << "Hades-Loader IA32 [Version " << VerNum << "]" << std::endl;
#else
#error Unsupported platform!
#endif
    std::wcout << "Copyright (C) 2010 Cypherjb. All rights reserved." << 
      std::endl;
    std::wcout << "Website: http://www.cypherjb.com/, Email: "
      "cypher.jb@gmail.com." << std::endl;
    std::wcout << "Built on " << __DATE__ << " at " << __TIME__ << "." << 
      std::endl;

    // Create loader window manager
    Hades::Loader::LoaderWindow MainWindow;

    // Create window
    if (!MainWindow.Create(GetDesktopWindow(), NULL, L"Hades Loader", 
      WS_OVERLAPPEDWINDOW))
    {
      BOOST_THROW_EXCEPTION(Hades::HadesError() << 
        Hades::ErrorFunction("wWinMain") << 
        Hades::ErrorString("Could not create loader window."));
    }

    // Resize window
    if (!MainWindow.ResizeClient(1280, 720))
    {
      BOOST_THROW_EXCEPTION(Hades::HadesError() << 
        Hades::ErrorFunction("wWinMain") << 
        Hades::ErrorString("Could not resize loader window."));
    }

    // Show window
    MainWindow.ShowWindow(nCmdShow);

    // Center window
    if (!MainWindow.CenterWindow())
    {
      BOOST_THROW_EXCEPTION(Hades::HadesError() << 
        Hades::ErrorFunction("wWinMain") << 
        Hades::ErrorString("Could not center loader window."));
    }

    // Initialize D3D
    MainWindow.InitD3D();
    
    // Load GUI
    MainWindow.LoadGUI();

    // Standard message loop
    MSG CurMsg;
    for (;;)
    {
      // More standard message loop stuff
      while (PeekMessage(&CurMsg, NULL, 0, 0, PM_REMOVE))
      {
        TranslateMessage(&CurMsg);
        DispatchMessage(&CurMsg);
      }

      // Break out of message loop on quit request
      if(CurMsg.message == WM_QUIT)
      {
        break;
      }

      // Perform rendering
      MainWindow.RenderFrame();
    }

    // Return exit code from PostQuitMessage
    return static_cast<int>(CurMsg.wParam);
  }
  catch (boost::exception const& e)
  {
    // Dump error information
    MessageBoxA(NULL, boost::diagnostic_information(e).c_str(), 
      "Hades-Loader", MB_OK);
  }
  catch (std::exception const& e)
  {
    // Dump error information
    MessageBoxA(NULL, e.what(), "Hades-Loader", MB_OK);
  }

  return 0;
}
