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

// C++ Standard Library
#include <iterator>
#include <algorithm>

// Hades
#include "CGUI.h"
#include "Error.h"
#include "CFont.h"
#include "CColor.h"

namespace Hades
{
  namespace GUI
  {
    CFont::CFont(CGUI& Gui, IDirect3DDevice9* pDevice, int Height, 
      std::string const& FaceName)
      : m_Gui(Gui), 
      m_pFont()
    {
      HRESULT Result = D3DXCreateFontA(
        pDevice, 
        -MulDiv(Height, GetDeviceCaps(GetDC(0), LOGPIXELSY), 72), 
        0, 
        FW_NORMAL, 
        0, 
        0, 
        DEFAULT_CHARSET, 
        OUT_DEFAULT_PRECIS, 
        DEFAULT_QUALITY, 
        DEFAULT_PITCH | FF_DONTCARE, 
        FaceName.c_str(), 
        &m_pFont);

      if (FAILED(Result))
      {
        BOOST_THROW_EXCEPTION(HadesGuiError() << 
          ErrorFunction("CFont::CFont") << 
          ErrorString("Could not create font.") << 
          ErrorCodeWin(Result));
      }

      m_pFont->PreloadCharacters(0, 255);
    }

    void CFont::OnLostDevice()
    {
      m_pFont->OnLostDevice();
    }

    void CFont::OnResetDevice(IDirect3DDevice9 * /*pDevice*/)
    {
      m_pFont->OnResetDevice();
    }

    void CFont::DrawString(int X, int Y, DWORD Flags, CColor* pColor, 
      std::string const& MyString, int /*Width*/)
    {
      m_Gui.GetSprite()->Begin(D3DXSPRITE_ALPHABLEND | 
        D3DXSPRITE_SORT_TEXTURE);

      D3DXMATRIX Matrix;
      D3DXMatrixTranslation(&Matrix, static_cast<float>(X), 
        static_cast<float>(Y), 0);
      m_Gui.GetSprite()->SetTransform(&Matrix);

      RECT DrawRect = { 0 };
      DWORD DrawFlags = DT_NOCLIP | ((Flags & FT_CENTER) ? DT_CENTER : 0) | 
        ((Flags & FT_VCENTER) ? DT_VCENTER : 0);
      m_pFont->DrawTextA(m_Gui.GetSprite(), MyString.c_str(), -1, &DrawRect, 
        DrawFlags, pColor->GetD3DColor());

      m_Gui.GetSprite()->End();
    }

    int CFont::GetStringWidth(std::string const& MyString) const
    {
      std::string NewString;
      NewString.reserve(MyString.size());
      std::transform(MyString.begin(), MyString.end(), 
        std::back_inserter(NewString), 
        [] (char Current)
      {
        return (Current == ' ') ? '.' : Current;
      });

      RECT MyRect = { 0 };
      m_pFont->DrawTextA(0, NewString.c_str(), -1, &MyRect, DT_CALCRECT, 0);

      return MyRect.right - MyRect.left;
    }

    int CFont::GetStringHeight() const
    {
      RECT rRect = { 0 };
      m_pFont->DrawTextA(0, "Y", -1, &rRect, DT_CALCRECT, 0);

      return rRect.bottom - rRect.top;
    }

    void CFont::CutString(int MaxWidth, std::string& MyString) const
    {
      int Index = 0;
      std::size_t Length = MyString.size();

      for(int Width = 0; Index < Length && Width + 10 < MaxWidth; )
      {
        char Current[2] = { MyString.c_str()[Index], 0 };
        Width += m_Gui.GetFont()->GetStringWidth(Current);
        ++Index;
      }

      if (Index < Length)
      {
        MyString[Index - 1] = '\0';
      }
    }
  }
}
