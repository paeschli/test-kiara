/*
 * HttpTransport.cpp
 *
 *  Created on: Feb 11, 2014
 *      Author: Dmitri Rubinstein
 */
#include "HttpTransport.hpp"
#include <KIARA/Impl/Network.hpp>
#include <DFC/Base/Utils/StaticInit.hpp>
#include <KIARA/Utils/URL.hpp>

#define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

namespace KIARA
{
namespace Transport
{

/// HttpAddress

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

HttpAddress::HttpAddress(const std::string &hostName, unsigned short port, const std::string &path, const Transport *transport)
    : TransportAddress(transport)
    , hostName_(hostName)
    , port_(port)
    , path_(path)
    , address_(resolveHostName(hostName))
{
    // normalize path
    if (path_.empty())
        path_ = "/";
    else
    {
        if (path_[0] != '/')
            path_ = "/" + path_;
    }
}

unsigned short HttpAddress::getPort() const
{
    return port_;
}

std::string HttpAddress::getHostName() const
{
    return hostName_;
}

std::string HttpAddress::getPath() const
{
    return path_;
}

bool HttpAddress::acceptConnection(const TransportAddress::Ptr &address) const
{
    if (!address || address->getTransport() != getTransport())
        return false;
    HttpAddress::Ptr other = boost::static_pointer_cast<HttpAddress>(address);

    if (other->address_ != address_)
    {
        std::string otherHostName = other->getHostName();
        std::string thisHostName = getHostName();

        if (otherHostName != thisHostName && thisHostName != "0.0.0.0")
            return false;
    }

    if (other->getPort() != getPort())
        return false;

    std::string otherPath = other->getPath();
    std::string thisPath = getPath();

    if (otherPath != thisPath && thisPath != "*")
        return false;

    return true;
}

bool HttpAddress::equals(const TransportAddress::Ptr &addr) const
{
    if (!addr || addr->getTransport() != getTransport())
        return false;
    HttpAddress::Ptr other = boost::static_pointer_cast<HttpAddress>(addr);
    return (other->address_ == address_ || other->hostName_ == hostName_) && other->port_ == port_ && other->path_ == path_;
}

std::string HttpAddress::toString() const
{
    std::ostringstream oss;
    oss << getTransport()->getName() << "://" << hostName_ << ":" << port_ << path_;
    return oss.str();
}

/// HttpConnection

HttpConnection::HttpConnection(const NetworkContext::Ptr& ctx, const Transport *transport, size_t bufferSize)
    : BufferedTcpConnection(ctx, bufferSize)
    , request_(transport)
    , response_(transport)
    , bufferOffset_(0)
    , bufferSize_(0)
    , parserStatus_(HttpRequestParser::PARSING_FINISHED)
    , readInProgress_(false)
{}

HttpConnection::~HttpConnection()
{
}

namespace
{
class DummyConnection: public Connection
{
public:

    typedef boost::shared_ptr<DummyConnection> Ptr;

    explicit DummyConnection() { }

    ~DummyConnection() { }

    virtual void handleStart() { }

    std::string getRemoteHostName() const { return ""; }

    unsigned short getRemotePort() const { return 0; }

    std::string getLocalHostName() const { return ""; }

