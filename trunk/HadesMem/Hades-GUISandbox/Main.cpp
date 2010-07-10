/*
Copyright (c) 2010 Jan Miguel Garcia (bobbysing)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// C++ Standard Library
#include <string>

// Boost
#include <boost/format.hpp>

// Windows
#include <shlobj.h>
#include <Windows.h>
#include <wininet.h>

// D3D
#include <d3d9.h>
#include <d3dx9.h>

// Hades
#include "Hades-GUI/CGUI.h"
#include "Hades-Common/Error.h"
#include "Hades-Common/Filesystem.h"
#include "Hades-Common/StringBuffer.h"

bool g_Running = true;
IDirect3DDevice9* g_pDevice = nullptr;
D3DPRESENT_PARAMETERS D3DPresentParams;
int g_MenuSize = 0;
Hades::GUI::GUI* gpGui = nullptr;
bool bReset = false;
HWND SandboxWnd = 0;
const int WndWidth = 800, WndHeight = 600;
unsigned long FramesPerSec = 0;
DWORD FPSTimer = 0;

std::string MainMenuCallback(const char *, Hades::GUI::CElement* pElement)
{
  // Get window corresponding to selected menu item
  if(auto pWindow = gpGui->GetWindowByString(pElement->GetString(false, 1), 1))
  {
    // Show window
    pWindow->SetVisible(!pWindow->IsVisible());
  }

  return std::string();
}

void AddMainMenuItem(Hades::GUI::CWindow* pWindow, const char* pszLabel, 
  const char* pszWindow)
{
  // Increase menu size
  pWindow->SetHeight(pWindow->GetHeight() + 25);

  // Create new menu button
  auto pButton = new Hades::GUI::CButton(*gpGui, 0);
  pButton->SetHeight(BUTTON_HEIGHT);
  pButton->SetWidth(100);
  pButton->SetRelPos(Hades::GUI::Pos(15, g_MenuSize * 25 + 10));
  pButton->SetString(pszLabel);
  pButton->SetString(pszWindow, 1);
  pButton->SetCallback(MainMenuCallback);

  // Add button to menu
  pWindow->AddElement(pButton);

  // Increase menu count
  ++g_MenuSize;
}

Hades::GUI::CWindow* CreateMainMenu()
{
  // Create XML element to initialize GUI element
  auto pElement(std::make_shared<TiXmlElement>("Window"));
  pElement->SetAttribute("absX", 100);
  pElement->SetAttribute("absY", 330);
  pElement->SetAttribute("width", 130);
  pElement->SetAttribute("height", 40);
  pElement->SetAttribute("string", "Main menu");
  pElement->SetAttribute("string2", "WINDOW_MAIN");
  pElement->SetAttribute("visible", 0);
  pElement->SetAttribute("closebutton", 1);

  // Reset menu size
  g_MenuSize = 0;

  // Create main menu window
  auto pWindow = gpGui->AddWindow(new Hades::GUI::CWindow(*gpGui, &*pElement));

  // Return pointer to window
  return pWindow;
}

void LoadGUI(IDirect3DDevice9* pDevice)
{
  // Create GUI
  gpGui = new Hades::GUI::GUI(pDevice);

  // Get current working directory
  std::wstring CurDir;
  if (!GetCurrentDirectory(MAX_PATH, Hades::Util::MakeStringBuffer(CurDir, 
    MAX_PATH)))
  {
    DWORD LastError = GetLastError();
    BOOST_THROW_EXCEPTION(Hades::HadesError() << 
      Hades::ErrorFunction("LoadGUI") << 
      Hades::ErrorString("Could not get current directory.") << 
      Hades::ErrorCodeWin(LastError));
  }

  // Set new working directory to GUI library folder
  std::wstring GuiPath((Hades::Windows::GetSelfDirPath() / L"/Gui/").
    file_string());
  if (!SetCurrentDirectory(GuiPath.c_str()))
  {
    DWORD LastError = GetLastError();
    BOOST_THROW_EXCEPTION(Hades::HadesError() << 
      Hades::ErrorFunction("LoadGUI") << 
      Hades::ErrorString("Could not set current directory.") << 
      Hades::ErrorCodeWin(LastError));
  }

  // Load interface data
  gpGui->LoadInterfaceFromFile("ColorThemes.xml");
  gpGui->LoadInterfaceFromFile("GuiTest_UI.xml");

  // Restore old working directory
  if (!SetCurrentDirectory(CurDir.c_str()))
  {
    DWORD LastError = GetLastError();
    BOOST_THROW_EXCEPTION(Hades::HadesError() << 
      Hades::ErrorFunction("LoadGUI") << 
      Hades::ErrorString("Could not restore current directory.") << 
      Hades::ErrorCodeWin(LastError));
  }

  // Create main menu
  auto pMainMenu = CreateMainMenu();
  AddMainMenuItem(pMainMenu, "Waypoints", "WINDOW_WAYPOINT_CONTROLS");
  AddMainMenuItem(pMainMenu, "Test 2", "WINDOW_TEST2");

  // Set main menu as visible
  pMainMenu->SetVisible(true);

  // Set GUI as visible
  gpGui->SetVisible(true);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // If GUI handled message then there's nothing left to do
  if(gpGui && (gpGui->GetMouse().HandleMessage(uMsg, wParam, lParam) || 
    gpGui->GetKeyboard().HandleMessage(uMsg, wParam, lParam)))
  {
    return 0;
  }

  switch(uMsg)
  {
  case WM_DESTROY:
  case WM_CLOSE:
    {
      // Destroy render window
      DestroyWindow(hWnd);
      // Unregister render window class
      UnregisterClass(L"Hades-GUI", GetModuleHandle(nullptr));
      // Stop running
      g_Running = false;

      return 0;
    }
  case WM_KEYDOWN:
    {
      if(GetAsyncKeyState(VK_UP) & 1)
      {
        if(gpGui)
        {
          // Get device from GUI instance
          IDirect3DDevice9* pDevice = gpGui->GetDevice();

          // Delete GUI instance
          delete gpGui;
          gpGui = nullptr;

          // Create new GUI instance
          LoadGUI(pDevice);
        }
      }
      else if(GetAsyncKeyState(VK_DOWN) & 1)
      {
        // Get device from GUI instance
        IDirect3DDevice9* pDevice = gpGui->GetDevice();

        // Set reset flag
        bReset = true;
        do
        {
          // Notify GUI of lost device
          gpGui->OnLostDevice();
          // Reset device
          pDevice->Reset(&D3DPresentParams);
          // Notify GUI of reset device
          gpGui->OnResetDevice(pDevice);
        }
        // Ensure device has been reset
        while(FAILED(pDevice->TestCooperativeLevel()));
        // Unset reset flag
        bReset = false;
      }
    }
  }

  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool InitWindowClass(HINSTANCE hInstance)
{
  // Initialize window class
  WNDCLASSEX MyWndClass = { sizeof(WNDCLASSEX) };
  MyWndClass.style = CS_OWNDC;
  MyWndClass.lpfnWndProc = WndProc;
  MyWndClass.cbClsExtra = 0;
  MyWndClass.cbWndExtra = 0;
  MyWndClass.hInstance = hInstance;
  MyWndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  MyWndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
  MyWndClass.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(
    BLACK_BRUSH));
  MyWndClass.lpszMenuName = nullptr;
  MyWndClass.lpszClassName = L"Hades-GUI";
  MyWndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

  // Register window class
  return RegisterClassEx(&MyWndClass) != 0;
}

void InitDevice(IDirect3D9* pD3D, HWND hWindow, IDirect3DDevice9** ppDevice)
{
  // Reset presentation params
  ZeroMemory(&D3DPresentParams, sizeof(D3DPRESENT_PARAMETERS));

  // Initialize presentation params
  D3DPresentParams.BackBufferCount = 1;
  D3DPresentParams.MultiSampleType = D3DMULTISAMPLE_NONE;
  D3DPresentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
  D3DPresentParams.hDeviceWindow = hWindow;
  D3DPresentParams.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
  D3DPresentParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
  D3DPresentParams.BackBufferFormat = D3DFMT_R5G6B5;
  D3DPresentParams.Windowed = TRUE;

  // Create device
  pD3D->CreateDevice(
    D3DADAPTER_DEFAULT, 
    D3DDEVTYPE_HAL, 
    hWindow, 
    D3DCREATE_HARDWARE_VERTEXPROCESSING, 
    &D3DPresentParams, 
    ppDevice);
}

void MessageHandler()
{
  // Retrieve and translate/dispatch all available messages
  MSG Msg = { 0 };
  while(PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&Msg);
    DispatchMessage(&Msg);

    ZeroMemory(&Msg, sizeof(MSG));
  }
}

void RenderScreen()
{
  // If a second has passed update FPS title bar counter
  if(GetTickCount() - FPSTimer >= 1000)
  {
    // Update title bar
    std::wstring const NewTitle((boost::wformat(L"Hades-GUI Sandbox. "
      L"FPS: %u.") %FramesPerSec).str());
    SetWindowText(SandboxWnd, NewTitle.c_str());

    // Reset FPS count and timer
    FramesPerSec = 0;
    FPSTimer = GetTickCount();
  }

  // Ensure device is in a valid state
  if(!FAILED(g_pDevice->TestCooperativeLevel()) && !bReset)
  {
    // Clear render target
    g_pDevice->Clear(0, 0, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

    // Begin render scene
    g_pDevice->BeginScene();

    // Render GUI if instance exists
    if(gpGui)
    {
      gpGui->Draw();
    }

    // End render scene
    g_pDevice->EndScene();

    // Present buffer
    g_pDevice->Present(0, 0, 0, 0);

    // Increase frame count
    ++FramesPerSec;
  }
}

int main()
{
  // Initialize window class
  if(!InitWindowClass(GetModuleHandle(NULL))) 
  {
    return -1;
  }

  // Create render window
  SandboxWnd = CreateWindow(
    L"Hades-GUI", 
    L"Hades-GUI Sandbox", 
    WS_OVERLAPPED | WS_SYSMENU | WS_VISIBLE | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, 
    0, 
    0, 
    WndWidth, 
    WndHeight, 
    0, 
    0, 
    GetModuleHandle(NULL), 
    0);

  // Ensure render window creation succeeded
  if(!SandboxWnd)
  {
    return -2;
  }

  // Move window to top-left of screen
  RECT wndRect = { 0 };
  GetClientRect(SandboxWnd, &wndRect);
  MoveWindow(SandboxWnd, 0, 0, WndWidth + (WndWidth - wndRect.right), 
    WndHeight + (WndHeight - wndRect.bottom), 1);

  // Create Direct3D
  IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
  if(!pD3D)
  {
    return -4;
  }

  // Initialize device
  InitDevice(pD3D, SandboxWnd, &g_pDevice);

  // Ensure device initialization succeeded
  if(!g_pDevice)
  {
    return -5;
  }

  // Create GUI
  LoadGUI(g_pDevice);

  // Run while 'running' flag is set
  while (g_Running)
  {
    // Quit if escape key is pressed
    if(GetAsyncKeyState(VK_ESCAPE) && GetForegroundWindow() == SandboxWnd)
    {
      break;
    }

    // Call message handler
    MessageHandler();

    // Render GUI
    RenderScreen();
  }

  return 0;
}
