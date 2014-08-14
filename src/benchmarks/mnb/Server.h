//
// Copyright (c) 2010, Object Computing, Inc.
// Modified from an example in the Boost ASIO documentation,
//    http://www.boost.org
//

#ifndef MIDDLEWARENEWSBRIEF_BOOST_SERVER_H
#define MIDDLEWARENEWSBRIEF_BOOST_SERVER_H

#include "Session.h"
#include "BoostUtil_Export.h"
#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class BoostUtil_Export Server
{
public:
  Server(boost::asio::io_service& io_service,
         short port,
         size_t num_messages,
         boost::function<void(char*,size_t,Session*)> read_callback =
            boost::function<void(char*,size_t,Session*)>());

  void handle_accept(Session* new_session,
                     const boost::system::error_code& error);

private:
  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;
  const size_t num_messages_;

  boost::function<void(char*,size_t,Session*)> read_callback_;
};

#endif
