// Copyright (c) 2010, Object Computing, Inc.
// All rights reserved.

#include "RelatedSym.h"
#include <boost/cstdint.hpp>
#include <string>
#include <iostream>

namespace MiddlewareNewsBrief
{
  RelatedSym RelatedSym::createTestData()
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
}

#if 0
std::ostream& operator<<(std::ostream &stream,
                         const MiddlewareNewsBrief::RelatedSym& source)
{
  stream << source.symbol;
  stream << source.orderQuantity;
  stream << source.side;
  stream << source.transactTime;
  stream << source.quoteType;
  stream << source.securityID;
  stream << source.securityIDSource;

  return stream;
}

std::istream& operator>>(std::istream &stream,
                         MiddlewareNewsBrief::RelatedSym& destination)
{
  stream >> destination.symbol;
  stream >> destination.orderQuantity;
  stream >> destination.side;
  stream >> destination.transactTime;
  stream >> destination.quoteType;
  stream >> destination.securityID;
  stream >> destination.securityIDSource;

  return stream;
}

#endif
