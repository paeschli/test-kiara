// Copyright (c) 2010, Object Computing, Inc.
// All rights reserved.

#ifndef MIDDLEWARENEWSBRIEF_MARKETDATAENTRY_H
#define MIDDLEWARENEWSBRIEF_MARKETDATAENTRY_H

#include "CommonTypes_Export.h"
#include <boost/cstdint.hpp>
#include <boost/serialization/access.hpp>
#include <iostream>
#include <string>

namespace MiddlewareNewsBrief
{
  struct CommonTypes_Export MarketDataEntry
  {
    friend class boost::serialization::access;

    boost::uint32_t mdUpdateAction;
    boost::uint32_t mdPriceLevel;
    double mdEntryType;
    boost::uint32_t openCloseSettleFlag;
    boost::uint32_t securityIDSource;
    boost::uint32_t securityID;
    boost::uint32_t rptSeq;
    double mdEntryPx;
    boost::uint32_t mdEntryTime;
    boost::uint32_t mdEntrySize;
    boost::uint32_t numberOfOrders;
    double tradingSessionID;
    double netChgPrevDay;
    boost::uint32_t tradeVolume;
    double tradeCondition;
    double tickDirection;
    double quoteCondition;
    boost::uint32_t aggressorSide;
    double matchEventIndicator;

    double dummy1;
    float dummy2;

    static MarketDataEntry createTestData();

  private:
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
      ar & mdUpdateAction;
      ar & mdPriceLevel;
      ar & mdEntryType;
      ar & openCloseSettleFlag;
      ar & securityID;
      ar & securityIDSource;
      ar & rptSeq;
      ar & mdEntryPx;
      ar & mdEntryTime;
      ar & mdEntrySize;
      ar & numberOfOrders;
      ar & tradingSessionID;
      ar & netChgPrevDay;
      ar & tradeVolume;
      ar & tradeCondition;
      ar & tickDirection;
      ar & quoteCondition;
      ar & aggressorSide;
      ar & matchEventIndicator;

      ar & dummy1;
      ar & dummy2;
    }
  };
}

#if 0
std::ostream& operator<<(std::ostream &stream,
                         const MiddlewareNewsBrief::MarketDataEntry& source);

std::istream& operator>>(std::istream &stream,
                         MiddlewareNewsBrief::MarketDataEntry& destination);
#endif

#endif
