/*
 * TransportMessage.cpp
 *
 *  Created on: Feb 14, 2014
 *      Author: Dmitri Rubinstein
 */

#include "TransportMessage.hpp"

namespace KIARA
{
namespace Transport
{

TransportMessage::TransportMessage(const Transport *transport)
    : transport_(transport)
{ }

TransportMessage::~TransportMessage() { }

void TransportMessage::clear()
{
    payload_.clear();
}

} // namespace KIARA
} // namespace Transport
