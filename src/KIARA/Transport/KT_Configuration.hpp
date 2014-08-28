/**
 * @file KT_Configuration.hpp
 * @author Mathias Hablützel <habl@zhaw.ch>
 * @version 1.0
 * @license TBD
 *
 * @brief Configuration class for network stack.
 */
#ifndef KT_CONFIGURATION_HPP
#define KT_CONFIGURATION_HPP

#include <string>
#include "KT_Configuration_glob.h"
#include "negotiation.h"

namespace KIARA {
namespace Transport {

/**
  * class KT_Configuration
  * 
  */

class KT_Configuration
{

private:

  // Layer 3 in the networking stack
  unsigned int _network_layer = 0;
  // Layer 4 in the networking stack
  unsigned int _transport_layer = 0;
  std::string _hostname = "";
  std::string _config_path = "";
  unsigned int _port_number = 0;
  // Optional crypto layer, based on OpenSSL
  unsigned int _crypto_layer = 0;
  unsigned int _application_layer = 0;
  unsigned int _application_type = 0;
  
public:

  KT_Configuration ( );
  virtual ~KT_Configuration ( );
  
  void negotiation ( neg_ctx_t* neg_ctx );

  void set_network_layer ( kt_network_layer network_layer );
  kt_network_layer get_network_layer ( ) const;
  void set_transport_layer ( kt_transport_layer transport_layer );
  kt_transport_layer get_transport_layer ( ) const;
  const std::string& get_hostname() const
  {
	  return _hostname;
  }
  const std::string& get_config_path() const
  {
	  return _config_path;
  }
  void set_hostname(const std::string& hostname)
  {
	  _hostname = hostname;
  }
  void set_config_path(const std::string& config_path)
  {
	  _config_path = config_path;
  }
  unsigned int get_port_number() const
  {
	  return _port_number;
  }
  void set_port_number(unsigned int portNumber)
  {
	  _port_number = portNumber;
  }
  void set_crypto_layer ( kt_crypto_layer crypto_layer );
  kt_crypto_layer get_crypto_layer ( ) const;
  void set_application_layer ( kt_application_layer application_layer );
  kt_application_layer get_application_layer ( ) const;
  void set_application_type ( kt_application_type application_type );
  kt_application_type get_application_type ( ) const;
  void set_host (kt_transport_layer transport_layer, std::string hostname, unsigned int port_number);

};
} // end of Transport namespace
} // end of KIARA namespace

#endif // KT_CONFIGURATION_HPP
