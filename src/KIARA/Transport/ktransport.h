/**
 * @file
 * @author Mathias Hablützel <habl@zhaw.ch>
 * @version 1.0
 * 
 * @section LICENSE
 * 
 * TBD
 * 
 * @section DESCRIPTION
 * 
 * This file defines enums for the C99 bindings used by the developer.
 */

#include <czmq.h>
#include <zmq.h>

#ifndef KTRANSPORT_H
#define	KTRANSPORT_H
//#ifdef	__cplusplus
//extern "C" {
//#endif

/**
 * @typedef Redefine zctx_t to our namespace
 */
typedef zctx_t kctx_t;

/**
 * @enum kt_network_layer
 * @brief Define supported network layers (OSI layer 3)
 */
enum kt_network_layer {
	IPLEGACY,
	IPV6
};

/**
 * @enum kt_transport_layer
 * @brief Define supported transport layers (OSI layer 4)
 */
enum kt_transport_layer {
	TCP,
	UDP,
	DCCP,
	SCTP
};

/**
 * @enum kt_crypto_layer
 * @brief Define crypto layer
 * @attention Not implemented
 * 
 * @todo This needs some more fine tuning like setting the protocol level,
 * desired algorithms, key exchanges etc.
 * <habl> 21.08.2013
 */
enum kt_crypto_layer {
	NONE,
	TLS,
	SSL
};

/**
 * @enum kt_application_layer
 * @brief Define application layer (OSI Layer 5)
 * @attention Currently only ZEROMQ and HTTP supported
 */
enum kt_application_layer {
	ZEROMQ,
	HTTP,
	RTSP,
	NTP,
	FTP,
	NFS
};

/**
 * @struct kt_network_stack_t
 * @brief Simple construct to hold different network configuration values.
 * 
 * @see kt_network_layer
 * @see kt_transport_layer
 * @see kt_crypto_layer
 * @see kt_application_layer
 */

struct kt_network_stack_t {
	enum kt_network_layer network;
	enum kt_transport_layer transport;
	enum kt_crypto_layer crypto;
	enum kt_application_layer application;
	uint16_t port;
};

/**
 * @enum kt_application_type
 * @brief Some senseful preselected application types.
 * 
 * The application type directly decides the communication pattern of the
 * socket and network and transport layers. Also it makes certain assumptions
 * that for e.g. a webserver implies HTTP with TCP on Port 80 and a streaming
 * service uses RTSP and UDP on port 554.
 * 
 * @note You may always oversteer the settings by manually setting the desired
 * parameters.
 */
typedef enum kt_application_type {
	WEBSERVER,
	PUBLISHER,
	STREAM,
	ROUNDROBIN,
	REQUESTREPLY
} kt_application_type;

/**
 * @typedef
 * @struct kt_messageraw_t
 * @brief Data structure to hold a message.
 * @note Unused.
 */
typedef struct kt_messageraw_t {
	zframe_t *_identity; ///< identity given to the message by zeromq router zsocket
                         ///@warning Don't fiddle with it or expect nothing to work anymore!
	char *msgData; ///< actual payload, that's what you look for
	int type; ///< reserved for later use
} kt_messageraw_t;

/**
 * @typedef
 * @struct kt_connconf_t
 * @brief Data structure that holds the whole network configuration.
 * 
 * It contains the network stack configuration (layer 3, 4, 5) and also
 * the type to behave accordingly and also set internal behavioural patterns and
 * contains an URL to connect to or bind to.
 */
typedef struct {
	struct kt_network_stack_t network_config;
	enum kt_application_type type;
	char *base_url;
} kt_connconf_t;

/**
 * @brief Creates an endpoint by prepending the protocol name and appending the
 * port number to the hostname.
 * @param config A struct containing the protocol, hostname/IP and port
 * @return A char* as a human readable string that was used by ZeroMQ
 * @note Unused
 * @deprecated Do not use as it was replaced by internal logic.
 */
char* compile_endpoint_string(kt_connconf_t config);

//#ifdef	__cplusplus
//}
//#endif

#endif	/* KTRANSPORT_H */

