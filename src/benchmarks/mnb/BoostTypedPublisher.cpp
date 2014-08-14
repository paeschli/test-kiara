// Copyright (c) 2010, Object Computing, Inc.
// All rights reserved.

#ifdef _MSC_VER
# pragma warning(disable:4996) // Disable VC warning from Boost serialization
# pragma warning(disable:4099) // Disable VC warning from Boost serialization
#endif

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <boost/asio.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include "Profiler.h"
#include "Functions_T.h"
#include "MarketData.h"
#include "QuoteRequest.h"

using boost::asio::ip::tcp;

int main(int argc, char* argv[])
{
  try
  {
    if (argc < 4)
    {
      std::cerr << "Usage: BoostTypedPublisher <num-messages> [<host> <port>]+\n";
      return 1;
    }
    size_t num_messages = std::atoi(argv[1]);
    const size_t used_args = 2;

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

    printf("Sending %d messages to %d subscribers\n",
           num_messages,
           num_subscribers);

    MiddlewareNewsBrief::MarketData md =
      MiddlewareNewsBrief::MarketData::createTestData();
    MiddlewareNewsBrief::QuoteRequest qr =
      MiddlewareNewsBrief::QuoteRequest::createTestData();

    MIDDLEWARENEWSBRIEF_PROFILER_TIME_TYPE start =
      MIDDLEWARENEWSBRIEF_PROFILER_GET_TIME;

    const size_t MAX_REPLY_BUFFER_SIZE = 1024;
    boost::shared_ptr<char> reply_buffer(new char[MAX_REPLY_BUFFER_SIZE]);

    for (size_t i=0; i < num_messages; ++i)
    {
      std::ostringstream request(std::ios::binary);
      boost::archive::binary_oarchive oa(request);

      // Send 10 MarketDatas for each QuoteRequest
      if (i % 10 == 5)
      {
        oa << MiddlewareNewsBrief::QuoteRequest::TOPIC;
        MiddlewareNewsBrief::stream(qr,oa,i);
      }
      else
      {
        oa << MiddlewareNewsBrief::MarketData::TOPIC;
        MiddlewareNewsBrief::stream(md,oa,i);
      }

      // Two loops here simulates pub/sub behavior,
      // where we publish to all subscribers before
      // looking for an echoed sample coming back

      for (size_t jj = 0; jj < num_subscribers; ++jj) {
        boost::asio::write(*(subscribers[jj]),
                           boost::asio::buffer(request.str().data(), request.str().size()));
      }

      // Reply is same size as request for this test
      size_t reply_buffer_size = request.str().size();
      if (reply_buffer_size > MAX_REPLY_BUFFER_SIZE)
      {
        std::cerr << "Maximum reply buffer size exceeded: " << reply_buffer_size << std::endl;
        return -1;
      }

      for (size_t jj = 0; jj < num_subscribers; ++jj)
      {
        memset(reply_buffer.get(),0,reply_buffer_size);
        size_t reply_length = boost::asio::read(*(subscribers[jj]),
            boost::asio::buffer(reply_buffer.get(), reply_buffer_size));

        std::istringstream reply_stream(std::string(reply_buffer.get(),reply_length),
                                        std::ios::binary);
        boost::archive::binary_iarchive ia(reply_stream);

        std::string topic_name;
        ia >> topic_name;

        if (topic_name == MiddlewareNewsBrief::MarketData::TOPIC)
        {
          MiddlewareNewsBrief::MarketData mdReply;
          if (MiddlewareNewsBrief::check(mdReply,ia,i) != 0) { return -1; }
        }
        else if (topic_name == MiddlewareNewsBrief::QuoteRequest::TOPIC)
        {
          MiddlewareNewsBrief::QuoteRequest qrReply;
          if (MiddlewareNewsBrief::check(qrReply,ia,i) != 0) { return -1; }
        }
        else
        {
          std::cerr << "Received invalid topic name: " << topic_name.c_str() << std::endl;
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
