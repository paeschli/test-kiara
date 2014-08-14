/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2013  German Research Center for Artificial Intelligence (DFKI)
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
 * remote_chat_server.cpp
 *
 *  Created on: Jul 22, 2013
 *      Author: Dmitri Rubinstein
 */
#include "ChatClient.hpp"
#include "ChatKiaraDecl.hpp"
#include <cstring>
#include <cstdlib>
#include <queue>
#include <iostream>
#include <sstream>
#include <cassert>
#include <queue>
#include <cstdlib>
#include <KIARA/kiara.h>
#include <KIARA/kiara_cxx_stdlib.hpp>

// Describe and Implement Service Functions

struct ServerData {
    std::string userName;
    std::queue<Message> messages;
} serverData;

KIARA_Result registerUserImpl(KIARA_ServiceFuncObj *service, const std::string &userName, const std::string &userPassword, ChatServerErrorData &exception)
{
    exception.errorCode = ChatServerError::INTERNAL_ERROR;
    exception.msg = "registerUser is not implemented";
    return KIARA_EXCEPTION;
}

KIARA_Result loginUserImpl(KIARA_ServiceFuncObj *service, UserId & result, const std::string & userName, const std::string & userPassword, ChatServerErrorData & exception)
{
    serverData.userName = userName;
    result = 1;
    return KIARA_SUCCESS;
}

KIARA_Result sendMessageImpl(KIARA_ServiceFuncObj *service, UserId recipient, const std::string & message, ChatServerErrorData & exception)
{
    Message m(2, message); // echo user is sender
    serverData.messages.push(m);
    return KIARA_SUCCESS;
}

KIARA_Result getUserIdByNameImpl(KIARA_ServiceFuncObj *service, UserId & result, const std::string & userName, ChatServerErrorData & exception)
{
    if (userName == "BROADCAST")
        result = 0;
    else if (userName == serverData.userName)
        result = 1;
    else if (userName == "echo")
        result = 2;
    else
    {
        exception.errorCode = ChatServerError::INVALID_USER_ID;
        exception.msg = "getUserIdByName";
        return KIARA_EXCEPTION;
    }
    return KIARA_SUCCESS;
}

KIARA_Result getUserNameByIdImpl(KIARA_ServiceFuncObj *service, std::string & result, UserId id, ChatServerErrorData & exception)
{
    switch (id)
    {
        case 0: result = "BROADCAST"; break;
        case 1: result = serverData.userName; break;
        case 2: result = "echo"; break;
        default: {
            exception.errorCode = ChatServerError::INVALID_USER_ID;
            exception.msg = "getUserNameById";
            return KIARA_EXCEPTION;
        }
    }
    return KIARA_SUCCESS;
}

KIARA_Result receiveMessageImpl(KIARA_ServiceFuncObj *service, Message & result, ChatServerErrorData & exception)
{
    if (!serverData.messages.empty())
    {
        result = serverData.messages.front();
        serverData.messages.pop();
        return KIARA_SUCCESS;
    }
    exception.errorCode = ChatServerError::NO_MESSAGES;
    exception.msg = "receiveMessage";
    return KIARA_EXCEPTION;
}

KIARA_Result logoutUserImpl(KIARA_ServiceFuncObj *service, ChatServerErrorData & exception)
{
    return KIARA_SUCCESS;
}

