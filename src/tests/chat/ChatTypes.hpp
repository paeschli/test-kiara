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
 * ChatTypes.hpp
 *
 *  Created on: Jul 23, 2013
 *      Author: Dmitri Rubinstein
 */

#ifndef CHATTYPES_HPP_INCLUDED
#define CHATTYPES_HPP_INCLUDED

#include <boost/cstdint.hpp>

typedef boost::int64_t UserId;

const UserId BROADCAST_MESSAGE = 0;

struct Message
{
    UserId sender;
    std::string message;

    Message()
        : sender(0)
        , message()
    {
    }

    Message(UserId userId, const std::string &text)
        : sender(userId)
        , message(text)
    {
    }
};

class ChatServerError
{
public:

    enum ErrorCode
    {
        INTERNAL_ERROR        = 0,
        INVALID_USER_ID       = 1,
        INVALID_USER_NAME     = 2,
        INVALID_USER_PASSWORD = 3,
        INVALID_AUTH_STATE    = 4,
        AUTHENTICATION_FAILED = 5,
        NO_MESSAGES           = 6
    };

    ChatServerError(ErrorCode errorCode, const std::string &msg)
        : errorCode_(errorCode)
        , msg_(msg)
    {
    }

    ErrorCode getErrorCode() const { return errorCode_; }

    const std::string & getMessage() const { return msg_; }

    void print(std::ostream &out);

private:
    ErrorCode errorCode_;
    std::string msg_;
};

struct ChatServerErrorData
{
    ChatServerError::ErrorCode errorCode;
    std::string msg;
};

#endif /* CHATTYPES_HPP_INCLUDED */
