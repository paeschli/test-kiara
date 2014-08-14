//
// Copyright (c) 2010, Object Computing, Inc.
// Modified from an example in the Boost ASIO documentation,
//    http://www.boost.org
//

#include "Session.h"
#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

Session::Session(boost::asio::io_service& io_service,
                 size_t num_messages,
                 boost::function<void(char*,size_t,Session*)> read_callback)
  : io_service_(io_service)
  , socket_(io_service)
  , num_messages_(num_messages)
  , message_count_(0)
  , read_callback_(read_callback)
{
}

tcp::socket&
Session::socket()
{
  return socket_;
}

void
Session::start()
{
  socket_.async_read_some(boost::asio::buffer(data_, max_length),
                          boost::bind(&Session::handle_read, this,
                                      boost::asio::placeholders::error,
                                      boost::asio::placeholders::bytes_transferred));
}

void
Session::handle_read(const boost::system::error_code& error,
                     size_t bytes_transferred)
{
  if (!error)
  {
    ++message_count_;
    if (this->read_callback_ != 0)
    {
      this->read_callback_(data_,bytes_transferred,this);
    }
    else
    {
      boost::asio::async_write(socket_,
                               boost::asio::buffer(data_, bytes_transferred),
                               boost::bind(&Session::handle_write, this,
                                           boost::asio::placeholders::error));
    }
  }
  else
  {
    delete this;
  }
}


void
Session::handle_write(const boost::system::error_code& error)
{
  // When *sny* session gets message_count_ messages, we stop
  if (message_count_ >= num_messages_)
  {
    io_service_.stop();
  }
  else
  {
    if (!error)
    {
      socket_.async_read_some(boost::asio::buffer(data_, max_length),
                              boost::bind(&Session::handle_read, this,
                                          boost::asio::placeholders::error,
                                          boost::asio::placeholders::bytes_transferred));
    }
    else
    {
      delete this;
    }
  }
}

void
Session::write(const char* buffer, size_t num_bytes)
{
  boost::asio::async_write(socket_,
                           boost::asio::buffer(buffer, num_bytes),
                           boost::bind(&Session::handle_write, this,
                                       boost::asio::placeholders::error));
}
