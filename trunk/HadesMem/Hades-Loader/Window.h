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
#include <memory>

// Windows
#include <Windows.h>
#include <atlbase.h>
#include <atlwin.h>

// WTL
#include <atlapp.h>
#include <atlddx.h>
#include <atluser.h>
#include <atlmisc.h>
#include <atlframe.h>
#include <atlcrack.h>
#include <atlctrls.h>
#include <atlsplit.h>
#include <atlctrlx.h>

// Hades
#include "Game.h"
#include "Resource.h"

namespace Hades
{
  namespace Loader
  {
    template <typename T>
    class CFrameWindowCustom : 
      public CFrameWindowImpl<T, CWindow, 
      CWinTraits<WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 
      WS_EX_APPWINDOW | WS_EX_WINDOWEDGE>>
    { };

    class LoaderWindow : public CFrameWindowCustom<LoaderWindow>
    {
    public:
      // Specify window class name
      DECLARE_FRAME_WND_CLASS(L"HadesLoaderWndClass", IDR_LOADERWINDOW)

      // WM_DESTROY message callback
      LRESULT OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam, 
        BOOL& bHandled);

      // WM_CREATE message callback
      LRESULT OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, 
        BOOL& bHandled);

      // WM_COMMAND message callback
      LRESULT OnCommand(UINT nMsg, WPARAM wParam, LPARAM lParam, 
        BOOL& bHandled);

      LRESULT OnFileExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, 
        BOOL& bHandled);

      LRESULT OnHelpAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, 
        BOOL& bHandled);

      LRESULT OnTVSelChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

      BEGIN_MSG_MAP(LoaderWindow)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        COMMAND_ID_HANDLER(ID_HELP_ABOUT, OnHelpAbout)
        COMMAND_ID_HANDLER(ID_FILE_EXIT, OnFileExit)
        MESSAGE_HANDLER(WM_COMMAND, OnCommand)
        NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnTVSelChanged)
        CHAIN_MSG_MAP(CFrameWindowCustom<LoaderWindow>)
      END_MSG_MAP()

      HWND CreateClient();

    private:
      GameMgr m_GameMgr;

      CSplitterWindow m_Splitter;

      CPaneContainer m_LeftPane;
      CPaneContainer m_RightPane;

      CTreeViewCtrlEx m_GameTree;
    };
  }
}
