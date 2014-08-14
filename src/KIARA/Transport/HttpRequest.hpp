//
// request.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef KIARA_TRANSPORT_HTTPREQUEST_HPP_INCLUDED
#define KIARA_TRANSPORT_HTTPREQUEST_HPP_INCLUDED

#include <string>
#include <vector>
#include "HttpMessage.hpp"

namespace KIARA
{
namespace Transport
{

/// A request received from a client.
class HttpRequest : public HttpMessage
{
public:

    std::string method;
    std::string uri;
    int httpVersionMajor;
    int httpVersionMinor;

    HttpRequest(const Transport *transport);

    void clear();

};

} // namespace Transport
} // namespace KIARA

#endif // KIARA_SERVER_HTTPREQUEST_HPP_INCLUDED
