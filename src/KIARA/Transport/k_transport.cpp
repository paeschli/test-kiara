/**
 * @file k_transport.cpp
 * @author Mathias Hablützel <habl@zhaw.ch>
 * @version 1.0
 * @license TBD
 *
 * @brief C99 wrapper for KIARA transport library
 * 
 * This is a C99 wrapper for the C++11 classes. The function usually take an
 * opaque pointer which is then dynamically casted to an C++ object. The
 * functions follow the API contract for KIARA defined in doc/Library API contract.md
 * 
 * @see doc/Library API contract.md
 */

#include "k_transport.h"

#include <iostream>
#include <vector>
#include <functional>

#include "KT_C99_CallbackWrapper.hpp"
#include "KT_Configuration.h"
#include "KT_Configuration.hpp"
#include "KT_Configuration_glob.h"
#include "KT_Msg.hpp"
#include "KT_Session.hpp"
#include "KT_Zeromq.hpp"

#include "KT_HTTP_Responder.hpp"
#include "KT_HTTP_Parser.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/* KIARA::Transport::KT_Msg C wrapper */
KIARA::Transport::KT_C99_CallbackWrapper* cb_wrapper;

/**
 * @brief Create a new message
 * @return An opaque pointer to a message
 */
kt_msg_t* kt_msg_new()
{
	return reinterpret_cast<kt_msg_t*>(new KIARA::Transport::KT_Msg());
}

/**
 * @brief Destroy a message
 * @param c_msg The message to be destroyed.
 */
void kt_msg_destroy(kt_msg_t* c_msg)
{
	KIARA::Transport::KT_Msg *msg = reinterpret_cast<KIARA::Transport::KT_Msg *>(c_msg);
	delete(msg);
}

/**
 * @brief Set the payload of the message.
 * @param c_msg The message to assign the payload to.
 * @param payload The actual payload.
 * @param payload_size The payloads size.
 * @note Does not take ownership of the passed payload.
 */
void kt_msg_set_payload(kt_msg_t* c_msg, void* payload, size_t payload_size)
{
	KIARA::Transport::KT_Msg *msg = reinterpret_cast<KIARA::Transport::KT_Msg *>(c_msg);
	char* payload_ptr = reinterpret_cast<char*>(payload);
        // Copy the payload into the vector
	std::vector<char> payload_vector(payload_ptr, payload_ptr + payload_size);
	msg->set_payload(payload_vector);
}

/**
 * @brief Get a pointer to the payload.
 * @param c_msg The message of which the pointer is to be extracted.
 * @return A pointer to the payload of the passed message.
 * @warning The payload is not copied, do not take ownership, the pointer lives
 *   as long the passed message exists. Destroy the message after use.
 * @see kt_msg_destroy
 */
void* kt_msg_get_payload(kt_msg_t* c_msg)
{
	KIARA::Transport::KT_Msg *msg = reinterpret_cast<KIARA::Transport::KT_Msg *>(c_msg);
        // Dangerous as kt_msg_set_payload does copy and this one does not.
        // FIXME: Fix handling to be identical, always copy.
	return msg->get_payload_ptr()->data();
}

/**
 * @brief Get the payload size.
 * @param c_msg The message of which the payloads size is to be extracted.
 * @return The message size.
 */
unsigned int kt_msg_get_payload_size(kt_msg_t* c_msg)
{
    KIARA::Transport::KT_Msg *msg = reinterpret_cast<KIARA::Transport::KT_Msg *>(c_msg);
    return msg->get_payload().size();
}

/**
 * @brief Generate a HTTP 200 OK kt_msg_t with the passed payload.
 * @param content The payload to be embedded in a HTTP response.
 * @param size Size of the payload.
 * @return An opaque pointer to the generated message.
 * @note Take ownership.
 * @warning This function is NOT intended to be used and available purely for
 *   testing purposes.
 * @deprecated If you want to render HTTP messages use your own library as this
 *   function may be removed without prior warning.
 * @see kt_msg_destroy
 */
kt_msg_t* kt_msg_http_reply(char* content, size_t size)
{
    kt_msg_t* msg = kt_msg_new();
    std::string payload = KIARA::Transport::KT_HTTP_Responder::generate_200_OK(std::vector<char>(content, content + size));
    char* p = reinterpret_cast<char*>(malloc(sizeof(char)*payload.size()));
    payload.copy(p,payload.size());
    kt_msg_set_payload(msg, p, payload.size());
    return msg;
}

