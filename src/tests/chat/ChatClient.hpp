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
 * ChatClient.hpp
 *
 *  Created on: 02.10.2012
 *      Author: Dmitri Rubinstein
 */

#ifndef CHATCLIENT_HPP_INCLUDED
#define CHATCLIENT_HPP_INCLUDED

#include <string>
#include <iostream>
#include "ChatTypes.hpp"

class ConnectionError
{
public:

    ConnectionError(const std::string &msg)
        : msg_(msg)
    { }

    const std::string & getMessage() const { return msg_; }

private:
    std::string msg_;
};

class ChatServerConnection
{
public:

    virtual void registerUser(const std::string &userName, const std::string &userPassword) = 0;

    virtual UserId loginUser(const std::string &userName, const std::string &userPassword) = 0;

    virtual void sendMessage(UserId recipient, const std::string &message) = 0;

    virtual UserId getUserIdByName(const std::string &userName) = 0;

    virtual std::string getUserNameById(UserId id) = 0;

    /// Returns false if there are no messages on the server
    virtual bool receiveMessage(UserId &sender, std::string &message) = 0;

    virtual void logoutUser() = 0;

    virtual ~ChatServerConnection() { }
};

class ChatClient
{
public:

    ChatClient(ChatServerConnection *connection);

    ~ChatClient();

    void run();

private:
    ChatServerConnection *connection_;
};

#endif /* CHATCLIENT_HPP_INCLUDED */
