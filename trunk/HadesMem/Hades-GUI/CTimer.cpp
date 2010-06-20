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
    CTimer::CTimer()
    {
      dwStop = 0;
    }

    CTimer::~CTimer()
    {
    }

    void CTimer::Start(float fSec)
    {
      dwStop = (DWORD)(fSec * 1000) + timeGetTime();
    }

    void CTimer::Stop()
    {
      dwStop = 0;
    }

    float CTimer::TimeLeft()
    {
      if (Running())
        return (((float)(dwStop - timeGetTime()) / 1000));
      return 0; 
    }

    bool CTimer::Running()
    {
      return (dwStop > timeGetTime());
    }
  }
}
