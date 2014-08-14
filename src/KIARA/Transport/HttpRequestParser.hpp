//
// request_parser.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef KIARA_TRANSPORT_HTTPREQUESTPARSER_HPP_INCLUDED
#define KIARA_TRANSPORT_HTTPREQUESTPARSER_HPP_INCLUDED

#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/scoped_ptr.hpp>
#include <iterator>
#include <utility>
#include "HttpRequest.hpp"

namespace KIARA
{
namespace Transport
{

/// Parser for incoming requests.
class HttpRequestParser
{
public:
    /// Construct ready to parse the request method.
    HttpRequestParser();

    ~HttpRequestParser();

    /// Reset to initial parser state.
    void reset();

    enum Status
    {
        PARSING_FAILED,
        PARSING_HEADERS,
        READING_CONTENT,
        PARSING_FINISHED
    };

    Status parse(HttpRequest& req, const char *data, size_t len, size_t &parsedLen);

private:
    class PrivateImpl;
    friend class PrivateImpl;
    boost::scoped_ptr<PrivateImpl> pimpl_;
};

} // namespace Transport
} // namespace KIARA

#endif // KIARA_SERVER_HTTPREQUESTPARSER_HPP_INCLUDED
