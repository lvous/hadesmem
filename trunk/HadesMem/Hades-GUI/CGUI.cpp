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
      TiXmlElement * pFontElement = pGUI->FirstChildElement("Font");
      if (pFontElement)
      {
        int FontSize = 0;
        pFontElement->QueryIntAttribute("size", &FontSize);

        char const* FontFace = pFontElement->Attribute("face");

        m_pFont.reset(new CFont(*this, GetDevice(), FontSize, FontFace));
      }

      // Initialize theme data if specified
      TiXmlElement * pColorThemes = pGUI->FirstChildElement("ColorThemes");
      if (pColorThemes)
      {
        const char* pszDefaultTheme = pColorThemes->Attribute("default");
        if (pszDefaultTheme)
        {
          m_sCurTheme = pszDefaultTheme;
        }

        for(TiXmlElement* pThemeElem = pColorThemes->FirstChildElement(); 
          pThemeElem; pThemeElem = pThemeElem->NextSiblingElement())
        {
          for(TiXmlElement* pElemElem = pThemeElem->FirstChildElement(); 
            pElemElem; pElemElem = pElemElem->NextSiblingElement())
          {
            SElement* CurElem = new SElement();

            const char* pszDefault = pElemElem->Attribute("default");
            if (pszDefault)
            {
              CurElem->sDefaultState = pszDefault;
            }

            for(TiXmlElement* pStateElem = pElemElem->FirstChildElement("State"); pStateElem; pStateElem = pStateElem->NextSiblingElement("State"))
            {
              const char * pszString = pStateElem->Attribute("string");

              if (!pszString)
              {
                continue;
              }

              SElementState* pState = CurElem->m_mStates[pszString] = new SElementState();

              pState->pParent = CurElem;

              for(TiXmlElement * pColorElement = pStateElem->FirstChildElement("Color"); pColorElement; pColorElement = pColorElement->NextSiblingElement("Color"))
              {
                pszString = pColorElement->Attribute("string");

                if (!pszString)
                {
                  continue;
                }

                pState->mColors[pszString] = new CColor(pColorElement);
              }

              for(TiXmlElement* pTexElem = pStateElem->FirstChildElement("Texture"); pTexElem; pTexElem = pTexElem->NextSiblingElement("Texture"))
              {
                std::stringstream TexPath;
                TexPath << pThemeElem->Value() << "/" << pTexElem->Attribute("path");

                CTexture* pTexture = pState->mTextures[pTexElem->Attribute("string")] = new CTexture(GetSprite(), TexPath.str().c_str());

                int TexAlpha = 0;
                if (!pTexElem->QueryIntAttribute("alpha", &TexAlpha))
                {
                  pTexture->SetAlpha(static_cast<BYTE>(TexAlpha));
                }
              }

              m_mThemes[pThemeElem->Value()][pElemElem->Value()] = CurElem;
            }
          }
        }
      }

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

    void CGUI::FillArea(int iX, int iY, int iWidth, int iHeight, D3DCOLOR d3dColor)
    {
      DrawLine(iX + iWidth / 2, iY, iX + iWidth / 2, iY + iHeight, iWidth, d3dColor);
    }

    void CGUI::DrawLine(int iStartX, int iStartY, int iEndX, int iEndY, int iWidth, D3DCOLOR d3dColor)
    {
      m_pLine->SetWidth(static_cast<float>(iWidth));

      D3DXVECTOR2 d3dxVector[2];

      d3dxVector[0] = D3DXVECTOR2(static_cast<float>(iStartX), static_cast<float>(iStartY));
      d3dxVector[1] = D3DXVECTOR2(static_cast<float>(iEndX), static_cast<float>(iEndY));

      m_pLine->Begin();
      m_pLine->Draw(d3dxVector, 2, d3dColor);
      m_pLine->End();
    }

    void CGUI::DrawOutlinedBox(int iX, int iY, int iWidth, int iHeight, D3DCOLOR d3dInnerColor, D3DCOLOR d3dBorderColor)
    {
      FillArea(iX + 1, iY + 1, iWidth - 2,  iHeight - 2, d3dInnerColor);

      DrawLine(iX,				iY,					iX,					iY + iHeight,		1, d3dBorderColor);
      DrawLine(iX + 1,			iY,					iX + iWidth - 1,	iY,					1, d3dBorderColor);
      DrawLine(iX + 1,			iY + iHeight - 1,	iX + iWidth - 1,	iY + iHeight - 1,	1, d3dBorderColor);
      DrawLine(iX + iWidth - 1,	iY,					iX + iWidth - 1,	iY + iHeight,		1, d3dBorderColor);
    }

    CWindow * CGUI::AddWindow(CWindow * pWindow) 
    {
      m_vWindows.push_back(pWindow);
      return m_vWindows.back();
    }

    void CGUI::BringToTop(CWindow * pWindow)
    {
      for(int i = 0; i < static_cast<int>(m_vWindows.size()); i++)
        if (m_vWindows[i] == pWindow)
          m_vWindows.erase(m_vWindows.begin() + i);
      m_vWindows.insert( m_vWindows.end(), pWindow);
    }

    void CGUI::Draw()
    {
      if (!IsVisible())
        return;

      PreDraw();

      for each(CWindow * pWindow in m_vWindows)
      {
        if (!pWindow->IsVisible())
          continue;

        pWindow->Draw();
      }

      GetMouse().Draw();
    }

    void CGUI::PreDraw()
    {
      if (!m_tPreDrawTimer.Running())
      {
        for(int iIndex = static_cast<int>(m_vWindows.size()) - 1; iIndex >= 0; iIndex--)
        {
          if (!m_vWindows[ iIndex ]->IsVisible())
            continue;

          m_vWindows[ iIndex ]->PreDraw();
        }

        m_tPreDrawTimer.Start(0.1f);
      }
    }

    void CGUI::MouseMove(CMouse & pMouse)
    {
      CElement * pDragging = GetMouse().GetDragging();
      if (!pDragging)
      {
        bool bGotWindow = false;

        for(int iIndex = static_cast<int>(m_vWindows.size()) - 1; iIndex >= 0; iIndex--)
        {
          if (!m_vWindows[ iIndex ]->IsVisible())
            continue;

          int iHeight = 0;

          if (!m_vWindows[ iIndex ]->GetMaximized())
            iHeight = TITLEBAR_HEIGHT;

          if (!bGotWindow && GetMouse().InArea(m_vWindows[ iIndex ], iHeight))
          {
            m_vWindows[ iIndex ]->MouseMove(pMouse);
            bGotWindow = true;
          }
          else
          {
            pMouse.SavePos();
            pMouse.SetPos(-1, -1);
            m_vWindows[ iIndex ]->MouseMove(pMouse);
            pMouse.LoadPos();
          }
        }
      }
      else
        pDragging->MouseMove(pMouse);
    }

    bool CGUI::KeyEvent(SKey sKey)
    {
      bool bTop = false;

      if (!sKey.m_vKey && (sKey.m_bDown || (GetMouse().GetWheel() && !sKey.m_bDown)))
      {
        CMouse & pMouse = GetMouse();

        std::vector<CWindow*> vRepeat;

        for(int iIndex = static_cast<int>(m_vWindows.size()) - 1; iIndex >= 0; iIndex--)
        {
          if (!m_vWindows[ iIndex ]->IsVisible())
            continue;

          if (!bTop)
          {
            int iHeight = 0;

            if (!m_vWindows[ iIndex ]->GetMaximized())
              iHeight = TITLEBAR_HEIGHT;

            if (pMouse.InArea(m_vWindows[ iIndex ], iHeight) && !bTop)
            {
              m_vWindows[ iIndex ]->KeyEvent(sKey);
              bTop = true;
            }
            else
              vRepeat.push_back(m_vWindows[ iIndex ]);
          }
          else
          {
            pMouse.SavePos();
            pMouse.SetPos(CPos(-1, -1));
            m_vWindows[ iIndex ]->KeyEvent(sKey);
            pMouse.LoadPos();
          }
        }

        for each(CWindow * pRepeat in vRepeat)
        {
          pMouse.SavePos();
          pMouse.SetPos(CPos(-1, -1));
          pRepeat->KeyEvent(sKey);
          pMouse.LoadPos();
        }
      }
      else
      {
        bTop = false;

        for(int iIndex = static_cast<int>(m_vWindows.size()) - 1; iIndex >= 0; iIndex--)
          if (m_vWindows[ iIndex ]->IsVisible())
          {
            if (m_vWindows[ iIndex ]->GetFocussedElement() && m_vWindows[ iIndex ]->GetMaximized())
              bTop = true;

            m_vWindows[ iIndex ]->KeyEvent(sKey);
          }

          if (!sKey.m_bDown)
            bTop = false;
      }

      return bTop;
    }

    void CGUI::OnLostDevice()
    {
      m_pDevice = 0;

      if (GetFont())
        GetFont()->OnLostDevice();
      GetSprite()->OnLostDevice();

      m_pLine->OnLostDevice();
    }

    void CGUI::OnResetDevice(IDirect3DDevice9 * pDevice)
    {
      m_pDevice = pDevice;

      if (GetFont())
        GetFont()->OnResetDevice(pDevice);
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

    CWindow * CGUI::GetWindowByString(std::string const& sString, int iIndex)
    {
      for(int i = 0; i < static_cast<int>(m_vWindows.size()); i++)
        if (m_vWindows[ i ]->GetString(false, iIndex) == sString)
          return m_vWindows[ i ];
      return 0;
    }

    SElement * CGUI::GetThemeElement(std::string const& sElement) const
    {
      std::map<std::string, tTheme>::const_iterator iIter = m_mThemes.find(m_sCurTheme);

      tTheme::const_iterator iSecondIter;
      if (iIter != m_mThemes.end())
        iSecondIter = iIter->second.find(sElement);

      return iSecondIter->second;
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
