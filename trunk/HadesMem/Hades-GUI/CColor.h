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

// C++ Standard Library
#include <map>

// TinyXML
#include "TinyXML\TinyXML.h"

namespace Hades
{
  namespace GUI
  {
    class CColor
    {
    public:
      CColor();
      CColor(int Red, int Green, int Blue, int Alpha);
      CColor(D3DCOLOR D3DColour);
      CColor(TiXmlElement* pElement);

      void SetD3DColor(D3DCOLOR D3DColour);

      void SetRed(int Red);
      void SetGreen(int Green);
      void SetBlue(int Blue);
      void SetAlpha(int Alpha);

      D3DCOLOR GetD3DColor() const;

      int GetRed() const;
      int GetGreen() const;
      int GetBlue() const;
      int GetAlpha() const;

      const CColor operator / (const int Divisor) const;
      const CColor operator * (const int Multiplicator) const;

      const CColor operator - (CColor const& SubColor) const;
      const CColor operator + (CColor const& AddColor) const;

    private:
      D3DCOLOR m_D3DColour;
    };

    struct SElement;

    struct SElementState
    {
      SElement* pParent;

      CColor* GetColor(std::string const& Name) const;
      CTexture* GetTexture(std::string const& Name) const;

      std::map<std::string, CColor*> m_Colours;
      std::map<std::string, CTexture*> m_Textures;
    };

    struct SElement
    {
      std::string sDefaultState;
      std::map<std::string, SElementState*> m_States;
    };
  }
}
