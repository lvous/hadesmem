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

// Boost
#pragma warning(push, 1)
#pragma warning(disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/shared_ptr.hpp>
#pragma warning(pop)

// HadesMem
#include "PEFile.h"
#include "ExportEnum.h"

namespace Hades
{
  namespace Memory
  {
    namespace Wrappers
    {
      class ExportEnumWrap : public ExportEnum
      {
      public:
        ExportEnumWrap(PeFile& MyPeFile)
          : ExportEnum(MyPeFile)
        { }

        boost::shared_ptr<Export> First()
        {
          // This is dangerous, but I haven't had time to think about the 
          // 'proper' solution yet, so this should work for now, but needs 
          // to be fixed in the future.
          // Todo: Fix this monstrosity.
          return boost::shared_ptr<Export>(ExportEnum::First().release());
        }

        boost::shared_ptr<Export> Next()
        {
          // This is dangerous, but I haven't had time to think about the 
          // 'proper' solution yet, so this should work for now, but needs 
          // to be fixed in the future.
          // Todo: Fix this monstrosity.
          return boost::shared_ptr<Export>(ExportEnum::Next().release());
        }
      };
    }
  }
}
