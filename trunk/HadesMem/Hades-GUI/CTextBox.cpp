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

#include "CGUI.h"

#pragma warning(push, 1)
#include <boost/thread/locks.hpp>
#pragma warning(pop)

namespace Hades
{
  namespace GUI
  {
    CTextBox::CTextBox(CGUI& Gui, TiXmlElement* pElement)
      : CElement(Gui)
    {
      SetElement(pElement);

      pSlider = new CHelperSlider(Gui, CPos(GetWidth() - HELPERSLIDER_WIDTH, 0), GetHeight());

      for(TiXmlElement* pString = pElement->FirstChildElement("Row"); pString; 
        pString = pString->NextSiblingElement("Row"))
      {
        AddString(pString->GetText());
      }

      SetThemeElement(m_Gui.GetThemeElement("TextBox"));

      if (!GetThemeElement())
      {
        MessageBoxA(0, "Theme element invalid.", "TextBox", 0);
      }
      else
      {
        SetElementState("Norm");
      }
    }

    void CTextBox::Draw()
    {
      boost::lock_guard<boost::recursive_mutex> MyLock(m_Mutex);

      CPos Pos = *GetParent()->GetAbsPos() + *GetRelPos();

      m_Gui.DrawOutlinedBox(Pos.GetX(), Pos.GetY(), GetWidth(), GetHeight(), 
        pInner->GetD3DColor(), pBorder->GetD3DColor());

      int iAddHeight = m_Gui.GetFont()->GetStringHeight();
      if (m_vStrings.size())
      {
        for(int i = pSlider->GetValue(), iHeight = 0; i <= 
          pSlider->GetMaxValue() && iHeight < GetHeight() - 
          m_Gui.GetFont()->GetStringHeight(); i++)
        {
          m_Gui.GetFont()->DrawString(Pos.GetX() + 3, Pos.GetY() + iHeight, 0, 
            pString, m_vStrings[ i ], GetWidth() - HELPERSLIDER_WIDTH);
          iHeight += iAddHeight;
        }
      }

      pSlider->Draw(Pos);
    }

    void CTextBox::PreDraw()
    {
      pSlider->PreDraw();
    }

    void CTextBox::MouseMove(CMouse & pMouse)
    {
      CPos Pos = *GetParent()->GetAbsPos() + *GetRelPos();

      SetMouseOver(pMouse.InArea(Pos.GetX(), Pos.GetY(), GetWidth(), 
        GetHeight()));

      pSlider->MouseMove(Pos, pMouse);
    }

    bool CTextBox::KeyEvent(SKey sKey)
    {
      CPos Pos = *GetParent()->GetAbsPos() + *GetRelPos();

      if (GetMouseOver() || (!sKey.m_Down && !m_Gui.GetMouse().GetWheel()))
      {
        return pSlider->KeyEvent(Pos, sKey);
      }

      return true;
    }

    void CTextBox::AddString(std::string MyString)
    {
      boost::lock_guard<boost::recursive_mutex> MyLock(m_Mutex);

      if (MyString.empty())
      {
        return;
      }

      std::vector<std::string> Pending;
      std::size_t StringLen = MyString.size();
      for (std::size_t i = StringLen; i && i--; )
      {
        if (MyString[i] == '\n')
        {
          MyString[i] = '\0';

          if (i + 1 < MyString.size())
          {
            if (MyString[i + 1] == '\r')
            {
              if (i + 2 < StringLen)
              {
                Pending.push_back(&MyString.c_str()[i + 2]);
              }
            }
            else
            {
              Pending.push_back(&MyString.c_str()[i + 1]);
            }
          }
        }
      }

      pSlider->SetMaxValue(m_vStrings.size());

      m_vStrings.push_back(MyString.c_str());

      int iHeight = 0;
      for(int i = pSlider->GetValue(); i <= pSlider->GetMaxValue(); i++)
      {
        float fWidth = static_cast<float>(m_Gui.GetFont()->GetStringWidth(
          m_vStrings[i]));
        int iLines = static_cast<int>(ceilf(fWidth / (GetWidth() - 
          HELPERSLIDER_WIDTH)));

        int iTempHeight = iLines * m_Gui.GetFont()->GetStringHeight();
        iHeight += iTempHeight;

        while(iHeight > GetHeight() - m_Gui.GetFont()->GetStringHeight())
        {
          pSlider->SetValue(pSlider->GetValue() + iLines);
          iHeight -= iTempHeight;
        }
      }

      std::for_each(Pending.rbegin(), Pending.rend(), 
        [this] (std::string const& Current)
      {
        AddString(Current);
      });
    }

    void CTextBox::Clear()
    {
      boost::lock_guard<boost::recursive_mutex> MyLock(m_Mutex);

      m_vStrings.clear();
    }

    void CTextBox::UpdateTheme(int iIndex)
    {
      SElementState * pState = GetElementState(iIndex);

      pString = pState->GetColor("String");
      pInner = pState->GetColor("Inner");
      pBorder = pState->GetColor("Border");
    }
  }
}
