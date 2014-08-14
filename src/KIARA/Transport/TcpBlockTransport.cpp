/*
 * TcpBlockTransport.cpp
 *
 *  Created on: Feb 10, 2014
 *      Author: Dmitri Rubinstein
 */

#include "TcpBlockTransport.hpp"
#include <uriparser/Uri.h>
#include <boost/system/error_code.hpp>
#include <boost/lexical_cast.hpp>
#include <KIARA/Impl/Network.hpp>
#include <DFC/Base/Utils/StaticInit.hpp>
#include <KIARA/CDT/kr_dumpdata.h>
#include <KIARA/Utils/URL.hpp>
#include <sstream>

#define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

namespace KIARA
{
namespace Transport
{

namespace
{

static inline uint32_t getInt32LE(char data[4])
{
    return ((uint32_t)(unsigned char)(data[0]) |
            ((uint32_t)(unsigned char)(data[1]) << 8) |
            ((uint32_t)(unsigned char)(data[2]) << 16) |
            ((uint32_t)(unsigned char)(data[3]) << 24));
}

static inline void setInt32LE(uint32_t num, char data[4])
{
    data[0] = num & 0xff;
    data[1] = (num>>8) & 0xff;
    data[2] = (num>>16) & 0xff;
    data[3] = (num>>24) & 0xff;
}

} // unnamed namespace

/// TcpBlockTransport

extern "C" {

static ::KIARA_Connection * kiara_getConnection(::KIARA_FuncObj *funcObj)
{
    return funcObj->base.connection;
}

static ::KIARA_Connection * kiara_getServiceConnection(::KIARA_ServiceFuncObj *funcObj)
{
    return funcObj->base.connection;
}

static KIARA_Result kiara_sendDataTcp(::KIARA_Connection *conn, const void *msgData, size_t msgDataSize, kr_dbuffer_t *destBuf)
{
    KIARA_Result result = KIARA_SUCCESS;

    KIARA::Impl::ClientConnection *cconn = ((KIARA::Impl::ClientConnection*)conn);
    TcpBlockConnection::Ptr tcpConn =
        boost::static_pointer_cast<TcpBlockConnection>(cconn->getTransportConnection());

    KIARA::Transport::TcpBlockRequest req(tcpConn->getTransport());

    req.getPayload().set(
        const_cast<void*>(msgData),
        msgDataSize,
        msgDataSize,
        KIARA::DBuffer::dont_free_tag());

    boost::system::error_code ec;

    if (!tcpConn->send(req, &ec))
    {
        KIARA::Impl::unwrap(conn)->setError(KIARA_GENERIC_ERROR, boost::lexical_cast<std::string>(ec));
        result = KIARA_NETWORK_ERROR;
    }
    else
    {
        KIARA::Transport::TcpBlockResponse res(tcpConn->getTransport());

        if (!tcpConn->receive(res, &ec))
        {
            KIARA::Impl::unwrap(conn)->setError(KIARA_GENERIC_ERROR, boost::lexical_cast<std::string>(ec));
            result = KIARA_NETWORK_ERROR;
        }

        res.getPayload().swap(destBuf);

        result = KIARA_SUCCESS;
    }

    return result;
}

}

static NetworkHandler tcpNetworkHandler = {
    kiara_getConnection,
    kiara_getServiceConnection,
    kiara_sendDataTcp
};

/// TcpBlockMessage

TcpBlockMessage::TcpBlockMessage(const Transport *transport)
    : TransportMessage(transport)
{
}

/// TcpBlockAddress

namespace
{
boost::asio::ip::address resolveHostName(const std::string &hostName)
{
    boost::asio::ip::address result;
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::ip::tcp::resolver::query query(hostName, "");
    boost::asio::ip::tcp::resolver::iterator iter, end;
    for (iter = resolver.resolve(query); iter != end; ++iter)
    {
        result = (*iter).endpoint().address();
        break;
    }
    return result;
}
} // unnamed namespace

TcpBlockAddress::TcpBlockAddress(const std::string &hostName, unsigned short port, const Transport *transport)
    : TransportAddress(transport)
    , hostName_(hostName)
    , port_(port)
    , address_(resolveHostName(hostName))
{
}

unsigned short TcpBlockAddress::getPort() const
{
    return port_;
}

std::string TcpBlockAddress::getHostName() const
{
    return hostName_;
}

bool TcpBlockAddress::acceptConnection(const TransportAddress::Ptr &address) const
{
    if (!address || address->getTransport() != getTransport())
        return false;
    TcpBlockAddress::Ptr other = boost::static_pointer_cast<TcpBlockAddress>(address);

    if (other->address_ != address_)
    {
        std::string otherHostName = other->getHostName();
        std::string thisHostName = getHostName();

        if (otherHostName != thisHostName && thisHostName != "0.0.0.0")
            return false;
    }

    if (other->getPort() != getPort())
        return false;
    return true;
}

bool TcpBlockAddress::equals(const TransportAddress::Ptr &addr) const
{
    if (!addr || addr->getTransport() != getTransport())
        return false;
    TcpBlockAddress::Ptr other = boost::static_pointer_cast<TcpBlockAddress>(addr);
    return (other->address_ == address_ || other->hostName_ == hostName_) && other->port_ == port_;
}

std::string TcpBlockAddress::toString() const
{
    std::ostringstream oss;
    oss << getTransport()->getName() << "://" << hostName_ << ":" << port_;
    return oss.str();
}

/// TcpBlockTransport

TcpBlockTransport::TcpBlockTransport()
    : Transport("tcp", 10)
{
    setHttpTransport(false);
    setSecureTransport(false);
    setNetworkHandler(tcpNetworkHandler);
}

Connection::Ptr TcpBlockTransport::openConnection(
    const std::string &url,
    const NetworkContext::Ptr& ctx,
    boost::system::error_code *errorCode) const
{
    UriParserStateA state;
    UriUriA parsedUri;
    state.uri = &parsedUri;
    TcpBlockConnection::Ptr result;

    if (uriParseUriA(&state, url.c_str()) != URI_SUCCESS)
    {
        /* Failure */
        if (errorCode)
        {
            errorCode->assign(boost::system::errc::invalid_argument, boost::system::system_category());
        }
    }
    else
    {
        std::string protocol(parsedUri.scheme.first, parsedUri.scheme.afterLast);
        if (protocol != "tcp")
        {
            errorCode->assign(boost::system::errc::invalid_argument, boost::system::system_category());
        }
        else
        {
            result.reset(new TcpBlockConnection(ctx, this));

            if (parsedUri.hostText.first)
            {
                std::string hostName(parsedUri.hostText.first, parsedUri.hostText.afterLast);
                std::string port(parsedUri.portText.first, parsedUri.portText.afterLast);
                boost::system::error_code ec;
                DFC_DEBUG("Open TCP connection "+hostName+":"+port);
                result->open(hostName, port, &ec);
                if (ec)
                {
                    result.reset();
                    if (errorCode)
                        *errorCode = ec;
                }
            }
        }
    }
    uriFreeUriMembersA(&parsedUri);

    return result;
}

Connection::Ptr TcpBlockTransport::createConnection(
    const NetworkContext::Ptr& ctx,
    boost::system::error_code *errorCode) const
{
    return Connection::Ptr(new TcpBlockConnection(ctx, this));
}

TransportAddress::Ptr TcpBlockTransport::createAddress(const std::string &_url) const
{
    URL url(_url);
    if (!url.isValid() || url.scheme != getName())
        return TransportAddress::Ptr();

    return TransportAddress::Ptr(new TcpBlockAddress(url.host, boost::lexical_cast<unsigned short>(url.port), this));
}

NetworkContext::Ptr TcpBlockTransport::createContext() const
{
	return NetworkContext::Ptr();
}

/// TcpBlockConnection

TcpBlockConnection::TcpBlockConnection(const NetworkContext::Ptr& ctx, const Transport *transport)
    : TcpConnection(ctx)
    , request_(transport)
    , response_(transport)
{
}

TcpBlockConnection::~TcpBlockConnection()
{
}


bool TcpBlockConnection::open(const std::string &address, const std::string &port, boost::system::error_code *errorCode)
{
    boost::asio::ip::tcp::resolver resolver(getSocket().get_io_service());
    boost::asio::ip::tcp::resolver::query query(address, port);
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    boost::asio::ip::tcp::resolver::iterator end;

    boost::system::error_code error = boost::asio::error::host_not_found;
    while (error && endpoint_iterator != end)
    {
        getSocket().close();
        getSocket().connect(*endpoint_iterator++, error);
    }
    if (error)
    {
        if (errorCode)
            *errorCode = error;
        return false;
    }
    return true;
}

bool TcpBlockConnection::send(const Request &request, boost::system::error_code *errorCode)
{
    DFC_IFDEBUG(kr_dump_data("TcpBlockClientConnection::send: ", stderr, (unsigned char *)request.getPayload().data(), request.getPayloadSize(), 0));

    char blockSizeData[4];
    setInt32LE(request.getPayloadSize(), blockSizeData);

    boost::system::error_code error;

    boost::array<boost::asio::const_buffer, 2> bufs = {
        boost::asio::buffer(blockSizeData),
        boost::asio::buffer(request.getPayload().data(), request.getPayloadSize())
    };

    boost::asio::write(getSocket(), bufs, error);
    if (error)
    {
        if (errorCode)
            *errorCode = error;
        return false;
    }

    return true;
}

bool TcpBlockConnection::receive(Response &response, boost::system::error_code *errorCode)
{
    char blockSizeData[4];

    boost::system::error_code error;
    boost::asio::read(getSocket(), boost::asio::buffer(blockSizeData), error);
    if (error)
    {
        if (errorCode)
            *errorCode = error;
        return false;
    }

    size_t blockSize = getInt32LE(blockSizeData);

    response.getPayload().resize_nocopy(blockSize);

    if (blockSize)
    {
        boost::asio::read(getSocket(), boost::asio::buffer(response.getPayload().data(), response.getPayloadSize()), error);
        if (error)
        {
            if (errorCode)
                *errorCode = error;
            return false;
        }
    }

    return true;
}

void TcpBlockConnection::handleStart()
{
    DFC_DEBUG("Handling incoming request, TcpBlockConnection::handleStart()");
    readBlockSize();
}

void TcpBlockConnection::readBlockSize()
{
    boost::asio::async_read(
        getSocket(),
        boost::asio::buffer(blockSizeData_),
        boost::bind(
            &TcpBlockConnection::handleReadBlockSize,
            boost::static_pointer_cast<TcpBlockConnection>(shared_from_this()),
            boost::asio::placeholders::error));
}

void TcpBlockConnection::handleReadBlockSize(const boost::system::error_code& e)
{
    if (!e)
    {
        size_t blockSize = getInt32LE(blockSizeData_.data());
        blockData_.resize(blockSize);

        boost::asio::async_read(
            getSocket(),
            boost::asio::buffer(blockData_),
            boost::bind(
                &TcpBlockConnection::handleReadBlock,
                boost::static_pointer_cast<TcpBlockConnection>(shared_from_this()),
                boost::asio::placeholders::error));
    }
}

void TcpBlockConnection::handleReadBlock(const boost::system::error_code& e)
{
    if (!e)
    {
        request_.getPayload().set(
            blockData_.size() ? &blockData_[0] : 0,
            blockData_.size(), blockData_.size(),
            DBuffer::dont_free_tag());
        response_.clear();

        RequestResult requestResult = handleRequest(request_, response_);

        if (requestResult != SEND_RESPONSE)
        {
            response_.clear();
            start();
        }

        setInt32LE(response_.getPayloadSize(), blockSizeData_.data());

        boost::array<boost::asio::const_buffer, 2> bufs = {
          boost::asio::buffer(blockSizeData_),
          boost::asio::buffer(response_.getPayload().data(), response_.getPayloadSize()) };

        boost::asio::async_write(
            getSocket(),
            bufs,
            getStrand().wrap(
                boost::bind(&TcpBlockConnection::handleWriteBlock,
                            boost::static_pointer_cast<TcpBlockConnection>(shared_from_this()),
                            boost::asio::placeholders::error)));
    }
}

void TcpBlockConnection::handleWriteBlock(const boost::system::error_code& e)
{
    if (!e)
    {
        response_.clear();
        handleStart();
    }
}

static TcpBlockTransport tcpBlockTransport;

DFC_STATIC_INIT_FUNC
{
    Transport::registerTransport(&tcpBlockTransport);
}

} // namespace Transport
} // namespace KIARA
