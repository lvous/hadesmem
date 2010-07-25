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

#include "Fwd.h"
#include "Pos.h"

namespace Hades
{
  namespace GUI
  {
    class Element
    {
      bool m_bHasFocus, m_bMouseOver;

      std::string m_sRaw[2], m_sFormatted[2];
      int m_iWidth, m_iHeight;

      Pos m_relPos, m_absPos;
      Window * m_pParent;

      Callback m_pCallback;

      SElement * m_pThemeElement[ 3 ];
      SElementState * m_pElementState[ 3 ];

    protected:
      class GUI& m_Gui;

    public:
      Element(class GUI& Gui);
      virtual ~Element() { }

      void SetElement(TiXmlElement * pElement);

      void SetParent(Window * pParent);
      Window * GetParent() const;

      void SetCallback(Callback pCallback);
      Callback GetCallback() const;

      void SetRelPos(Pos relPos);
      void SetAbsPos(Pos absPos);

      Pos GetRelPos() const;
      Pos GetAbsPos() const;

      void SetWidth(int iWidth);
      void SetHeight(int iHeight);

      int GetWidth() const;
      int GetHeight() const;

      bool HasFocus() const;

      void SetString(std::string sString, int iIndex = 0);
      std::string GetString(bool bReplaceVars = false, int iIndex = 0);
      std::string GetFormatted(int iIndex = 0) const;

      bool GetMouseOver() const;
      bool SetMouseOver(bool bMouseOver);

      SElement * SetThemeElement(SElement * pThemeElement, int iIndex = 0);
      SElement * GetThemeElement(int iIndex = 0) const;

      void SetElementState(std::string sState, int iIndex = 0);
      SElementState * GetElementState(int iIndex = 0) const;

      virtual void UpdateTheme(int iIndex);

      virtual void Draw();
      virtual void PreDraw();
      virtual void MouseMove(Mouse & pMouse);
      virtual bool KeyEvent(Key Key);
    };
  }
}
