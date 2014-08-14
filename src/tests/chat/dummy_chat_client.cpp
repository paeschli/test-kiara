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
 * dummy_chat_client.cpp
 *
 *  Created on: 02.10.2012
 *      Author: Dmitri Rubinstein
 */
#include "ChatClient.hpp"
#include <queue>

class DummyServerConnection : public ChatServerConnection
{
public:

    virtual void registerUser(const std::string &userName, const std::string &userPassword)
    {
        throw ChatServerError(ChatServerError::INTERNAL_ERROR, "registerUser is not implemented");
    }

    virtual UserId loginUser(const std::string &userName, const std::string &userPassword)
    {
        userName_ = userName;
        return 1;
    }

    virtual void sendMessage(UserId recipient, const std::string &message)
    {
        Message m(2, message); // echo user is sender
        messages_.push(m);
    }

    virtual UserId getUserIdByName(const std::string &userName)
    {
        if (userName == "BROADCAST")
            return 0;
        if (userName == userName_)
            return 1;
        if (userName == "echo")
            return 2;
        throw ChatServerError(ChatServerError::INVALID_USER_ID, "getUserIdByName");
    }

    virtual std::string getUserNameById(UserId id)
    {
        switch (id)
        {
            case 0: return "BROADCAST";
            case 1: return userName_;
            case 2: return "echo";
            default: break;
        }
        throw ChatServerError(ChatServerError::INVALID_USER_ID, "getUserNameById");
    }

    /// Returns false if there are no messages on the server
    virtual bool receiveMessage(UserId &sender, std::string &message)
    {
        if (!messages_.empty())
        {
            Message msg = messages_.front();
            sender = msg.sender;
            message = msg.message;
            messages_.pop();
            return true;
        }
        return false;
    }

    virtual void logoutUser()
    {
    }

    virtual ~DummyServerConnection()
    {
    }

private:
    std::string userName_;
    std::queue<Message> messages_;
};

int main(int argc, char **argv)
{
    DummyServerConnection connection;
    ChatClient client(&connection);
    client.run();
}
