/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2012, 2013  German Research Center for Artificial Intelligence (DFKI)
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
 * remote_chat_client.cpp
 *
 *  Created on: 04.10.2012
 *      Author: Dmitri Rubinstein
 */
#include "ChatClient.hpp"
#include <cstring>
#include <cstdlib>
#include <queue>
#include <iostream>
#include <sstream>
#include <cassert>
#include "ChatKiaraDecl.hpp"
#include <KIARA/kiara.h>
#include <KIARA/kiara_cxx_stdlib.hpp>

KIARA_CXX_DECL_FUNC(RegisterUser,
    KIARA_CXX_FUNC_ARG(const std::string &, userName)
    KIARA_CXX_FUNC_ARG_ANNOTATED(const std::string &, userPassword, SECURE)
    KIARA_CXX_FUNC_EXCEPTION(ChatServerErrorData &, exception))

KIARA_CXX_DECL_FUNC(LoginUser,
    KIARA_CXX_FUNC_RESULT(UserId &, result)
    KIARA_CXX_FUNC_ARG(const std::string &, userName)
    KIARA_CXX_FUNC_ARG(const std::string &, userPassword)
    KIARA_CXX_FUNC_EXCEPTION(ChatServerErrorData &, exception))

KIARA_CXX_DECL_FUNC(SendMessage,
    KIARA_CXX_FUNC_ARG(UserId, recipient)
    KIARA_CXX_FUNC_ARG(const std::string &, message)
    KIARA_CXX_FUNC_EXCEPTION(ChatServerErrorData &, exception))

KIARA_CXX_DECL_FUNC(GetUserIdByName,
    KIARA_CXX_FUNC_RESULT(UserId &, result)
    KIARA_CXX_FUNC_ARG(const std::string &, userName)
    KIARA_CXX_FUNC_EXCEPTION(ChatServerErrorData &, exception))

KIARA_CXX_DECL_FUNC(GetUserNameById,
    KIARA_CXX_FUNC_RESULT(std::string &, result)
    KIARA_CXX_FUNC_ARG(UserId, id)
    KIARA_CXX_FUNC_EXCEPTION(ChatServerErrorData &, exception))

KIARA_CXX_DECL_FUNC(ReceiveMessage,
    KIARA_CXX_FUNC_RESULT(Message &, result)
    KIARA_CXX_FUNC_EXCEPTION(ChatServerErrorData &, exception))

KIARA_CXX_DECL_FUNC(LogoutUser,
    KIARA_CXX_FUNC_EXCEPTION(ChatServerErrorData &, exception))

// Automatic connection closer based on RAII
class AutoCloseConnection
{
public:

    AutoCloseConnection(KIARA_Connection *&conn)
        : conn_(conn)
    { }

    ~AutoCloseConnection()
    {
        if (conn_)
            kiaraCloseConnection(conn_);
        conn_ = 0;
    }

private:
    KIARA_Connection *&conn_;
};

class KiaraServerConnection : public ChatServerConnection
{
public:

    KiaraServerConnection(KIARA_Context *ctx, const char *host, const int port)
        : ctx_(ctx)
        , conn_(0)
    {
        std::ostringstream ost;
        ost << "http://"<<host<<":"<<port<<"/service";

        std::cout<<"Connecting with "<<ost.str()<<std::endl;

        conn_ = kiaraOpenConnection(ctx, ost.str().c_str());
        if (!conn_)
        {
            std::cerr<<"Could not open connection : "<<kiaraGetContextError(ctx)<<std::endl;
            exit(1);
        }

#define CHECK_STUB(s)                                                                           \
        if (!s)                                                                                 \
        {                                                                                       \
            std::cerr<<"Could not generate stub: "<<kiaraGetConnectionError(conn_)<<std::endl;   \
            exit(1);                                                                            \
        }

        std::cout<<"Generating stubs..."<<std::endl;

        // We generate stubs for service methods associated with the opened connection
        registerUserStub_ = KIARA_GENERATE_CLIENT_FUNC(conn_, "ChatServer.registerUser", RegisterUser, "");
        CHECK_STUB(registerUserStub_);
        loginUserStub_    = KIARA_GENERATE_CLIENT_FUNC(conn_, "ChatServer.loginUser", LoginUser, "");
        CHECK_STUB(loginUserStub_);
        sendMessageStub_  = KIARA_GENERATE_CLIENT_FUNC(conn_, "ChatServer.sendMessage", SendMessage, "");
        CHECK_STUB(sendMessageStub_);
        getUserIdByNameStub_  = KIARA_GENERATE_CLIENT_FUNC(conn_, "ChatServer.getUserIdByName", GetUserIdByName, "");
        CHECK_STUB(getUserIdByNameStub_);
        getUserNameByIdStub_  = KIARA_GENERATE_CLIENT_FUNC(conn_, "ChatServer.getUserNameById", GetUserNameById, "");
        CHECK_STUB(getUserNameByIdStub_);
        receiveMessageStub_   = KIARA_GENERATE_CLIENT_FUNC(conn_, "ChatServer.receiveMessage", ReceiveMessage, "");
        CHECK_STUB(receiveMessageStub_);
        logoutUserStub_      = KIARA_GENERATE_CLIENT_FUNC(conn_, "ChatServer.logoutUser", LogoutUser, "");
        CHECK_STUB(logoutUserStub_);

        std::cout<<"Done"<<std::endl;
    }

