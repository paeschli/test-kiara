// Copyright (c) 2010, Object Computing, Inc.
// All rights reserved.

#ifdef _MSC_VER
# pragma warning(disable:4996) // Disable VC warning from Boost serialization
# pragma warning(disable:4099) // Disable VC warning from Boost serialization
#endif

#include "Server.h"
#include "Session.h"
#include "MarketData.h"
#include "QuoteRequest.h"
#include "Functions_T.h"
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/detail/atomic_count.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

void on_data_available(char* buffer, size_t num_bytes, Session* session, boost::detail::atomic_count* msg_counter);

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: BoostTypedSubscriber <num-messages> <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;
    boost::detail::atomic_count msg_counter(0);

    size_t num_messages = std::atoi(argv[1]);
    Server s(io_service,
             std::atoi(argv[2]),
             num_messages,
             boost::bind(&on_data_available,_1,_2,_3,&msg_counter));

    printf("Waiting; running for %d messages\n", num_messages);

    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}

void
on_data_available(char* buffer, size_t num_bytes, Session* session, boost::detail::atomic_count* msg_counter)
{
  std::istringstream request(std::string(buffer,num_bytes),std::ios::binary);
  boost::archive::binary_iarchive ia(request);

  std::string topic_name;
  ia >> topic_name;

  std::ostringstream reply(std::ios::binary);
  boost::archive::binary_oarchive oa(reply);
  oa << topic_name;

  size_t counter = *msg_counter;
  ++(*msg_counter);

  if (topic_name == MiddlewareNewsBrief::MarketData::TOPIC)
  {
    MiddlewareNewsBrief::MarketData md;
    MiddlewareNewsBrief::check_and_restream(md,ia,oa,counter);
  }
  else if (topic_name == MiddlewareNewsBrief::QuoteRequest::TOPIC)
  {
    MiddlewareNewsBrief::QuoteRequest qr;
    MiddlewareNewsBrief::check_and_restream(qr,ia,oa,counter);
  }
  else
  {
    std::cerr << "Received invalid topic name: " << topic_name.c_str() << std::endl;
    return;
  }

  session->write(reply.str().data(), reply.str().size());
}
