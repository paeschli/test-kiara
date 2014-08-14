// Copyright (c) 2010, Object Computing, Inc.
// All rights reserved.

#include "MarketData.h"
#include <boost/cstdint.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace MiddlewareNewsBrief
{
  const std::string MarketData::TOPIC = "MarketData";

  MarketData MarketData::createTestData()
  {
    MarketData data;

    data.isEcho = false;
    data.counter = 0;

    data.applVersionID = 1.0;
    data.messageType = 100;
    data.senderCompID = 121213.0;
    data.msgSeqNum = 4;
    data.sendingTime = 00162635;
    data.tradeDate = 20100422;

    data.mdEntries.push_back(MarketDataEntry::createTestData());
    data.mdEntries.push_back(MarketDataEntry::createTestData());
    data.mdEntries.push_back(MarketDataEntry::createTestData());

    return data;
  }
}

#if 0

std::ostream& operator<<(std::ostream &stream,
                         const MiddlewareNewsBrief::MarketData& source)
{
  stream << source.securityID;
  stream << source.applVersionID;
  stream << source.messageType;
  stream << source.senderCompID;
  stream << source.msgSeqNum;
  stream << source.sendingTime;
  stream << source.tradeDate;

  stream << source.isEcho;
  stream << source.counter;

  stream << source.mdEntries.size();
  for (size_t i = 0; i < source.mdEntries.size(); ++i) {
    stream << source.mdEntries[i];
  }

  return stream;
}

std::istream& operator>>(std::istream &stream,
                         MiddlewareNewsBrief::MarketData& destination)
{
  stream >> destination.securityID;
  stream >> destination.applVersionID;
  stream >> destination.messageType;
  stream >> destination.senderCompID;
  stream >> destination.msgSeqNum;
  stream >> destination.sendingTime;
  stream >> destination.tradeDate;

  stream >> destination.isEcho;
  stream >> destination.counter;

  size_t numEntries;
  stream >> numEntries;

  std::vector<MiddlewareNewsBrief::MarketDataEntry> mdEntries(numEntries);
  destination.mdEntries.swap(mdEntries);

  for (size_t i = 0; i < numEntries; ++i)
  {
    stream >> destination.mdEntries[i];
  }

  return stream;
}


#endif