    KIARA_ATTRIBUTE_NORETURN void throwException(int result, ChatServerErrorData &err)
    {
        if (result == KIARA_EXCEPTION)
            throw ChatServerError(err.errorCode, err.msg);

        if (kiaraGetConnectionError(conn_))
        {
            throw ConnectionError(kiaraGetConnectionError(conn_));
        }

        // we should be never in this state
        throw ChatServerError(ChatServerError::INTERNAL_ERROR, "Invalid server response");
    }

    virtual void registerUser(const std::string &userName, const std::string &userPassword)
    {
        ChatServerErrorData exception;
        int result = registerUserStub_(userName, userPassword, exception);
        if (result == KIARA_SUCCESS)
            return;

        throwException(result, exception);
    }

    virtual UserId loginUser(const std::string &userName, const std::string &userPassword)
    {
        ChatServerErrorData exception;

        UserId sender;
        int result = loginUserStub_(sender, userName, userPassword, exception);
        if (result == KIARA_SUCCESS)
            return sender;

        throwException(result, exception);
    }

    virtual void sendMessage(UserId recipient, const std::string &message)
    {
        ChatServerErrorData exception;
        int result = sendMessageStub_(recipient, message, exception);
        if (result == KIARA_SUCCESS)
            return;

        throwException(result, exception);
    }

    virtual UserId getUserIdByName(const std::string &userName)
    {
        ChatServerErrorData exception;
        UserId userId;
        int result = getUserIdByNameStub_(userId, userName, exception);
        if (result == KIARA_SUCCESS)
            return userId;

        throwException(result, exception);
    }

    virtual std::string getUserNameById(UserId id)
    {
        ChatServerErrorData exception;
        std::string userName;
        int result = getUserNameByIdStub_(userName, id, exception);
        if (result == KIARA_SUCCESS)
            return userName;

        throwException(result, exception);
    }

    /// Returns false if there are no messages on the server
    virtual bool receiveMessage(UserId &sender, std::string &message)
    {
        ChatServerErrorData exception;
        Message msg;
        std::string userName;
        int result = receiveMessageStub_(msg, exception);
        if (result == KIARA_SUCCESS)
        {
            sender = msg.sender;
            message = msg.message;
            return true;
        }
        if (result == KIARA_EXCEPTION && (ChatServerError::ErrorCode)exception.errorCode == ChatServerError::NO_MESSAGES)
            return false;

        throwException(result, exception);
    }

    virtual void logoutUser()
    {
        // We will close connection in any case when
        // we leave this function:
        AutoCloseConnection ac(conn_);

        ChatServerErrorData exception;
        int result = logoutUserStub_(exception);
        if (result == KIARA_SUCCESS)
            return;

        throwException(result, exception);
    }

    virtual ~KiaraServerConnection()
    {
        AutoCloseConnection ac(conn_);
    }

private:
    KIARA_Context *ctx_;
    KIARA_Connection *conn_;

    RegisterUser registerUserStub_;
    LoginUser loginUserStub_;
    SendMessage sendMessageStub_;
    GetUserIdByName getUserIdByNameStub_;
    GetUserNameById getUserNameByIdStub_;
    ReceiveMessage receiveMessageStub_;
    LogoutUser logoutUserStub_;
};

void printUsageInfo(const char *appName)
{
    std::cout<<
            "KIARA Chat Client\n"
            "Usage:\n"
            << appName << " [options] host port\n\n"
            "  -h    | -help | --help              : prints this and exit\n"
            "  -i    | --info                      : print info\n"
            <<std::endl;
}

#define ARG(str) (!strcmp(argv[i], str))
#define ARG_STARTS_WITH(str,len) (!strncmp(argv[i], str, len))
#define ARG_CONTAINS(str) strstr(argv[i], str)
#define APP_ERROR(msg) { std::cerr<<msg<<std::endl; exit(1); }

int main(int argc, char **argv)
{

    /* This code is required for testing tool when compiled with MS CRT library and valgrind */
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    /* Initialize KIARA library */
    kiaraInit(&argc, argv);

    int port = 8080;
    const char *host = "localhost";
    //bool printInfoAndExit = false;
    std::vector<const char *> args;

    // parse command line
    int i;
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (ARG("-help") || ARG("--help") || ARG("-h"))
            {
                printUsageInfo(argv[0]);
                exit(0);
            }
            if (ARG("-i") || ARG("--info"))
            {
                // printInfoAndExit = true;
            }
            else if (ARG("--"))
            {
                i++;
                break;
            }
            else
            {
                APP_ERROR("Unknown option "<<argv[i]);
            }
        }
        else
        {
            // no '-' prefix, assume this is an argument
            args.push_back(argv[i]);
        }
    }

    // read rest files
    if (args.size() >= 1)
        host = args[0];

    if (args.size() >= 2)
        port = atoi(args[1]);

    KIARA_Context *ctx = kiaraNewContext();

    /* Run client */
    {
        KiaraServerConnection connection(ctx, host, port);
        ChatClient client(&connection);
        client.run();
    }

    /* Finalize context and library */
    kiaraFreeContext(ctx);
    kiaraFinalize();
}
