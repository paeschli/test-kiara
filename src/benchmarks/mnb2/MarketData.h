// Copyright (c) 2010, Object Computing, Inc.
// All rights reserved.

#ifndef MIDDLEWARENEWSBRIEF_MARKETDATA_H
#define MIDDLEWARENEWSBRIEF_MARKETDATA_H

#include "MarketDataEntry.h"
#include "CommonTypes_Export.h"
#include <boost/cstdint.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace MiddlewareNewsBrief
{
  struct CommonTypes_Export MarketData
  {
    friend class boost::serialization::access;

    static const std::string TOPIC;

    boost::uint32_t securityID;
    double applVersionID;
    double messageType;
    double senderCompID;
    boost::uint32_t msgSeqNum;
    boost::uint32_t sendingTime;
    boost::uint32_t tradeDate;

    bool isEcho;
    boost::uint32_t counter;

    std::vector<MiddlewareNewsBrief::MarketDataEntry> mdEntries;

    static MarketData createTestData();

  private:
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & securityID;
        ar & applVersionID;
        ar & messageType;
        ar & senderCompID;
        ar & msgSeqNum;
        ar & sendingTime;
        ar & tradeDate;

        ar & isEcho;
        ar & counter;

        ar & mdEntries;
    }
  };
}

#if 0

std::ostream& operator<<(std::ostream &stream,
                         const MiddlewareNewsBrief::MarketData& source);

std::istream& operator>>(std::istream &stream,
                         MiddlewareNewsBrief::MarketData& destination);

#endif

#endif