    unsigned short getLocalPort() const { return 0; }
};
} // unnamed namespace

extern "C" {

static ::KIARA_Connection * http_getConnection(::KIARA_FuncObj *funcObj)
{
    return funcObj->base.connection;
}

static ::KIARA_Connection * http_getServiceConnection(::KIARA_ServiceFuncObj *funcObj)
{
    return funcObj->base.connection;
}

static int http_sendData(::KIARA_Connection *conn, const void *msgData, size_t msgDataSize, kr_dbuffer_t *destBuf)
{
   int result = 0;

   std::string errorMsg;
   if (!KIARA::URLLoader::sendData(
       ((KIARA::Impl::ClientConnection*)conn)->getURLLoaderConnection(),
       ((KIARA::Impl::ClientConnection*)conn)->getConnectionURI(),
       ((KIARA::Impl::ClientConnection*)conn)->getMimeType().c_str(),
       msgData,
       msgDataSize,
       destBuf,
       KIARA::Impl::unwrap(conn)->getContext()->getSecurityConfiguraton(),
       &errorMsg))
    {
       KIARA::Impl::unwrap(conn)->setError(KIARA_GENERIC_ERROR, errorMsg);
       result = KIARA_NETWORK_ERROR;
    }
    else
    {
        result = KIARA_SUCCESS;
    }
    return result;
}

}

static NetworkHandler httpNetworkHandler = {
    http_getConnection,
    http_getServiceConnection,
    http_sendData
};

HttpTransport::HttpTransport()
    : Transport("http", 20)
{
    setHttpTransport(true);
    setSecureTransport(false);
    setNetworkHandler(httpNetworkHandler);
}

/*
tinfo.setName("https");
tinfo.setPriority(19);
tinfo.setHttpTransport(true);
tinfo.setSecureTransport(true);
tinfo.setNetworkHandler(getHttpNetworkHandler());
transports_[tinfo.getName()] = tinfo;
*/

Connection::Ptr HttpTransport::openConnection(
    const std::string &url,
    const NetworkContext::Ptr& ctx,
    boost::system::error_code *errorCode) const
{
    return DummyConnection::Ptr(new DummyConnection);
}

Connection::Ptr HttpTransport::createConnection(
	const NetworkContext::Ptr& ctx,
    boost::system::error_code *errorCode) const
{
    return Connection::Ptr(new HttpConnection(ctx, this));
}

TransportAddress::Ptr HttpTransport::createAddress(const std::string &_url) const
{
    URL url(_url);
    if (!url.isValid() || url.scheme != getName())
        return TransportAddress::Ptr();

    return TransportAddress::Ptr(new HttpAddress(url.host, boost::lexical_cast<unsigned short>(url.port), url.path, this));
}

NetworkContext::Ptr HttpTransport::createContext() const
{
	return NetworkContext::Ptr();
}

/// HttpConnection

void HttpConnection::handleStart()
{
    DFC_DEBUG("HttpConnection::handleStart() called");
    if (parserStatus_ == HttpRequestParser::PARSING_FAILED ||
        parserStatus_ == HttpRequestParser::PARSING_FINISHED)
    {
        DFC_DEBUG("start: reset request");
        requestParser_.reset();
        request_.clear();
        parserStatus_ = HttpRequestParser::PARSING_FINISHED;
    }

    response_.clear();
    startRead();
}

void HttpConnection::startRead()
{
    DFC_DEBUG("HttpConnection::startRead() called");
    if (bufferOffset_ != 0)
    {
        // there are still data in the buffer
        DFC_DEBUG("Calling HttpConnection::handleBuffer()");
        handleBuffer(getBufferData()+bufferOffset_, bufferSize_);
    }
    else
    {
        if (!readInProgress_)
        {
            readInProgress_ = true;
            DFC_DEBUG("Calling BufferedTcpConnection::startReadBuffer()");
            startReadBuffer();
        }
    }
}

void HttpConnection::handleRead(const boost::system::error_code& e, std::size_t bytesTransferred)
{
    DFC_DEBUG("HttpConnection::handleRead() called");
    readInProgress_ = false;

    if (!e)
    {
        DFC_DEBUG("handleRead buffer: "<<std::string(getBufferData(), getBufferData()+bytesTransferred));

        handleBuffer(getBufferData(), bytesTransferred);
    }
    else
    {
        DFC_DEBUG("SYSTEM ERROR "<<e);
    }

    // If an error occurs then no new asynchronous operations are started. This
    // means that all shared_ptr references to the connection object will
    // disappear and the object will be destroyed automatically after this
    // handler returns. The connection class's destructor closes the socket.
}

void HttpConnection::handleBuffer(char *bufferBegin, std::size_t bytesTransferred)
{
    std::size_t bytesParsed;
    char * bufferEnd = bufferBegin + bytesTransferred;
    parserStatus_ = requestParser_.parse(request_, bufferBegin, bytesTransferred, bytesParsed);
    char * newBufferBegin = bufferBegin + bytesParsed;

    DFC_DEBUG("buffer: "<<std::string(bufferBegin, bufferEnd));
    DFC_DEBUG("parsing result: "<<parserStatus_);

    if (newBufferBegin == bufferEnd)
    {
        bufferOffset_ = 0;
        bufferSize_ = 0; // do we need this ?
    }
    else
    {
        bufferOffset_ = newBufferBegin - getBufferData();
        bufferSize_ = bufferEnd - newBufferBegin;
    }

    DFC_DEBUG("bufferOffset = "<<bufferOffset_<<" bufferSize = "<<bufferSize_);
    DFC_DEBUG("(A) new buffer: "<<std::string(newBufferBegin, bufferEnd));
    DFC_DEBUG("(B) new buffer: "<<std::string(getBufferData()+bufferOffset_, getBufferData()+bufferOffset_+bufferSize_))

    if (parserStatus_ == HttpRequestParser::READING_CONTENT || parserStatus_ ==  HttpRequestParser::PARSING_FINISHED)
    {
        // check if we have Expect:100-continue
        HttpRequest::HeaderList::iterator it = request_.findHeader("Expect");
        if (it != request_.headers_end())
        {
            if (boost::algorithm::iequals(it->value, "100-continue"))
            {
                request_.getHeaders().erase(it);
                response_.setDefaultHTMLResponse(HttpResponse::CONTINUE);
            }
            else
                response_.setDefaultHTMLResponse(HttpResponse::EXPECTATION_FAILED);

            std::vector<boost::asio::const_buffer> buffers;
            response_.toBuffers(buffers);
            startWriteBuffers(buffers);
        }

        if (parserStatus_ == HttpRequestParser::PARSING_FINISHED)
        {
            handleRequest(request_, response_);
            std::vector<boost::asio::const_buffer> buffers;
            response_.toBuffers(buffers);
            startWriteBuffers(buffers);
        }
        else
        {
            // continue reading content
            startRead();
        }
    }
    else if (parserStatus_ == HttpRequestParser::PARSING_FAILED)
    {
        response_.setDefaultHTMLResponse(HttpResponse::BAD_REQUEST);
        std::vector<boost::asio::const_buffer> buffers;
        response_.toBuffers(buffers);
        startWriteBuffers(buffers);
    }
    else // PARSING_HEADERS
    {
        startRead();
    }
}

void HttpConnection::handleWrite(const boost::system::error_code& e)
{
    if (!e)
        handleStart();
#if 0
    // following code should be executed when connection to be closed by server
    if (!e)
    {
        // Initiate graceful connection closure.
        boost::system::error_code ignored_ec;
        socket_.shutdown(boost::asio::ip::tcp::getSocket::shutdown_both, ignored_ec);
    }
#endif

    // No new asynchronous operations are started. This means that all shared_ptr
    // references to the connection object will disappear and the object will be
    // destroyed automatically after this handler returns. The connection class's
    // destructor closes the socket.
}

static HttpTransport httpTransport;

DFC_STATIC_INIT_FUNC
{
    Transport::registerTransport(&httpTransport);
}

} // namespace Transport
} // namespace KIARA
