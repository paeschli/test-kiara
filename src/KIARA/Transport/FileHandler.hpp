//
// request_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef KIARA_TRANSPORT_FILEHANDLER_HPP_INCLUDED
#define KIARA_TRANSPORT_FILEHANDLER_HPP_INCLUDED

#include "HttpRequestHandler.hpp"
#include <string>
#include <boost/noncopyable.hpp>

namespace KIARA
{
namespace Transport
{

struct HttpResponse;
struct HttpRequest;

/// The common handler for all incoming requests.
class FileHandler: public HttpRequestHandler
{
public:
    /// Construct with a directory containing files to be served.
    explicit FileHandler(const std::string& docRoot);

    /// Handle a request and produce a reply.
    void handleRequest(const HttpRequest& req, HttpResponse& rep);

private:
    /// The directory containing the files to be served.
    std::string docRoot_;
};

} // namespace Transport
} // namespace KIARA

#endif // KIARA_SERVER_FILE_HANDLER_HPP_INCLUDED
