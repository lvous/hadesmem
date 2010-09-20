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

#if defined(_MSC_VER)
#define HADES_COMPILER_MSVC
#elif defined(__MINGW32__)
#define HADES_COMPILER_MINGW
#else
static_assert(false, "Unsupported compiler!");
#endif

#if defined(HADES_COMPILER_MSVC)
#define HADES_DISABLE_WARNINGS_PUSH() __pragma(warning(push, 1))\
__pragma(warning (disable: ALL_CODE_ANALYSIS_WARNINGS))\
__pragma(warning (disable: 4706))\
__pragma(warning (disable: 4702))
#define HADES_DISABLE_WARNINGS_POP() __pragma(warning(pop))
#elif defined(HADES_COMPILER_MINGW)
#define HADES_DISABLE_WARNINGS_PUSH()
#define HADES_DISABLE_WARNINGS_POP()
#else
static_assert(false, "Unsupported compiler!");
#endif
