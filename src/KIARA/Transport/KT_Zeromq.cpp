#include "KT_Zeromq.hpp"
#include <iostream>

namespace KIARA {
namespace Transport {

KT_Zeromq::KT_Zeromq() {
    _context = zmq_ctx_new();
	zmq::context_t _context_mt(1);
}

KT_Zeromq::~KT_Zeromq() {
    zmq_ctx_destroy(_context);
}

/**
 * @brief Poller function run in a separate thread which calls the callback.
 * @param socket Indicates on which socket it polls. (Unused)
 * @param endpoint The endpoint on which the polling occurs.
 */
void
KT_Zeromq::poller(void* socket, std::string endpoint) {
    // Find the session according to the endpoint.
    KT_Session* sess = _sessions->find(endpoint)->second;
    while (!interupted) {
        KT_Msg* msg = new KT_Msg;
        // Receive the incoming message and dispatch it to the callback.
        if (0 == recv(*sess, *msg, 0))
            _std_callback(*msg, sess, this);
    }
}

/**
 * @brief Creates a socket according to the settings
 * @param socket_type Different socket types. @see ktransport.h
 * @param listener Differentiate if the socket acts as server or client.
 * @return A pointer to the socket.
 */
void* KT_Zeromq::create_socket(unsigned int socket_type, bool listener) {
    void* socket = nullptr;
    int errcode = 0;

    // Differentiate between the socket types and then if they act as listener.
    switch (socket_type) {
        case KT_STREAM:
        case KT_WEBSERVER:
            socket = zmq_socket(_context, ZMQ_STREAM);
            errcode = errno;
            break;
        case KT_REQUESTREPLY:
            if (listener) {
                socket = zmq_socket(_context, ZMQ_REP);
            } else {
                socket = zmq_socket(_context, ZMQ_REQ);
            }
            errcode = errno;
            break;
        case KT_PUBLISHSUBSCRIBE:
            if (listener) {
                socket = zmq_socket(_context, ZMQ_PUB);
            } else {
                socket = zmq_socket(_context, ZMQ_SUB);
            }
            errcode = errno;
            break;
		case KT_REQUESTREPLYMT:
			socket = zmq_socket(_context, ZMQ_STREAM);
            errcode = errno;
			break;
        default:
            break;
    }
    errno = errcode;
    return socket;
}

/**
 * @brief Connects to configured remote host.
 * @return int 0 if successful, non-zero on failure.
 * @param ret Return a KT_Session pointer when successful.
 */
int
KT_Zeromq::connect(KT_Session** ret) {
    void* socket = nullptr;
    int errcode = 0;

    // Create the socket and check if it's valid.
    socket = create_socket(_configuration.get_application_type(), false);

    if (nullptr == socket) {
        errno = errcode;
        return -1;
    }

    KT_Configuration config = get_configuration();

    // Build the hostname to connect to
    // Example: tcp://domain.tld:1234
    // FIXME: memory leak
    std::string* binding_name = new std::string;

    // Currently only tcp is supported, but technically ZeroMQ also supports
    // udp and sctp.
    if (KT_TCP == config.get_transport_layer()) {
        binding_name->append("tcp://");
    } else {
        return -1;
    }

    // Build hostname and port
    binding_name->append(config.get_hostname());
    binding_name->append(":");
    binding_name->append(std::to_string(config.get_port_number()));

    // FIXME: fix dangerous pointer to underlying data structure
    const char* host = binding_name->c_str();

    // Actually connect to the remote host.
    int rc = zmq_connect(socket, host);

    if (0 != rc) {
        errno = errcode;
        return -1;
    }

    // If the the communication paradigm is Publish/Subscribe
    // subscribe to everything.
    // TODO: Add code to alter the subscription.
    if (KT_PUBLISHSUBSCRIBE == config.get_application_type()) {
        // Just subscribe to everything
        zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0);
    }

    // Every connection has an ZeroMQ-identifier which is retrieved and
    // stored into the session object. Also add the socket and endpoint
    // name into the session object and return this to the caller.
    // Note: I don't know why the session is stored as member value anyway.
    uint8_t id [256];
    size_t id_size = 256;
    zmq_getsockopt(socket, ZMQ_IDENTITY, id, &id_size);

    KT_Session* session = new KT_Session();
    session->set_socket(socket);
    session->set_endpoint(*binding_name);
    std::vector<char> identifier(id, id + id_size);
    session->set_identifier(std::move(identifier));
    _sessions->insert(std::make_pair(*binding_name, session));

