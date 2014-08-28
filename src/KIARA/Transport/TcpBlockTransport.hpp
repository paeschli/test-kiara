/*
 * TcpBlockTransport.hpp
 *
 *  Created on: Feb 10, 2014
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_TRANSPORT_TCPBLOCKTRANSPORT_HPP_INCLUDED
#define KIARA_TRANSPORT_TCPBLOCKTRANSPORT_HPP_INCLUDED

#include <KIARA/Common/stdint.h>
#include <KIARA/Utils/DBuffer.hpp>
#include "Transport.hpp"
#include "KT_Zeromq.hpp"
#include <boost/asio/ip/address.hpp>

namespace KIARA
{
namespace Transport
{

class TcpBlockMessage: public TransportMessage
{
public:
    TcpBlockMessage(const Transport *transport);
};


class TcpBlockAddress: public TransportAddress
{
public:

    typedef boost::shared_ptr<TcpBlockAddress> Ptr;

    TcpBlockAddress(const std::string &hostName, unsigned short port, const Transport *transport);

    std::string getHostName() const;

    unsigned short getPort() const;

    bool acceptConnection(const TransportAddress::Ptr &address) const;

    bool equals(const TransportAddress::Ptr &other) const;

    std::string toString() const;

private:
    std::string hostName_;
    unsigned short port_;
    boost::asio::ip::address address_;
};

class TcpBlockTransport: public Transport
{
public:

    TcpBlockTransport();

    Connection::Ptr openConnection(
        const std::string &url,
        const NetworkContext::Ptr& ctx,
        boost::system::error_code *errorCode = 0) const;

    Connection::Ptr createConnection(
    	const NetworkContext::Ptr& ctx,
        boost::system::error_code *errorCode = 0) const;

    TransportAddress::Ptr createAddress(const std::string &url) const;

    NetworkContext::Ptr createContext() const;

};

class TcpZmqConnection: public Connection
{
public:

    typedef boost::shared_ptr<TcpZmqConnection> Ptr;

    explicit TcpZmqConnection() { }

    ~TcpZmqConnection() { }

    virtual void handleStart() { }

    std::string getRemoteHostName() const { return ""; }

    unsigned short getRemotePort() const { return 0; }

    std::string getLocalHostName() const { return ""; }

    unsigned short getLocalPort() const { return 0; }
	
	KT_Connection* connection;
	
	KT_Session* session;
};

class TcpBlockConnection;
typedef boost::shared_ptr<TcpBlockConnection> TcpBlockConnectionPtr;

typedef TcpBlockMessage TcpBlockRequest;
typedef TcpBlockMessage TcpBlockResponse;

/// Tcp connection transfer with sized blocks
class TcpBlockConnection: public TcpConnection
{
public:
    typedef TcpBlockConnectionPtr Ptr;

    typedef TcpBlockRequest Request;
    typedef TcpBlockResponse Response;

    explicit TcpBlockConnection(const NetworkContext::Ptr& ctx, const Transport *transport);
	
	/*explicit TcpBlockConnection(const NetworkContext::Ptr& ctx, char *transport, char *supper);*/

    virtual ~TcpBlockConnection();

    const Transport * getTransport() const { return request_.getTransport(); }

    bool open(const std::string &address, const std::string &port, boost::system::error_code *errorCode = 0);

    bool send(const Request &request, boost::system::error_code *errorCode = 0);

    bool receive(Response &response, boost::system::error_code *errorCode = 0);

    /// Start the first asynchronous operation for the connection.
    void handleStart();
	
	std::string host;

private:

    void readBlockSize();

    void handleReadBlockSize(const boost::system::error_code& e);

    void handleReadBlock(const boost::system::error_code& e);

    void handleWriteBlock(const boost::system::error_code& e);

    boost::array<char, 4> blockSizeData_;
    std::vector<char> blockData_;

    /// The incoming request.
    Request request_;

    /// The reply to be sent back to the client.
    Response response_;
};

} // namespace Transport
} // namespace KIARA

#endif /* KIARA_TRANSPORT_TCPBLOCKTRANSPORT_HPP_INCLUDED */
