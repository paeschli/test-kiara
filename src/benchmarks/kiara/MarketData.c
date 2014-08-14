// Copyright (c) 2010, Object Computing, Inc.
// All rights reserved.

#include "MarketData.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

MarketData MarketData_createTestData()
{
    MarketData data;

    data.isEcho = 0;
    data.counter = 0;

    data.applVersionID = 1.0;
    data.messageType = 100;
    data.senderCompID = 121213.0;
    data.msgSeqNum = 4;
    data.sendingTime = 00162635;
    data.tradeDate = 20100422;

    data.num_mdEntries = 3;
    data.mdEntries = malloc(sizeof(MarketDataEntry)*3);
    data.mdEntries[0] = MarketDataEntry_createTestData();
    data.mdEntries[1] = MarketDataEntry_createTestData();
    data.mdEntries[2] = MarketDataEntry_createTestData();

    return data;
}

void MarketData_destroy(MarketData *data)
{
    data->num_mdEntries = 0;
    free(data->mdEntries);
    data->mdEntries = NULL;
}

void MarketData_create(MarketData *data, int num_entries)
{
    data->num_mdEntries = num_entries;
    if (num_entries == 0)
    {
        data->mdEntries = NULL;
    }
    else
    {
        data->mdEntries = (MarketDataEntry *)malloc(sizeof(data->mdEntries[0])*num_entries);
    }
}

void MarketData_copy(MarketData *dest, const MarketData *src)
{
    dest->securityID = src->securityID;
    dest->applVersionID = src->applVersionID;
    dest->messageType = src->messageType;
    dest->senderCompID = src->senderCompID;
    dest->msgSeqNum = src->msgSeqNum;
    dest->sendingTime = src->sendingTime;
    dest->tradeDate = src->tradeDate;

    dest->isEcho = src->isEcho;
    dest->counter = src->counter;

    if (dest->num_mdEntries != src->num_mdEntries)
    {
        MarketData_destroy(dest);
        MarketData_create(dest, src->num_mdEntries);
    }
    memcpy(dest->mdEntries, src->mdEntries, sizeof(src->mdEntries[0]) * src->num_mdEntries);
}
