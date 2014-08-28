/** 
 * @file k_transport.h
 * @author Mathias Hablützel <habl@zhaw.ch>
 * @version 1.0
 * @license TBD
 * @brief Header file for C99 bindings.
 */

#ifndef KT_TRANSPORT
#define KT_TRANSPORT

#include <stddef.h>

// Version number
#define KT_TRANSPORT_MAJOR 0
#define KT_TRANSPORT_MINOR 3
#define KT_TRANSPORT_PATCH 2

#ifdef __cplusplus
extern "C" {
#endif

struct kt_conn_session{
    void* connection;
    void* session;
};


// @typdef typedefs to guarantee type safety when using them along with the wrapper
typedef struct kt_conn_session kt_conn_session_t;
typedef struct kt_msg kt_msg_t;
typedef void (*kt_handle_t)(kt_conn_session_t*, kt_msg_t*);
typedef struct kt_configuration kt_configuration_t;

// generic functions
kt_msg_t* kt_msg_new ();
void kt_msg_destroy ( kt_msg_t* );
void kt_msg_set_payload ( kt_msg_t*, void*, size_t);
void* kt_msg_get_payload ( kt_msg_t* );
unsigned int kt_msg_get_payload_size ( kt_msg_t* );
kt_handle_t* kt_create_handle ( void* (*)(kt_conn_session_t*, kt_msg_t*) );

// HTTP functions, not part of API, just for example/demo purposes
// WARNING: THESE FUNCTIONS MAY BE DROPPED, REMOVED, DEPRECATED, MODIFIED OR
//          CHANGED WITHOUT PRIOR NOTIFICATION! THESE FUNCTIONS MAY KILL YOUR
//          CAT, BURN DOWN YOUR HOUSE OR SIMPLY WIPE YOUR HARDDISK!
//          BY USING THEM YOU AGREE THAT YOU KNOW EXACTLY WHAT THE CODE DOES
//          AND YOU ALSO AGREE TO SAY 'my bad' IF SOMETHING GOES WRONG!
kt_msg_t* kt_msg_http_reply (char*, size_t);
char* kt_msg_http_get_payload (kt_msg_t*);

// client functions

/* Connect to a remote endpoint
 *
 * example:
 * 
 * kt_conn_session_t* sess = kt_connect ( "http://domain.tld" );
 */
kt_conn_session_t* kt_connect ( const kt_configuration_t* );

/* Send data to the remote host
 *
 * example: 
 * 
 * kt_conn_session_t* sess = kt_connect ( "tcp://127.0.0.1:5555" );
 * kt_msg_t* msg = kt_msg_new ();
 * char* data = "bla";
 * msg.payload = (void*) data;
 * msg.payload_size = 3;
 *
 * int rc = kt_send ( sess, msg, 0 );
 * assert ( rc == 0 );
 *
 * kt_disconnect ( sess );
 */
int kt_send ( kt_conn_session_t*, kt_msg_t*, int );

/* Receive data from remote endpoint
 *
 * example:
 *
 * kt_conn_session_t* sess = kt_connect ( "tcp://127.0.0.1:5555" );
 * kt_msg_t* msg = kt_recv ( sess, 0 );
 * printf ("%s\n", msg.payload );
 * kt_msg_destroy ( msg );
 *
 * kt_disconnect ( sess );
 */
kt_msg_t* kt_recv ( kt_conn_session_t*, int );

/* Disconnects from remote endpoint
 *
 * example:
 *
 * kt_conn_session_t* sess = kt_connect ( "tcp://127.0.0.1:5555" );
 * kt_disconnect ( sess );
 */
void* kt_disconnect ( kt_conn_session_t* );

// server functions
kt_conn_session_t* kt_init_server ( kt_configuration_t*, kt_handle_t );
void kt_register_handle ( kt_conn_session_t*, kt_handle_t );
int kt_run_server ( kt_conn_session_t* );
void* kt_stop_server ( kt_conn_session_t*, int );

#ifdef __cplusplus
}
#endif

#endif // K_TRANSPORT
