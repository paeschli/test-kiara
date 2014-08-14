// Copyright (c) 2010, Object Computing, Inc.
// All rights reserved.

#include "MarketDataEntry.h"
#include <boost/cstdint.hpp>
#include <iostream>
#include <string>

namespace MiddlewareNewsBrief
{
  MarketDataEntry MarketDataEntry::createTestData()
  {
    MarketDataEntry entry;

    entry.mdUpdateAction = 1;
    entry.mdPriceLevel = 2;
    entry.mdEntryType = 7;
    entry.openCloseSettleFlag = 3;
    entry.securityID = 99;
    entry.securityIDSource = 9;
    entry.rptSeq = 2;
    entry.mdEntryPx = 100.0;
    entry.mdEntryTime = 12345;
    entry.mdEntrySize = 50;
    entry.numberOfOrders = 10;
    entry.tradingSessionID = 2;
    entry.netChgPrevDay = 10.0;
    entry.tradeVolume = 30;
    entry.tradeCondition = double('W');
    entry.tickDirection = 0;
    entry.quoteCondition = double('C');
    entry.aggressorSide = 2;
    entry.matchEventIndicator = double('1');

    entry.dummy1 = 1;
    entry.dummy2 = 2;

    return entry;
  }
}

#if 0

std::ostream& operator<<(std::ostream &stream,
                         const MiddlewareNewsBrief::MarketDataEntry& source)
{
  stream << source.mdUpdateAction;
  stream << source.mdPriceLevel;
  stream << source.mdEntryType;
  stream << source.openCloseSettleFlag;
  stream << source.securityID;
  stream << source.securityIDSource;
  stream << source.rptSeq;
  stream << source.mdEntryPx;
  stream << source.mdEntryTime;
  stream << source.mdEntrySize;
  stream << source.numberOfOrders;
  stream << source.tradingSessionID;
  stream << source.netChgPrevDay;
  stream << source.tradeVolume;
  stream << source.tradeCondition;
  stream << source.tickDirection;
  stream << source.quoteCondition;
  stream << source.aggressorSide;
  stream << source.matchEventIndicator;

  return stream;
}

std::istream& operator>>(std::istream &stream,
                         MiddlewareNewsBrief::MarketDataEntry& destination)
{
  stream >> destination.mdUpdateAction;
  stream >> destination.mdPriceLevel;
  stream >> destination.mdEntryType;
  stream >> destination.openCloseSettleFlag;
  stream >> destination.securityID;
  stream >> destination.securityIDSource;
  stream >> destination.rptSeq;
  stream >> destination.mdEntryPx;
  stream >> destination.mdEntryTime;
  stream >> destination.mdEntrySize;
  stream >> destination.numberOfOrders;
  stream >> destination.tradingSessionID;
  stream >> destination.netChgPrevDay;
  stream >> destination.tradeVolume;
  stream >> destination.tradeCondition;
  stream >> destination.tickDirection;
  stream >> destination.quoteCondition;
  stream >> destination.aggressorSide;
  stream >> destination.matchEventIndicator;

  return stream;
}

#endif
