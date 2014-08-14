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
 * ChatKiaraDecl.hpp
 *
 *  Created on: Jul 23, 2013
 *      Author: Dmitri Rubinstein
 */

#ifndef CHATKIARADECL_HPP_INCLUDED
#define CHATKIARADECL_HPP_INCLUDED

#include "ChatTypes.hpp"
#include <KIARA/kiara.h>
#include <KIARA/kiara_cxx_stdlib.hpp>

static KIARA_UserType * Message_Allocate(void)
{
    return (KIARA_UserType *)new Message;
}

static void Message_Deallocate(KIARA_UserType *value)
{
    delete (Message*)value;
}

// we will use Message class as IDL's Message struct
KIARA_CXX_DECL_STRUCT_WITH_API(Message,
    KIARA_CXX_STRUCT_MEMBER(sender)
    KIARA_CXX_STRUCT_MEMBER(message),
    KIARA_CXX_USER_API(AllocateType, Message_Allocate)
    KIARA_CXX_USER_API(DeallocateType, Message_Deallocate))

KIARA_CXX_DECL_ENUM(ChatServerError::ErrorCode,
        KIARA_CXX_ENUM_CONST(ChatServerError::INTERNAL_ERROR)
        KIARA_CXX_ENUM_CONST(ChatServerError::INVALID_USER_ID)
        KIARA_CXX_ENUM_CONST(ChatServerError::INVALID_USER_NAME)
        KIARA_CXX_ENUM_CONST(ChatServerError::INVALID_USER_PASSWORD)
        KIARA_CXX_ENUM_CONST(ChatServerError::INVALID_AUTH_STATE)
        KIARA_CXX_ENUM_CONST(ChatServerError::AUTHENTICATION_FAILED)
        KIARA_CXX_ENUM_CONST(ChatServerError::NO_MESSAGES))


/* FIXME Currently we don't support arbitrary exception types
KIARA_CXX_DECL_STRUCT(ChatServerErrorData,
    KIARA_CXX_STRUCT_MEMBER(errorCode)
    KIARA_CXX_STRUCT_MEMBER(msg)
)
*/

// FIXME This is simplified support for exceptions

static int ChatServerErrorData_SetGenericError(KIARA_UserType *uexc, int errorCode, const char *errorMessage)
{
    ChatServerErrorData *exception = (ChatServerErrorData*)uexc;
    exception->errorCode = (ChatServerError::ErrorCode)errorCode;
    exception->msg = errorMessage ? errorMessage : "";

    return KIARA_SUCCESS;
}

static int ChatServerErrorData_GetGenericError(KIARA_UserType *uexc, int *errorCode, const char **errorMessage)
{
    ChatServerErrorData *exception = (ChatServerErrorData*)uexc;
    *errorCode = exception->errorCode;
    *errorMessage = exception->msg.c_str();
    return KIARA_SUCCESS;
}

static KIARA_UserType * ChatServerErrorData_Allocate(void)
{
    return (KIARA_UserType *)new ChatServerErrorData;
}

static void ChatServerErrorData_Deallocate(KIARA_UserType *value)
{
    delete (ChatServerErrorData*)value;
}

KIARA_CXX_DECL_OPAQUE_TYPE(ChatServerErrorData,
    KIARA_CXX_USER_API(SetGenericError, ChatServerErrorData_SetGenericError)
    KIARA_CXX_USER_API(GetGenericError, ChatServerErrorData_GetGenericError)
    KIARA_CXX_USER_API(AllocateType, ChatServerErrorData_Allocate)
    KIARA_CXX_USER_API(DeallocateType, ChatServerErrorData_Deallocate))

/* Server declarations */

KIARA_CXX_DECL_SERVICE(OnRegisterUser,
    KIARA_CXX_SERVICE_ARG(const std::string &, userName)
    KIARA_CXX_SERVICE_ARG(const std::string &, userPassword)
    KIARA_CXX_SERVICE_EXCEPTION(ChatServerErrorData &, exception))

KIARA_CXX_DECL_SERVICE(OnLoginUser,
    KIARA_CXX_SERVICE_RESULT(UserId &, result)
    KIARA_CXX_SERVICE_ARG(const std::string &, userName)
    KIARA_CXX_SERVICE_ARG(const std::string &, userPassword)
    KIARA_CXX_SERVICE_EXCEPTION(ChatServerErrorData &, exception))

KIARA_CXX_DECL_SERVICE(OnSendMessage,
    KIARA_CXX_SERVICE_ARG(UserId, recipient)
    KIARA_CXX_SERVICE_ARG(const std::string &, message)
    KIARA_CXX_SERVICE_EXCEPTION(ChatServerErrorData &, exception))

KIARA_CXX_DECL_SERVICE(OnGetUserIdByName,
    KIARA_CXX_SERVICE_RESULT(UserId &, result)
    KIARA_CXX_SERVICE_ARG(const std::string &, userName)
    KIARA_CXX_SERVICE_EXCEPTION(ChatServerErrorData &, exception))

KIARA_CXX_DECL_SERVICE(OnGetUserNameById,
    KIARA_CXX_SERVICE_RESULT(std::string &, result)
    KIARA_CXX_SERVICE_ARG(UserId, id)
    KIARA_CXX_SERVICE_EXCEPTION(ChatServerErrorData &, exception))

KIARA_CXX_DECL_SERVICE(OnReceiveMessage,
    KIARA_CXX_SERVICE_RESULT(Message &, result)
    KIARA_CXX_SERVICE_EXCEPTION(ChatServerErrorData &, exception))

KIARA_CXX_DECL_SERVICE(OnLogoutUser,
    KIARA_CXX_SERVICE_EXCEPTION(ChatServerErrorData &, exception))

#endif /* CHATKIARADECL_HPP_INCLUDED */
