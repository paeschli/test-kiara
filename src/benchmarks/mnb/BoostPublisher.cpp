// Copyright (c) 2010, Object Computing, Inc.
// All rights reserved.

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>

#include "Profiler.h"

using boost::asio::ip::tcp;

int main(int argc, char* argv[])
{
  try
  {
    if (argc < 5)
    {
      std::cerr << "Usage: BoostPublisher <message-size> <num-messages> [<host> <port>]+\n";
      return 1;
    }
    size_t message_size = std::atoi(argv[1]);
    size_t num_messages = std::atoi(argv[2]);
    const size_t used_args = 3;

    boost::asio::io_service io_service;

    size_t num_subscribers = (argc - used_args) / 2;
    std::vector<boost::shared_ptr<tcp::socket> > subscribers;
    subscribers.reserve(num_subscribers);

    for (size_t i=0; i < num_subscribers; ++i)
    {
      tcp::resolver resolver(io_service);
      tcp::resolver::query query(tcp::v4(), argv[i*2+used_args], argv[i*2+used_args+1]);
      tcp::resolver::iterator iterator = resolver.resolve(query);

      boost::shared_ptr<tcp::socket> s(new tcp::socket(io_service));
      subscribers.push_back(s);
      s->connect(*iterator);
    }

    printf("Sending %d messages of size %d to %d subscribers\n",
           num_messages,
           message_size,
           num_subscribers);

    boost::shared_ptr<char> request(new char[message_size]);
    boost::shared_ptr<char> reply(new char[message_size]);

    MIDDLEWARENEWSBRIEF_PROFILER_TIME_TYPE start =
      MIDDLEWARENEWSBRIEF_PROFILER_GET_TIME;

    for (size_t i=0; i < num_messages; ++i) {

      memset(request.get(), 0, message_size);
      memset(reply.get(), 0, message_size);

      // Two loops here simulates pub/sub behavior,
      // where we publish to all subscribers before
      // looking for an echoed sample coming back

      for (size_t jj = 0; jj < num_subscribers; ++jj) {
        boost::asio::write(*(subscribers[jj]), boost::asio::buffer(request.get(), message_size));
      }

      for (size_t jj = 0; jj < num_subscribers; ++jj) {
        size_t reply_length = boost::asio::read(*(subscribers[jj]),
            boost::asio::buffer(reply.get(), message_size));
        if (reply_length != message_size)
        {
          std::cerr << "Message reply size mismatch; expected "
                    << message_size << ", received " << reply_length << std::endl;
          return -1;
        }
      }
    }

    MIDDLEWARENEWSBRIEF_PROFILER_TIME_TYPE finish =
      MIDDLEWARENEWSBRIEF_PROFILER_GET_TIME;

    MIDDLEWARENEWSBRIEF_PROFILER_TIME_TYPE elapsed =
      MIDDLEWARENEWSBRIEF_PROFILER_DIFF(finish,start);

    double latency = (double) elapsed / (num_messages * 2.0) / (double)(num_subscribers);
    printf("\n\nAverage latency in %s: %.3f\n\n\n",
           MIDDLEWARENEWSBRIEF_PROFILER_TIME_UNITS,
           latency);
    printf("Finished\n");
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