    *ret = session;

    return 0;
}

/**
 * @brief Send a message to remote host.
 * @param message Message for the recipient.
 * @param session Session object to identify the remote target.
 * @param linger ZeroMQ does not support linger, value is being ignored.
 * @note Linger time is not supported by ZeroMQ, sending is queuing.
 */
int
KT_Zeromq::send(KT_Msg& message, KT_Session& session, int linger) {
    // If the application is using ZMQ_STREAM send first the identity of
    // the connection to ZeroMQ otherwise it won't know where to send the message.
    switch (_configuration.get_application_type()) {
        case (KT_STREAM):
        case (KT_WEBSERVER):
            int rc = zmq_send(session.get_socket(), session.get_identifier().data(), session.get_identifier().size(), ZMQ_SNDMORE);
            int errcode = errno;
            if (session.get_identifier().size() != static_cast<std::vector<char>::size_type> (rc)) {
                std::cerr << "Failed to send identity to 0mq socket: " << std::strerror(errcode) << std::endl;
                errno = errcode;
                return -1;
            }
    }

    // Now actually send the passed message.
	int rc;
	if(message.is_binary_transport()) {
		rc = zmq_send(session.get_socket(), message.get_payload_binary(), message.get_size(), 0);
	}
	else {
		rc = zmq_send(session.get_socket(), message.get_payload().data(), message.get_payload().size(), 0);
	}
    int errcode = errno;
    if (message.get_payload().size() != static_cast<std::vector<char>::size_type> (rc)) {
        errno = errcode;
        return -1;
    }

    return 0;
}

/**
 * @brief Receive a message from remote host.
 * @param session The session on which to receive the message.
 * @param ret The received message reference in which the payload is copied.
 * @param linger Linger time before it aborts if receiving synchronous,
 *    0 will block forever, -1 will make the call asynchronous and only
 *    return a message if there was one previously received.
 * @return Zero if successful, non-zero on failure.
 * @note Linger time is not supported by ZeroMQ, receive is blocking.
 */
int
KT_Zeromq::recv(KT_Session& session, KT_Msg& ret, int linger) {
    KT_Msg message;
    std::vector<char> buffer;
    std::vector<char> identifier;
    int size;

    // If the application uses ZMQ_STREAM first remove the identity and store
    // it in the session object.
    switch (_configuration.get_application_type()) {
        case (KT_STREAM):
        case (KT_WEBSERVER):
            //std::cerr << "KT_STREAM, stripping zmq_identity" << std::endl;
            zmq_msg_t id;
            zmq_msg_init(&id);
            size = zmq_msg_recv(&id, session.get_socket(), 0);
            identifier.resize(size_t(size));
            char* id_ptr = (char*) zmq_msg_data(&id);
            identifier = std::vector<char>(id_ptr, id_ptr + size);
            session.set_identifier(std::move(identifier));
            zmq_msg_close(&id);
    }

    // Retrieve the actual message.
    zmq_msg_t msg;
    zmq_msg_init(&msg);
    size = zmq_msg_recv(&msg, session.get_socket(), 0);
    if (-1 == size) {
        std::cerr << std::strerror(errno) << std::endl;
        ret = std::move(message);
        return -1;
    }

    // Store the received payload in the KT_Msg *ret
	ret.set_payload((const void*) zmq_msg_data(&msg));
	ret.set_size((size_t)size);
    char* msg_ptr = (char*) zmq_msg_data(&msg);
    buffer = std::vector<char>(msg_ptr, msg_ptr + size);

    ret.set_payload(buffer);
    zmq_msg_close(&msg);

    return 0;
}

/**
 * @brief Disconnect from remote host.
 * @param session Terminate this current session/connection.
 * @return Zero on success, non-zero on failure.
 */
int
KT_Zeromq::disconnect(KT_Session& session) {
    zmq_close(session.get_socket());
    _sessions->erase(session.get_endpoint());
    return 0;
}

/**
 * @brief Register a callback function which is called upon receiving a message
 *   when acting as a server.
 * @param callback Function to be called when a message arrives
 * @warning Callback function must accept KT_Msg* and KT_Session* object
 */
int
KT_Zeromq::register_callback(std::function<void(KT_Msg&, KT_Session*, KT_Connection*) > callback) {
    _std_callback = callback;
    return 0;
}

int
KT_Zeromq::register_callback_str(std::function<DBuffer*(KT_Msg&, KT_Session*, KT_Connection*) > callback) {
    _std_callback_str = callback;
    return 0;
}

/**
 * @brief Bind according to the configuration and listen to incoming messages.
 * @return Zero on success, non-zero on failure.
 * @note Requires a valid callback handler which is called when a message is
 * received, it binds according to the set configuration
 */
int
KT_Zeromq::bind(void) {
    void* socket = nullptr;
    int errcode = 0;

    // Create a socket and verify if it' valid.
    socket = create_socket(_configuration.get_application_type(), true);

    if (nullptr == socket) {
        errno = errcode;
        return -1;
    }

    // Build the hostname to connect to
    // Example: tcp://domain.tld:1234
    KT_Configuration config = get_configuration();
    // memory leak
    std::string* binding_name = new std::string;
    
    // Currently only tcp is supported, but technically ZeroMQ also supports
    // udp and sctp.
    if (KT_TCP == config.get_transport_layer()) {
        binding_name->append("tcp://");
    } else {
        return -1;
    }

    // Build hostname and port
    binding_name->append(config.get_hostname());
    binding_name->append(":");
    binding_name->append(std::to_string(config.get_port_number()));
	
	KT_Session* session = new KT_Session();
    // Actually bind the socket.
	if(_configuration.get_application_type() == KT_REQUESTREPLYMT){
		proxy_thread = new std::thread(&KT_Zeromq::proxy, this, session, binding_name->c_str());
		session->set_endpoint(binding_name->c_str());
		_sessions->insert(std::make_pair(binding_name->c_str(), session));
		return 0;
	}
	else {
		int rc = zmq_bind(socket, binding_name->c_str());
		errcode = errno;
		if (0 != rc) {
			errno = errcode;
			return -1;
		}
		session->set_socket(socket);
	}

    // Create a new session and store it as a member
    
    
    session->set_endpoint(binding_name->c_str());
    _sessions->insert(std::make_pair(binding_name->c_str(), session));

    // Except if it's Publish/Subscribe start a poller thread listening forf
    // incoming messages which are later dispatched to the callback function.
    // Be advised that this code does not check for race-conditions or other
    // multi-threading specific problems. In short: This code is not thread-safe!
    if (KT_PUBLISHSUBSCRIBE != _configuration.get_application_type()) {
        interupted = false;
        poller_thread = new std::thread(&KT_Zeromq::poller, this, socket, binding_name->c_str());
    }
    return 0;
}

void
KT_Zeromq::proxy(KT_Session* session, std::string binding_name) {
	zmq::socket_t clients (_context_mt, ZMQ_ROUTER);
	clients.bind (binding_name.c_str());
	session->set_socket(clients);
	zmq::socket_t workers(_context_mt, ZMQ_DEALER);
	workers.bind ("inproc://workers");
	
	for (int thread_nbr = 0; thread_nbr != 1; thread_nbr++) {
		worker_thread = new std::thread(&KT_Zeromq::worker, this, (void *) &_context_mt, binding_name);
		std::cout << "Adding Thread " << worker_thread->get_id() << std::endl;
	}
	zmq::proxy (clients, workers, NULL);
}

void
KT_Zeromq::worker(void *arg, std::string endpoint){
	zmq::context_t *context = (zmq::context_t *) arg;
	zmq::socket_t socket (*context, ZMQ_REP);
	socket.connect ("inproc://workers");
	while (true) {
        //  Wait for next request from client
        zmq::message_t request;
        socket.recv (&request);
		
		char* msg_ptr = (char*) request.data();
		//std::cout << msg_ptr << std::endl;
		
		KT_Msg* ret = new KT_Msg;
		
		ret->set_payload( (const void*) request.data() );
		ret->set_size( (size_t) request.size() );
		KT_Session* sess = _sessions->find(endpoint)->second;
		//call kiara
		DBuffer *res = _std_callback_str(*ret, sess, this);
		
		std::cout << res->data() << std::endl;
		
		std::string resp;
		//std::cout << std::this_thread::get_id() << std::endl;
        //  Send reply back to client
        zmq::message_t reply((int)res->size());
        memcpy ((void *) reply.data(), res->data(), (int)res->size());
        socket.send (reply);
    }
}

/**
 * @brief Stops listening to incoming messages.
 * @return Zero on success, non-zero on failure.
 */
int
KT_Zeromq::unbind() {
    interupted = true;
    poller_thread->join();
    return 0;
}

} /* namespace Transport */
} /* namespace KIARA */
