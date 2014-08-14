//
// Copyright (c) 2010, Object Computing, Inc.
// Modified from an example in the Boost ASIO documentation,
//    http://www.boost.org
//

#ifndef MIDDLEWARENEWSBRIEF_BOOST_SESSION_H
#define MIDDLEWARENEWSBRIEF_BOOST_SESSION_H

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include "BoostUtil_Export.h"

using boost::asio::ip::tcp;

class BoostUtil_Export Session
{
public:
  Session(boost::asio::io_service& io_service,
          size_t num_messages,
          boost::function<void(char*,size_t,Session*)> read_callback =
            boost::function<void(char*,size_t,Session*)>() );

  tcp::socket& socket();

  void start();

  void handle_read(const boost::system::error_code& error,
                   size_t bytes_transferred);

  void handle_write(const boost::system::error_code& error);

  void write(const char* buffer, size_t num_bytes);

private:
  boost::asio::io_service& io_service_;
  tcp::socket socket_;
  enum { max_length = 8096 };
  char data_[max_length];
  const size_t num_messages_;
  size_t message_count_;

  boost::function<void(char*,size_t,Session*)> read_callback_;
};

#endif
