// C++ Standard Library
#include <iostream>

// HadesMem
#include "WoWAuction.h"

// Program entry-point.
int wmain(int /*argc*/, wchar_t* /*argv*/[], wchar_t* /*envp*/[])
{
  try
  {
    Hades::Memory::MemoryMgr MyMemory(L"wow.exe");
    WoWAuctionHouse MyWoWAuctionHouse(MyMemory);

    for (;;)
    {
      // Output
      std::wcout << "Enter AH list index:" << std::endl;

      // Get index
      unsigned long Index = 0;
      while (!(std::wcin >> Index))
      {
        std::cout << "Invalid index." << std::endl;
        std::wcin.clear();
        std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), 
          '\n');
      }
      std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), 
        '\n');

      // Dump auction
      MyWoWAuctionHouse.DumpAuction(WoWAuctionHouse::List_List, Index);
    }
  }
  catch (boost::exception const& e)
  {
    // Dump error information
    std::cout << boost::diagnostic_information(e);
  }
  catch (std::exception const& e)
  {
    // Dump error information
    std::wcout << "Error! " << e.what() << std::endl;
  }

  // Stop console window closing
  std::wcin.clear();
  std::wcin.sync();
  std::wcin.get();
}
