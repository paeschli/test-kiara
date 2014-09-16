/**
 * @file KT_Connection.hpp
 * @author Mathias Hablützel <habl@zhaw.ch>
 * @version 1.0
 * @license TBD
 *
 * @brief Connection interface.
 */
 
#ifndef KT_CONNECTION_HPP
#define KT_CONNECTION_HPP

#include <string>
#include <map>
#include <functional>
#include <KIARA/Utils/DBuffer.hpp>

#include "KT_Configuration.hpp"
#include "KT_Msg.hpp"
#include "KT_Session.hpp"
#include "zmq.hpp"

namespace KIARA {
namespace Transport {

/**
 * @class The abstract class acting as interface for concrete implementations
 *   like KT_Zeromq.cpp Also note that certain methods are not overwritten in
 *   the implementation as they are pretty generic.
 * 
 * This interface acts as a as the main component of SCALN.
 * 
 * @see doc/API Library Contract.md
 */
class KT_Connection
{
protected:

  void* _context;
  zmq::context_t _context_mt;
  std::map< std::string, KT_Session* >* _sessions;
  KT_Configuration _configuration;
  std::function<void(KT_Msg&, KT_Session*, KT_Connection*)> _std_callback;
  std::function<DBuffer*(KT_Msg&, KT_Session*, KT_Connection*)> _std_callback_str;
  
public:

  KT_Connection ( );
  virtual ~KT_Connection ( );
  KT_Connection (std::string const& host);

  /**
   * @brief Connect to remote host.
   * @return int 0 if successful.
   * @param ret Return a KT_Session pointer when successful.
   */

  virtual int
  connect ( KT_Session** ret) = 0;

  /**
   * @brief Send a message to remote host.
   * @return int 0 if successful.
   * @param message Message for the receipient.
   * @param linger Linger time before it aborts if sending synchronous,
   *    0 will block forever, -1 will make the call asynchronous.
   */

  virtual int
  send (KT_Msg& message, KT_Session& session, int linger = 0) = 0;

  /**
   * @brief Receive a message.
   * @return int 0 if successful.
   * @param linger Linger time before it aborts if receiving synchronous,
   *    0 will block forever, -1 will make the call asynchronous and only
   *    return a message if there was one previously received.
   */

  virtual int
  recv (KT_Session& session,  KT_Msg& ret, int linger = 0) = 0;

  /**
   * @brief Disconnect from remote host.
   * @param session The session/connection to disconnect from.
   * @return Zero on success, non-zero on failure.
   */
  virtual int
  disconnect (KT_Session& session) = 0;

  /**
   * @brief callback function must accept KT_Msg* and KT_Session* object.
   * @param callback Function to be called when a message arrives.
   * @return Zero on success, non-zero on failure.
   */

  virtual int
  register_callback (std::function<void(KT_Msg&, KT_Session*, KT_Connection*)>) = 0;
  
  virtual int
  register_callback_str (std::function<DBuffer*(KT_Msg&, KT_Session*, KT_Connection*)>) = 0;

  /**
   * @brief bind requires a valid callback handler which is called when a message is
   * received, it binds according to the set configuration.
   * @return Zero on success, non-zero on failure.
   */
  virtual int
  bind () = 0;
  
  /**
   * @brief Stops listening to incoming messages.
   * @return Zero on success, non-zero on failure.
   */
  virtual int
  unbind () = 0;
  
  /**
   * @brief Set the value of _context.
   * @param context the new value of _context.
   */
  virtual void set_context ( void* context ) {
      _context = context;
  }

  /**
   * @brief Get the value of _context.
   * @return the value of _context.
   */
  virtual void const* get_context ( ) const {
    return _context;
  }

  /**
   * @brief Set the value of _session.
   * @param session the new value of _session.
   */
  virtual void set_session ( std::map< std::string, KT_Session* >* session ) {
      _sessions = session;
  }

  /**
   * @brief Get the value of _session.
   * @return the value of _session.
   */
  virtual std::map< std::string, KT_Session* >* const get_session ( ) const {
    return _sessions;
  }

  /**
   * @brief Set the value of _configuration.
   * @param configuration the new value of _configuration.
   */
  virtual void set_configuration ( KT_Configuration const& configuration ) {
      _configuration = configuration;
  }

  /**
   * @brief Get the value of _configuration.
   * @return the value of _configuration.
   */
  virtual KT_Configuration const& get_configuration ( ) const {
    return _configuration;
  }
  
  
}; // end of KT_Connection class
} // end of Transport namespace
} // end of KIARA namespace

#endif // KT_CONNECTION_HPP
