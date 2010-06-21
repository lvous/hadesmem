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

namespace Hades
{
  namespace GUI
  {
    CColor::CColor() 
      : m_D3DColour(0)
    { }

    CColor::CColor(int Red, int Green, int Blue, int Alpha)
      : m_D3DColour(0)
    {
      SetD3DColor(D3DCOLOR_RGBA(Red, Green, Blue, Alpha));
    }

    CColor::CColor(D3DCOLOR D3DColour)
      : m_D3DColour(0)
    {
      SetD3DColor(D3DColour);
    }

    CColor::CColor(TiXmlElement* pElement)
      : m_D3DColour(0)
    {
      int Colours[4] = { 0 };

      pElement->QueryIntAttribute("r", &Colours[0]);
      pElement->QueryIntAttribute("g", &Colours[1]);
      pElement->QueryIntAttribute("b", &Colours[2]);
      pElement->QueryIntAttribute("a", &Colours[3]);

      SetD3DColor(D3DCOLOR_RGBA(Colours[0], Colours[1], Colours[2], 
        Colours[3]));
    }

    const CColor CColor::operator / (const int Divisor) const
    {
      return CColor(GetRed() / Divisor, GetGreen() / Divisor, 
        GetBlue() / Divisor, GetAlpha());
    }

    const CColor CColor::operator - (CColor const& SubColor) const
    {
      return CColor(GetRed() - SubColor.GetRed(), 
        GetGreen() - SubColor.GetGreen(), GetBlue() - SubColor.GetBlue(), 
        GetAlpha());
    }

    const CColor CColor::operator + (CColor const& AddColor) const
    {
      return CColor(GetRed() + AddColor.GetRed(), 
        GetGreen() + AddColor.GetGreen(), GetBlue() + AddColor.GetBlue(), 
        GetAlpha());
    }

    const CColor CColor::operator * (const int Multiplicator) const
    {
      return CColor(GetRed() * Multiplicator, GetGreen() * Multiplicator, 
        GetBlue() * Multiplicator, GetAlpha());
    }

    void CColor::SetD3DColor(D3DCOLOR D3DColour)
    {
      m_D3DColour = D3DColour;
    }

    void CColor::SetRed(int Red)
    {
      SetD3DColor(D3DCOLOR_RGBA(Red, GetGreen(), GetBlue(), GetAlpha()));
    }

    void CColor::SetGreen(int Green)
    {
      SetD3DColor(D3DCOLOR_RGBA(GetRed(), Green, GetBlue(), GetAlpha()));
    }

    void CColor::SetBlue(int Blue)
    {
      SetD3DColor(D3DCOLOR_RGBA(GetRed(), GetGreen(), Blue, GetAlpha()));
    }

    void CColor::SetAlpha(int Alpha)
    {
      SetD3DColor(D3DCOLOR_RGBA(GetRed(), GetGreen(), GetBlue(), Alpha));
    }

    D3DCOLOR CColor::GetD3DColor() const
    {
      return m_D3DColour;
    }

    int CColor::GetRed() const
    {
      return (GetD3DColor() >> 16) & 0xFF;
    }

    int CColor::GetGreen() const
    {
      return (GetD3DColor() >> 8) & 0xFF;
    }

    int CColor::GetBlue() const
    {
      return GetD3DColor() & 0xFF;
    }

    int CColor::GetAlpha() const
    {
      return GetD3DColor() >> 24;
    }

    CColor* SElementState::GetColor(std::string const& Name) const
    {
      auto Iter = m_Colours.find(Name);

      if (Iter == m_Colours.end())
      {
        Iter = pParent->m_States.find(pParent->sDefaultState)->second->
          m_Colours.find(Name);

        if (Iter == pParent->m_States.find(pParent->sDefaultState)->second->
          m_Colours.end())
        {
          MessageBoxA(0, "Color not found.", Name.c_str(), 0);
        }
      }

      return Iter->second;
    }

    CTexture * SElementState::GetTexture(std::string const& Name) const
    {
      auto Iter = m_Textures.find(Name);

      if (Iter == m_Textures.end())
      {
        Iter = pParent->m_States.find(pParent->sDefaultState)->second->
          m_Textures.find(Name);

        if (Iter == pParent->m_States.find(pParent->sDefaultState)->second->
          m_Textures.end())
        {
          MessageBoxA(0, "Texture not found.", Name.c_str(), 0);
        }
      }

      return Iter->second;
    }
  }
}
