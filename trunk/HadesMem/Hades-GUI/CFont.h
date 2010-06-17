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

#include <stdio.h>
#include <string>
#include "D3D9.h"
#include "CD3DRender.h"
#include "CColor.h"

class CFont
{
#ifdef USE_D3DX
	ID3DXFont * m_pFont;
#else
	CD3DFont * m_pFont;
#endif

  class CGUI& m_Gui;

public:
	CFont(class CGUI& Gui, IDirect3DDevice9 * pDevice, int iHeight, const char * pszFaceName );
	~CFont();

	void OnLostDevice();
	void OnResetDevice( IDirect3DDevice9 * pDevice );

	void DrawString( int iX, int iY, DWORD dwFlags, CColor * pColor, std::string sString, int iWidth = 0 );

	int GetStringWidth( const char * pszString ) const;
	int GetStringHeight() const;

	void CutString( int iWidth, std::string & rString ) const;
};