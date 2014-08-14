// Copyright (c) 2010, Object Computing, Inc.
// All rights reserved.

#include "QuoteRequest.h"
#include "RelatedSym.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

QuoteRequest QuoteRequest_createTestData()
{
    QuoteRequest req;

    req.securityID = 2112;
    req.applVersionID = 1.0;
    req.messageType = 100;
    req.senderCompID = 7881;
    req.msgSeqNum = 4;
    req.sendingTime = 00162635;
    req.quoteReqID = (double)'R';

    req.isEcho = 0;
    req.counter = 0;

    req.num_related = 3;
    req.related = malloc(sizeof(RelatedSym)*3);
    req.related[0] = RelatedSym_createTestData();
    req.related[1] = RelatedSym_createTestData();
    req.related[2] = RelatedSym_createTestData();

    return req;
}

void QuoteRequest_destroy(QuoteRequest *req)
{
    req->num_related = 0;
    free(req->related);
    req->related = NULL;
}

void QuoteRequest_create(QuoteRequest *req, int num_related)
{
    req->num_related = num_related;
    if (num_related == 0)
    {
        req->related = NULL;
    }
    else
    {
        req->related = (RelatedSym *)malloc(sizeof(req->related[0])*num_related);
    }
}

void QuoteRequest_copy(QuoteRequest *dest, const QuoteRequest *src)
{
    dest->securityID = src->securityID;
    dest->applVersionID = src->applVersionID;
    dest->messageType = src->messageType;
    dest->senderCompID = src->senderCompID;
    dest->msgSeqNum = src->msgSeqNum;
    dest->sendingTime = src->sendingTime;
    dest->quoteReqID = src->quoteReqID;

    dest->isEcho = src->isEcho;
    dest->counter = src->counter;

    if (dest->num_related != src->num_related)
    {
        QuoteRequest_destroy(dest);
        QuoteRequest_create(dest, src->num_related);
    }
    memcpy(dest->related, src->related, sizeof(src->related[0]) * src->num_related);
}
