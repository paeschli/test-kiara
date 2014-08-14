//
// reply.hpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef KIARA_TRANSPORT_HTTPRESPONSE_HPP_INCLUDED
#define KIARA_TRANSPORT_HTTPRESPONSE_HPP_INCLUDED

#include <string>
#include <vector>
#include "HttpMessage.hpp"

namespace KIARA
{
namespace Transport
{

/// A response to be sent to a client.
class HttpResponse : public HttpMessage
{
public:

    /// The status of the response.
    enum StatusType
    {
        CONTINUE = 100,
        OK = 200,
        CREATED = 201,
        ACCEPTED = 202,
        NO_CONTENT = 204,
        MULTIPLE_CHOICES = 300,
        MOVED_PERMANENTLY = 301,
        MOVED_TEMPORARILY = 302,
        NOT_MODIFIED = 304,
        BAD_REQUEST = 400,
        UNAUTHORIZED = 401,
        FORBIDDEN = 403,
        NOT_FOUND = 404,
        EXPECTATION_FAILED = 417,
        INTERNAL_SERVER_ERROR = 500,
        NOT_IMPLEMENTED = 501,
        BAD_GATEWAY = 502,
        SERVICE_UNAVAILABLE = 503
    } status;

    HttpResponse(const Transport *transport);

    /// Set a stock response with HTML message
    void setDefaultHTMLResponse(StatusType status, bool keepAlive = true);

    void setTextResponse(const std::string &text, const std::string &mimeType = "text/plain", StatusType status = OK, bool keepAlive = true);

    void setDataResponse(const void *data, size_t dataSize, const std::string &mimeType = "text/plain", StatusType status = OK, bool keepAlive = true)
    {
        clear();
        getPayload().copy_mem(data, dataSize);
        setResponseHeaders(mimeType, status, keepAlive);
    }

    /// This function assumes that payload was set separately
    void setResponseHeaders(const std::string &mimeType = "text/plain", StatusType status = OK, bool keepAlive = true);

    /// Convert the response into a vector of buffers. The buffers do not own the
    /// underlying memory blocks, therefore the response object must remain valid and
    /// not be changed until the write operation has completed.
    void toBuffers(std::vector<boost::asio::const_buffer> &buffers);

    /// Clear all data
    void clear();
};

} // namespace Transport
} // namespace KIARA

#endif // KIARA_SERVER_HTTPRESPONSE_HPP_INCLUDED
