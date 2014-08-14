/* Copyright (c) 2010, Object Computing, Inc.
 * All rights reserved.
 */
#ifndef MIDDLEWARENEWSBRIEF_RELATEDSYM_H
#define MIDDLEWARENEWSBRIEF_RELATEDSYM_H

#include "CommonTypes_Export.h"
#include <KIARA/Common/stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RelatedSym
{
    double symbol;
    uint64_t orderQuantity;
    uint32_t side;
    uint64_t transactTime;
    uint32_t quoteType;
    uint32_t securityID;
    uint32_t securityIDSource;

    double dummy1;
    float dummy2;
} RelatedSym;

RelatedSym CommonTypes_Export RelatedSym_createTestData(void);

#ifdef __cplusplus
}
#endif

#endif
