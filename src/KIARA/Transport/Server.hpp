//
// server.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef KIARA_TRANSPORT_SERVER_HPP_INCLUDED
#define KIARA_TRANSPORT_SERVER_HPP_INCLUDED

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/function.hpp>
#include "Transport.hpp"

namespace KIARA
{
namespace Transport
{

/// The top-level class of the TCP/IP server.
class Server: private boost::noncopyable
{
public:

    typedef boost::function<Connection::Ptr (const NetworkContext::Ptr& ctx)> CreateConnectionFn;

    /// Construct the server with the specified size of the thread pool
    explicit Server(std::size_t threadPoolSize);

    /// Construct the server to listen on the specified TCP address_ and port and
    /// the specified size of the thread pool
    explicit Server(const std::string& address, const std::string& port, CreateConnectionFn createConnectionFn,
                    std::size_t threadPoolSize);

    void listen(const std::string& address, const std::string& port, CreateConnectionFn createConnectionFn);

    /// Run the server's io_service loop.
    void run();

private:

    class ServerEntry: private boost::noncopyable
    {
        friend class Server;
    public:

        ServerEntry(const NetworkContext::Ptr& ctx)
            : address_()
            , port_()
            , acceptor_(boost::dynamic_pointer_cast<AsioNetworkContext>(ctx)->getIoService())
            , createConnectionFn_()
            , newConnection_()
        { }

        ServerEntry(const NetworkContext::Ptr& ctx, const std::string &address, const std::string &port,
                    CreateConnectionFn createConnectionFn)
            : address_(address)
            , port_(port)
            , acceptor_(boost::dynamic_pointer_cast<AsioNetworkContext>(ctx)->getIoService())
            , createConnectionFn_(createConnectionFn)
            , newConnection_()
        { }

        const std::string & getAddress() const { return address_; }

        const std::string & getPort() const { return port_; }

        const TcpConnectionPtr & getConnection() const { return newConnection_; }

//        boost::asio::io_service & getIOService() { return acceptor_.get_io_service(); }

    private:
        /// Server address_
        std::string address_;

        /// Server port_
        std::string port_;

        /// Acceptor used to listen for incoming connections.
        boost::asio::ip::tcp::acceptor acceptor_;

        CreateConnectionFn createConnectionFn_;

        /// The next connection to be accepted.
        TcpConnectionPtr newConnection_;
    };

    /// Initiate an asynchronous accept operation.
    void startAcceptAll();

    void startAccept(ServerEntry &serverEntry);

    /// Handle completion of an asynchronous accept operation.
    void handleAccept(ServerEntry &serverEntry, const boost::system::error_code& e);

    /// Handle a request to stop the server.
    void handleStop();

    typedef boost::ptr_vector<ServerEntry> ServerEntryList;

    /// The number of threads that will call io_service::run().
    std::size_t threadPoolSize_;

    /// The io_service used to perform asynchronous operations.
    AsioNetworkContext::Ptr ctx_;
    boost::asio::io_service io_service_;

    /// The signal_set is used to register for process termination notifications.
    boost::asio::signal_set signals_;

    ServerEntryList serverEntries_;
};

} // namespace Transport
} // namespace KIARA

#endif // KIARA_TRANSPORT_SERVER_HPP_INCLUDED
