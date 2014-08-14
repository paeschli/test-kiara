// Copyright (c) 2010, Object Computing, Inc.
// All rights reserved.

#ifndef MIDDLEWARENEWSBRIEF_QUOTEREQUEST_H
#define MIDDLEWARENEWSBRIEF_QUOTEREQUEST_H

#include "RelatedSym.h"
#include "CommonTypes_Export.h"
#include <boost/cstdint.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace MiddlewareNewsBrief
{

   struct CommonTypes_Export QuoteRequest
   {
      static const std::string TOPIC;

      friend class boost::serialization::access;

      boost::uint32_t securityID;
      double applVersionID;
      double messageType;
      double senderCompID;
      boost::uint32_t msgSeqNum;
      boost::uint32_t sendingTime;
      double quoteReqID;

      bool isEcho;
      boost::uint32_t counter;

      std::vector<RelatedSym> related;

      static QuoteRequest createTestData();

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
          ar & quoteReqID;

          ar & isEcho;
          ar & counter;

          ar & related;
      }
   };
}

#if 0

std::ostream& operator<<(std::ostream &stream,
                         const MiddlewareNewsBrief::QuoteRequest& source);

std::istream& operator>>(std::istream &stream,
                         MiddlewareNewsBrief::QuoteRequest& destination);
#endif

#endif
