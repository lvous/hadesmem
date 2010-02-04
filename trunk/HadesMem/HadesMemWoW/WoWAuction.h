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
  unsigned long Unk20;
  unsigned long Unk24;
  unsigned long Unk28;
  unsigned long Unk2C;
  unsigned long Unk30;
  unsigned long Unk34;
  unsigned long Unk38;
  unsigned long Unk3C;
  unsigned long Unk40;
  unsigned long Unk44;
  unsigned long Unk48;
  unsigned long Unk4C;
  unsigned long Unk50;
  unsigned long Unk54;
  unsigned long Unk58;
  unsigned long Unk5C;
  unsigned long Unk60;
  unsigned long Unk64;
  unsigned long Unk68;
  unsigned long Unk6C;
  unsigned long Unk70;
  unsigned long Unk74;
  unsigned long Unk78;
  unsigned long Unk7C;
  unsigned long BidPrice;
  unsigned long Unk84;
  unsigned long BuyoutPrice;
  unsigned long Unk8C;
  unsigned long Unk90;
  unsigned long Unk94;
  unsigned long Unk98;
  unsigned long Unk9C;
};

static_assert(sizeof(WoWAuctionData) == 0xA0, "Size of WoWAuctionData is "
  "incorrect.");

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
      auto pListAuctionArray = m_Memory.Read<WoWAuctionData*>(
        AuctionItemsListAddr);
      auto MyAuctionData = m_Memory.Read<WoWAuctionData>(pListAuctionArray + 
        Index);

      std::wcout << "Unk00: " << MyAuctionData.Unk00 << "." << std::endl;
      std::wcout << "Unk04: " << MyAuctionData.Unk04 << "." << std::endl;
      std::wcout << "Unk08: " << MyAuctionData.Unk08 << "." << std::endl;
      std::wcout << "Unk0C: " << MyAuctionData.Unk0C << "." << std::endl;
      std::wcout << "Unk10: " << MyAuctionData.Unk10 << "." << std::endl;
      std::wcout << "Unk14: " << MyAuctionData.Unk14 << "." << std::endl;
      std::wcout << "Unk18: " << MyAuctionData.Unk18 << "." << std::endl;
      std::wcout << "Unk1C: " << MyAuctionData.Unk1C << "." << std::endl;
      std::wcout << "Unk20: " << MyAuctionData.Unk20 << "." << std::endl;
      std::wcout << "Unk24: " << MyAuctionData.Unk24 << "." << std::endl;
      std::wcout << "Unk28: " << MyAuctionData.Unk28 << "." << std::endl;
      std::wcout << "Unk2C: " << MyAuctionData.Unk2C << "." << std::endl;
      std::wcout << "Unk30: " << MyAuctionData.Unk30 << "." << std::endl;
      std::wcout << "Unk34: " << MyAuctionData.Unk34 << "." << std::endl;
      std::wcout << "Unk38: " << MyAuctionData.Unk38 << "." << std::endl;
      std::wcout << "Unk3C: " << MyAuctionData.Unk3C << "." << std::endl;
      std::wcout << "Unk40: " << MyAuctionData.Unk40 << "." << std::endl;
      std::wcout << "Unk44: " << MyAuctionData.Unk44 << "." << std::endl;
      std::wcout << "Unk48: " << MyAuctionData.Unk48 << "." << std::endl;
      std::wcout << "Unk4C: " << MyAuctionData.Unk4C << "." << std::endl;
      std::wcout << "Unk50: " << MyAuctionData.Unk50 << "." << std::endl;
      std::wcout << "Unk54: " << MyAuctionData.Unk54 << "." << std::endl;
      std::wcout << "Unk58: " << MyAuctionData.Unk58 << "." << std::endl;
      std::wcout << "Unk5C: " << MyAuctionData.Unk5C << "." << std::endl;
      std::wcout << "Unk60: " << MyAuctionData.Unk60 << "." << std::endl;
      std::wcout << "Unk64: " << MyAuctionData.Unk64 << "." << std::endl;
      std::wcout << "Unk68: " << MyAuctionData.Unk68 << "." << std::endl;
      std::wcout << "Unk6C: " << MyAuctionData.Unk6C << "." << std::endl;
      std::wcout << "Unk70: " << MyAuctionData.Unk70 << "." << std::endl;
      std::wcout << "Unk74: " << MyAuctionData.Unk74 << "." << std::endl;
      std::wcout << "Unk78: " << MyAuctionData.Unk78 << "." << std::endl;
      std::wcout << "Unk7C: " << MyAuctionData.Unk7C << "." << std::endl;
      std::wcout << "BidPrice: " << MyAuctionData.BidPrice << "." << std::endl;
      std::wcout << "Unk84: " << MyAuctionData.Unk84 << "." << std::endl;
      std::wcout << "BuyoutPrice: " << MyAuctionData.BuyoutPrice << "." << std::endl;
      std::wcout << "Unk8C: " << MyAuctionData.Unk8C << "." << std::endl;
      std::wcout << "Unk90: " << MyAuctionData.Unk90 << "." << std::endl;
      std::wcout << "Unk94: " << MyAuctionData.Unk94 << "." << std::endl;
      std::wcout << "Unk98: " << MyAuctionData.Unk98 << "." << std::endl;
      std::wcout << "Unk9C: " << MyAuctionData.Unk9C << "." << std::endl;

      break;
    }

  default:
    assert(!"Unknown AH list.");
  }
}
