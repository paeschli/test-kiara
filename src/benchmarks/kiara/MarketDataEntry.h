/* Copyright (c) 2010, Object Computing, Inc.
 * All rights reserved.
 */

#ifndef MIDDLEWARENEWSBRIEF_MARKETDATAENTRY_H
#define MIDDLEWARENEWSBRIEF_MARKETDATAENTRY_H

#include "CommonTypes_Export.h"
#include <KIARA/Common/stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MarketDataEntry
{
    uint32_t mdUpdateAction;
    uint32_t mdPriceLevel;
    double mdEntryType;
    uint32_t openCloseSettleFlag;
    uint32_t securityIDSource;
    uint32_t securityID;
    uint32_t rptSeq;
    double mdEntryPx;
    uint32_t mdEntryTime;
    uint32_t mdEntrySize;
    uint32_t numberOfOrders;
    double tradingSessionID;
    double netChgPrevDay;
    uint32_t tradeVolume;
    double tradeCondition;
    double tickDirection;
    double quoteCondition;
    uint32_t aggressorSide;
    double matchEventIndicator;

    double dummy1;
    float dummy2;
} MarketDataEntry;

MarketDataEntry CommonTypes_Export MarketDataEntry_createTestData(void);

#ifdef __cplusplus
}
#endif

#endif
