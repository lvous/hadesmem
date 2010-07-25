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

#include "GUI.h"

namespace Hades
{
  namespace GUI
  {
    class Window : public Element
    {
    public:
      Window(class GUI& Gui, TiXmlElement* pElement);
      ~Window();

      void AddElement(Element * pElement);

      void Draw();
      void PreDraw();
      void MouseMove(Mouse & pMouse);
      bool KeyEvent(Key Key);

      void SetMaximized(bool bMaximized);
      bool GetMaximized();

      void SetVisible(bool bVisible);
      bool IsVisible();

      void SetDragging(bool bDragging);
      bool GetDragging();

      void SetCloseButton(bool bEnabled);
      bool GetCloseButton();

      void SetFocussedElement(Element * pElement);
      Element * GetFocussedElement();

      Element * GetElementByString(const char * pszString, int iIndex = 0);

      void BringToTop(Element * pElement);

      void UpdateTheme(int iIndex);

    private:
      template <typename T>
      void LoadElement(std::string const& Name)
      {
        for(TiXmlElement* pElement = pElement->FirstChildElement(Name); 
          pElement; pElement = pElement->NextSiblingElement(Name))
        {
          AddElement(new T(m_Gui, pElement));
        }
      }

      bool m_bMaximized, m_bDragging, m_bVisible;
      Pos posDif;
      std::vector<Element*> m_vElements;
      Element * m_pFocussedElement;

      bool m_bCloseButtonEnabled;
      Timer m_tCloseButtonPressed;

      Colour* m_pTitle;
      Colour* m_pBodyInner;
      Colour* m_pBodyBorder;
      Texture* m_pTitlebar;
      Texture* m_pButton;
    };
  }
}
