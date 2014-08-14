/*
 * HttpTransport.hpp
 *
 *  Created on: Feb 11, 2014
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_TRANSPORT_HTTPTRANSPORT_HPP_INCLUDED
#define KIARA_TRANSPORT_HTTPTRANSPORT_HPP_INCLUDED

#include "Transport.hpp"
#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestHandler.hpp"
#include "HttpRequestParser.hpp"
#include <boost/asio/ip/address.hpp>
#include <string>

namespace KIARA
{
namespace Transport
{

class HttpAddress: public TransportAddress
{
public:

    typedef boost::shared_ptr<HttpAddress> Ptr;

    HttpAddress(const std::string &hostName, unsigned short port, const std::string &path, const Transport *transport);

    std::string getHostName() const;

    unsigned short getPort() const;

    std::string getPath() const;

    bool acceptConnection(const TransportAddress::Ptr &address) const;

    bool equals(const TransportAddress::Ptr &other) const;

    std::string toString() const;

private:
    std::string hostName_;
    unsigned short port_;
    std::string path_;
    boost::asio::ip::address address_;
};

class HttpTransport: public Transport
{
public:

    HttpTransport();

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

class HttpConnection;
typedef boost::shared_ptr<HttpConnection> HttpConnectionPtr;

/// Represents a single HTTP connection from a client.
class HttpConnection: public BufferedTcpConnection
{
public:

    typedef HttpConnectionPtr Ptr;

    /// Construct a connection with the given io_service.
    explicit HttpConnection(const NetworkContext::Ptr& ctx, const Transport *transport, size_t bufferSize = 8192);

    virtual ~HttpConnection();


private:

    /// Start the first asynchronous operation for the connection.
    void handleStart();

    void startRead();

    /// Handle completion of a read operation.
    void handleRead(const boost::system::error_code& e, std::size_t bytesTransferred);

    void handleBuffer(char *bufferStart, std::size_t bytesTransferred);

    /// Handle completion of a write operation.
    void handleWrite(const boost::system::error_code& e);

    /// The incoming request.
    HttpRequest request_;

    /// The parser for the incoming request.
    HttpRequestParser requestParser_;

    /// The response to be sent back to the client.
    HttpResponse response_;

    /// Offset into the buffer
    size_t bufferOffset_;

    /// Buffer size
    size_t bufferSize_;

    /// Flag if we should continue parsing when new data are arrived
    HttpRequestParser::Status parserStatus_;

    /// True if currently a async read is in progress
    bool readInProgress_;
};

} // namespace Transport
} // namespace KIARA


#endif /* KIARA_TRANSPORT_HTTPTRANSPORT_HPP_INCLUDED */
