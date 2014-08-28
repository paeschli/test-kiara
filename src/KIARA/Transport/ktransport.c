/**
 * @file
 * @author Mathias Hablützel <habl@zhaw.ch>
 * @version 1.0
 * 
 * @section LICENSE
 * 
 * TBD
 */

#include "ktransport.h"

/**
 * @brief Static array for transport prefixes
 * @deprecated
 */
static const char* kt_transport_prefix[] = {
	"tcp://",
	"udp://",
	"dccp://",
	"sctp://"
};

char* compile_endpoint_string(kt_connconf_t config)
{
	//FIXME: Dynamic allocation of memory for endpoint
	char *endpoint = malloc(256);
	snprintf(endpoint, 255, "%s%s:%i",
		kt_transport_prefix[config.network_config.transport],
		config.base_url,
		config.network_config.port
		);
	return endpoint;
}
