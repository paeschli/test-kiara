// Copyright (c) 2010, Object Computing, Inc.
// All rights reserved.

#include "MarketDataEntry.h"

MarketDataEntry MarketDataEntry_createTestData(void)
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
    entry.tradeCondition = (double)'W';
    entry.tickDirection = 0;
    entry.quoteCondition = (double)'C';
    entry.aggressorSide = 2;
    entry.matchEventIndicator = (double)'1';

    entry.dummy1 = 1;
    entry.dummy2 = 2;

    return entry;
}
