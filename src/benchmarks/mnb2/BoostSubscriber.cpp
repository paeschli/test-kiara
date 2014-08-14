// Copyright (c) 2010, Object Computing, Inc.
// All rights reserved.

#include "Server.h"
#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 4)
    {
      std::cerr << "Usage: BoostSubscriber <message-size> <num-messages> <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    size_t message_size = std::atoi(argv[1]);
    size_t num_messages = std::atoi(argv[2]);
    Server s(io_service, std::atoi(argv[3]), num_messages);

    printf("Waiting; running for %d messages\n", num_messages);

    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
