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

#include "GUI.h"

namespace Hades
{
  namespace GUI
  {
    void Element::SetElement(TiXmlElement * pElement)
    {
      SetMouseOver(false);

      if (!pElement)
        return;

      int iTempX = 0, iTempY = 0;

      if (pElement->QueryIntAttribute("relX", &iTempX) == TIXML_NO_ATTRIBUTE)
        iTempX = 0;
      if (pElement->QueryIntAttribute("relY", &iTempY) == TIXML_NO_ATTRIBUTE)
        iTempY = 0;

      SetRelPos(Pos(iTempX, iTempY));

      if (pElement->QueryIntAttribute("absX", &iTempX) == TIXML_NO_ATTRIBUTE)
        iTempX = 0;
      if (pElement->QueryIntAttribute("absY", &iTempY) == TIXML_NO_ATTRIBUTE)
        iTempY = 0;

      SetAbsPos(Pos(iTempX, iTempY));

      if (pElement->QueryIntAttribute("width", &iTempX) == TIXML_NO_ATTRIBUTE)
        iTempX = 0;
      if (pElement->QueryIntAttribute("height", &iTempY) == TIXML_NO_ATTRIBUTE)
        iTempY = 0;

      SetWidth(iTempX);
      SetHeight(iTempY);

      const char * pszString = pElement->Attribute("string");
      if (pszString)
        SetString(pszString);

      pszString = pElement->Attribute("string2");
      if (pszString)
        SetString(pszString, 1);

      const char * pszCallback = pElement->Attribute("callback");
      if (pszCallback)
      {
        SetCallback(m_Gui.GetCallback(pszCallback));

        if (!GetCallback())
          MessageBoxA(0, "Callback invalid", pszCallback, 0);
      }
      else
        SetCallback(0);

      SetMouseOver(false);
    }

    void Element::SetParent(Window * pParent)
    {
      m_pParent = pParent;
    }

    Window * Element::GetParent() const
    {
      return m_pParent;
    }

    void Element::SetCallback(Callback pCallback)
    {
      m_pCallback = pCallback;
    }

    Callback Element::GetCallback() const
    {
      return m_pCallback;
    }

    void Element::SetRelPos(Pos relPos)
    {
      m_relPos = relPos;
    }

    Pos Element::GetRelPos() const
    {
      return m_relPos;
    }

    void Element::SetAbsPos(Pos absPos)
    {
      m_absPos = absPos;
    }

    Pos Element::GetAbsPos() const
    {
      return m_absPos;
    }

    void Element::SetWidth(int iWidth)
    {
      m_iWidth = iWidth;
    }

    int Element::GetWidth() const
    {
      return m_iWidth;
    }

    void Element::SetHeight(int iHeight)
    {
      m_iHeight = iHeight;
    }

    int Element::GetHeight() const
    {
      return m_iHeight;
    }

    bool Element::HasFocus() const
    {
      return GetParent()->GetFocussedElement() == this;
    }

    void Element::SetString(std::string sString, int iIndex)
    {
      if (static_cast<int>(sString.length()) > 255)
        return;

      m_sRaw[ iIndex ] = sString;
    }

    std::string Element::GetString(bool bReplaceVars, int iIndex)
    {
      std::string & sFormatted = m_sFormatted[ iIndex ] = m_sRaw[ iIndex ];

      if (bReplaceVars)
      {
        std::string::size_type sPos = 0;
        while((sPos = sFormatted.find("$", sPos)) != std::string::npos)
        {
          for(std::map<std::string,Callback>::const_reverse_iterator iIter = m_Gui.GetCallbackMap().rbegin(); iIter != m_Gui.GetCallbackMap().rend(); iIter++)
          {
            const std::string & sName = iIter->first;

            if (!sFormatted.compare(sPos, sName.length(), sName))
            {
              sFormatted = sFormatted.replace(sPos, sName.length(), iIter->second(0, this));
              break;
            }
          }
          sPos++;
        }

        return sFormatted;
      }

      return m_sRaw[ iIndex ];
    }

    std::string Element::GetFormatted(int iIndex) const
    {
      return m_sFormatted[ iIndex ];
    }

    bool Element::GetMouseOver() const
    {
      return m_bMouseOver;
    }

    bool Element::SetMouseOver(bool bMouseOver)
    {
      return m_bMouseOver = bMouseOver;
    }

    SElement * Element::SetThemeElement(SElement * pThemeElement, int iIndex)
    {
      return m_pThemeElement[ iIndex ] = pThemeElement;
    }

    SElement * Element::GetThemeElement(int iIndex) const
    {
      return m_pThemeElement[ iIndex ];
    }

    void Element::SetElementState(std::string sState, int iIndex)
    {
      m_pElementState[ iIndex ] = GetThemeElement(iIndex)->m_States[ sState ];

      if (!m_pElementState)
        m_pElementState[ iIndex ] = GetThemeElement(iIndex)->m_States[ GetThemeElement(iIndex)->sDefaultState ];

      UpdateTheme(iIndex);
    }

    SElementState * Element::GetElementState(int iIndex) const
    {
      return m_pElementState[ iIndex ];
    }

    void Element::UpdateTheme(int)
    {
    }

    void Element::Draw()
    {
    }

    void Element::PreDraw()
    {
    }

    void Element::MouseMove(Mouse &)
    {
    }

    bool Element::KeyEvent(Key)
    {
      return true;
    }

    Element::Element(GUI& Gui) 
      : m_Gui(Gui)
    {
    }
  }
}
