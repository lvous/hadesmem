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

#include "CGUI.h"

class CHelperSlider : public CHorizontalSliderBar
{
	bool m_bMouseOver[ 3 ], m_bPressed[ 2 ], m_bDragged;
	CTimer m_tUpArrow, m_tDownArrow;

	SElement * m_pThemeElement[ 3 ];
	SElementState * m_pElementState[ 3 ];

	void SetDragged( bool bDragged );
	bool GetDragged();

	CColor * pInner, * pBorder;
	CTexture * pSlider, * pUpArrow, * pDownArrow;

public:
	CHelperSlider( CPos relPos, int iHeight );

	void Draw( CPos basePos );
	void PreDraw();
	void MouseMove( CPos basePos, CMouse & pMouse );
	bool KeyEvent( CPos basePos, SKey sKey );

	void UpdateTheme( int iIndex );
};