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
 * ChatClient.cpp
 *
 *  Created on: 02.10.2012
 *      Author: Dmitri Rubinstein
 */

#include "ChatClient.hpp"
#include <boost/assert.hpp>
#include <iostream>
#include <cstdio>
#include <cctype>

void ChatServerError::print(std::ostream &out)
{
    out << "error code: ";
    switch (errorCode_)
    {
        case INTERNAL_ERROR:    out << "Internal server error"; break;
        case INVALID_USER_ID:   out << "Invalid user ID"; break;
        case INVALID_USER_NAME: out << "Invalid user name"; break;
        case INVALID_USER_PASSWORD: out << "Invalid user password"; break;
        case INVALID_AUTH_STATE: out << "Invalid authentication state"; break;
        case AUTHENTICATION_FAILED: out << "Authentication failed"; break;
        case NO_MESSAGES:       out << "No messages"; break;
    }
    if (!msg_.empty())
    {
        out << ", message: " << msg_;
    }
    out << std::flush;
}

ChatClient::ChatClient(ChatServerConnection *connection)
    : connection_(connection)
{
    BOOST_ASSERT(connection_ != 0);
}

ChatClient::~ChatClient()
{
    if (connection_)
    {
        try
        {
            connection_->logoutUser();
        }
        catch (ChatServerError &error)
        {
            std::cerr<<"Remote error while logout: ";
            error.print(std::cerr);
            std::cerr<<std::endl;
        }
        catch (ConnectionError &error)
        {
            std::cerr<<"Connection error while logout: "<<error.getMessage()<<std::endl;
        }
    }
}

void ChatClient::run()
{
    // 1. Login
    std::string userName, password;
    std::cout << "Enter user name: ";
    std::getline(std::cin, userName);
    std::cout << "Enter password: ";
    std::getline(std::cin, password); // Not really safe, but this is just an example :)

    UserId userId;
    try
    {
        userId = connection_->loginUser(userName, password);
    }
    catch (ChatServerError &error)
    {
        std::cerr<<"Remote error while logging: ";
        error.print(std::cerr);
        std::cerr<<std::endl;
        return;
    }
    catch (ConnectionError &error)
    {
        std::cerr<<"Connection error while logging: "<<error.getMessage()<<std::endl;
        return;
    }

    // 2. Message loop
    std::cout << "Enter \":q\" for exiting" << std::endl;

    UserId sender;
    std::string message;

    bool quit = false;
    bool printPrompt = true;
    while (!quit)
    {
        // Output prompt
        if (printPrompt)
        {
            std::cout << userName << ": ";
            printPrompt = false;
        }

        std::getline(std::cin, message);

        // Remove final newline
        if (message.length() && message[message.length()-1] == '\n')
        {
            message.resize(message.length()-1);
        }

        if (message == ":q")
        {
            std::cout << "Exiting." <<std::endl;
            quit = true;
            break;
        }

        // Send message
        try
        {
            connection_->sendMessage(BROADCAST_MESSAGE, message);
        }
        catch (ChatServerError &error)
        {
            std::cerr<<"Remote Exception: ";
            error.print(std::cerr);
            std::cerr<<std::endl;
            printPrompt = true;
        }
        catch (ConnectionError &error)
        {
            std::cerr<<"Connection error while sending message: "<<error.getMessage()<<std::endl;
            return;
        }

        // Receive messages
        try
        {
            const int MAX_MESSAGES = 100;
            for (int i = 0; i < MAX_MESSAGES; ++i)
            {
                if (!connection_->receiveMessage(sender, message))
                    break;
                std::string name = connection_->getUserNameById(sender);
                std::cout << "From: " <<name << " (id=" << sender << ") Message: "
                          << message << std::endl << std::flush;
                printPrompt = true;
            }
        }
        catch (ChatServerError &error)
        {
            error.print(std::cerr);
            printPrompt = true;
        }
        catch (ConnectionError &error)
        {
            std::cerr<<"Connection error while receiving message: "<<error.getMessage()<<std::endl;
            return;
        }
    }

    // Close connection

    connection_->logoutUser();
    connection_ = 0;
}
