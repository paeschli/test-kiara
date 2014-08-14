/*
 * Transport.cpp
 *
 *  Created on: Feb 14, 2014
 *      Author: Dmitri Rubinstein
 */

#include "Transport.hpp"
#include <DFC/Base/Utils/StaticInit.hpp>
#include <KIARA/Utils/URL.hpp>
#include <boost/assert.hpp>

#define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

namespace KIARA
{
namespace Transport
{

NetworkContext::~NetworkContext() {}
AsioNetworkContext::~AsioNetworkContext() {}

/// ConnectionHandler

ConnectionHandler::~ConnectionHandler()
{
}

/// Connection

Connection::Connection(ConnectionHandler * connectionHandler)
    : connectionHandler_(connectionHandler)
{
}

Connection::~Connection()
{
    if (connectionHandler_)
        connectionHandler_->onDestroy(this);
}

void Connection::setConnectionHandler(ConnectionHandler * connectionHandler)
{
    connectionHandler_ = connectionHandler;
}


/// TcpConnection

TcpConnection::TcpConnection(const NetworkContext::Ptr& ctx)
    : strand_(boost::dynamic_pointer_cast<AsioNetworkContext>(ctx)->getIoService()),
      socket_(boost::dynamic_pointer_cast<AsioNetworkContext>(ctx)->getIoService())
{
}

TcpConnection::~TcpConnection()
{
}

std::string TcpConnection::getRemoteHostName() const
{
    boost::system::error_code ec;
    boost::asio::ip::tcp::socket::endpoint_type endpoint = socket_.remote_endpoint(ec);
    return !ec ? endpoint.address().to_string() : "";
}

unsigned short TcpConnection::getRemotePort() const
{
    boost::system::error_code ec;
    boost::asio::ip::tcp::socket::endpoint_type endpoint = socket_.remote_endpoint(ec);
    return !ec ? endpoint.port() : 0;
}

std::string TcpConnection::getLocalHostName() const
{
    boost::system::error_code ec;
    boost::asio::ip::tcp::socket::endpoint_type endpoint = socket_.local_endpoint(ec);
    return !ec ? endpoint.address().to_string() : "";
}

unsigned short TcpConnection::getLocalPort() const
{
    boost::system::error_code ec;
    boost::asio::ip::tcp::socket::endpoint_type endpoint = socket_.local_endpoint(ec);
    return !ec ? endpoint.port() : 0;
}

/// BufferedTcpConnection

BufferedTcpConnection::BufferedTcpConnection(const NetworkContext::Ptr& ctx, size_t bufferSize)
    : TcpConnection(ctx), buffer_(bufferSize)
{
}

BufferedTcpConnection::~BufferedTcpConnection()
{
}

void BufferedTcpConnection::startReadBuffer()
{
    DFC_DEBUG("BufferedTcpConnection::startReadBuffer() called");
    getSocket().async_read_some(boost::asio::buffer(buffer_),
        getStrand().wrap(
            boost::bind(&BufferedTcpConnection::handleRead,
                        boost::static_pointer_cast<BufferedTcpConnection>(shared_from_this()), boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred)));
}

/// TransportAddress

TransportAddress::TransportAddress(const Transport *transport)
    : transport_(transport)
{
}

TransportAddress::~TransportAddress()
{
}

/// Transport

Transport::Transport(const char * name, int priority)
    : name_(name)
    , priority_(priority)
{
}

Transport::~Transport()
{
}

const Transport * Transport::getTransportByName(const char *transportName)
{
    TransportMap::iterator it = transports_.find(transportName);
    return it != transports_.end() ? it->second : 0;
}

const Transport * Transport::getTransportByURL(const std::string &url)
{
    std::string scheme;
    if (!URL::getScheme(url, scheme))
        return 0;
    return getTransportByName(scheme.c_str());
}

void Transport::registerTransport(const Transport *transport)
{
    BOOST_ASSERT(transport != 0);
    transports_[transport->getName()] = transport;
}

Transport::TransportMap Transport::transports_;

} // namespace Transport
} // namespace KIARA
