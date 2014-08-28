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
 * Network.hpp
 *
 *  Created on: Feb 7, 2014
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_IMPL_NETWORK_HPP_INCLUDED
#define KIARA_IMPL_NETWORK_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <KIARA/Impl/Core.hpp>
#include <KIARA/Utils/DBuffer.hpp>

namespace KIARA
{

namespace Transport
{
class HttpResponse;
class HttpRequest;
}

namespace Impl
{

class Service;

class Connection : public Base
{
public:

    typedef std::map<KIARA::FunctionType::Ptr, KIARA_FuncObj*> FuncObjMap;
    Connection(Context *context, const std::string &transportName);

    virtual ~Connection();

    const std::string & getTransportName() const { return transportName_; }

    KIARA_FuncObj * createFuncObj();
    void destroyFuncObj(KIARA_FuncObj *funcObj);

    KIARA_FuncObj * generateClientFuncObj(const char *idlMethodName, KIARA_GetDeclType declTypeGetter, const char *mapping);

    KIARA_ConnectionData * getConnectionData() const { return data_; }
    void setConnectionData(KIARA_ConnectionData *data) { data_ = data; }

    virtual const char * getConnectionURI() const = 0;

    KIARA::RuntimeEnvironment & getRuntimeEnvironment() const { return *runtimeEnvironment_; }

    const KIARA::Transport::Connection::Ptr & getTransportConnection() const
    {
        return transportConnection_;
    }

protected:
    FuncObjMap funcObjects_;
    KIARA_ConnectionData *data_;
    std::string transportName_;
    KIARA::RuntimeEnvironment *runtimeEnvironment_;
    KIARA::Transport::Connection::Ptr transportConnection_;

    void setTransportName(const std::string &transportName) { transportName_ = transportName; }
    void setTransportConnection(const KIARA::Transport::Connection::Ptr &transportConnection)
    {
        transportConnection_ = transportConnection;
    }
};

class ClientConnection : public Connection
{
public:

    ClientConnection(Context *context, const std::string &uri);

    ClientConnection(Context *context, const std::string &transportName, const KIARA::Transport::Connection::Ptr &transportConnection);

    ~ClientConnection();

    const char * getConnectionURI() const
    {
        return uri_.c_str();
    }

    KIARA::URLLoader::Connection * getURLLoaderConnection() const
    {
        return urlLoaderConnection_;
    }

    const std::string & getMimeType() const { return mimeType_; }

private:
    std::string uri_;
    KIARA_InitNetworkFunc initFunc_;
    KIARA_FinalizeNetworkFunc finalizeFunc_;
    KIARA_GetMimeType getMimeType_;

    KIARA::URLLoader::Connection *urlLoaderConnection_;
    std::string mimeType_;
};

struct ServiceFuncRecord
{
    std::string idlMethodName;
    KIARA::FunctionType::Ptr funcType;
    std::string serviceMethodName;
    KIARA::FunctionType::Ptr serviceMethodType;
    KIARA_ServiceFunc serviceFuncPtr;

    ServiceFuncRecord(
        const std::string &idlMethodName,
        const KIARA::FunctionType::Ptr &fty,
        const std::string &serviceMethodName,
        const KIARA::FunctionType::Ptr &serviceMethodType,
        KIARA_ServiceFunc serviceFuncPtr)
    : idlMethodName(idlMethodName)
    , funcType(fty)
    , serviceMethodName(serviceMethodName)
    , serviceMethodType(serviceMethodType)
    , serviceFuncPtr(serviceFuncPtr)
    {
    }
};
typedef std::vector<ServiceFuncRecord> ServiceFuncRecordList;

class ServiceHandler : public Base
{
public:
    typedef std::map<KIARA_ServiceFunc, KIARA_ServiceFuncObj*> SyncServiceFuncObjMap;
    typedef std::map<std::string, KIARA_ServiceFuncObj*> DispatchMap;

    ServiceHandler(Service *service, const Transport::Transport *transport, const std::string &protocolName);
    ~ServiceHandler();

    Service * getService() const { return service_; }

    const std::string & getMimeType() const { return mimeType_; }

    KIARA_ServiceFuncObj * createServiceFuncObj();
    void destroyServiceFuncObj(KIARA_ServiceFuncObj *funcObj);

    KIARA_Result compileServiceFunc(const ServiceFuncRecord &serviceFunc);

    const KIARA::ProtocolInfo & getProtocolInfo() { return protocolInfo_; }

    void dbgSimulateCall(const char *requestData);

    void performCall(Connection *connection, const DBuffer &requestData, DBuffer &responseData);
	
	void performCallZmq(const char *in_data, size_t in_size, DBuffer *response);

    KIARA::RuntimeEnvironment & getRuntimeEnvironment() const { return *runtimeEnvironment_; }

