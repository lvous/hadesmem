/*
This file is part of HadesMem.
Copyright � 2010 Cypherjb (aka Chazwazza, aka Cypher). 
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

#pragma once

// C++ Standard Library
#include <string>
#include <memory>

// Boost
#include <boost/noncopyable.hpp>

// Hades
#include "Hades-Memory/Memory.h"

namespace Hades
{
  // Kernel exception type
  class KernelError : public virtual HadesError 
  { };

  // Hades kernel
  class Kernel : private boost::noncopyable
  {
  public:
    // Constructor
    Kernel();

    // Initialize kernel
    virtual void Initialize();

    // Get memory manager
    virtual std::shared_ptr<Memory::MemoryMgr> GetMemoryMgr();

    // Load and initialize a Hades helper module
    virtual void LoadModule(std::wstring const& Module);

  private:
    // Memory manager
    std::shared_ptr<Memory::MemoryMgr> m_Memory;
  };
}
