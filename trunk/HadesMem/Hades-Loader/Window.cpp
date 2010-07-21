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
#include "About.h"
#include "Window.h"
#include "Resource.h"
#include "Hades-Common/Error.h"

namespace Hades
{
  namespace Loader
  {
    // WM_DESTROY message callback
    LRESULT LoaderWindow::OnDestroy( UINT /*nMsg*/, WPARAM /*wParam*/, 
      LPARAM /*lParam*/, BOOL& /*bHandled*/ )
    {
      PostQuitMessage(0);
      return 0;
    }

    // WM_COMMAND message callback
    LRESULT LoaderWindow::OnCommand(UINT /*nMsg*/, WPARAM wParam, 
      LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
      try
      {
        // Get message ID
        WORD MsgId = LOWORD(wParam);

        // Get game data for message id
        auto pGameData(m_GameMgr.GetDataForMessageId(MsgId));

        // If entry existed attempt to launch
        if (pGameData)
        {
          // Launch game
          m_GameMgr.LaunchGame(*pGameData);
        }
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

    // WM_CREATE message callback
    LRESULT LoaderWindow::OnCreate(UINT /*nMsg*/, WPARAM /*wParam*/, 
      LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
      try
      {
        // Resize window
        if (!ResizeClient(1280, 720))
        {
          DWORD LastError = GetLastError();
          BOOST_THROW_EXCEPTION(HadesError() << 
            ErrorFunction("LoaderWindow::OnCreate") << 
            ErrorString("Could not resize loader window.") << 
            ErrorCodeWin(LastError));
        }

        // Center window
        if (!CenterWindow())
        {
          DWORD LastError = GetLastError();
          BOOST_THROW_EXCEPTION(HadesError() << 
            ErrorFunction("LoaderWindow::OnCreate") << 
            ErrorString("Could not center loader window.") << 
            ErrorCodeWin(LastError));
        }

        m_hWndClient = CreateClient();

        // Get menu for window
        CMenuHandle MainMenu(GetMenu());
        if (MainMenu.IsNull())
        {
          DWORD LastError = GetLastError();
          BOOST_THROW_EXCEPTION(HadesError() << 
            ErrorFunction("LoaderWindow::OnCreate") << 
            ErrorString("Could not get main menu.") << 
            ErrorCodeWin(LastError));
        }

        // Get game menu
        CMenuHandle GameMenu(MainMenu.GetSubMenu(1));
        if (MainMenu.IsNull())
        {
          DWORD LastError = GetLastError();
          BOOST_THROW_EXCEPTION(HadesError() << 
            ErrorFunction("LoaderWindow::OnCreate") << 
            ErrorString("Could not get game menu.") << 
            ErrorCodeWin(LastError));
        }

        // Load game list
        m_GameMgr.LoadConfigDefault();

        // Get game list
        auto GameList(m_GameMgr.GetAllData());

        // Add all games to menu
        std::for_each(GameList.begin(), GameList.end(), 
          [&] (std::shared_ptr<GameMgr::MenuData> Current)
        {
          if (!GameMenu.AppendMenuW(MF_STRING, Current->MessageId, 
            Current->Name.c_str()))
          {
            DWORD LastError = GetLastError();
            BOOST_THROW_EXCEPTION(HadesError() << 
              ErrorFunction("LoaderWindow::OnCreate") << 
              ErrorString("Could not append game to game menu.") << 
              ErrorCodeWin(LastError));
          }
        });
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

    // ID_FILE_EXIT command callback
    LRESULT LoaderWindow::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, 
      HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
      try
      {
        // Send close request
        if (!PostMessage(WM_CLOSE, 0, 0))
        {
          DWORD LastError = GetLastError();
          BOOST_THROW_EXCEPTION(HadesError() << 
            ErrorFunction("LoaderWindow::OnFileExit") << 
            ErrorString("Could not send close message.") << 
            ErrorCodeWin(LastError));
        }
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

    // ID_HELP_ABOUT command callback
    LRESULT LoaderWindow::OnHelpAbout(WORD /*wNotifyCode*/, WORD /*wID*/, 
      HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
      // Create about dialog
      AboutDialog MyAboutDialog;
      // Show about dialog (modal)
      MyAboutDialog.DoModal();

      return 0;
    }

    // Create client area of window
    HWND LoaderWindow::CreateClient()
    {
      // Get client rect
      CRect ClientRect;
      if (!GetClientRect(&ClientRect))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(HadesError() << 
          ErrorFunction("LoaderWindow::CreateClient") << 
          ErrorString("Could not get client rect.") << 
          ErrorCodeWin(LastError));
      }

      // Create vertical splitter
      if (!m_Splitter.Create(m_hWnd, ClientRect, NULL, WS_CHILD | WS_VISIBLE | 
        WS_CLIPSIBLINGS | WS_CLIPCHILDREN))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(HadesError() << 
          ErrorFunction("LoaderWindow::CreateClient") << 
          ErrorString("Could not get create splitter.") << 
          ErrorCodeWin(LastError));
      }

      // Set splitter pos
      if (!m_Splitter.SetSplitterPos(200))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(HadesError() << 
          ErrorFunction("LoaderWindow::CreateClient") << 
          ErrorString("Could not set splitter pos.") << 
          ErrorCodeWin(LastError));
      }

      // Disable resizing of splitter
      m_Splitter.SetSplitterExtendedStyle(SPLIT_NONINTERACTIVE);

      // Create left pane
      if (!m_LeftPane.Create(m_Splitter.m_hWnd))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(HadesError() << 
          ErrorFunction("LoaderWindow::CreateClient") << 
          ErrorString("Could not create left pane.") << 
          ErrorCodeWin(LastError));
      }

      // Set left pane title
      if (!m_LeftPane.SetTitle(L"Navigation"))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(HadesError() << 
          ErrorFunction("LoaderWindow::CreateClient") << 
          ErrorString("Could not set left pane title.") << 
          ErrorCodeWin(LastError));
      }

      // Disable left pane close button
      m_LeftPane.SetPaneContainerExtendedStyle(PANECNT_NOCLOSEBUTTON);

      // Attach left pane to left of splitter
      if (!m_Splitter.SetSplitterPane(0, m_LeftPane))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(HadesError() << 
          ErrorFunction("LoaderWindow::CreateClient") << 
          ErrorString("Could not attach left pane to splitter.") << 
          ErrorCodeWin(LastError));
      }

