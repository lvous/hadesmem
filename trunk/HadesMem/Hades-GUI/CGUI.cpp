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
    // Constructor
    CGUI::CGUI(IDirect3DDevice9* pDevice) 
      : m_bVisible(false), 
      m_bReload(false), 
      m_pMouse(), 
      m_pKeyboard(), 
      m_pFont(), 
      m_pDevice(pDevice), 
      m_pSprite(), 
      m_pLine(), 
      m_tPreDrawTimer(), 
      m_vWindows(), 
      m_sCurTheme(), 
      m_mThemes(), 
      m_mCallbacks()
    {
      // Ensure device is valid
      if (!m_pDevice)
      {
        BOOST_THROW_EXCEPTION(HadesGuiError() << 
          ErrorFunction("CGUI::CGUI") << 
          ErrorString("Invalid device."));
      }

      // Create D3DX sprite
      HRESULT SpriteResult = D3DXCreateSprite(pDevice, &m_pSprite);
      if (FAILED(SpriteResult))
      {
        BOOST_THROW_EXCEPTION(HadesGuiError() << 
          ErrorFunction("CGUI::CGUI") << 
          ErrorString("Could not create sprite.") << 
          ErrorCodeWin(SpriteResult));
      }

      // Create D3DX line
      HRESULT LineResult = D3DXCreateLine(pDevice, &m_pLine);
      if (FAILED(LineResult))
      {
        BOOST_THROW_EXCEPTION(HadesGuiError() << 
          ErrorFunction("CGUI::CGUI") << 
          ErrorString("Could not create line.") << 
          ErrorCodeWin(LineResult));
      }

      // Create mouse manager
      m_pMouse.reset(new CMouse(*this, pDevice));

      // Create keyboard manager
      m_pKeyboard.reset(new CKeyboard(*this));

      // Add default callbacks
      AddCallback("Value", SliderValue);
      AddCallback("MaxValue", MaxValue);
      AddCallback("MinValue", MinValue);
    }

    // Destructor
    CGUI::~CGUI()
    {
      // Delete all windows
      std::for_each(m_vWindows.begin(), m_vWindows.end(), 
        [] (CWindow* pWindow)
      {
        delete pWindow;
      });
    }

    // Load interface data from file
    void CGUI::LoadInterfaceFromFile(std::string const& Path)
    {
      // Create XML doc
      TiXmlDocument Document;
      
      // Load interface file into doc
      if (!Document.LoadFile(Path.c_str()))
      {
        std::stringstream ErrorMsg;
        ErrorMsg << "XML error \"" << Document.ErrorDesc() << "\".";

        BOOST_THROW_EXCEPTION(HadesGuiError() << 
          ErrorFunction("CGUI::LoadInterfaceFromFile") << 
          ErrorString(ErrorMsg.str()));
      }

      // Get handle to doc
      TiXmlHandle hDoc(&Document);

      // Ensure file format is valid
      TiXmlElement* pGUI = hDoc.FirstChildElement("GUI").Element();
      if (!pGUI)
      {
        return;
      }

      // Set font if specified
      TiXmlElement* pFontElement = pGUI->FirstChildElement("Font");
      if (pFontElement)
      {
        int FontSize = 0;
        pFontElement->QueryIntAttribute("size", &FontSize);

        char const* FontFace = pFontElement->Attribute("face");

        m_pFont.reset(new CFont(*this, GetDevice(), FontSize, FontFace));
      }

      // Initialize theme data if specified
      TiXmlElement* pColorThemes = pGUI->FirstChildElement("ColorThemes");
      if (pColorThemes)
      {
        // Set default theme if specified
        const char* pszDefaultTheme = pColorThemes->Attribute("default");
        if (pszDefaultTheme)
        {
          m_sCurTheme = pszDefaultTheme;
        }

        // Loop over all themes
        for(TiXmlElement* pThemeElem = pColorThemes->FirstChildElement(); 
          pThemeElem; pThemeElem = pThemeElem->NextSiblingElement())
        {
          // Loop over all theme elems
          for(TiXmlElement* pElemElem = pThemeElem->FirstChildElement(); 
            pElemElem; pElemElem = pElemElem->NextSiblingElement())
          {
            // Create new theme elem
            SElement* CurElem = new SElement();

            // Set default state if specified
            const char* pszDefault = pElemElem->Attribute("default");
            if (pszDefault)
            {
              CurElem->sDefaultState = pszDefault;
            }

            // Loop over all theme elem states
            for(TiXmlElement* pStateElem = pElemElem->
              FirstChildElement("State"); pStateElem; pStateElem = 
              pStateElem->NextSiblingElement("State"))
            {
              // Get state name
              const char * pszString = pStateElem->Attribute("string");
              if (!pszString)
              {
                continue;
              }

              // Create new state
              SElementState* pState = CurElem->m_States[pszString] = 
                new SElementState();

              // Set state parent to current theme
              pState->pParent = CurElem;

              // Loop over all state colours
              for(TiXmlElement* pColourElem = pStateElem->
                FirstChildElement("Color"); pColourElem; pColourElem = 
                pColourElem->NextSiblingElement("Color"))
              {
                // Get name of target element
                pszString = pColourElem->Attribute("string");
                if (!pszString)
                {
                  continue;
                }

                // Create colour for element
                pState->m_Colours[pszString] = new CColor(pColourElem);
              }

              // Loop over all state textures
              for(TiXmlElement* pTexElem = pStateElem->
                FirstChildElement("Texture"); pTexElem; pTexElem = 
                pTexElem->NextSiblingElement("Texture"))
              {
                // Generate path to texture
                std::stringstream TexPath;
                TexPath << pThemeElem->Value() << "/" << pTexElem->
                  Attribute("path");

                // Create texture for element
                CTexture* pTexture = pState->m_Textures[pTexElem->
                  Attribute("string")] = new CTexture(GetSprite(), 
                  TexPath.str().c_str());

                // Set texture alpha if specified
                int TexAlpha = 0;
                if (pTexElem->QueryIntAttribute("alpha", &TexAlpha) == 
                  TIXML_SUCCESS)
                {
                  pTexture->SetAlpha(static_cast<BYTE>(TexAlpha));
                }
              }

              // Store theme elem
              m_mThemes[pThemeElem->Value()][pElemElem->Value()] = CurElem;
            }
          }
        }
      }

      // Create windows if specified
      TiXmlElement* pWindows = pGUI->FirstChildElement("Windows");
      if (pWindows)
      {
        for(TiXmlElement* pWindowElem = pWindows->FirstChildElement(); 
          pWindowElem; pWindowElem = pWindowElem->NextSiblingElement())
        {
          AddWindow(new CWindow(*this, pWindowElem));
        }
      }
    }

    void CGUI::FillArea(int X, int Y, int Width, int Height, D3DCOLOR MyColour)
    {
      DrawLine(X + Width / 2, Y, X + Width / 2, Y + Height, Width, MyColour);
    }

    void CGUI::DrawLine(int StartX, int StartY, int EndX, int EndY, int Width, 
      D3DCOLOR D3DColour)
    {
      m_pLine->SetWidth(static_cast<float>(Width));

      D3DXVECTOR2 MyVec[2];
      MyVec[0] = D3DXVECTOR2(static_cast<float>(StartX), 
        static_cast<float>(StartY));
      MyVec[1] = D3DXVECTOR2(static_cast<float>(EndX), 
        static_cast<float>(EndY));

      m_pLine->Begin();
      m_pLine->Draw(MyVec, 2, D3DColour);
      m_pLine->End();
    }

    void CGUI::DrawOutlinedBox(int X, int Y, int Width, int Height, 
      D3DCOLOR InnerColour, D3DCOLOR BorderColour)
    {
      FillArea(X + 1, Y + 1, Width - 2, Height - 2, InnerColour);

      DrawLine(X, Y, X, Y + Height, 1, BorderColour);
      DrawLine(X + 1, Y, X + Width - 1,	Y, 1, BorderColour);
      DrawLine(X + 1, Y + Height - 1,	X + Width - 1, Y + Height - 1, 1, 
        BorderColour);
      DrawLine(X + Width - 1,	Y, X + Width - 1,	Y + Height, 1, BorderColour);
    }

    CWindow* CGUI::AddWindow(CWindow* pWindow) 
    {
      m_vWindows.push_back(pWindow);
      return pWindow;
    }

    void CGUI::BringToTop(CWindow* pWindow)
    {
      m_vWindows.erase(std::remove(m_vWindows.begin(), m_vWindows.end(), 
        pWindow), m_vWindows.end());
      m_vWindows.push_back(pWindow);
    }

    void CGUI::Draw()
    {
      if (!IsVisible())
      {
        return;
      }

      PreDraw();

      std::for_each(m_vWindows.begin(), m_vWindows.end(), 
        [] (CWindow* pWindow)
      {
        if (pWindow->IsVisible())
        {
          pWindow->Draw();
        }
      });

      GetMouse().Draw();
    }

    void CGUI::PreDraw()
    {
      if (!m_tPreDrawTimer.Running())
      {
        std::for_each(m_vWindows.rbegin(), m_vWindows.rend(), 
          [] (CWindow* pWindow)
        {
          if (pWindow->IsVisible())
          {
            pWindow->PreDraw();
          }
        });

        m_tPreDrawTimer.Start(0.1f);
      }
    }

    void CGUI::MouseMove(CMouse & MyMouse)
    {
      CElement* pDragging = GetMouse().GetDragging();
      if (!pDragging)
      {
        bool GotWindow = false;

        std::for_each(m_vWindows.rbegin(), m_vWindows.rend(), 
          [&] (CWindow* pWindow)
        {
          if (!pWindow->IsVisible())
          {
            return;
          }

          int Height = 0;
          if (!pWindow->GetMaximized())
          {
            Height = TITLEBAR_HEIGHT;
          }

          if (!GotWindow && GetMouse().InArea(pWindow, Height))
          {
            pWindow->MouseMove(MyMouse);
            GotWindow = true;
          }
          else
          {
            MyMouse.SavePos();
            MyMouse.SetPos(-1, -1);
            pWindow->MouseMove(MyMouse);
            MyMouse.LoadPos();
          }
        });
      }
      else
      {
        pDragging->MouseMove(MyMouse);
      }
    }

    bool CGUI::KeyEvent(SKey MyKey)
    {
      bool Top = false;

      if (!MyKey.m_Key && (MyKey.m_Down || (GetMouse().GetWheel() && 
        !MyKey.m_Down)))
      {
        CMouse& MyMouse = GetMouse();

        std::vector<CWindow*> Repeat;

        // Note: Cannot use iterators as CWindow::KeyEvent calls 
        // CGUI::BringToTop which in turn modifies the container
        for (std::size_t i = m_vWindows.size(); i && i--; )
        {
          CWindow* pWindow = m_vWindows[i];

          if (!pWindow->IsVisible())
          {
            continue;
          }

          if (!Top)
          {
            int Height = 0;
            if (!pWindow->GetMaximized())
            {
              Height = TITLEBAR_HEIGHT;
            }

            if (MyMouse.InArea(pWindow, Height) && !Top)
            {
              pWindow->KeyEvent(MyKey);
              Top = true;
            }
            else
            {
              Repeat.push_back(pWindow);
            }
          }
          else
          {
            MyMouse.SavePos();
            MyMouse.SetPos(CPos(-1, -1));
            pWindow->KeyEvent(MyKey);
            MyMouse.LoadPos();
          }
        }

        std::for_each(Repeat.begin(), Repeat.end(), 
          [&] (CWindow* pRepeat)
        {
          MyMouse.SavePos();
          MyMouse.SetPos(CPos(-1, -1));
          pRepeat->KeyEvent(MyKey);
          MyMouse.LoadPos();
        });
      }
      else
      {
        Top = false;

        std::for_each(m_vWindows.rbegin(), m_vWindows.rend(), 
          [&] (CWindow* pWindow)
        {
          if (pWindow->IsVisible())
          {
            if (pWindow->GetFocussedElement() && pWindow->GetMaximized())
            {
              Top = true;
            }

            pWindow->KeyEvent(MyKey);
          }

          if (!MyKey.m_Down)
          {
            Top = false;
          }
        });
      }

      return Top;
    }

    void CGUI::OnLostDevice()
    {
      m_pDevice = 0;

      if (GetFont())
      {
        GetFont()->OnLostDevice();
      }
      GetSprite()->OnLostDevice();

      m_pLine->OnLostDevice();
    }

    void CGUI::OnResetDevice(IDirect3DDevice9 * pDevice)
    {
      m_pDevice = pDevice;

      if (GetFont())
      {
        GetFont()->OnResetDevice(pDevice);
      }
      GetSprite()->OnResetDevice();

      m_pLine->OnResetDevice();
    }

    CMouse & CGUI::GetMouse() const
    {
      return *m_pMouse;
    }

    CKeyboard * CGUI::GetKeyboard() const
    {
      return &*m_pKeyboard;
    }

    IDirect3DDevice9 * CGUI::GetDevice() const
    {
      return m_pDevice;
    }

    CFont * CGUI::GetFont() const
    {
      return &*m_pFont;
    }

    ID3DXSprite * CGUI::GetSprite() const
    {
      return m_pSprite;
    }

    CWindow* CGUI::GetWindowByString(std::string const& MyString, int Index)
    {
      auto Iter = std::find_if(m_vWindows.begin(), m_vWindows.end(), 
        [&] (CWindow* pWindow)
      {
        return pWindow->GetString(false, Index) == MyString;
      });

      return Iter != m_vWindows.end() ? *Iter : nullptr;
    }

    SElement* CGUI::GetThemeElement(std::string const& sElement) const
    {
      auto Iter = m_mThemes.find(m_sCurTheme);

      tTheme::const_iterator SecondIter;
      if (Iter != m_mThemes.end())
      {
        SecondIter = Iter->second.find(sElement);
      }

      return SecondIter->second;
    }

    void CGUI::SetVisible(bool bVisible)
    {
      m_bVisible = bVisible;
    }

    bool CGUI::IsVisible() const
    {
      return m_bVisible;
    }

    bool CGUI::ShouldReload() const
    {
      return m_bReload;
    }

    void CGUI::Reload()
    {
      m_bReload = true;
    }
  }
}
