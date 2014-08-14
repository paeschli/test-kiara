//
// Copyright (c) 2010, Object Computing, Inc.
// Modified from an example in the Boost ASIO documentation,
//    http://www.boost.org
//

#include "Server.h"
#include "Session.h"
#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

Server::Server(boost::asio::io_service& io_service,
               short port,
               size_t num_messages,
               boost::function<void(char*,size_t,Session*)> read_callback)
  : io_service_(io_service),
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
    num_messages_(num_messages),
    read_callback_(read_callback)
{
  Session* new_session =
    new Session(io_service_, num_messages_, read_callback_);
  acceptor_.async_accept(new_session->socket(),
                         boost::bind(&Server::handle_accept, this, new_session,
                                     boost::asio::placeholders::error));
}

void
Server::handle_accept(Session* new_session,
                      const boost::system::error_code& error)
{
  if (!error)
  {
    new_session->start();
    new_session = new Session(io_service_,num_messages_,read_callback_);
    acceptor_.async_accept(new_session->socket(),
                           boost::bind(&Server::handle_accept, this, new_session,
                                       boost::asio::placeholders::error));
  }
  else
  {
    delete new_session;
  }
}
