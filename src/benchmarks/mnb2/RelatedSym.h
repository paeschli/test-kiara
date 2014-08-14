// Copyright (c) 2010, Object Computing, Inc.
// All rights reserved.

#ifndef MIDDLEWARENEWSBRIEF_RELATEDSYM_H
#define MIDDLEWARENEWSBRIEF_RELATEDSYM_H

#include "CommonTypes_Export.h"
#include <boost/cstdint.hpp>
#include <boost/serialization/access.hpp>
#include <string>
#include <iostream>

namespace MiddlewareNewsBrief
{
  struct CommonTypes_Export RelatedSym
  {
    friend class boost::serialization::access;

    double symbol;
    boost::uint64_t orderQuantity;
    boost::uint32_t side;
    boost::uint64_t transactTime;
    boost::uint32_t quoteType;
    boost::uint32_t securityID;
    boost::uint32_t securityIDSource;

    double dummy1;
    float dummy2;

    static RelatedSym createTestData();

  private:
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
      ar & symbol;
      ar & orderQuantity;
      ar & side;
      ar & transactTime;
      ar & quoteType;
      ar & securityID;
      ar & securityIDSource;

      ar & dummy1;
      ar & dummy2;
    }
  };
}

#if 0
std::ostream& operator<<(std::ostream &stream,
                         const MiddlewareNewsBrief::RelatedSym& source);

std::istream& operator>>(std::istream &stream,
                         MiddlewareNewsBrief::RelatedSym& destination);
#endif

#endif
