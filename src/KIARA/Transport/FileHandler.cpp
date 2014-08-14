//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "FileHandler.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "MimeTypes.hpp"
#include "HttpResponse.hpp"
#include "HttpRequest.hpp"

namespace KIARA
{
namespace Transport
{

FileHandler::FileHandler(const std::string& docRoot)
    : docRoot_(docRoot)
{
}

void FileHandler::handleRequest(const HttpRequest& req, HttpResponse& rep)
{
    // Decode url to path.
    std::string request_path;
    if (!decodeURL(req.uri, request_path))
    {
        rep.setDefaultHTMLResponse(HttpResponse::BAD_REQUEST);
        return;
    }

    // Request path must be absolute and not contain "..".
    if (request_path.empty() || request_path[0] != '/' || request_path.find("..") != std::string::npos)
    {
        rep.setDefaultHTMLResponse(HttpResponse::BAD_REQUEST);
        return;
    }

    // If path ends in slash (i.e. is a directory) then add "index.html".
    if (request_path[request_path.size() - 1] == '/')
    {
        request_path += "index.html";
    }

    // Determine the file extension.
    std::size_t last_slash_pos = request_path.find_last_of("/");
    std::size_t last_dot_pos = request_path.find_last_of(".");
    std::string extension;
    if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
    {
        extension = request_path.substr(last_dot_pos + 1);
    }

    // Open the file to send back.
    std::string full_path = docRoot_ + request_path;
    std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
    if (!is)
    {
        rep.setDefaultHTMLResponse(HttpResponse::NOT_FOUND);
        return;
    }

    // Fill out the reply to be sent to the client.
    rep.status = HttpResponse::OK;
    char buf[512];
    while (is.read(buf, sizeof(buf)).gcount() > 0)
        rep.getPayload().append_mem(buf, is.gcount());
    rep.getHeaders().resize(2);
    rep.getHeaders()[0].name = "Content-Length";
    rep.getHeaders()[0].value = boost::lexical_cast<std::string>(rep.getPayload().size());
    rep.getHeaders()[1].name = "Content-Type";
    rep.getHeaders()[1].value = MimeTypes::extensionToType(extension);
}

} // namespace Transport
} // namespace KIARA