/**
 * @brief Extract the payload from an HTTP message.
 * @param c_msg The message of which the payload is extracted.
 * @return A pointer to the payload.
 * @warning This function is NOT intended to be used and available purely for
 *   testing purposes.
 * @deprecated If you want to extract HTTP messages use your own library as this
 *   function may be removed without prior warning.
 * @note Take ownership, free the returned pointer.
 */
char* kt_msg_http_get_payload(kt_msg_t* c_msg)
{
    KIARA::Transport::KT_Msg *msg = reinterpret_cast<KIARA::Transport::KT_Msg *> (c_msg);
    
    // Instantiate the parser and extract the payload
    KIARA::Transport::KT_HTTP_Parser parser (*msg);
    std::string s = parser.get_payload();
    
    // Append a NULL byte because it's returned as a c-string
    s.append(1, '\0');
    
    // Copy the string to the heap.
    char* ptr = (char*)malloc(sizeof(char) * s.size());
    s.copy(ptr, s.size());
    return ptr;
}

/**
 * @brief Connect to the remote host.
 * @param conf The connection configuration.
 * @return A connection session.
 */
kt_conn_session_t* kt_connect(const kt_configuration_t* conf)
{
    const KIARA::Transport::KT_Configuration* config =
            reinterpret_cast<const KIARA::Transport::KT_Configuration*>(conf);
    KIARA::Transport::KT_Connection* connection;

    // Differentiate between the implementations, currently only ZeroMQ supported
    if (KT_ZEROMQ == config->get_application_layer())
    {
            connection = new KIARA::Transport::KT_Zeromq();
            connection->set_configuration(*config);
    }
    else {
        std::cerr << "Could not connect, you indicated an unsupported application layer" << std::endl;
        return nullptr;
    }

    // Connect and assign session
    KIARA::Transport::KT_Session* session = nullptr;
    if (0 != connection->connect(&session))
    {
        std::cerr << "Failed to connect" << std::endl;
    }

    if (nullptr == session)
    {
        std::cerr << "Session object was not set" << std::endl;
        return nullptr;
    }

    // Store the connection and session in the struct
    kt_conn_session_t* conn_session = new kt_conn_session;
    conn_session->connection = connection;
    conn_session->session = session;
    return conn_session;
}

/**
 * @brief Disconnect from remote host.
 * @param conn_session The connection-session to close.
 * @return The assigned data structure to the session.
 * @note Take ownership of the data structure.
 */
void* kt_disconnect(kt_conn_session_t* conn_session)
{
    KIARA::Transport::KT_Connection* connection =
            reinterpret_cast<KIARA::Transport::KT_Connection*>(conn_session->connection);
    KIARA::Transport::KT_Session* session =
            reinterpret_cast<KIARA::Transport::KT_Session*>(conn_session->session);

    // Close the connection.
    connection->disconnect(*session);

    // Clean up.
    delete connection;
    void* k_user_data = session->get_k_user_data();
    delete session;

    return k_user_data;
}

/**
 * @brief Sends a message to the remote host.
 * @param conn_session Connection/Session struct.
 * @param msg Message to be send.
 * @param linger Time of how long sending should be tried.
 * @return Zero on success, non-zero on failure.
 * @warning Not all transport stacks support linger.
 * @note Takes ownership of the message.
 */
int kt_send(kt_conn_session_t* conn_session, kt_msg_t* msg, int linger)
{
    KIARA::Transport::KT_Connection* connection =
            reinterpret_cast<KIARA::Transport::KT_Connection*> (conn_session->connection);
    KIARA::Transport::KT_Session* session =
            reinterpret_cast<KIARA::Transport::KT_Session*> (conn_session->session);
    KIARA::Transport::KT_Msg* message =
            reinterpret_cast<KIARA::Transport::KT_Msg*> (msg);

    // Send the message.
    int ret = connection->send(*message, *session, linger);

    // Destroy the message as we take ownership.
    kt_msg_destroy(msg);
    // Better be safe than sorry.
    msg = nullptr;

    return ret;
}

/**
 * @brief Receive a message.
 * @param conn_session Connection/Session structure on which a message shall received
 * @param linger How long it shall wait for a message, 0 for infinite, any other
 *   number is interpreted as seconds.
 * @return The received message
 * @warning Currently linger is not implemented and not necessarily supported by
 *   the actual transport stack.
 */
kt_msg_t* kt_recv(kt_conn_session_t* conn_session, int linger)
{
    KIARA::Transport::KT_Connection* connection =
            reinterpret_cast<KIARA::Transport::KT_Connection*>(conn_session->connection);
    KIARA::Transport::KT_Session* session =
            reinterpret_cast<KIARA::Transport::KT_Session*>(conn_session->session);

    KIARA::Transport::KT_Msg* message = new KIARA::Transport::KT_Msg;
    if (0 != connection->recv(*session, *message, linger)) {
        std::cerr << "Receive failed" << std::endl;
        return nullptr;
    }

    return reinterpret_cast<kt_msg_t*>(message);
}

