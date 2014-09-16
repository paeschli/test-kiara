/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2013, 2014  German Research Center for Artificial Intelligence (DFKI)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
 * Network.cpp
 *
 *  Created on: Feb 7, 2014
 *      Author: Dmitri Rubinstein
 */

#include "Network.hpp"
#include <KIARA/Runtime/RuntimeContext.hpp>
#include <KIARA/Runtime/RuntimeEnvironment.hpp>
#include <KIARA/Utils/URL.hpp>
#include <KIARA/IDL/IDLWriter.hpp>
#include <KIARA/Transport/Transport.hpp>
#include <KIARA/Transport/HttpTransport.hpp>
#include <KIARA/Transport/TcpBlockTransport.hpp>
#include <KIARA/CDT/kr_dumpdata.h>
#include <uriparser/Uri.h>
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include "../Transport/KT_Zeromq.hpp"
#include "../Transport/KT_HTTP_Parser.hpp"
#include "../Transport/KT_HTTP_Responder.hpp"
#include "../Transport/KT_Msg.hpp"

#define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>


namespace KIARA
{

namespace Impl
{
	
void callback_handler ( KIARA::Transport::KT_Msg&, KIARA::Transport::KT_Session*, KIARA::Transport::KT_Connection* );

DBuffer* callback_handler_mt ( KIARA::Transport::KT_Msg&, KIARA::Transport::KT_Session*, KIARA::Transport::KT_Connection* );
	
/// Connection

Connection::Connection(Context *context, const std::string &transportName)
    : Base(context)
    , funcObjects_()
    , data_(0)
    , transportName_(transportName)
    , runtimeEnvironment_(0)
    , transportConnection_()
{
}

Connection::~Connection()
{
    // destroy all func objects
    for (FuncObjMap::iterator it = funcObjects_.begin(), end = funcObjects_.end();
            it != end; ++it)
    {
        destroyFuncObj(it->second);
        //it->second = 0;
    }
    delete runtimeEnvironment_;
}

KIARA_FuncObj * Connection::createFuncObj()
{
    KIARA_FuncObj *fobj = new KIARA_FuncObj;
    fobj->func = 0;
    fobj->base.vafunc = 0;
    fobj->base.connection = wrap(this);
    fobj->base.funcType = 0;
    return fobj;
}

void Connection::destroyFuncObj(KIARA_FuncObj *funcObj)
{
    delete funcObj;
}

// ClientConnection

ClientConnection::ClientConnection(Context *context, const std::string &uri)
    : Connection(context, "http")  // initially connection is always http
    , uri_()
    , initFunc_(0)
    , finalizeFunc_(0)
    , getMimeType_(0)
    , urlLoaderConnection_(0)
{
    std::string serverConfigText;
    std::string errorMsg;

    runtimeEnvironment_ = context->getRuntimeContext().createEnvironment();
    if (!runtimeEnvironment_)
    {
        setError(KIARA_INIT_ERROR, "Could not create runtime environment");
        return;
    }

    if (!runtimeEnvironment_->startInitialization(&errorMsg))
    {
        setError(KIARA_INIT_ERROR, "Could not initialize runtime environment: "+errorMsg);
        return;
    }

    urlLoaderConnection_ = KIARA::URLLoader::createConnection();

    // connect and get a serverConfigText
    // bool result = KIARA::URLLoader::load(uri, serverConfigText);

    // FIXME here a global security configuration is used, which is setup when node is started
    //bool result = KIARA::URLLoader::load(urlLoaderConnection_, uri, serverConfigText, context->getSecurityConfiguraton());
	URL *kt_url = new URL(uri, true);
	
	KIARA::Transport::KT_Configuration config;
	config.set_application_type ( KT_STREAM );

	config.set_host( KT_TCP, kt_url->host, atoi(kt_url->port.c_str()));

	KIARA::Transport::KT_Connection* connection = new KIARA::Transport::KT_Zeromq ();
	connection->set_configuration (config);

	KIARA::Transport::KT_Session* session = nullptr;
	
	if ( 0 != connection->connect(&session) )
	{
		std::cerr << "Failed to connect" << std::endl;
	}

	if (nullptr == session)
	{
		std::cerr << "Session object was not set" << std::endl;
	}

	KIARA::Transport::KT_Msg request;
	std::string payload ( "GET /service HTTP/1.1\r\nUser-Agent: KIARA\r\nHost: localhost:5555\r\n\r\n" );

	request.set_payload(payload);

	if (0 != connection->send(request, *session, 0))
	{
		std::cerr << "Failed to send payload" << std::endl;
	}
	
	KIARA::Transport::KT_Msg reply;
	if (0 != connection->recv(*session, reply, 0))
		std::cerr << "Receive failed" << std::endl;

	KIARA::Transport::KT_HTTP_Parser parser (reply);
	
    /*if (!parser)
    {
        KIARA::URLLoader::deleteConnection(urlLoaderConnection_);
        urlLoaderConnection_ = 0;
        setError(KIARA_CONNECTION_ERROR, "Could not load server configuration");
        return;
    }*/
    KIARA::ServerConfiguration serverConfig;
	
    if (!serverConfig.fromJSON(parser.get_payload()))
    {
        KIARA::URLLoader::deleteConnection(urlLoaderConnection_);
        urlLoaderConnection_ = 0;
        setError(KIARA_CONNECTION_ERROR, "Could not parse server configuration");
        return;
    }

    // load IDL

    if (!serverConfig.idlContents.empty())
    {
        std::istringstream iss(serverConfig.idlContents);
        if (!context->loadIDL(iss, uri))
        {
            setError(KIARA_CONNECTION_ERROR, "Could not parse IDL from configuration string");
            return;
        }
    }
    else if (!serverConfig.idlURL.empty())
    {
        std::string idlURL = KIARA::URL::resolve(uri, serverConfig.idlURL);

        bool samePeer = KIARA::URL::httpsEqHostAndPort(uri, idlURL);

        if (!samePeer)
        {  if (!context->loadIDLFromURL(idlURL))
           {
              setError(KIARA_CONNECTION_ERROR, "Could not load IDL from "+idlURL+" URL");
              return;
           }
        }
        else
        {
           if (!context->loadIDLFromURL(urlLoaderConnection_, idlURL))
           {
              setError(KIARA_CONNECTION_ERROR, "Could not load IDL from "+idlURL+" URL");
              return;
           }
        }
    }
    else
    {
        setError(KIARA_CONNECTION_ERROR, "No IDL specified in server configuration");
        return;
    }

    // find matching endpoint
    KIARA::ServerInfo *serverInfo = 0;
   const Transport::Transport *transport = 0;

    for (size_t i = 0; i < serverConfig.servers.size(); ++i)
    {
        KIARA::ServerInfo &si = serverConfig.servers[i];

        const Transport::Transport *t = Transport::Transport::getTransportByName(si.transport.name.c_str());

        if (t)
        {
            // we change selected endpoint only if priority is higher
            // i.e. when priority value is less than current one
            if (transport && transport->getPriority() < t->getPriority())
                continue;

            serverInfo = &si;
            transport = t;
        }
    }

    if (!serverInfo)
    {
        setError(KIARA_CONNECTION_ERROR, "No matching endpoint found");
        return;
    }

    DFC_DEBUG("Selected transport: "<<transport->getName());
    DFC_DEBUG("Selected protocol: "<<serverInfo->protocol.name);

    const std::vector<std::string> & llvmModuleNames = context->getLLVMModuleNames();
    for (std::vector<std::string>::const_iterator it = llvmModuleNames.begin(),
        end = llvmModuleNames.end(); it != end; ++it)
    {
        DFC_DEBUG("Loading LLVM module "<<*it);
        if (!getRuntimeEnvironment().loadModule(*it, &errorMsg))
        {
            setError(KIARA_CONNECTION_ERROR, "Could not load " + (*it) + " module: " + errorMsg);
            return;
        }
    }

    // load required protocol
    if (!getRuntimeEnvironment().loadComponent(serverInfo->protocol.name, &errorMsg))
    {
        setError(KIARA_CONNECTION_ERROR, "Could not load " + serverInfo->protocol.name + " component: "+errorMsg);
        return;
    }

    if (!getRuntimeEnvironment().finishInitialization(&errorMsg))
    {
        setError(KIARA_INIT_ERROR, "Could not finish initialization of runtime environment: "+errorMsg);
        return;
    }

    setTransportName(serverInfo->transport.name);

    Transport::NetworkHandler nh = transport->getNetworkHandler();

    //getRuntimeEnvironment().registerExternalFunction("getConnection", (void*)nh.getConnection);
    //getRuntimeEnvironment().registerExternalFunction("getServiceConnection", (void*)nh.getServiceConnection);
    getRuntimeEnvironment().registerExternalFunction("sendData", (void*)nh.sendData);

    URL configUrl(uri);
    if (!configUrl.isValid())
    {
        setError(KIARA_NETWORK_ERROR, "Invalid URL: "+uri);
    }

    // Setup URI for connection
    uri_ = KIARA::URL::resolve(uri, serverInfo->transport.url);

    DFC_DEBUG("Open connection to: "<<uri_);

    boost::system::error_code ec;
    setTransportConnection(transport->openConnection(uri_, context->getIOService(), &ec));
    if (ec)
    {
        setError(KIARA_NETWORK_ERROR, boost::lexical_cast<std::string>(ec));
    }

    getMimeType_ =
        (KIARA_GetMimeType)(intptr_t)
        getRuntimeEnvironment().requestPointerToFunction("getMimeType");
    mimeType_ = getMimeType_();
}

ClientConnection::ClientConnection(
    Context *context,
    const std::string &transportName,
    const KIARA::Transport::Connection::Ptr &transportConnection)
    : Connection(context, transportName)
    , uri_()
    , initFunc_(0)
    , finalizeFunc_(0)
    , getMimeType_(0)
    , urlLoaderConnection_(0)
{
    setTransportConnection(transportConnection);
}

ClientConnection::~ClientConnection()
{
    KIARA::URLLoader::deleteConnection(urlLoaderConnection_);
    if (finalizeFunc_)
        finalizeFunc_(wrap(this)); // FIXME add check for success of the operation
}

/// Server::ServerConnectionHandler

class Server::ServerConnectionHandler : public KIARA::Impl::Connection,
                                        public KIARA::Transport::ConnectionHandler
{
public:
    explicit ServerConnectionHandler(Server &server, const Transport::Transport *transport);
    ~ServerConnectionHandler();

    const char * getConnectionURI() const;

    ServiceHandler * getServiceHandler() const { return serviceHandler_; }

protected:

    void onStart(const Transport::ConnectionPtr &connection);

    Transport::RequestResult onRequest(
        const Transport::ConnectionPtr &connection,
        const Transport::TransportMessage &request,
        Transport::TransportMessage &response);

    void onDestroy(const Transport::Connection *connection);


private:
    const Transport::Transport *transport_;
    ServiceHandler *serviceHandler_;
    Server &server_;
};

Server::ServerConnectionHandler::ServerConnectionHandler(Server &server, const Transport::Transport *transport)
    : KIARA::Impl::Connection(server.getContext(), transport->getName())
    , transport_(transport)
    , serviceHandler_(0)
    , server_(server)
{
}

Server::ServerConnectionHandler::~ServerConnectionHandler()
{
}

const char * Server::ServerConnectionHandler::getConnectionURI() const
{
    // FIXME What should we return here ?
    return "";
}

void Server::ServerConnectionHandler::onStart(const Transport::ConnectionPtr &connection)
{
    DFC_DEBUG("Called ServerConnectionHandler::onStart, detected transport: " << transport_->getName());
    // FIXME currently does not work for http protocol
    if (strcmp(transport_->getName(), "http") != 0)
    {
        URL url;
        url.scheme = transport_->getName();
        url.host = connection->getLocalHostName();
        url.port = boost::lexical_cast<std::string>(connection->getLocalPort());
        Transport::TransportAddress::Ptr addr = transport_->createAddress(url.toString());

        serviceHandler_ = server_.findAcceptingServiceHandler(addr);
    }
}

Transport::RequestResult Server::ServerConnectionHandler::onRequest(
    const Transport::ConnectionPtr &connection,
    const Transport::TransportMessage &request,
    Transport::TransportMessage &response)
{
    return server_.handleRequest(*this, connection, request, response);
}

void Server::ServerConnectionHandler::onDestroy(const Transport::Connection *connection)
{
    delete this;
}

/// Server

/*
 * requirements:
 *
 * on a single port can be only one transport
 * a single transport may support multiple protocols,
 * but only if there is a way to identify protocol.
 *
 */

Server::Server(
        Context *context,
        const std::string &address,
        unsigned int port,
        const std::string &configPath)
    : Base(context)
    , Transport::Server(/*thread pool size = */1)
    , configHost_(address)
    , configPort_(port)
    , configPath_(configPath)
{
    // listen for negotiation connections
    addPortListener(configHost_, configPort_, "http");
}

Server::~Server()
{
    for (ServiceHandlerMap::iterator it = serviceHandlers_.begin(), end = serviceHandlers_.end(); it != end; ++it)
    {
        delete it->second;
        it->second = 0;
    }
}

bool Server::addPortListener(const std::string &host, unsigned int port, const std::string &transportName)
{
    DFC_DEBUG("Server::addPortListener: "<<host<<":"<<port<<" "<<transportName);
	
    HostAndPort hostAndPort(host, port);
    TransportEntryList::iterator it = transportEntries_.find(hostAndPort);
    if (it != transportEntries_.end())
    {
        if (transportName != it->second->transport->getName())
        {
            setError(KIARA_NETWORK_ERROR, "Port "+boost::lexical_cast<std::string>(port)
                +" already bound to transport "+std::string(it->second->transport->getName()));
            return false;
        }
        return true;
    }

    const Transport::Transport *transport =
        Transport::Transport::getTransportByName(transportName.c_str());

    TransportEntry::Ptr tentry(new TransportEntry(transport));
    transportEntries_[hostAndPort] = tentry;
    /*listen(host, boost::lexical_cast<std::string>(port),
           boost::bind(&Server::createConnection, this, hostAndPort, _1));*/
	
	//ZMQ implementation
	
	KIARA::Transport::KT_Configuration config;
	if(!transportName.compare("http")) {
		std::cout << "Server::addPortListener HTTP: "<<host<<":"<<port<<" "<<transportName << std::endl;
		config.set_application_type ( KT_STREAM );
	}
	if(!transportName.compare("tcp")) {
		std::cout << "Server::addPortListener TCP: "<<host<<":"<<port<<" "<<transportName << std::endl;
		config.set_application_type ( KT_REQUESTREPLYMT );
	}
	config.set_transport_layer( KT_TCP );
	config.set_hostname( host );
	config.set_port_number( port );
	config.set_config_path(configPath_);

	KIARA::Transport::KT_Connection* connection = new KIARA::Transport::KT_Zeromq ();
	connection->set_configuration (config);

	if(connection->get_configuration().get_application_type() == KT_REQUESTREPLYMT) {
		connection->register_callback_str( &callback_handler_mt );
	}
	else {
		connection->register_callback( &callback_handler );
	}
	//connection->register_callback( &callback_handler );
	connection->bind();
	
	connection->get_session()->begin()->second->set_k_user_data(this);
	//End ZMQ implementation
	return true;
}

DBuffer* callback_handler_mt ( KIARA::Transport::KT_Msg& msg, KIARA::Transport::KT_Session* sess, KIARA::Transport::KT_Connection* connection ) {
	std::string payload = "";
	std::string res = "";
	DBuffer *response = new DBuffer();
	
	Server *server = (Server*) sess->get_k_user_data();
	
	if(connection->get_configuration().get_application_type() == KT_REQUESTREPLYMT) {
		std::vector<char> answer_vector = msg.get_payload();
		std::string answer(answer_vector.begin(), answer_vector.end());
		
		KIARA::Transport::TcpBlockTransport *myTransport = new KIARA::Transport::TcpBlockTransport();
		Transport::TcpBlockAddress::Ptr addr(
			new Transport::TcpBlockAddress(
				connection->get_configuration().get_hostname(),
				connection->get_configuration().get_port_number(),
				myTransport
			)
		);
		
		if (ServiceHandler *serviceHandler = server->findAcceptingServiceHandler(addr))
		{
			
			serviceHandler->performCallZmq((const char*)msg.get_payload_binary(), msg.get_size(), response);
			res.append(response->data());
			
			std::string res_final;
			res_final = res.substr(0, response->size());
			payload = res_final;
		}
	}
	
	return response;
}

void callback_handler ( KIARA::Transport::KT_Msg& msg, KIARA::Transport::KT_Session* sess, KIARA::Transport::KT_Connection* connection ) {
	
	std::string payload = "";
	std::string res = "";
	
	Server *server = (Server*) sess->get_k_user_data();
	
	if(connection->get_configuration().get_application_type() == KT_STREAM) {
		std::cout << "HTTP request handling" << std::endl;
		KIARA::Transport::KT_HTTP_Parser parser (msg);
		
		if(parser.get_url().compare( 0, connection->get_configuration().get_config_path().length(), connection->get_configuration().get_config_path()) == 0)
		{
			KIARA::ServerConfiguration serverConfiguration;
			server->generateServerConfiguration(serverConfiguration, connection->get_configuration().get_hostname(), "");
			std::string config = serverConfiguration.toJSON();
			payload = KIARA::Transport::KT_HTTP_Responder::generate_200_OK( std::vector<char> (config.begin(), config.end() ) );
		}
		else 
		{
			KIARA::Transport::HttpTransport *myTransport = new KIARA::Transport::HttpTransport();
			Transport::HttpAddress::Ptr addr(
				new Transport::HttpAddress(
					connection->get_configuration().get_hostname(),
					connection->get_configuration().get_port_number(),
					parser.get_url().substr( 0, strlen(parser.get_url().c_str()) ),
					myTransport
				)
			);
			if (ServiceHandler *serviceHandler = server->findAcceptingServiceHandler(addr))
			{
				DBuffer *response = new DBuffer();
				serviceHandler->performCallZmq(parser.get_payload().c_str(), strlen(parser.get_payload().c_str()), response);
				res.append(response->data());

				std::string res_final;
				res_final = res.substr(0, response->size());

				payload = KIARA::Transport::KT_HTTP_Responder::generate_200_OK( std::vector<char> (res_final.begin(), res_final.end() ) );
			}
		}
	}
	if(connection->get_configuration().get_application_type() == KT_REQUESTREPLY) {
		std::cout << "TCP request handling" << std::endl;
		std::vector<char> answer_vector = msg.get_payload();
		std::string answer(answer_vector.begin(), answer_vector.end());
		
		KIARA::Transport::TcpBlockTransport *myTransport = new KIARA::Transport::TcpBlockTransport();
		Transport::TcpBlockAddress::Ptr addr(
			new Transport::TcpBlockAddress(
				connection->get_configuration().get_hostname(),
				connection->get_configuration().get_port_number(),
				myTransport
			)
		);
		
		if (ServiceHandler *serviceHandler = server->findAcceptingServiceHandler(addr))
		{
			DBuffer *response = new DBuffer();
			serviceHandler->performCallZmq(answer.c_str(), answer.length(), response);
			res.append(response->data());
			
			std::string res_final;
			res_final = res.substr(0, response->size());
			payload = res_final;
		}
	}

	KIARA::Transport::KT_Msg message;
	message.set_payload ( payload );
	
	connection->send ( message, (*sess), 0 );
}

Transport::RequestResult Server::handleRequest(
    Server::ServerConnectionHandler &handler,
    const Transport::Connection::Ptr &connection,
    const Transport::TransportMessage &request,
    Transport::TransportMessage &response)
{
    DFC_DEBUG("Server::handleRequest: transport name: "<<request.getTransport()->getName());

    // FIXME CAUTION !!!
    // FIXME This code is run in multithreaded context, be careful what data structures
    //       are accessed there.
    //       Don't access without mutexes:
    //       - KIARA Database (Type System, IDL information, etc.)

    if (strcmp(request.getTransport()->getName(), "http") == 0)
    {
        const Transport::HttpRequest &req = static_cast<const Transport::HttpRequest &>(request);
        Transport::HttpResponse &res = static_cast<Transport::HttpResponse &>(response);

        // Decode url to path.
        std::string requestPath;
        if (!KIARA::Transport::HttpRequestHandler::decodeURL(req.uri, requestPath))
        {
            res.setDefaultHTMLResponse(KIARA::Transport::HttpResponse::BAD_REQUEST);
            return Transport::SEND_RESPONSE;
        }

        DFC_DEBUG("KIARA_Server::handleRequest: path: "<<requestPath);

        if (requestPath == configPath_)
        {
            ServerConfiguration serverConfiguration;
            generateServerConfiguration(serverConfiguration, connection->getLocalHostName(), connection->getRemoteHostName());
            res.setTextResponse(serverConfiguration.toJSON(), "application/json");
            return Transport::SEND_RESPONSE;
        }
		
        Transport::HttpAddress::Ptr addr(
            new Transport::HttpAddress(connection->getLocalHostName(), connection->getLocalPort(), requestPath, request.getTransport()));

        if (ServiceHandler *serviceHandler = findAcceptingServiceHandler(addr))
        {
            DFC_DEBUG("KIARA_Server: Found service !");

            serviceHandler->performCall(&handler, req.getPayload(), res.getPayload());

            res.setResponseHeaders(serviceHandler->getMimeType());
            return Transport::SEND_RESPONSE;
        }

        res.setDefaultHTMLResponse(KIARA::Transport::HttpResponse::NOT_FOUND);
        return Transport::SEND_RESPONSE;
    }

    DFC_IFDEBUG(kr_dump_data("KIARA_Server::handleTcpRequest Request contents", stderr,
        (unsigned char *)request.getPayload().data(), request.getPayloadSize(), 0));

    if (ServiceHandler *serviceHandler = handler.getServiceHandler())
    {
        DFC_DEBUG("KIARA_Server: Found service !");
        DFC_IFDEBUG(kr_dump_data("KIARA_Server: Request contents", stderr,
            (unsigned char *)request.getPayload().data(), request.getPayloadSize(), 0));

        serviceHandler->performCall(&handler, request.getPayload(), response.getPayload());
        return Transport::SEND_RESPONSE;
    }

    // we response with empty message in the case of error
    response.clear();
    return Transport::SEND_RESPONSE;
}

void Server::generateServerConfiguration(
    ServerConfiguration &serverConfiguration,
    const std::string &localHostName,
    const std::string &remoteHostName)
{
    KIARA::ServerInfo serverInfo;
    for (ServiceHandlerMap::const_iterator it = serviceHandlers_.begin(), end = serviceHandlers_.end(); it != end; ++it)
    {
        serverInfo.clear();
        serverInfo.protocol = it->second->getProtocolInfo();
        serverInfo.services.assign(1, "*");
        serverInfo.transport.name = it->first->getTransport()->getName();

        URL serviceUrl(it->first->toString());
        if (!serviceUrl.isValid())
        {
            continue;
        }

        if (serviceUrl.host == "0.0.0.0")
        {
            serviceUrl.host = localHostName;
        }

        serverInfo.transport.url = serviceUrl.toString();

        serverConfiguration.servers.push_back(serverInfo);
    }

    for (ServiceSet::iterator it = services_.begin(), end = services_.end(); it != end; ++it)
    {
        serverConfiguration.idlContents += (*it)->getIDLContents();
    }
}

KIARA_Result Server::addService(const char *path, const char *protocol, Service *service)
{
    // FIXME What about service management ? If server owns all added services, we need to delete them when they are not used
    //       Reference counting ?


    std::string urlStr = KIARA::URL::resolve(getConfigURL(), path);

    URL url(urlStr);

    DFC_DEBUG("Server::addService: "<<url.toString());

    if (!url.isValid())
    {
        setError(KIARA_NETWORK_ERROR, "Invalid URL: "+urlStr);
        return getErrorCode();
    }

    const Transport::Transport *transport = Transport::Transport::getTransportByName(url.scheme.c_str());
    if (!transport)
    {
        setError(KIARA_NETWORK_ERROR, "Unsupported transport: "+url.scheme);
        return getErrorCode();
    }

    ServiceHandler *handler = new ServiceHandler(service, transport, protocol);
    if (handler->isError())
    {
        setError(handler->getError());
        return getErrorCode();
    }

    unsigned int port;

    try
    {
        port = boost::lexical_cast<unsigned int>(url.port);
    }
    catch (const boost::bad_lexical_cast &e)
    {
        setError(KIARA_NETWORK_ERROR, "Invalid port: "+urlStr);
        return getErrorCode();
    }

    // FIXME this will fail when transport changes for a given host/port combination
    if (!addPortListener(url.host, port, url.scheme))
        return getErrorCode();

    Transport::TransportAddress::Ptr address = transport->createAddress(urlStr);

    DFC_DEBUG("Register: "<<url.scheme<<" "<<address->toString());

    ServiceHandlerMap::iterator it = serviceHandlers_.begin();
    ServiceHandlerMap::iterator end = serviceHandlers_.end();
    for (; it != end; ++it)
    {
        if (it->first->equals(address))
            break;
    }

    if (it != end)
    {
        delete it->second;
        it->second = handler;
    }
    else
    {
        serviceHandlers_.push_back(std::make_pair(address, handler));
    }
    services_.insert(service);
    return KIARA_SUCCESS;
}

KIARA_Result Server::removeService(Service *service)
{
    KIARA_Result status = KIARA_FAILURE;
    for (ServiceHandlerMap::iterator it = serviceHandlers_.begin(), end = serviceHandlers_.end(); it != end;)
    {
        if (it->second->getService() == service)
        {
            serviceHandlers_.erase(it++);
            status = KIARA_SUCCESS;
        }
        else
            ++it;
    }

    services_.erase(service);

    return status;
}

KIARA_Result Server::run()
{
    try
    {
        Transport::Server::run();
    }
    catch (boost::system::system_error &e)
    {
        setError(KIARA_NETWORK_ERROR, e.what());
        return getErrorCode();
    }
    return KIARA_SUCCESS;
}

ServiceHandler * Server::findAcceptingServiceHandler(const Transport::TransportAddress::Ptr &address) const
{
    for (ServiceHandlerMap::const_iterator it = serviceHandlers_.begin(), end = serviceHandlers_.end();
        it != end; ++it)
    {
        if (it->first->acceptConnection(address))
            return it->second;
    }
    return 0;
}

Transport::Connection::Ptr Server::createConnection(const HostAndPort &hostAndPort, const Transport::NetworkContext::Ptr& ctx)
{
    // If the hostAndPort is not in transportEntries_, an empty
    // Transport::Connection::Ptr is returned
    TransportEntryList::iterator it = transportEntries_.find(hostAndPort);
    if (it == transportEntries_.end() || !it->second)
        return Transport::Connection::Ptr();

    const Transport::Transport *transport = it->second->transport;

    // Creates a new instance of a connection object
    Transport::Connection::Ptr conn = transport->createConnection(ctx);

    // Adds a request handler which is called upon reception of incomming
    // data.
    Server::ServerConnectionHandler *handler = new Server::ServerConnectionHandler(*this, it->second->transport);
    conn->setConnectionHandler(handler);
    return conn;
}

// KIARA_ServiceHandler

ServiceHandler::ServiceHandler(Service *service, const Transport::Transport *transport, const std::string &protocolName)
    : Base(service->getContext())
    , service_(service)
    , protocolInfo_()
    , mimeType_("text/plain")
    , syncServiceFuncObjMap_()
    , dispatchMap_()
    , createResponseMessage_(0)
    , getMessageMethodName_(0)
    , freeMessage_(0)
    , setGenericErrorMessage_(0)
    , getMimeType_(0)
    , runtimeEnvironment_(0)
{
    assert(transport != 0);

    protocolInfo_.name = protocolName;

    runtimeEnvironment_ = getContext()->getRuntimeContext().createEnvironment();
    if (!runtimeEnvironment_)
    {
        setError(KIARA_INIT_ERROR, "Could not create runtime environment");
        return;
    }

    std::string errorMsg;
    if (!runtimeEnvironment_->startInitialization(&errorMsg))
    {
        setError(KIARA_INIT_ERROR, "Could not initialize runtime environment: "+errorMsg);
        return;
    }

    const std::vector<std::string> & llvmModuleNames = getContext()->getLLVMModuleNames();
    for (std::vector<std::string>::const_iterator it = llvmModuleNames.begin(),
        end = llvmModuleNames.end(); it != end; ++it)
    {
        DFC_DEBUG("Loading LLVM module "<<*it);
        if (!getRuntimeEnvironment().loadModule(*it, &errorMsg))
        {
            setError(KIARA_CONNECTION_ERROR, "Could not load " + (*it) + " module: " + errorMsg);
            return;
        }
    }

    if (!runtimeEnvironment_->loadComponent(protocolName, &errorMsg))
    {
        setError(KIARA_INIT_ERROR, "Could not load " + protocolName + " component: "+errorMsg);
        return;
    }

    if (!runtimeEnvironment_->finishInitialization(&errorMsg))
    {
        setError(KIARA_INIT_ERROR, "Could not finish initialization of runtime environment: "+errorMsg);
        return;
    }

    const Transport::NetworkHandler &nh = transport->getNetworkHandler();
    //runtimeEnvironment_->registerExternalFunction("getConnection", (void*)nh.getConnection);
    //runtimeEnvironment_->registerExternalFunction("getServiceConnection", (void*)nh.getServiceConnection);
    runtimeEnvironment_->registerExternalFunction("sendData", (void*)nh.sendData);

    createRequestMessageFromData_ =
        (KIARA_CreateRequestMessageFromData)(intptr_t)
        getRuntimeEnvironment().requestPointerToFunction("createRequestMessageFromData");

    createResponseMessage_ =
        (KIARA_CreateResponseMessage)(intptr_t)
        getRuntimeEnvironment().requestPointerToFunction("createResponseMessage");
	
	createResponseMessageZmq_ =
        (KIARA_CreateResponseMessageZmq)(intptr_t)
        getRuntimeEnvironment().requestPointerToFunction("createResponseMessageZmq");

    getMessageMethodName_ =
        (KIARA_GetMessageMethodName)(intptr_t)
        getRuntimeEnvironment().requestPointerToFunction("getMessageMethodName");

    freeMessage_ =
        (KIARA_FreeMessage)(intptr_t)
        getRuntimeEnvironment().requestPointerToFunction("freeMessage");

    getMessageData_ =
        (KIARA_GetMessageData)(intptr_t)
        getRuntimeEnvironment().requestPointerToFunction("getMessageData");

    setGenericErrorMessage_ =
        (KIARA_SetGenericErrorMessage)(intptr_t)
        getRuntimeEnvironment().requestPointerToFunction("setGenericErrorMessage");
    getMimeType_ =
        (KIARA_GetMimeType)(intptr_t)
        getRuntimeEnvironment().requestPointerToFunction("getMimeType");

    if (!getMimeType_)
    {
        setError(KIARA_INIT_ERROR, "Could not compile getMimeType() function");
        return;
    }

    mimeType_ = getMimeType_();

    // compile all functions registered in service
    for (ServiceFuncRecordList::const_iterator it = service->serviceFuncList_.begin(),
        end = service->serviceFuncList_.end(); it != end; ++it)
    {
        if (compileServiceFunc(*it) != KIARA_SUCCESS)
            return;
    }
}

ServiceHandler::~ServiceHandler()
{
    for (DispatchMap::iterator it = dispatchMap_.begin(),
        end = dispatchMap_.end(); it != end; ++it)
    {
        destroyServiceFuncObj(it->second);
    }
    delete runtimeEnvironment_;
}

KIARA_ServiceFuncObj * ServiceHandler::createServiceFuncObj()
{
    KIARA_ServiceFuncObj *fobj = new KIARA_ServiceFuncObj;
    fobj->func = 0;
    fobj->base.vafunc = 0;
    fobj->base.syncHandler = 0;
    fobj->base.connection = 0;
    fobj->base.funcType = 0;
    return fobj;
}

void ServiceHandler::destroyServiceFuncObj(KIARA_ServiceFuncObj *funcObj)
{
    delete funcObj;
}

void ServiceHandler::installServiceFunc(const std::string &idlMethodName, KIARA_ServiceFunc serviceFuncPtr, KIARA_ServiceFuncObj *serviceFuncObj)
{
    DispatchMap::iterator it = dispatchMap_.find(idlMethodName);
    if (it != dispatchMap_.end())
    {
        // FIXME should we update syncServiceFuncObjMap_ here ?
        if (it->second == serviceFuncObj)
            return;
        // destroy old one
        destroyServiceFuncObj(it->second);
        it->second = serviceFuncObj;
    }
    else
        dispatchMap_[idlMethodName] = serviceFuncObj;
    syncServiceFuncObjMap_[serviceFuncPtr] = serviceFuncObj;
}

void ServiceHandler::dbgSimulateCall(const char *requestData)
{
    KIARA_Message *inMsg = createRequestMessageFromData_(requestData, strlen(requestData));
    if (!inMsg)
    {
        std::cerr<<"Invalid request data: '"<<requestData<<"'"<<std::endl;
        return;
    }
    const char *methodName = getMessageMethodName_(inMsg);
    std::cerr<<"METHOD NAME: "<<methodName<<std::endl;

    DispatchMap::iterator it = dispatchMap_.find(methodName);
    if (it == dispatchMap_.end())
    {
        std::cerr<<"NO SUCH METHOD "<<methodName<<std::endl;
        return;
    }

    KIARA_Message *outMsg = createResponseMessage_(NULL, inMsg);

    KIARA_Result result = it->second->base.syncHandler(NULL, outMsg, inMsg);

    if (result == KIARA_SUCCESS || result == KIARA_EXCEPTION)
    {
        kr_dbuffer_t buf;
        kr_dbuffer_init(&buf);
        getMessageData_(outMsg, &buf);
        kr_dbuffer_make_cstr(&buf);
        std::cerr<<"RESULT: "<<kr_dbuffer_data(&buf)<<std::endl;
        kr_dbuffer_destroy(&buf);
    }
    else
    {
        std::cerr<<"ERROR: "<<result<<", "<<kiaraGetErrorName(result)<<std::endl;
    }

    freeMessage_(outMsg);
    freeMessage_(inMsg);
}

void ServiceHandler::performCallZmq(const char *in_data, size_t in_size, DBuffer *response)
{
	KIARA_Message *inMsg = createRequestMessageFromData_(in_data, in_size);
	if (!inMsg)
    {

        KIARA_Message *outMsg = createResponseMessageZmq_(0);
        setGenericErrorMessage_(outMsg, KIARA_FAILURE, "Invalid request data");
        //getMessageData_(outMsg, responseData.get_dbuffer());
        freeMessage_(outMsg);
    }
    else
    {

        const char *methodName = getMessageMethodName_(inMsg);

        DispatchMap::iterator it = dispatchMap_.find(methodName);

        if (it == dispatchMap_.end())
        {
            KIARA_Message *outMsg = createResponseMessageZmq_(0);
            std::string errorStr = "No such method: " + std::string(methodName);
            setGenericErrorMessage_(outMsg, KIARA_FAILURE, errorStr.c_str());
            getMessageData_(outMsg, response->get_dbuffer());
            freeMessage_(outMsg);
            return;
        }

        KIARA_Message *outMsg = createResponseMessageZmq_(inMsg);
        /** FIXME Temporary solution */
        KIARA_ServiceFuncObj funcObj;
        memcpy(&funcObj, it->second, sizeof(KIARA_ServiceFuncObj));
        //funcObj.base.connection = wrap((void*));
		
        KIARA_Result result = funcObj.base.syncHandler(&funcObj, outMsg, inMsg);

        if (result == KIARA_SUCCESS || result == KIARA_EXCEPTION)
        {
            getMessageData_(outMsg, response->get_dbuffer());
        }
        else
        {
            setGenericErrorMessage_(outMsg, result, kiaraGetErrorName(result));
            getMessageData_(outMsg, response->get_dbuffer());
        }

        freeMessage_(outMsg);
        freeMessage_(inMsg);
    }
}

void ServiceHandler::performCall(Connection *connection, const DBuffer &requestData, DBuffer &responseData)
{
    KIARA_Message *inMsg = createRequestMessageFromData_(requestData.data(), requestData.size());
    if (!inMsg)
    {
        KIARA_Message *outMsg = createResponseMessage_(0, 0);
        setGenericErrorMessage_(outMsg, KIARA_FAILURE, "Invalid request data");
        getMessageData_(outMsg, responseData.get_dbuffer());
        freeMessage_(outMsg);
    }
    else
    {
        const char *methodName = getMessageMethodName_(inMsg);

        DispatchMap::iterator it = dispatchMap_.find(methodName);
        if (it == dispatchMap_.end())
        {
            KIARA_Message *outMsg = createResponseMessage_(0, 0);
            std::string errorStr = "No such method: " + std::string(methodName);
            setGenericErrorMessage_(outMsg, KIARA_FAILURE, errorStr.c_str());
            getMessageData_(outMsg, responseData.get_dbuffer());
            freeMessage_(outMsg);
            return;
        }

        KIARA_Message *outMsg = createResponseMessage_(wrap(connection), inMsg);

        /** FIXME Temporary solution */
        KIARA_ServiceFuncObj funcObj;
        memcpy(&funcObj, it->second, sizeof(KIARA_ServiceFuncObj));
        funcObj.base.connection = wrap(connection);

        KIARA_Result result = funcObj.base.syncHandler(&funcObj, outMsg, inMsg);

        if (result == KIARA_SUCCESS || result == KIARA_EXCEPTION)
        {
            getMessageData_(outMsg, responseData.get_dbuffer());
			std::cout << responseData.data() << std::endl;
        }
        else
        {
            setGenericErrorMessage_(outMsg, result, kiaraGetErrorName(result));
            getMessageData_(outMsg, responseData.get_dbuffer());
        }

        DFC_IFDEBUG(kr_dump_data("performCall: RESPONSE DATA: ", stderr, (unsigned char *)responseData.data(), responseData.size(), 0));
        DFC_IFDEBUG(kr_dump_data("performCall: REQUEST DATA: ", stderr, (unsigned char *)requestData.data(), requestData.size(), 0));

        freeMessage_(outMsg);
        freeMessage_(inMsg);
    }
}

void ServiceHandler::convertMessageToString(KIARA_Message *msg, std::string &destStr)
{
    kr_dbuffer_t buf;
    kr_dbuffer_init(&buf);
    getMessageData_(msg, &buf);
    destStr.assign(kr_dbuffer_data(&buf), kr_dbuffer_size(&buf));
    kr_dbuffer_destroy(&buf);
}

// KIARA_Service

Service::Service(Context *context)
    : Base(context)
{
}

Service::~Service()
{
}

KIARA_Result Service::loadIDL(const char *fileName)
{
    if (!getContext()->loadIDL(fileName))
    {
        setError(KIARA_INVALID_OPERATION, std::string("Could not parse IDL from '")+fileName+"'");
        return getErrorCode();
    }
    return KIARA_SUCCESS;
}

KIARA_Result Service::loadIDLFromString(const char *idlLanguage, const char *idlContents)
{
    if (!boost::algorithm::iequals(idlLanguage, "KIARA"))
    {
        setError(KIARA_INVALID_ARGUMENT, std::string("Could not parse IDL, unknown language: ")+idlLanguage);
        return getErrorCode();
    }
    std::istringstream iss(idlContents);
    if (!getContext()->loadIDL(iss, "<string>"))
    {
        setError(KIARA_INVALID_OPERATION, "Could not parse IDL from string");
        return getErrorCode();
    }
    return KIARA_SUCCESS;
}

std::string Service::getIDLContents()
{
    KIARA::IDLWriter writer(getContext()->getModule());
    std::ostringstream oss;
    writer.write(oss);
    return oss.str();
}

} // namespace Impl

} // namespace KIARA
