/**
 * @file KT_Configuration_glob.h
 * @author Mathias Hablützel <habl@zhaw.ch>
 * @version 1.0
 * @license TBD
 *
 * @brief Global static configuration variables.
 */

#ifndef KT_CONFIGURATION_GLOB_H_
#define KT_CONFIGURATION_GLOB_H_

#ifdef __cplusplus
extern "C" {
#endif

// network layer
typedef unsigned int kt_network_layer;
#define KT_IPLEGACY 4
#define KT_IPV6     6

// transport layer
// see RFC 790 for numbers
typedef unsigned int kt_transport_layer;
#define KT_TCP    6
#define KT_UDP   21
#define KT_DCCP  33
#define KT_SCTP 132

/* TODO: This needs some more fine tuning like setting the protocol level,
 * desired algorithms, key exchanges etc.
 * <habl> 21.08.2013
 */

// crypto layer
typedef unsigned int kt_crypto_layer;
#define KT_NONE   1
#define KT_SSL    2
#define KT_TLS    3

// application layer
typedef unsigned int kt_application_layer;
#define KT_ZEROMQ  1
#define KT_HTTP    2
#define KT_RTSP    3
#define KT_NTP     4
#define KT_FTP     5
#define KT_NFS     6

/* The application type directly decides the communication pattern of the
 * socket and network and transport layers. Also it makes certain assumptions
 * that for e.g. a webserver implies HTTP with TCP on Port 80 and a streaming
 * service uses RTSP and UDP on port 554.
 */

typedef unsigned int kt_application_type;
#define KT_WEBSERVER        1
#define KT_PUBLISHSUBSCRIBE 2
#define KT_STREAM           3
#define KT_ROUNDROBIN       4
#define KT_REQUESTREPLY     5
#define KT_REQUESTREPLYMT   6

#ifdef __cplusplus
}
#endif

#endif /* KT_CONFIGURATION_GLOB_H_ */
