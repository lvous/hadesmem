/*
This file is part of HadesMem.
Copyright (C) 2010 Joshua Boyce (aka RaptorFactor, Cypherjb, Cypher, Chazwazza).
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
#include <boost/filesystem.hpp>

// Hades
#include "HadesRenderer/Renderer.hpp"
#include "HadesCommon/EnsureCleanup.hpp"

namespace Hades
{
  namespace Kernel
  {
    class Kernel
    {
    public:
      class Error : public virtual HadesError 
      { };
      
      virtual void LoadExtension(boost::filesystem::path const& Path);
      
      virtual void OnFrame(Hades::GUI::Renderer& pRenderer);

    private:
      std::vector<Hades::Windows::EnsureFreeLibrary> m_Extensions;
    };
  }
}
