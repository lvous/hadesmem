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

    LRESULT LoaderWindow::OnHelpAbout(WORD /*wNotifyCode*/, WORD /*wID*/, 
      HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
      AboutDialog MyAboutDialog;
      MyAboutDialog.DoModal();
      return 0;
    }
  }
}
