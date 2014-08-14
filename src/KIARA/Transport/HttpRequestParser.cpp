//
// request_parser.cpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "HttpRequestParser.hpp"
#include "HttpRequest.hpp"
#include "http_parser.h"
#include <KIARA/CDT/kr_dumpdata.h>

// #define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

namespace KIARA
{
namespace Transport
{

class HttpRequestParser::PrivateImpl
{
public:
    http_parser parser;
    http_parser_settings settings;

    bool lastWasValue;
    Header header;
    HttpRequestParser::Status status;

    HttpRequest *request;

    PrivateImpl(HttpRequestParser &parent)
        : lastWasValue(false)
        , status(HttpRequestParser::PARSING_HEADERS)
        , request(0)
    {
        http_parser_init(&parser, HTTP_REQUEST);
        parser.data = this;
        settings.on_message_begin = messageBeginCB;
        settings.on_url = urlCB;
        settings.on_status = statusCB;
        settings.on_header_field = headerFieldCB;
        settings.on_header_value = headerValueCB;
        settings.on_headers_complete = headersCompleteCB;
        settings.on_body = bodyCB;
        settings.on_message_complete = messageCompleteCB;
    }

    void reset()
    {
        http_parser_init(&parser, HTTP_REQUEST);
        parser.data = this;
    }

    size_t execute(HttpRequest &request, const char *data, size_t len)
    {
        this->request = &request;
        size_t result = http_parser_execute(&parser, &settings, data, len);
        this->request = 0;
        return result;
    }

    void addHeader()
    {
        // smart addition
        request->getHeaders().resize(request->getHeaders().size()+1);
        request->getHeaders().back().swap(header);
    }

    static int messageBeginCB(http_parser *parser)
    {
        PrivateImpl *This = reinterpret_cast<PrivateImpl*>(parser->data);
        This->request->clear();
        This->status = HttpRequestParser::PARSING_HEADERS;

        DFC_DEBUG("BEGIN MESSAGE");

        return 0;
    }

    static int urlCB(http_parser *parser, const char *at, size_t length)
    {
        PrivateImpl *This = reinterpret_cast<PrivateImpl*>(parser->data);
        This->request->uri.append(at, at+length);
        return 0;
    }

    static int statusCB(http_parser *parser, const char *at, size_t length)
    {
        PrivateImpl *This = reinterpret_cast<PrivateImpl*>(parser->data);
        // TODO store to the request
        return 0;
    }

    static int headerFieldCB(http_parser *parser, const char *at, size_t length)
    {
        PrivateImpl *This = reinterpret_cast<PrivateImpl*>(parser->data);

        if (This->lastWasValue)
        {
            This->addHeader();

            //if (request->getHeaders().size() == MAX_HEADER_LINES) ;// error!

            This->header.value.clear();
            This->header.name.assign(at, at+length);
        }
        else
        {
            assert(This->header.value.length() == 0);

            This->header.name.append(at, at+length);
        }

        This->lastWasValue = false;

        return 0;
    }

    static int headerValueCB(http_parser *parser, const char *at, size_t length)
    {
        PrivateImpl *This = reinterpret_cast<PrivateImpl*>(parser->data);

        if (!This->lastWasValue)
        {
            This->header.value.assign(at, at+length);
        }
        else
        {
            This->header.value.append(at, at+length);
        }

        This->lastWasValue = true;

        return 0;
    }

    static int headersCompleteCB(http_parser *parser)
    {
        PrivateImpl *This = reinterpret_cast<PrivateImpl*>(parser->data);
        if (This->lastWasValue)
        {
            This->addHeader();
            This->lastWasValue = false;
            This->status = HttpRequestParser::READING_CONTENT;
        }
        return 0;
    }

    static int bodyCB(http_parser *parser, const char *at, size_t length)
    {
        PrivateImpl *This = reinterpret_cast<PrivateImpl*>(parser->data);
        This->request->getPayload().append_mem(at, length);
        return 0;
    }

    static int messageCompleteCB(http_parser *parser)
    {
        PrivateImpl *This = reinterpret_cast<PrivateImpl*>(parser->data);

        This->status = HttpRequestParser::PARSING_FINISHED;

        This->request->method = http_method_str((enum http_method)parser->method);
        This->request->httpVersionMajor = parser->http_major;
        This->request->httpVersionMinor = parser->http_minor;

        DFC_DEBUG("Method: "<<This->request->method);
        DFC_DEBUG("URI: "<<This->request->uri);
        DFC_DEBUG("HTTP VERSION MAJOR: "<<This->request->httpVersionMajor);
        DFC_DEBUG("HTTP VERSION MINOR: "<<This->request->httpVersionMinor);
        {
            for (HttpMessage::HeaderList::const_iterator it = This->request->headers_begin(),
                end = This->request->headers_end(); it != end; ++it)
            {
                DFC_DEBUG("HEADER "<<it->name<<" : "<<it->value);
            }
            //buffers.push_back(boost::asio::buffer(getPayload().data(), getPayload().size()));
        }
        DFC_IFDEBUG(kr_dump_data("body: ", stderr, (unsigned char*)This->request->getPayload().data(), This->request->getPayloadSize(), 1));
        DFC_DEBUG("http_parser: MESSAGE COMPLETE !");

        return 0;
    }
};

HttpRequestParser::HttpRequestParser()
    : pimpl_(new PrivateImpl(*this))
{
}

HttpRequestParser::~HttpRequestParser()
{
}

void HttpRequestParser::reset()
{
    pimpl_->reset();
}

HttpRequestParser::Status HttpRequestParser::parse(HttpRequest& req, const char *data, size_t len, size_t &parsedLen)
{
    parsedLen = pimpl_->execute(req, data, len);
    if (pimpl_->parser.upgrade)
    {
        /* handle new protocol */
        DFC_DEBUG("http_parser: upgrade");
    }
    else if (parsedLen != len)
    {
        /* Handle error. Usually just close the connection. */
        DFC_DEBUG("http_parser: error: "<<parsedLen<<" != "<<len);
        return PARSING_FAILED;
    }

    return pimpl_->status;
}

} // namespace Transport
} // namespace KIARA
