/*
 * HttpRequest.cpp
 *
 *  Created on: Feb 14, 2014
 *      Author: Dmitri Rubinstein
 */

#include "HttpRequest.hpp"

namespace KIARA
{
namespace Transport
{

HttpRequest::HttpRequest(const Transport *transport)
    : HttpMessage(transport)
    , method()
    , uri()
    , httpVersionMajor(0)
    , httpVersionMinor(0)
{
}

void HttpRequest::clear()
{
    HttpMessage::clear();
    method.clear();
    uri.clear();
    httpVersionMajor = 0;
    httpVersionMinor = 0;
}

} // namespace Transport
} // namespace KIARA
