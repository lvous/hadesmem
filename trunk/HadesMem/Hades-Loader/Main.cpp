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

// C++ Standard Library
#include <string>
#include <iostream>

// Windows
#include <Windows.h>

// Hades
#include "Hades-Common/Error.h"
#include "Hades-Common/Logger.h"

int CALLBACK wWinMain(
  __in  HINSTANCE /*hInstance*/,
  __in  HINSTANCE /*hPrevInstance*/,
  __in  LPWSTR /*lpCmdLine*/,
  __in  int /*nCmdShow*/)
{
  try
  {
    // Initialize logger
    Hades::Util::InitLogger(L"Hades-Loader-Log", L"Hades-Loader-Debug");

    // Hades version number
    std::wstring const VerNum(L"TRUNK");

    // Version and copyright output
#if defined(_M_X64)
    std::wcout << "Hades-Loader AMD64 [Version " << VerNum << "]" << std::endl;
#elif defined(_M_IX86)
    std::wcout << "Hades-Loader IA32 [Version " << VerNum << "]" << std::endl;
#else
#error Unsupported platform!
#endif
    std::wcout << "Copyright (C) 2010 Cypherjb. All rights reserved." << 
      std::endl;
    std::wcout << "Website: http://www.cypherjb.com/, Email: "
      "cypher.jb@gmail.com." << std::endl;
    std::wcout << "Built on " << __DATE__ << " at " << __TIME__ << "." << 
      std::endl;
  }
  catch (boost::exception const& e)
  {
    // Dump error information
    MessageBoxA(NULL, boost::diagnostic_information(e).c_str(), 
      "Hades-Loader", MB_OK);
  }
  catch (std::exception const& e)
  {
    // Dump error information
    MessageBoxA(NULL, e.what(), "Hades-Loader", MB_OK);
  }

  return 0;
}
