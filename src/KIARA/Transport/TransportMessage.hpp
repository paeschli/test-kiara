/*
 * TransportMessage.hpp
 *
 *  Created on: Feb 14, 2014
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_TRANSPORT_TRANSPORTMESSAGE_HPP_INCLUDED
#define KIARA_TRANSPORT_TRANSPORTMESSAGE_HPP_INCLUDED

#include <KIARA/Utils/DBuffer.hpp>

namespace KIARA
{
namespace Transport
{

class Transport;

class TransportMessage
{
public:

    virtual ~TransportMessage();

    const Transport * getTransport() const { return transport_; }

    const DBuffer & getPayload() const { return payload_; }

    DBuffer & getPayload() { return payload_; }

    size_t getPayloadSize() const { return payload_.size(); }

    /// Clear all data
    virtual void clear();

protected:
    TransportMessage(const Transport *transport);
private:
    const Transport * transport_;
    DBuffer payload_;
};

} // Transport
} // KIARA

#endif /* KIARA_TRANSPORT_TRANSPORTMESSAGE_HPP_INCLUDED */
