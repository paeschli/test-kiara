//
// reply.cpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "HttpResponse.hpp"
#include <string>
#include <boost/lexical_cast.hpp>
#include <algorithm>

namespace KIARA
{
namespace Transport
{

namespace StatusStrings
{

const std::string continue_ = "HTTP/1.1 100 Continue\r\n";
const std::string ok = "HTTP/1.1 200 OK\r\n";
const std::string created = "HTTP/1.1 201 Created\r\n";
const std::string accepted = "HTTP/1.1 202 Accepted\r\n";
const std::string no_content = "HTTP/1.1 204 No Content\r\n";
const std::string multiple_choices = "HTTP/1.1 300 Multiple Choices\r\n";
const std::string moved_permanently = "HTTP/1.1 301 Moved Permanently\r\n";
const std::string moved_temporarily = "HTTP/1.1 302 Moved Temporarily\r\n";
const std::string not_modified = "HTTP/1.1 304 Not Modified\r\n";
const std::string bad_request = "HTTP/1.1 400 Bad Request\r\n";
const std::string unauthorized = "HTTP/1.1 401 Unauthorized\r\n";
const std::string forbidden = "HTTP/1.1 403 Forbidden\r\n";
const std::string not_found = "HTTP/1.1 404 Not Found\r\n";
const std::string expectation_failed = "HTTP/1.1 417 Expectation Failed\r\n";
const std::string internal_server_error = "HTTP/1.1 500 Internal Server Error\r\n";
const std::string not_implemented = "HTTP/1.1 501 Not Implemented\r\n";
const std::string bad_gateway = "HTTP/1.1 502 Bad Gateway\r\n";
const std::string service_unavailable = "HTTP/1.1 503 Service Unavailable\r\n";

boost::asio::const_buffer toBuffer(HttpResponse::StatusType status)
{
    switch (status)
    {
        case HttpResponse::OK:
            return boost::asio::buffer(ok);
        case HttpResponse::CONTINUE:
            return boost::asio::buffer(continue_);
        case HttpResponse::CREATED:
            return boost::asio::buffer(created);
        case HttpResponse::ACCEPTED:
            return boost::asio::buffer(accepted);
        case HttpResponse::NO_CONTENT:
            return boost::asio::buffer(no_content);
        case HttpResponse::MULTIPLE_CHOICES:
            return boost::asio::buffer(multiple_choices);
        case HttpResponse::MOVED_PERMANENTLY:
            return boost::asio::buffer(moved_permanently);
        case HttpResponse::MOVED_TEMPORARILY:
            return boost::asio::buffer(moved_temporarily);
        case HttpResponse::NOT_MODIFIED:
            return boost::asio::buffer(not_modified);
        case HttpResponse::BAD_REQUEST:
            return boost::asio::buffer(bad_request);
        case HttpResponse::UNAUTHORIZED:
            return boost::asio::buffer(unauthorized);
        case HttpResponse::FORBIDDEN:
            return boost::asio::buffer(forbidden);
        case HttpResponse::NOT_FOUND:
            return boost::asio::buffer(not_found);
        case HttpResponse::EXPECTATION_FAILED:
            return boost::asio::buffer(expectation_failed);
        case HttpResponse::INTERNAL_SERVER_ERROR:
            return boost::asio::buffer(internal_server_error);
        case HttpResponse::NOT_IMPLEMENTED:
            return boost::asio::buffer(not_implemented);
        case HttpResponse::BAD_GATEWAY:
            return boost::asio::buffer(bad_gateway);
        case HttpResponse::SERVICE_UNAVAILABLE:
            return boost::asio::buffer(service_unavailable);
        default:
            return boost::asio::buffer(internal_server_error);
    }
}

} // namespace StatusStrings

namespace MiscStrings
{

const char name_value_separator[] = { ':', ' ' };
const char crlf[] = { '\r', '\n' };

} // namespace MiscStrings

void HttpResponse::toBuffers(std::vector<boost::asio::const_buffer> &buffers)
{
    buffers.push_back(StatusStrings::toBuffer(status));
    HttpMessage::toBuffers(buffers);
}

namespace StockReplies
{

const char continue_[] = "";
const char ok[] = "";
const char created[] = "<html>"
    "<head><title>Created</title></head>"
    "<body><h1>201 Created</h1></body>"
    "</html>";
const char accepted[] = "<html>"
    "<head><title>Accepted</title></head>"
    "<body><h1>202 Accepted</h1></body>"
    "</html>";
const char no_content[] = "<html>"
    "<head><title>No Content</title></head>"
    "<body><h1>204 Content</h1></body>"
    "</html>";
const char multiple_choices[] = "<html>"
    "<head><title>Multiple Choices</title></head>"
    "<body><h1>300 Multiple Choices</h1></body>"
    "</html>";
const char moved_permanently[] = "<html>"
    "<head><title>Moved Permanently</title></head>"
    "<body><h1>301 Moved Permanently</h1></body>"
    "</html>";
const char moved_temporarily[] = "<html>"
    "<head><title>Moved Temporarily</title></head>"
    "<body><h1>302 Moved Temporarily</h1></body>"
    "</html>";
const char not_modified[] = "<html>"
    "<head><title>Not Modified</title></head>"
    "<body><h1>304 Not Modified</h1></body>"
    "</html>";
const char bad_request[] = "<html>"
    "<head><title>Bad Request</title></head>"
    "<body><h1>400 Bad Request</h1></body>"
    "</html>";
const char unauthorized[] = "<html>"
    "<head><title>Unauthorized</title></head>"
    "<body><h1>401 Unauthorized</h1></body>"
    "</html>";
const char forbidden[] = "<html>"
    "<head><title>Forbidden</title></head>"
    "<body><h1>403 Forbidden</h1></body>"
    "</html>";
const char not_found[] = "<html>"
    "<head><title>Not Found</title></head>"
    "<body><h1>404 Not Found</h1></body>"
    "</html>";
const char expectation_failed[] = "<html>"
    "<head><title>Expectation Failed</title></head>"
    "<body><h1>417 Expectation Failed</h1></body>"
    "</html>";
const char internal_server_error[] = "<html>"
    "<head><title>Internal Server Error</title></head>"
    "<body><h1>500 Internal Server Error</h1></body>"
    "</html>";
const char not_implemented[] = "<html>"
    "<head><title>Not Implemented</title></head>"
    "<body><h1>501 Not Implemented</h1></body>"
    "</html>";
const char bad_gateway[] = "<html>"
    "<head><title>Bad Gateway</title></head>"
    "<body><h1>502 Bad Gateway</h1></body>"
    "</html>";
const char service_unavailable[] = "<html>"
    "<head><title>Service Unavailable</title></head>"
    "<body><h1>503 Service Unavailable</h1></body>"
    "</html>";

const char * toString(HttpResponse::StatusType status)
{
    switch (status)
    {
        case HttpResponse::CONTINUE:
            return continue_;
        case HttpResponse::OK:
            return ok;
        case HttpResponse::CREATED:
            return created;
        case HttpResponse::ACCEPTED:
            return accepted;
        case HttpResponse::NO_CONTENT:
            return no_content;
        case HttpResponse::MULTIPLE_CHOICES:
            return multiple_choices;
        case HttpResponse::MOVED_PERMANENTLY:
            return moved_permanently;
        case HttpResponse::MOVED_TEMPORARILY:
            return moved_temporarily;
        case HttpResponse::NOT_MODIFIED:
            return not_modified;
        case HttpResponse::BAD_REQUEST:
            return bad_request;
        case HttpResponse::UNAUTHORIZED:
            return unauthorized;
        case HttpResponse::FORBIDDEN:
            return forbidden;
        case HttpResponse::NOT_FOUND:
            return not_found;
        case HttpResponse::EXPECTATION_FAILED:
            return expectation_failed;
        case HttpResponse::INTERNAL_SERVER_ERROR:
            return internal_server_error;
        case HttpResponse::NOT_IMPLEMENTED:
            return not_implemented;
        case HttpResponse::BAD_GATEWAY:
            return bad_gateway;
        case HttpResponse::SERVICE_UNAVAILABLE:
            return service_unavailable;
        default:
            return internal_server_error;
    }
}

} // namespace stock_replies


HttpResponse::HttpResponse(const Transport * transport)
    : HttpMessage(transport)
    , status(HttpResponse::OK)
{
}

void HttpResponse::setDefaultHTMLResponse(StatusType _status, bool keepAlive)
{
    clear();
    status = _status;
    const char * statusResponse = StockReplies::toString(_status);
    getPayload().copy_mem(statusResponse, strlen(statusResponse));
    getHeaders().resize(keepAlive ? 3 : 2);
    getHeaders()[0].name = "Content-Length";
    getHeaders()[0].value = boost::lexical_cast<std::string>(getPayload().size());
    getHeaders()[1].name = "Content-Type";
    getHeaders()[1].value = "text/html";
    if (keepAlive)
    {
        getHeaders()[2].name = "Connection";
        getHeaders()[2].value = "keep-alive";
    }
}

void HttpResponse::setTextResponse(const std::string &text, const std::string &mimeType, StatusType _status, bool keepAlive)
{
    setDataResponse(text.data(), text.length(), mimeType, _status, keepAlive);
}

void HttpResponse::setResponseHeaders(const std::string &mimeType, StatusType _status, bool keepAlive)
{
    getHeaders().clear();
    status = _status;
    getHeaders().resize(keepAlive ? 3 : 2);
    getHeaders()[0].name = "Content-Length";
    getHeaders()[0].value = boost::lexical_cast<std::string>(getPayload().size());
    getHeaders()[1].name = "Content-Type";
    getHeaders()[1].value = mimeType;
    if (keepAlive)
    {
        getHeaders()[2].name = "Connection";
        getHeaders()[2].value = "keep-alive";
    }
}

void HttpResponse::clear()
{
    HttpMessage::clear();
    status = OK;
}

} // namespace Transport
} // namespace KIARA
