// Copyright (c) 2010, Object Computing, Inc.
// All rights reserved.

#include "RelatedSym.h"

RelatedSym RelatedSym_createTestData(void)
{
    RelatedSym sym;

    sym.symbol = 321.0;
    sym.orderQuantity = 25;
    sym.side = 1;
    sym.transactTime = 00162635;
    sym.quoteType = 1;
    sym.securityID = 99;
    sym.securityIDSource = 9;

    sym.dummy1 = 1;
    sym.dummy2 = 2;

    return sym;
}
