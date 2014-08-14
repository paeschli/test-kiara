// Server.cpp implementation is based on:
//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "Server.hpp"
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

#define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

namespace KIARA
{
namespace Transport
{

#define MULTITHREADED_SERVER

Server::Server(std::size_t threadPoolSize)
    : threadPoolSize_(threadPoolSize), ctx_(new AsioNetworkContext), signals_(ctx_->getIoService())
{
}

Server::Server(const std::string& address, const std::string& port, CreateConnectionFn createConnectionFn,
               std::size_t threadPoolSize)
    : threadPoolSize_(threadPoolSize), ctx_(new AsioNetworkContext), signals_(ctx_->getIoService())
{
    listen(address, port, createConnectionFn);
}

void Server::listen(const std::string& address, const std::string& port, CreateConnectionFn createConnectionFn)
{
    serverEntries_.push_back(new ServerEntry(ctx_, address, port, createConnectionFn));
}

void Server::run()
{

    DFC_DEBUG("Called Server::run()");
    // Register to handle the signals that indicate when the server should exit.
    // It is safe to register for the same signal multiple times in a program,
    // provided all registration for the specified signal is made through Asio.
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
    signals_.async_wait(boost::bind(&Server::handleStop, this));

    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    // boost::asio::ip::tcp::resolver resolver(io_service_);
    boost::asio::ip::tcp::resolver resolver(boost::dynamic_pointer_cast<AsioNetworkContext>(ctx_)->getIoService());

    for (ServerEntryList::iterator it = serverEntries_.begin(), end = serverEntries_.end();
        it != end; ++it)
    {
        boost::asio::ip::tcp::resolver::query query(it->address_, it->port_);
        boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
        it->acceptor_.open(endpoint.protocol());
        it->acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        it->acceptor_.bind(endpoint);
        it->acceptor_.listen();
    }

    startAcceptAll();

#ifdef MULTITHREADED_SERVER
    // Create a pool of threads to run all of the io_services.
    std::vector<boost::shared_ptr<boost::thread> > threads;
    for (std::size_t i = 0; i < threadPoolSize_; ++i)
    {
        boost::shared_ptr<boost::thread> thread(
            new boost::thread(boost::bind(&boost::asio::io_service::run, &boost::dynamic_pointer_cast<AsioNetworkContext>(ctx_)->getIoService())));
        threads.push_back(thread);
    }

    // Wait for all threads in the pool to exit.
    for (std::size_t i = 0; i < threads.size(); ++i)
        threads[i]->join();
#else
    boost::dynamic_pointer_cast<AsioNetworkContext>(ctx_)->getIoService().run();
#endif
}

void Server::startAcceptAll()
{
    DFC_DEBUG("Accepting incoming connections ...");
    for (ServerEntryList::iterator it = serverEntries_.begin(), end = serverEntries_.end();
        it != end; ++it)
    {
        startAccept(*it);
    }
}

void Server::startAccept(ServerEntry &serverEntry)
{
    serverEntry.newConnection_ =
        boost::dynamic_pointer_cast<TcpConnection>(serverEntry.createConnectionFn_(ctx_));

    if (serverEntry.newConnection_)
    {
        DFC_DEBUG("Registering acceptor ...");
        serverEntry.acceptor_.async_accept(serverEntry.newConnection_->getSocket(),
            boost::bind(&Server::handleAccept, this, boost::ref(serverEntry), boost::asio::placeholders::error));
    }
    // FIXME newConnection_ is NULL, should we do something ?
}

void Server::handleAccept(ServerEntry &serverEntry, const boost::system::error_code& e)
{
    DFC_DEBUG("Accepting new connection");
    if (!e)
    {
        DFC_DEBUG("Starting new connection");
        serverEntry.newConnection_->start();
    }

    startAccept(serverEntry);
}

void Server::handleStop()
{
    boost::dynamic_pointer_cast<AsioNetworkContext>(ctx_)->getIoService().stop();
}

} // namespace Transport
} // namespace KIARA
