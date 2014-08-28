/**
 * @file KT_Session.hpp
 * @author Mathias Hablützel <habl@zhaw.ch>
 * @version 1.0
 * @license TBD
 * @brief KIARA Transport Library Session
 * 
 * This class stores session related data and also contains an opaque pointer
 * to data used by the upper layers of KIARA.
 */

#ifndef KT_SESSION_HPP
#define KT_SESSION_HPP

#include <string>
#include <vector>

#include "KT_Client.hpp"

namespace KIARA {
namespace Transport {

/**
 * @class KT_Session
 * @brief Is used to identify a current session issued by a connection.
 */
class KT_Session
{
private:
  /// A pointer to an opaque data structure which is needed by the upper
  /// layers of KIARA and will not be modified by the transport library.
  void* _k_user_data = nullptr;
  /// A pointer to the session relevant socket.
  void* _socket = nullptr;
  /// The endpoint in a human readable way.
  std::string _endpoint;
  /// Required by ZeroMQ to identify the connection.
  std::vector< char > _identifier;
  
public:

    KT_Session();
    virtual ~KT_Session();

    void set_socket(void* socket)
    {
        _socket = socket;
    }

    void* get_socket()
    {
        return _socket;
    }

    void set_endpoint(std::string endpoint)
    {
        _endpoint = endpoint;
    }

    std::string get_endpoint()
    {
        return _endpoint;
    }

    /**
     * @brief Set the value of _k_user_data
     * 
     * k_user_data is a pointer to an opaque data structure which is needed by the
     * upper layers of KIARA and will not be modified by the transport library.
     * 
     * @param k_user_data the new value of _k_user_data
     */
    void set_k_user_data(void* k_user_data)
    {
        _k_user_data = k_user_data;
    }

    /**
     * @brief Get the value of _k_user_data
     * 
     * k_user_data is a pointer to an opaque data structure which is needed by the
     * upper layers of KIARA and will not be modified by the transport library.
     * 
     * @return the value of _k_user_data
     */
    void* get_k_user_data()
    {
        return _k_user_data;
    }

    const std::vector< char >& get_identifier() const
    {
        return _identifier;
    }

    void set_identifier(const std::vector< char >& identifier)
    {
        _identifier = identifier;
    }
};
} // end of Transport namespace
} // end of KIARA namespace

#endif // KT_SESSION_HPP