int main(int argc, char **argv)
{

    /* This code is required for testing tool when compiled with MS CRT library and valgrind */
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    /* Initialize KIARA */
    kiaraInit(&argc, argv);

#if 0
    /* Manual test of the service wrapper function */
    {

        std::string result, * result_ptr;
        UserId id;
        ChatServerErrorData exception;

        void *args[3];
        KIARA_Result status;
        KIARA_DeclType *ty;
        KIARA_DeclService *s;

        id = 2;
        result_ptr = &result;

        args[0] = &result_ptr;
        args[1] = &id;
        args[2] = &exception;

        ty = _KIARA_CXX_DTYPE_GETTER(GetUserNameById)();
        s = ty->typeDecl.serviceDecl;
        status = s->serviceWrapperFunc(/*Connection*/NULL, args, 3);
        std::cout<<"status = "<<status<<"\nresult = "<<result<<std::endl;
    }
#endif

    KIARA_Context *ctx = kiaraNewContext();

    const char *port = NULL;
    const char *protocol = NULL;

    if (argc > 1)
    {
        port = argv[1];
    }
    else
    {
        port = "8080";
    }

    if (argc > 2)
    {
        protocol = argv[2];
    }
    else
    {
        protocol = "jsonrpc";
    }

    printf("Server port: %s\n", port);
    printf("Protocol: %s\n", protocol);

    KIARA_Service *service =  kiaraNewService(ctx);

    KIARA_Result result = kiaraLoadServiceIDLFromString(service,
        "KIARA",
        "typedef i64 UserId "
        "const UserId BROADCAST_MESSAGE = 0 "
        "enum ErrorCode { "
        "  INTERNAL_ERROR        = 0, "
        "  INVALID_USER_ID       = 1, "
        "  INVALID_USER_NAME     = 2, "
        "  INVALID_USER_PASSWORD = 3, "
        "  INVALID_AUTH_STATE    = 4, "
        "  AUTHENTICATION_FAILED = 5, "
        "  NO_MESSAGES           = 6  "
        "}"
        "exception ChatServerError { "
        "    ErrorCode errorCode, "
        "    string message "
        "} "
        "struct Message { "
        "  UserId sender, "
        "  string message "
        "}"
        "service ChatServer { "
        "    void registerUser(string userName, string userPassword) throws (ChatServerError error) "
        "    UserId loginUser(string userName, string userPassword) throws (ChatServerError error) "
        "    void sendMessage(UserId recipient, string message) throws (ChatServerError error) "
        "    UserId getUserIdByName(string userName) throws (ChatServerError error) "
        "    string getUserNameById(UserId id) throws (ChatServerError error) "
        "    Message receiveMessage() throws (ChatServerError error) "
        "    void logoutUser() throws (ChatServerError error) "
        "} ");
    if (result != KIARA_SUCCESS)
    {
        std::cerr<<"Error: could not parse IDL: "
                 <<kiaraGetErrorName(result)<<" "
                 <<kiaraGetServiceError(service)
                 <<std::endl;
        exit(1);
    }

    std::cout<<"Register ChatServer.registerUser ..."<<std::endl;

    result = KIARA_REGISTER_SERVICE_FUNC(service, "ChatServer.registerUser", OnRegisterUser, "", registerUserImpl);
    if (result != KIARA_SUCCESS)
    {
        std::cerr<<"Error: registration failed: "
                 <<kiaraGetErrorName(result)<<" "
                 <<kiaraGetServiceError(service)
                 <<std::endl;
        exit(1);
    }

    std::cout<<"Register ChatServer.loginUser ..."<<std::endl;

    result = KIARA_REGISTER_SERVICE_FUNC(service, "ChatServer.loginUser", OnLoginUser, "", loginUserImpl);
    if (result != KIARA_SUCCESS)
    {
        std::cerr<<"Error: registration failed: "
                 <<kiaraGetErrorName(result)<<" "
                 <<kiaraGetServiceError(service)
                 <<std::endl;
        exit(1);
    }

    std::cout<<"Register ChatServer.sendMessage ..."<<std::endl;

    result = KIARA_REGISTER_SERVICE_FUNC(service, "ChatServer.sendMessage", OnSendMessage, "", sendMessageImpl);
    if (result != KIARA_SUCCESS)
    {
        std::cerr<<"Error: registration failed: "
                 <<kiaraGetErrorName(result)<<" "
                 <<kiaraGetServiceError(service)
                 <<std::endl;
        exit(1);
    }

    std::cout<<"Register ChatServer.getUserIdByName ..."<<std::endl;

    result = KIARA_REGISTER_SERVICE_FUNC(service, "ChatServer.getUserIdByName", OnGetUserIdByName, "", getUserIdByNameImpl);
    if (result != KIARA_SUCCESS)
    {
        std::cerr<<"Error: registration failed: "
                 <<kiaraGetErrorName(result)<<" "
                 <<kiaraGetServiceError(service)
                 <<std::endl;
        exit(1);
    }

    std::cout<<"Register ChatServer.getUserNameById ..."<<std::endl;

    result = KIARA_REGISTER_SERVICE_FUNC(service, "ChatServer.getUserNameById", OnGetUserNameById, "", getUserNameByIdImpl);
    if (result != KIARA_SUCCESS)
    {
        std::cerr<<"Error: registration failed: "
                 <<kiaraGetErrorName(result)<<" "
                 <<kiaraGetServiceError(service)
                 <<std::endl;
        exit(1);
    }

    std::cout<<"Register ChatServer.receiveMessage ..."<<std::endl;

    result = KIARA_REGISTER_SERVICE_FUNC(service, "ChatServer.receiveMessage", OnReceiveMessage, "", receiveMessageImpl);
    if (result != KIARA_SUCCESS)
    {
        std::cerr<<"Error: registration failed: "
                 <<kiaraGetErrorName(result)<<" "
                 <<kiaraGetServiceError(service)
                 <<std::endl;
        exit(1);
    }

    std::cout<<"Register ChatServer.logoutUser ..."<<std::endl;

    result = KIARA_REGISTER_SERVICE_FUNC(service, "ChatServer.logoutUser", OnLogoutUser, "", logoutUserImpl);
    if (result != KIARA_SUCCESS)
    {
        std::cerr<<"Error: registration failed: "
                 <<kiaraGetErrorName(result)<<" "
                 <<kiaraGetServiceError(service)
                 <<std::endl;
        exit(1);
    }

    KIARA_Server *server = kiaraNewServer(ctx, "0.0.0.0", atoi(port), "/service");

    kiaraAddService(server, "/rpc/chat", protocol, service);

    printf("Starting server...\n");

    result = kiaraRunServer(server);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: could not start server: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServerError(server));
    }

    kiaraFreeServer(server);

    kiaraFreeService(service);

    kiaraFreeContext(ctx);

    kiaraFinalize();

    return 0;
}
