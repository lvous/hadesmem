#pragma once

// HadesMem
#include "WoWAddress.h"
#include "HadesMem/Memory.h"

struct WoWAuctionData
{
  unsigned long Unk00;
  unsigned long Unk04;
  unsigned long Unk08;
  unsigned long Unk0C;
  unsigned long Unk10;
  unsigned long Unk14;
  unsigned long Unk18;
  unsigned long Unk1C;
};

class WoWAuctionHouse
{
public:
  WoWAuctionHouse(Hades::Memory::MemoryMgr const& MyMemory);

  enum List
  {
    List_Owner, 
    List_Bidder, 
    List_List
  };

  void DumpAuction(List MyList, unsigned long Index);

private:
  WoWAuctionHouse& operator= (WoWAuctionHouse const&);

  Hades::Memory::MemoryMgr const& m_Memory;
};

WoWAuctionHouse::WoWAuctionHouse(Hades::Memory::MemoryMgr const& MyMemory) 
  : m_Memory(MyMemory)
{ }

void WoWAuctionHouse::DumpAuction(WoWAuctionHouse::List MyList, 
  unsigned long Index)
{
  switch (MyList)
  {
  case List_Owner:
    {
      break;
    }

  case List_Bidder:
    {
      break;
    }

  case List_List:
    {
      break;
    }

  default:
    assert(!"Unknown AH list.");
  }
}
