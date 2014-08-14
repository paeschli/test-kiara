// Copyright (c) 2010, Object Computing, Inc.
// All rights reserved.

#include "QuoteRequest.h"
#include "RelatedSym.h"
#include <boost/cstdint.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace MiddlewareNewsBrief
{
  const std::string QuoteRequest::TOPIC = "QuoteRequest";

  QuoteRequest QuoteRequest::createTestData()
  {
    QuoteRequest req;

    req.securityID = 2112;
    req.applVersionID = "1.0";
    req.messageType = "100";
    req.senderCompID = "Test Exchange";
    req.msgSeqNum = 4;
    req.sendingTime = 00162635;
    req.quoteReqID = "R";

    req.isEcho = false;
    req.counter = 0;

    req.related.reserve(3);
    req.related.push_back(RelatedSym::createTestData());
    req.related.push_back(RelatedSym::createTestData());
    req.related.push_back(RelatedSym::createTestData());

    return req;
  }
}

#if 0

std::ostream& operator<<(std::ostream &stream,
                         const MiddlewareNewsBrief::QuoteRequest& source)
{
  stream << source.securityID;
  stream << source.applVersionID;
  stream << source.messageType;
  stream << source.senderCompID;
  stream << source.msgSeqNum;
  stream << source.sendingTime;
  stream << source.quoteReqID;

  stream << source.isEcho;
  stream << source.counter;

  stream << source.related.size();
  for (size_t i = 0; i < source.related.size(); ++i) {
    stream << source.related[i];
  }

  return stream;
}

std::istream& operator>>(std::istream &stream,
                         MiddlewareNewsBrief::QuoteRequest& destination)
{
  stream >> destination.securityID;
  stream >> destination.applVersionID;
  stream >> destination.messageType;
  stream >> destination.senderCompID;
  stream >> destination.msgSeqNum;
  stream >> destination.sendingTime;
  stream >> destination.quoteReqID;

  stream >> destination.isEcho;
  stream >> destination.counter;

  size_t numRelated;
  stream >> numRelated;

  std::vector<MiddlewareNewsBrief::RelatedSym> related(numRelated);
  destination.related.swap(related);

  for (size_t i = 0; i < numRelated; ++i)
  {
    stream >> destination.related[i];
  }

  return stream;
}


#endif
