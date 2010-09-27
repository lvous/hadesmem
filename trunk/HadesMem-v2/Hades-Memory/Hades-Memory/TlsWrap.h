/*
This file is part of HadesMem.
Copyright © 2010 RaptorFactor (aka Cypherjb, Cypher, Chazwazza). 
<http://www.raptorfactor.com/> <raptorfactor@raptorfactor.com>

HadesMem is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

HadesMem is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with HadesMem.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

// C++ Standard Library
#include <vector>

// Hades
#include "TlsDir.h"

namespace Hades
{
  namespace Memory
  {
    namespace Wrappers
    {
      class TlsDirWrappers : public TlsDir
      {
      public:
        struct TlsCallbackList
        {
          std::vector<DWORD_PTR> List;
        };

        TlsDirWrappers(PeFile& MyPeFile) 
          : TlsDir(MyPeFile)
        { }

        TlsCallbackList GetCallbacks() const
        {
          std::vector<PIMAGE_TLS_CALLBACK> const Callbacks(TlsDir::
            GetCallbacks());
          TlsCallbackList MyList;
          std::transform(Callbacks.begin(), Callbacks.end(), 
            std::back_inserter(MyList.List), 
            [] (PIMAGE_TLS_CALLBACK Current)
          {
            return reinterpret_cast<DWORD_PTR>(Current);
          });
          return MyList;
        }
      };
    }
  }
}
