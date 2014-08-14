/* Copyright (c) 2010, Object Computing, Inc.
 * All rights reserved.
 */

#ifndef MIDDLEWARENEWSBRIEF_MARKETDATA_H
#define MIDDLEWARENEWSBRIEF_MARKETDATA_H

#include "MarketDataEntry.h"
#include "CommonTypes_Export.h"
#include <KIARA/Common/stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MarketData
{
    uint32_t securityID;
    double applVersionID;
    double messageType;
    double senderCompID;
    uint32_t msgSeqNum;
    uint32_t sendingTime;
    uint32_t tradeDate;

    int isEcho;
    uint32_t counter;

    int num_mdEntries;
    MarketDataEntry *mdEntries;
} MarketData;

MarketData CommonTypes_Export MarketData_createTestData(void);

void CommonTypes_Export MarketData_destroy(MarketData *data);

void CommonTypes_Export MarketData_create(MarketData *data, int num_entries);

void CommonTypes_Export MarketData_copy(MarketData *dest, const MarketData *src);

#ifdef __cplusplus
}
#endif

#endif
