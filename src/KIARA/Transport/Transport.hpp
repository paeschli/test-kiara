/*
 * Transport.hpp
 *
 *  Created on: Feb 14, 2014
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_TRANSPORT_TRANSPORT_HPP_INCLUDED
#define KIARA_TRANSPORT_TRANSPORT_HPP_INCLUDED

#include <KIARA/Impl/API.h>
#include "TransportMessage.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>

#define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

namespace KIARA
{
namespace Transport
{

struct NetworkHandler
{
    KIARA_GetConnection getConnection;
    KIARA_GetServiceConnection getServiceConnection;
    KIARA_SendData sendData;
};

class Connection;
typedef boost::shared_ptr<Connection> ConnectionPtr;

// Forward declaration
class NetworkContext;
typedef boost::shared_ptr<NetworkContext> NetworkContextPtr;
class AsioNetworkContext;
typedef boost::shared_ptr<AsioNetworkContext> AsioNetworkContextPtr;

// Base class for the network context object
class NetworkContext
{
public:
	typedef NetworkContextPtr Ptr;
	virtual ~NetworkContext();
};

// Derived class for the network context object for Boost.Asio
class AsioNetworkContext : public NetworkContext
{
public:
	~AsioNetworkContext();
	typedef AsioNetworkContextPtr Ptr;
	// Get a reference of the io_service
	boost::asio::io_service& getIoService () {return io_service_;}

private:
	boost::asio::io_service io_service_;
};

enum RequestResult
{
    REQUEST_FAILED,
    SEND_RESPONSE,
    ASYNC_RESPONSE
};

class ConnectionHandler
{
public:

    virtual ~ConnectionHandler();

    virtual void onStart(const ConnectionPtr &connection) { }

    virtual RequestResult onRequest(
        const ConnectionPtr &connection,
        const TransportMessage &request,
        TransportMessage &response) = 0;

    virtual void onDestroy(const Connection *connection) { }

};

/// Represents a single connection
class Connection: public boost::enable_shared_from_this<Connection>, private boost::noncopyable
{
public:

    typedef ConnectionPtr Ptr;

    Connection(ConnectionHandler * connectionHandler = 0);

    virtual ~Connection();

    virtual std::string getRemoteHostName() const = 0;

    virtual unsigned short getRemotePort() const = 0;

    virtual std::string getLocalHostName() const = 0;

    virtual unsigned short getLocalPort() const = 0;

    /// Start the first asynchronous operation for the connection.
    /// Don't call multiple times !!!
    void start()
    {
        DFC_DEBUG("Handling incoming request");
        if (connectionHandler_)
        {
            DFC_DEBUG("connectionHandler_ is !NULL");
            connectionHandler_->onStart(shared_from_this());
        }
        DFC_DEBUG("handleStart()");
        handleStart();
    }

    ConnectionHandler * getConnectionHandler() const { return connectionHandler_; }

    void setConnectionHandler(ConnectionHandler * connectionHandler);

protected:

    virtual void handleStart() = 0;

    RequestResult handleRequest(const TransportMessage &request, TransportMessage &response)
    {
        return connectionHandler_ ? connectionHandler_->onRequest(shared_from_this(), request, response) : REQUEST_FAILED;
    }

private:
    ConnectionHandler * connectionHandler_;
};


class TcpConnection;
typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;

/// Represents a single connection from a client.
class TcpConnection: public Connection
{
public:

    typedef TcpConnectionPtr Ptr;

    explicit TcpConnection(const NetworkContext::Ptr&);

    virtual ~TcpConnection();

    std::string getRemoteHostName() const;

    unsigned short getRemotePort() const;

    std::string getLocalHostName() const;

    unsigned short getLocalPort() const;

    /// Get the socket associated with the connection.
    boost::asio::ip::tcp::socket& getSocket() { return socket_; }

    boost::asio::io_service::strand& getStrand() { return strand_; }

    /// Start the first asynchronous operation for the connection.
    virtual void handleStart() = 0;

private:

    /// Strand to ensure the connection's handlers are not called concurrently.
    boost::asio::io_service::strand strand_;

    /// Socket for the connection.
    boost::asio::ip::tcp::socket socket_;
};

class BufferedTcpConnection;
typedef boost::shared_ptr<BufferedTcpConnection> BufferedTcpConnectionPtr;

class BufferedTcpConnection: public TcpConnection
{
public:

    typedef BufferedTcpConnectionPtr Ptr;

    explicit BufferedTcpConnection(const NetworkContext::Ptr& ctx, size_t bufferSize = 8192);

    virtual ~BufferedTcpConnection();

    const std::vector<char> & getBuffer() const { return buffer_; }

protected:

    char * getBufferData() { return buffer_.size() ? &buffer_[0] : 0; }

    const char * getBufferData() const { return buffer_.size() ? &buffer_[0] : 0; }

    size_t getBufferSize() const { return buffer_.size(); }

    /// Start read operation to the buffer, will call handleRead on completion
    void startReadBuffer();

    /// Start write operation of specified buffers, will call handleWrite on completion
    template <typename ConstBufferSequence>
    void startWriteBuffers(const ConstBufferSequence &buffers)
    {
        boost::asio::async_write(getSocket(), buffers,
            getStrand().wrap(
                boost::bind(&BufferedTcpConnection::handleWrite,
                            boost::static_pointer_cast<BufferedTcpConnection>(shared_from_this()),
                            boost::asio::placeholders::error)));
    }

    /// Handle completion of a read operation.
    virtual void handleRead(const boost::system::error_code& e, std::size_t bytes_transferred) = 0;

    /// Handle completion of a write operation.
    virtual void handleWrite(const boost::system::error_code& e) = 0;

private:

    /// Buffer for incoming data.
    std::vector<char> buffer_;
};

class Transport;

class TransportAddress;
typedef boost::shared_ptr<TransportAddress> TransportAddressPtr;

class TransportAddress
{
public:

    typedef TransportAddressPtr Ptr;

    TransportAddress(const Transport *transport);

    virtual ~TransportAddress();

    const Transport * getTransport() const { return transport_; }

    virtual unsigned short getPort() const = 0;

    virtual std::string getHostName() const = 0;

    virtual bool acceptConnection(const TransportAddress::Ptr &address) const = 0;

    virtual bool equals(const TransportAddress::Ptr &other) const = 0;

    virtual std::string toString() const = 0;

private:
    const Transport *transport_;
};

class Transport
{
public:

    virtual ~Transport();

    const char * getName() const { return name_; }

    // the higher the value the lower the priority
    int getPriority() const { return priority_; }

    bool isHttpTransport() const { return httpTransport_; }

    bool isSecureTransport() const { return secureTransport_; }

    const NetworkHandler & getNetworkHandler() const { return networkHandler_; }

    virtual Connection::Ptr openConnection(
        const std::string &url,
        const NetworkContext::Ptr& ctx,
        boost::system::error_code *errorCode = 0) const = 0;

    virtual Connection::Ptr createConnection(
        const NetworkContext::Ptr& ctx,
        boost::system::error_code *errorCode = 0) const = 0;

    virtual TransportAddress::Ptr createAddress(const std::string &url) const = 0;

    virtual NetworkContext::Ptr createContext() const = 0;

    static const Transport * getTransportByName(const char *transportName);

    static const Transport * getTransportByURL(const std::string &url);

    static void registerTransport(const Transport *transport);

protected:

    Transport(const char * name, int priority);

    void setNetworkHandler(const NetworkHandler &networkHandler) { networkHandler_ = networkHandler; }
    void setPriority(int priority) { priority_ = priority; }
    void setHttpTransport(bool httpTransport) { httpTransport_ = httpTransport; }
    void setSecureTransport(bool secureTransport) { secureTransport_ = secureTransport; }

private:
    const char * name_;
    int priority_;
    bool httpTransport_;
    bool secureTransport_;
    NetworkHandler networkHandler_;

    typedef std::map<std::string, const Transport *> TransportMap;
    static TransportMap transports_;
};

} // namespace Transport
} // namespace KIARA

#endif /* KIARA_TRANSPORT_TRANSPORT_HPP_INCLUDED */