      // Create right pane
      if (!m_RightPane.Create(m_Splitter.m_hWnd))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(HadesError() << 
          ErrorFunction("LoaderWindow::CreateClient") << 
          ErrorString("Could not create right pane.") << 
          ErrorCodeWin(LastError));
      }

      // Set right pane title
      if (!m_RightPane.SetTitle(L"Test"))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(HadesError() << 
          ErrorFunction("LoaderWindow::CreateClient") << 
          ErrorString("Could not set right pane title.") << 
          ErrorCodeWin(LastError));
      }

      // Disable right pane close button
      m_RightPane.SetPaneContainerExtendedStyle(PANECNT_NOCLOSEBUTTON);


      // Attach right pane to right of splitter
      if (!m_Splitter.SetSplitterPane(1, m_RightPane))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(HadesError() << 
          ErrorFunction("LoaderWindow::CreateClient") << 
          ErrorString("Could not attach right pane to splitter.") << 
          ErrorCodeWin(LastError));
      }

      // Create navigation tree
      if (!m_NavTree.Create(m_LeftPane.m_hWnd, rcDefault, NULL, WS_CHILD | 
        WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE))
      {
        DWORD LastError = GetLastError();
        BOOST_THROW_EXCEPTION(HadesError() << 
          ErrorFunction("LoaderWindow::CreateClient") << 
          ErrorString("Could not create nav tree.") << 
          ErrorCodeWin(LastError));
      }

      // Set up navigation tree
      CTreeItem HadesNode(m_NavTree.InsertItem(L"Hades", TVI_ROOT, TVI_LAST));
      m_NavTreeMap[HadesNode.AddHead(L"Foo", 0)] = CreateFooWindow();
      m_NavTreeMap[HadesNode.AddHead(L"Bar", 0)] = CreateBarWindow();

      // Expand hades node
      m_NavTree.Expand(HadesNode);

      // Set left pane client as navigation tree
      m_LeftPane.SetClient(m_NavTree.m_hWnd);

      // Return splitter handle as client
      return m_Splitter.m_hWnd;
    }
    
    // TVN_SELCHANGED notification callback
    LRESULT LoaderWindow::OnTVSelChanged(int /*idCtrl*/, LPNMHDR pnmh, 
      BOOL& /*bHandled*/)
    {
      try
      {
        // Handle messages for nav tree
        if (pnmh->hwndFrom == m_NavTree)
        {
          // Get tree view data
          auto lpTV = reinterpret_cast<LPNMTREEVIEW>(pnmh);

          // Get selection
          CTreeItem Selection(lpTV->itemNew.hItem, &m_NavTree);

          // Set window from nav tree map
          auto Iter = m_NavTreeMap.find(Selection);
          if (Iter != m_NavTreeMap.end())
          {
            m_RightPane.SetClient(Iter->second);
          }
        }
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
  }
}
