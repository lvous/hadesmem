/*
This file is part of HadesMem.
Copyright © 2010 Cypherjb (aka Chazwazza, aka Cypher). 
<http://www.cypherjb.com/> <cypher.jb@gmail.com>

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

// Hades
#include "Kernel.h"
#include "DotNet.h"
#include "Scripting.h"
#include "Hades-D3D9/GuiMgr.h"

namespace Hades
{
  namespace Wrappers
  {
    WriteLn::WriteLn(Kernel* pKernel)
      : m_pKernel(pKernel)
    { }

    void WriteLn::operator()(std::string const& Input) const
    {
      HADES_SCRIPTING_TRYCATCH_BEGIN
        m_pKernel->GetGuiMgr()->Print(Input);
      HADE_SCRIPTING_TRYCATCH_END
    }

    LoadExt::LoadExt(Kernel* pKernel)
      : m_pKernel(pKernel)
    { }

    void LoadExt::operator()(std::string const& LoadExt) const
    {
      HADES_SCRIPTING_TRYCATCH_BEGIN
        m_pKernel->LoadExtension(boost::lexical_cast<std::wstring>(LoadExt));
      HADE_SCRIPTING_TRYCATCH_END
    }

    DotNet::DotNet(DotNetMgr* pDotNet)
      : m_pDotNet(pDotNet)
    { }

    void DotNet::operator()(std::string const& Assembly, 
      std::string const& Type, std::string const& Method, 
      std::string const& Parameters) const
    {
      HADES_SCRIPTING_TRYCATCH_BEGIN
        m_pDotNet->LoadAssembly(
          boost::lexical_cast<std::wstring>(Assembly), 
          boost::lexical_cast<std::wstring>(Type), 
          boost::lexical_cast<std::wstring>(Method), 
          boost::lexical_cast<std::wstring>(Parameters));
      HADE_SCRIPTING_TRYCATCH_END
    }
  }
}
