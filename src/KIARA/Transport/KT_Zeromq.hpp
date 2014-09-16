/**
 * @file KT_Zeromq.hpp
 * @author Mathias Hablützel <habl@zhaw.ch>
 * @version 1.0
 * @license TBD
 * @brief KIARA Transport Library for ZeroMQ Connections
 * 
 * This implements the KT_Connection using the ZeroMQ transport library written
 * by Peter Hintjens et al. It is specialized for communication patterns like
 * Request/Reply, Publish/Subscribe etc.
 * 
 * @warning It is strongly discouraged to use this implementation if you plan to
 * use plain TCP streams. ZeroMQ is specialized for Request/Reply,
 * Publish/Subscribe (amongst other) patterns and does only support stream as
 * a makeshift.
 * 
 * @note Currently supported patterns are Stream (see warning), Request/Reply
 * and Publish/Subscribe.
 */

#ifndef KT_ZEROMQ_HPP
#define KT_ZEROMQ_HPP

#include <zmq.h>
#include <cstring>
#include <cassert>
#include <cerrno>
#include <string>
#include <thread>
#include <vector>
#include <KIARA/Utils/DBuffer.hpp>

#include "k_transport.h"
#include "KT_Configuration.hpp"
#include "KT_Connection.hpp"
#include "KT_Msg.hpp"
#include "KT_Session.hpp"
#include "zmq.hpp"

namespace KIARA {
namespace Transport {

/**
  * @class KT_Zeromq
  * @brief Implementation of the interface KT_Connection
 *  @see KT_Connection
  */

class KT_Zeromq : public KT_Connection
{
private:
    /// Thread used for polling. Requires C++11 with std::thread support
	std::thread* poller_thread;
	std::thread* worker_thread;
	std::thread* proxy_thread;
    /// Indicates if the poller thread has to terminate, it' not acting as signal.
	bool interupted;
	void poller(void* socket, std::string endpoint);
	void* create_socket(unsigned int socket_type, bool listener);

public:

  KT_Zeromq();
  virtual ~KT_Zeromq();
  
  /**
   * @brief Create a ZeroMQ connection with specifying the remote host
   * @param host A string containing an URL
   * @warning Unimplemented!
   */
  KT_Zeromq(std::string const& host);

  int
  connect(KT_Session** ret);

  int
  send(KT_Msg& message, KT_Session& session, int linger = 0);

  int
  recv(KT_Session& session, KT_Msg& ret, int linger = 0);
  
  int
  disconnect(KT_Session& session);

  int
  register_callback(std::function<void(KT_Msg&, KT_Session*, KT_Connection*)>);
  
  int
  register_callback_str(std::function<DBuffer*(KT_Msg&, KT_Session*, KT_Connection*)>);
  
  void
  worker(void *arg, std::string endpoint);
  
  void
  proxy(KT_Session* session, std::string binding_name);

  int bind(void);

  int unbind(void);

}; // end of KT_Connection class
} // end of Transport namespace
} // end of KIARA namespace

#endif // KT_CONNECTION_HPP
