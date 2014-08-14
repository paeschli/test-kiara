//
// request_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef KIARA_TRANSPORT_HTTPREQUESTHANDLER_HPP_INCLUDED
#define KIARA_TRANSPORT_HTTPREQUESTHANDLER_HPP_INCLUDED

#include <string>
#include <boost/noncopyable.hpp>

namespace KIARA
{
namespace Transport
{

struct HttpResponse;
struct HttpRequest;

/// The common handler for all incoming requests.
class HttpRequestHandler: private boost::noncopyable
{
public:
    HttpRequestHandler() { }

    virtual ~HttpRequestHandler();

    /// Handle a request and produce a reply.
    virtual void handleRequest(const HttpRequest& req, HttpResponse& rep) = 0;

    /// Perform URL-decoding on a string. Returns false if the encoding was
    /// invalid.
    static bool decodeURL(const std::string& in, std::string& out);
};

} // namespace Transport
} // namespace KIARA

#endif // HTTP_SERVER3_REQUEST_HANDLER_HPP