    void installServiceFunc(const std::string &idlMethodName, KIARA_ServiceFunc serviceFuncPtr, KIARA_ServiceFuncObj *serviceFuncObj);

protected:
    Service *service_;
    KIARA::ProtocolInfo protocolInfo_;
    std::string mimeType_;

    SyncServiceFuncObjMap syncServiceFuncObjMap_;
    DispatchMap dispatchMap_;
    KIARA_CreateRequestMessageFromData createRequestMessageFromData_;
    KIARA_CreateResponseMessage createResponseMessage_;
	KIARA_CreateResponseMessageZmq createResponseMessageZmq_;
    KIARA_GetMessageMethodName getMessageMethodName_;
    KIARA_FreeMessage freeMessage_;
    KIARA_GetMessageData getMessageData_;
    KIARA_SetGenericErrorMessage setGenericErrorMessage_;
    KIARA_GetMimeType getMimeType_;
    KIARA::RuntimeEnvironment *runtimeEnvironment_;

    void convertMessageToString(KIARA_Message *msg, std::string &destStr);
};

class Service : public Base
{
    friend class ServiceHandler;
public:

    typedef std::map<KIARA_ServiceFunc, KIARA_ServiceFuncObj*> SyncServiceFuncObjMap;
    typedef std::map<std::string, KIARA_ServiceFuncObj*> DispatchMap;
    Service(Context *context);
    virtual ~Service();

    KIARA_Result loadIDL(const char *fileName);

    KIARA_Result loadIDLFromString(const char *idlLanguage, const char *idlContents);

    KIARA_Result registerServiceFunc(const char *idlMethodName, KIARA_GetDeclType declTypeGetter, const char *mapping, KIARA_ServiceFunc func);

    std::string getIDLContents();

protected:
    ServiceFuncRecordList serviceFuncList_;
};

class Server: public Base, public Transport::Server
{
    class ServerConnectionHandler;
    friend class ServerConnectionHandler;
public:

    Server(Context *context, const std::string &address, unsigned int port, const std::string &configPath);

    virtual ~Server();

    KIARA_Result addService(const char *path, const char *protocol, Service *service);

    KIARA_Result removeService(Service *service);

    KIARA_Result run();

    std::string getConfigURL() const
    {
        return "http://" + configHost_ + ":" + boost::lexical_cast<std::string>(configPort_) + "/" + configPath_;
    }

    ServiceHandler * findAcceptingServiceHandler(const Transport::TransportAddress::Ptr &address) const;
	
	void generateServerConfiguration(
        ServerConfiguration &serverConfiguration,
        const std::string &localHostName,
        const std::string &remoteHostName);

private:

    typedef std::pair<std::string, unsigned int> HostAndPort;
    typedef std::pair<std::string, std::string> TransportAndPath;
    typedef std::pair<Transport::TransportAddress::Ptr, ServiceHandler*> TransportAddressAndServiceHandler;

    struct TransportEntry
    {
        typedef boost::shared_ptr<TransportEntry> Ptr;

        const Transport::Transport *transport;
        unsigned int numServices;

        TransportEntry(const Transport::Transport *transport)
            : transport(transport)
            , numServices(0)
        { }
    };

    bool addPortListener(const std::string &host, unsigned int port, const std::string &transportName);

    TransportEntry::Ptr getTransportEntry(const std::string &host, unsigned int port);

    Transport::RequestResult handleRequest(
        Server::ServerConnectionHandler &handler,
        const Transport::Connection::Ptr &connection,
        const Transport::TransportMessage &request,
        Transport::TransportMessage &response);

    Transport::Connection::Ptr createConnection(const HostAndPort &hostAndPort, const Transport::NetworkContext::Ptr& ctx);

    std::string configHost_;    // server configuration address
    unsigned int configPort_;   // server configuration port
    std::string configPath_;    // server configuration path
    typedef std::set<Service*> ServiceSet;
    typedef std::vector<TransportAddressAndServiceHandler> ServiceHandlerMap; // map transport address to service handler

    typedef std::map<HostAndPort, TransportEntry::Ptr> TransportEntryList;

    ServiceSet services_;
    ServiceHandlerMap serviceHandlers_;
    TransportEntryList transportEntries_;
};

DEFINE_WRAPPER_FUNCTIONS(::KIARA::Impl::Connection, ::KIARA_Connection)
DEFINE_WRAPPER_FUNCTIONS(::KIARA::Impl::Service, ::KIARA_Service)
DEFINE_WRAPPER_FUNCTIONS(::KIARA::Impl::Server, ::KIARA_Server)

} // namespace Impl

} // namespace KIARA

#endif /* KIARA_IMPL_NETWORK_HPP_INCLUDED */
