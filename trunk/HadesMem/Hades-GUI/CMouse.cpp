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

// Hades
#include "CGUI.h"
#include "CColor.h"
#include "CMouse.h"
#include "CElement.h"

#define DBLCLICK_TIME 0.3f

namespace Hades
{
  namespace GUI
  {
    CMouse::CMouse(CGUI& Gui, IDirect3DDevice9* pDevice)
      : m_Gui(Gui), 
      m_pDevice(pDevice), 
      m_Pos(), 
      m_BakPos(), 
      m_pInnerColor(new CColor(255, 255, 255, 255)), 
      m_pBorderColor(new CColor(0, 0, 0, 255)), 
      m_pDraggingElement(nullptr), 
      m_LeftButtonState(0), 
      m_RightButtonState(0), 
      m_MiddleButtonState(0), 
      m_WheelState(0), 
      m_LeftButtonTimer(), 
      m_RightButtonTimer(), 
      m_MiddleButtonTimer()
    {
      SetLeftButton(0);
      SetRightButton(0);
      SetMiddleButton(0);
      SetWheel(0);
    }

    bool CMouse::HandleMessage(unsigned int uMsg, WPARAM wParam, LPARAM lParam)
    {
      if (!m_Gui.IsVisible() || uMsg < WM_MOUSEFIRST || uMsg > WM_MOUSELAST)
      {
        return false;
      }

      bool IsDown = false;

      switch(uMsg)
      {
      case WM_MOUSEMOVE:
        {
          SetPos(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
          m_Gui.MouseMove(*this);
          return false;
        }

      case WM_LBUTTONDOWN:
        SetLeftButton(1);
        IsDown = true;
        break;
      case WM_RBUTTONDOWN:
        SetRightButton(1);
        IsDown = true;
        break;
      case WM_MBUTTONDOWN:
        SetMiddleButton(1);
        IsDown = true;
        break;

      case WM_LBUTTONUP:
        SetLeftButton(0);
        break;
      case WM_RBUTTONUP:
        SetRightButton(0);
        break;
      case WM_MBUTTONUP:
        SetMiddleButton(0);
        break;

      case WM_MOUSEWHEEL:
        float Delta = GET_WHEEL_DELTA_WPARAM(wParam);

        if (Delta > 0.0f)
        {
          SetWheel(1);
        }
        else if (Delta < 0.0f)
        {
          SetWheel(2);
        }
        else
        {
          SetWheel(0);
        }

        break;
      }

      return m_Gui.KeyEvent(SKey(0, IsDown));
    }

    void CMouse::SetPos(CPos MyPos)
    {
      m_Pos = MyPos;
    }

    void CMouse::SetPos(int X, int Y)
    {
      m_Pos.SetX(X);
      m_Pos.SetY(Y);
    }

    CPos CMouse::GetPos() const
    {
      return m_Pos;
    }

    bool CMouse::InArea(int X, int Y, int Width, int Height) const
    {
      return (m_Pos.GetX() >= X && m_Pos.GetX() <= X + Width && 
        m_Pos.GetY() >= Y && m_Pos.GetY() <= Y + Height);
    }

    bool CMouse::InArea(CElement* pElement, int Height) const
    {
      if (!Height)
      {
        Height = pElement->GetHeight();
      }

      return InArea(pElement->GetAbsPos()->GetX(), 
        pElement->GetAbsPos()->GetY(), 
        pElement->GetWidth(), Height);
    }

    void CMouse::Draw()
    {
    }

    int CMouse::GetLeftButton(int State)
    {
      int Ret = m_LeftButtonState;

      if (State != -1)
      {
        SetLeftButton(State);
      }

      return Ret;
    }

    int CMouse::GetRightButton(int State)
    {
      int Ret = m_RightButtonState;

      if (State != -1)
      {
        SetRightButton(State);
      }

      return Ret;
    }

    int CMouse::GetMiddleButton(int State)
    {
      int Ret = m_MiddleButtonState;

      if (State != -1)
      {
        SetMiddleButton(State);
      }

      return Ret;
    }

    int CMouse::GetWheel(int State)
    {
      int Ret = m_WheelState;

      if (State != -1)
      {
        SetWheel(State);
      }

      return Ret;
    }

    void CMouse::SetLeftButton(int State)
    {
      if (State == 1)
      {
        if (m_LeftButtonTimer.Running())
        {
          m_LeftButtonState = 2;
          m_LeftButtonTimer.Stop();
        }
        else
        {
          m_LeftButtonState = 1;
          m_LeftButtonTimer.Start(DBLCLICK_TIME);
        }
      }
      else
      {
        m_LeftButtonState = State;
      }
    }

    void CMouse::SetRightButton(int State)
    {
      if (State == 1)
      {
        if (m_RightButtonTimer.Running())
        {
          m_RightButtonState = 2;
          m_RightButtonTimer.Stop();
        }
        else
        {
          m_RightButtonState = 1;
          m_RightButtonTimer.Start(DBLCLICK_TIME);
        }
      }
      else
      {
        m_RightButtonState = State;
      }
    }

    void CMouse::SetMiddleButton(int State)
    {
      if (State == 1)
      {
        if (m_MiddleButtonTimer.Running())
        {
          m_MiddleButtonState = 2;
          m_MiddleButtonTimer.Stop();
        }
        else
        {
          m_MiddleButtonState = 1;
          m_MiddleButtonTimer.Start(DBLCLICK_TIME);
        }
      }
      else
      {
        m_MiddleButtonState = State;
      }
    }

    void CMouse::SetWheel(int State)
    {
      m_WheelState = State;
    }

    void CMouse::SetDragging(CElement* pElement)
    {
      m_pDraggingElement = pElement;
    }

    CElement * CMouse::GetDragging() const
    {
      return m_pDraggingElement;
    }

    void CMouse::SavePos()
    {
      m_BakPos = m_Pos;
    }

    void CMouse::LoadPos()
    {
      m_Pos = m_BakPos;
    }

    CPos CMouse::GetSavedPos() const
    {
      return m_BakPos;
    }
  }
}