/**
 * @brief Initialize the server but not bind it.
 * @param conf The configuration used by the server.
 * @param callback_handle Callback function that is called upon message reception.
 * @return A connection/session struct
 */
kt_conn_session_t* kt_init_server(kt_configuration_t* conf,
        kt_handle_t callback_handle)
{
    KIARA::Transport::KT_Configuration* config =
            reinterpret_cast<KIARA::Transport::KT_Configuration*>(conf);
    KIARA::Transport::KT_Connection* connection;

    // Get configuration and initialize KT_Zeromq object
    if (KT_WEBSERVER == config->get_application_type()) {
        if (0 == config->get_port_number()) {
            // TODO: How to check if we have enough privileges to allocate port 80?
            config->set_port_number(8080);
        }
        if (KT_ZEROMQ == config->get_application_layer()) {
            connection = new KIARA::Transport::KT_Zeromq();
            connection->set_configuration(*config);
            goto connection_object_created;
        } else {
            std::cerr << "Could not connect, you indicated an unsupported application layer" << std::endl;
        }
    }
    if (KT_REQUESTREPLY == config->get_application_type()) {
        if (0 == config->get_port_number()) {
            config->set_port_number(5555);
        }
        connection = new KIARA::Transport::KT_Zeromq();
        connection->set_configuration(*config);
        goto connection_object_created;
    }
    if (KT_ZEROMQ == config->get_application_layer())
    {
        connection = new KIARA::Transport::KT_Zeromq();
        connection->set_configuration(*config);
        goto connection_object_created;
    }
    else {
        std::cerr << "Could not connect, you indicated an unsupported application layer" << std::endl;
        return nullptr;
    }

    // Save connection and session data
    connection_object_created:
    kt_conn_session_t* conn_session = new kt_conn_session;
    conn_session->connection = connection;
    conn_session->session = nullptr;

    // Register the callback
    // TODO: Add some check that you can't run the server without a valid callback.
    if (NULL != callback_handle) {
        kt_register_handle(conn_session, callback_handle);
    }

    return conn_session;
}

/**
 * @brief Register the callback function for the server.
 * @param conn_session Connection/session struct to which the callback is attached
 * @param callback_handle Callback function
 * @note Usually you don't call this function directly, use kt_init_server
 * @see kt_init_server
 */
void kt_register_handle(kt_conn_session_t* conn_session, kt_handle_t callback_handle)
{
    KIARA::Transport::KT_Connection* connection =
            reinterpret_cast<KIARA::Transport::KT_Connection*>(conn_session->connection);
    // Wrapper needed to map C++ objects to C structs which are passed to the
    // callback function in C.
    // Memory leak, cb_wrapper is never deleted
    cb_wrapper = new KIARA::Transport::KT_C99_CallbackWrapper(callback_handle);
    connection->register_callback(cb_wrapper->make_function());
}

/**
 * @brief Start the server.
 * @param conn_session Connection/session struct of the server.
 * @return Zero on success, non-zero on failure.
 */
int kt_run_server ( kt_conn_session_t* conn_session )
{
    KIARA::Transport::KT_Connection* connection =
            reinterpret_cast<KIARA::Transport::KT_Connection*> (conn_session->connection);

    // Bind the socket/connection for listening to connections.
    int ret = connection->bind();
    if ( 0 != ret )
    {
        std::cerr << "Failed to bind" << std::endl;
        std::cerr << std::strerror(ret) << std::endl;
        return -1;
    }
    conn_session->session = reinterpret_cast<void*>(connection->get_session()->begin()->second);
    return 0;
}

/**
 * @brief Stop the server.
 * @param conn_session Connection/session struct of the server.
 * @param linger Time to wait before it kills the server.
 * @return The assigned data structure to the session.
 * @note Take ownership of the data structure.
 * @warning Linger time is not implemented!
 */
void* kt_stop_server ( kt_conn_session_t* conn_session, int linger )
{
    KIARA::Transport::KT_Connection* connection =
            reinterpret_cast<KIARA::Transport::KT_Connection*> (conn_session->connection);
    KIARA::Transport::KT_Session* session =
            reinterpret_cast<KIARA::Transport::KT_Session*> (conn_session->session);
    // Unbind the server and stop listening to incoming connections.
    connection->unbind();
    // Return the session data assigned by the programmer.
    return session->get_k_user_data();
}

#ifdef __cplusplus
}
#endif
