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

#pragma once

#define SAFE_DELETE( pData ) if( pData ){ delete pData; pData = 0; }

#undef SAFE_RELEASE
#define SAFE_RELEASE( pInterface ) if( pInterface ) { pInterface->Release(); pInterface = 0; }

#define TITLEBAR_HEIGHT 24
#define BUTTON_HEIGHT	20
#define HELPERSLIDER_WIDTH 20

#define FCR_NONE	0x0
#define FCR_BOLD 	0x1
#define FCR_ITALICS 0x2

#define FT_NONE		0x0
#define FT_CENTER	0x1
#define FT_BORDER	0x2
#define FT_VCENTER	0x4
#define FT_SINGLELINE 0x8

#define D3DFVF_BITMAPFONT	(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)
#define D3DFVF_PRIMITIVES	(D3DFVF_XYZRHW|D3DFVF_DIFFUSE)

// Forward declarations
class CTexture;
class CTimer;
class CPos;
class CColor;
class CMouse;
class CKeyboard;
class CElement;
class CWindow;
class CButton;
class CCheckBox;
class CProgressBar;
class CTextBox;
class CListBox;

// Windows API
#include <Windows.h>
#include <Shlwapi.h>
#include <WindowsX.h>
#include <atlbase.h>

// C++ Standard Library
#include <map>
#include <set>
#include <limits>
#include <iomanip>
#include <sstream>
#include <vector>
#include <functional>
#include <memory>

// Hades-GUI
#include "D3D9.h"
#include "TinyXML\tinyxml.h"
#include "CTexture.h"
#include "CFont.h"
#include "CTimer.h"
#include "CPos.h"
#include "CColor.h"
#include "CMouse.h"
#include "CKeyboard.h"
#include "CElement.h"
#include "CWindow.h"
#include "CHorizontalSliderBar.h"
#include "CVerticalSliderBar.h"
#include "CHelperSlider.h"
#include "CButton.h"
#include "CCheckBox.h"
#include "CProgressBar.h"
#include "CText.h"
#include "CEditBox.h"
#include "CDropDown.h"
#include "CTextBox.h"
#include "CListBox.h"

typedef std::function<std::string (const char* pszArgs, CElement* pElement)> tCallback;

class CGUI
{
public:

	CGUI( IDirect3DDevice9 * pDevice );
	~CGUI();

	void LoadInterfaceFromFile( const char * pszFilePath );

	void FillArea( int iX, int iY, int iWidth, int iHeight, D3DCOLOR d3dColor );
	void DrawLine( int iStartX, int iStartY, int iEndX, int iEndY, int iWidth, D3DCOLOR d3dColor );
	void DrawOutlinedBox( int iX, int iY, int iWidth, int iHeight, D3DCOLOR d3dInnerColor, D3DCOLOR d3dBorderColor );

	CWindow * AddWindow( CWindow * pWindow );
	void BringToTop( CWindow * pWindow );

	void Draw();
	void PreDraw();
	void MouseMove( CMouse & pMouse );
	bool KeyEvent( SKey sKey );

	void OnLostDevice();
	void OnResetDevice( IDirect3DDevice9 * pDevice );

	CMouse & GetMouse() const;
	CKeyboard * GetKeyboard() const;

	IDirect3DDevice9 * GetDevice() const;
	CFont * GetFont() const;
	ID3DXSprite * GetSprite() const;

	CWindow * GetWindowByString( std::string sString, int iIndex = 0 );

	SElement * GetThemeElement( std::string sElement ) const;

	void SetVisible( bool bVisible );
	bool IsVisible() const;

	bool ShouldReload() const;
	void Reload();

	tCallback const & GetCallback( std::string sString ) const
	{
		return m_mCallbacks.find( sString )->second;
	}
	void AddCallback( std::string sString, tCallback pCallback )
	{
		m_mCallbacks[ sString ] = pCallback;
	}
	std::map<std::string,tCallback> const & GetCallbackMap() const
	{
		return m_mCallbacks;
	}

private:
  bool m_bVisible, m_bReload;

  std::shared_ptr<CMouse> m_pMouse;
  std::shared_ptr<CKeyboard> m_pKeyboard;
  std::shared_ptr<CFont> m_pFont;

  IDirect3DDevice9 * m_pDevice;

  CComPtr<ID3DXSprite> m_pSprite;

  CComPtr<ID3DXLine> m_pLine;

  CTimer m_tPreDrawTimer;

  std::vector<CWindow*> m_vWindows;

  std::string m_sCurTheme;

  typedef std::map<std::string, SElement*> tTheme;
  std::map<std::string, tTheme> m_mThemes;

  std::map<std::string, tCallback> m_mCallbacks;
};
