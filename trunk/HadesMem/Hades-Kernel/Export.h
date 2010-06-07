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

#pragma once

#if defined(HADES_KERNEL_EXPORT) && defined(HADES_KERNEL_IMPORT)
#error "Compilation mode invalid for Hades-Kernel headers"
#endif

#if defined(HADES_KERNEL_EXPORT)
#define HADES_KERNEL_EXPORT_INTERNAL __declspec(dllexport)
#elif defined(HADES_KERNEL_IMPORT)
#define HADES_KERNEL_EXPORT_INTERNAL __declspec(dllimport)
#else
#error "Compilation mode not selected for Hades-Kernel headers"
#endif
