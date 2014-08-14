/* Copyright (c) 2010, Object Computing, Inc.
 * All rights reserved.
 */

#ifndef MIDDLEWARENEWSBRIEF_QUOTEREQUEST_H
#define MIDDLEWARENEWSBRIEF_QUOTEREQUEST_H

#include "RelatedSym.h"
#include "CommonTypes_Export.h"
#include <KIARA/Common/stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CommonTypes_Export QuoteRequest
{
    uint32_t securityID;
    double applVersionID;
    double messageType;
    double senderCompID;
    uint32_t msgSeqNum;
    uint32_t sendingTime;
    double quoteReqID;

    int isEcho;
    uint32_t counter;

    int num_related;
    RelatedSym *related;
} QuoteRequest;

QuoteRequest CommonTypes_Export QuoteRequest_createTestData(void);

void CommonTypes_Export QuoteRequest_destroy(QuoteRequest *req);

void CommonTypes_Export QuoteRequest_create(QuoteRequest *data, int num_related);

void CommonTypes_Export QuoteRequest_copy(QuoteRequest *dest, const QuoteRequest *src);

#endif
