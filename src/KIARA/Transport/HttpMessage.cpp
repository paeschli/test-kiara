/*
 * HttpMessage.cpp
 *
 *  Created on: Feb 14, 2014
 *      Author: Dmitri Rubinstein
 */

#include "HttpMessage.hpp"
#include <boost/lexical_cast.hpp>

namespace KIARA
{
namespace Transport
{

namespace
{
    struct MatchHeader
    {
        MatchHeader(const std::string &name)
            : name_(name)
        { }

        bool operator ()(const Header &header) const
        {
            return boost::algorithm::iequals(header.name, name_);
        }

    private:
        const std::string &name_;
    };
}

namespace MiscStrings
{

const char name_value_separator[] = { ':', ' ' };
const char crlf[] = { '\r', '\n' };

} // namespace MiscStrings

HttpMessage::HttpMessage(const Transport *transport)
    : TransportMessage(transport)
{
}

void HttpMessage::toBuffers(std::vector<boost::asio::const_buffer> &buffers)
{
    for (std::size_t i = 0; i < headers_.size(); ++i)
    {
        Header& h = headers_[i];
        buffers.push_back(boost::asio::buffer(h.name));
        buffers.push_back(boost::asio::buffer(MiscStrings::name_value_separator));
        buffers.push_back(boost::asio::buffer(h.value));
        buffers.push_back(boost::asio::buffer(MiscStrings::crlf));
    }
    buffers.push_back(boost::asio::buffer(MiscStrings::crlf));
    buffers.push_back(boost::asio::buffer(getPayload().data(), getPayload().size()));
}

void HttpMessage::setHeader(const std::string &name, const std::string &value)
{
    std::vector<Header>::iterator it = std::find_if(headers_.begin(), headers_.end(), MatchHeader(name));
    if (it != headers_.end())
        it->value = value;
    else
        headers_.push_back(Header(name, value));
}

void HttpMessage::addHeader(const std::string &name, const std::string &value)
{
    headers_.push_back(Header(name, value));
}

bool HttpMessage::getContentLength(size_t &contentLength)
{
    HeaderList::const_iterator it = findHeader("Content-Length");
    if (it == headers_.end())
        return false;
    try
    {
        contentLength = boost::lexical_cast<size_t>(it->value);
    }
    catch (const boost::bad_lexical_cast &e)
    {
        return false;
    }
    return true;
}

size_t HttpMessage::computeContentLength()
{
    size_t payloadSize = getPayload().size();
    setHeader("Content-Length", boost::lexical_cast<std::string>(payloadSize));
    return payloadSize;
}

void HttpMessage::clear()
{
    TransportMessage::clear();
    headers_.clear();
}

} // namespace Transport
} // namespace KIARA
