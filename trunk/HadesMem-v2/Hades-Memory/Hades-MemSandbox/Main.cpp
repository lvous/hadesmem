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

// Windows API
#include <tchar.h>
#include <crtdbg.h>
#include <Windows.h>

// C++ Standard Library
#include <vector>
#include <string>
#include <iterator>
#include <iostream>
#include <exception>

// Boost
#pragma warning(push, 1)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <boost/timer.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#pragma warning(pop)

// Hades
#include "Hades-Memory/Memory.h"
#include "Hades-Common/Logger.h"
#include "Hades-Memory/AutoLink.h"
#include "Hades-MemScript/AutoLink.h"
#include "Hades-MemScript/Scripting.h"

// Ensure correct stream type is used.
// Fixme: Implement this in a less disgusting way.
#ifdef _UNICODE
#define tcin wcin
#define tcout wcout
#define tvalue wvalue
#else
#define tcin cin
#define tcout cout
#define tvalue value
#endif

bool GetInput(Hades::Memory::ScriptMgr& MyScriptMgr) 
{
  // Prompt for input
  std::tcout << ">";

  // Get command from user
  std::basic_string<TCHAR> Input;
  while (!std::getline(std::tcin, Input) || Input.empty())
  {
    std::tcout << "Invalid command." << std::endl;
    std::tcout << ">";
  }

  // Check for quit request
  if (Input == _T("quit") || Input == _T("exit"))
  {
    return false;
  }

  // Run script
  MyScriptMgr.RunString(boost::lexical_cast<std::string>(Input));

  return true;
}

// Program entry-point.
int _tmain(int argc, TCHAR* argv[])
{
  // Program timer
  boost::timer ProgTimer;

  try
  {
    // Attempt to detect memory leaks in debug mode
#ifdef _DEBUG
    int CurrentFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    int NewFlags = (_CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF | 
      _CRTDBG_CHECK_ALWAYS_DF);
    _CrtSetDbgFlag(CurrentFlags | NewFlags);
#endif

    // Hades version number
    std::basic_string<TCHAR> const VerNum(_T("TRUNK"));

    // Version and copyright output
#if defined(_M_X64)
    std::tcout << "Hades-MemHack AMD64 [Version " << VerNum << "]\n";
#elif defined(_M_IX86)
    std::tcout << "Hades-MemHack IA32 [Version " << VerNum << "]\n";
#else
#error Unsupported platform!
#endif
    std::tcout << "Copyright (C) 2010 RaptorFactor. All rights reserved." << 
      std::endl;
    std::tcout << "Website: http://www.raptorfactor.com/, "
      "Email: raptorfactor@raptorfactor.com." << std::endl;
    std::tcout << "Built on " << __DATE__ << " at " << __TIME__ << "." << 
      std::endl << std::endl;

    // Auto-close flag (Set by Boost.ProgramOptions)
    bool KeepOpen = false;
    // Path to script file (Set by Boost.ProgramOptions)
    std::basic_string<TCHAR> FilePath;
    // Script string (Set by Boost.ProgramOptions)
    std::basic_string<TCHAR> ScriptStr;

    // Set program option descriptions
    boost::program_options::options_description OptsDesc("Allowed options");
    OptsDesc.add_options()
      ("help", "display help")
      ("keep-open", boost::program_options::tvalue<bool>(&KeepOpen)->
        zero_tokens(), "keep console window open")
      ("file", boost::program_options::tvalue<std::basic_string<TCHAR>>(
        &FilePath), "file to execute")
      ("string", boost::program_options::tvalue<std::basic_string<TCHAR>>(
        &ScriptStr), "string to execute")
      ;

    // Parse program options
    boost::program_options::variables_map Opts;
    boost::program_options::store(boost::program_options::parse_command_line(
      argc, argv, OptsDesc), Opts);
    boost::program_options::notify(Opts);

    // Print help if requested
    if (Opts.count("help")) 
    {
      // Print help
      std::cout << OptsDesc << std::endl;

      // Stop window from automatically closing if required
      if (KeepOpen)
      {
        std::tcin.clear();
        std::tcin.sync();
        std::tcin.get();
      }

      // Quit
      return 1;
    }

    // Create script manager
    Hades::Memory::ScriptMgr MyScriptMgr;

    // If user has passed in a file-name then run it
    if (!FilePath.empty())
    {
      // Get file
      boost::filesystem::path const FilePathReal(FilePath);
      if (!boost::filesystem::exists(FilePathReal))
      {
        BOOST_THROW_EXCEPTION(Hades::HadesError() << 
          Hades::ErrorFunction("wmain") << 
          Hades::ErrorString("Requested file could not be found."));
      }

      // Run script
      MyScriptMgr.RunFile(FilePathReal.string());
    }
    // If user has passed in a string then run it
    else if (!ScriptStr.empty())
    {
      MyScriptMgr.RunString(boost::lexical_cast<std::string>(ScriptStr));
    }
    // Otherwise process commands from user
    else
    {
      for (;;)
      {
        try
        {
          if (!GetInput(MyScriptMgr))
          {
            break;
          }
        }
        catch (std::exception const& e)
        {
          // Dump error information
          std::cout << boost::diagnostic_information(e);
        }
      }
    }

    // Print elapsed time
    std::tcout << "\nElapsed Time: " << ProgTimer.elapsed() << "." << 
      std::endl;

    // Stop window from automatically closing if required
    if (KeepOpen)
    {
      std::tcin.clear();
      std::tcin.sync();
      std::tcin.get();
    }
  }
  catch (std::exception const& e)
  {
    // Dump error information
    std::cout << boost::diagnostic_information(e);

    // Print elapsed time
    std::tcout << "\nElapsed Time: " << ProgTimer.elapsed() << "." << 
      std::endl;

    // Always keep window open in case of an error
    std::tcin.clear();
    std::tcin.sync();
    std::tcin.get();
  }
}
