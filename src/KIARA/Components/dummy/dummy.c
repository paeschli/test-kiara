/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2014  German Research Center for Artificial Intelligence (DFKI)
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
 * dummy.c
 *
 *  Created on: 19.04.2014
 *      Author: Dmitri Rubinstein
 */

#include <KIARA/Components/api.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <KIARA/Common/Config.h>
#include <KIARA/CDT/kr_dumpdata.h>

#include "kiara_module.h"
#include "binaryio.h"

// #define KIARA_DO_DEBUG
#if defined(KIARA_DO_DEBUG) && !defined(NDEBUG)
#define KIARA_PING() KIARA_DEBUGF("CALLED: %s\n", KIARA_FUNCID())
#define KIARA_IFDEBUG(code) code
#define KIARA_DEBUGF(...) fprintf(stderr, __VA_ARGS__)
#else
#define KIARA_PING()
#define KIARA_IFDEBUG(code)
#define KIARA_DEBUGF(...)
#endif

KIARA_INFO_BEGIN
KIARA_COMMAND("llvmMarkFunctionsWithAlwaysInline")
KIARA_INFO_END

#define HAVE_STRDUP
#ifndef HAVE_STRDUP
static char *strdup(const char *str)
{
    size_t len;
    char *newstr;

    if (!str)
        return (char *)NULL;

    len = strlen(str);

    if(len >= ((size_t)-1) / sizeof(char))
        return (char *)NULL;

    newstr = malloc((len+1)*sizeof(char));
    if(!newstr)
        return (char *)NULL;

    memcpy(newstr,str,(len+1)*sizeof(char));

    return newstr;
}
#endif

static size_t xstrnlen(const char *string, size_t maxlen)
{
    const char *end = memchr (string, '\0', maxlen);
    return end ? (size_t) (end - string) : maxlen;
}

static char * xstrndup(char const *s, size_t n)
{
    size_t len = xstrnlen (s, n);
    char *new = malloc (len + 1);

    if (new == NULL)
        return NULL;

    new[len] = '\0';
    return memcpy (new, s, len);
}

#define DUMMY_MESSAGE_SIZE 1020 /* 1024 */

enum DUMMY_MessageKind
{
    DUMMY_NOT_A_MESSAGE = 0,
    DUMMY_REQUEST       = 1,
    DUMMY_RESPONSE      = 2,
    DUMMY_EXCEPTION     = 3
};

struct KIARA_Message
{
    enum DUMMY_MessageKind kind;
    char *methodName;
};

static void dumpMessage(KIARA_Message *msg)
{
    const char *msgType;

    fprintf(stderr, "Message {\n");

    switch (msg->kind)
    {
        case DUMMY_NOT_A_MESSAGE: msgType = "NOT A MESSAGE"; break;
        case DUMMY_REQUEST: msgType = "REQUEST"; break;
        case DUMMY_RESPONSE: msgType = "RESPONSE"; break;
        case DUMMY_EXCEPTION: msgType = "EXCEPTION"; break;
        default: msgType = "<Unknown Type>"; break;
    }

    fprintf(stderr, "  %s\n", msgType);
    fprintf(stderr, " Method name: %s\n", msg->methodName);
    fprintf(stderr, "}\n");
}

static KIARA_ALWAYS_INLINE KIARA_Message * createNewMessage(enum DUMMY_MessageKind kind)
{
   KIARA_Message *msg = malloc(sizeof(KIARA_Message));
   msg->kind = kind;
   msg->methodName = NULL;
   return msg;
}

static KIARA_ALWAYS_INLINE void setMethodName(KIARA_Message *msg, const char *methodName, size_t methodNameLen)
{
    KIARA_PING();

    char *tmp = (char*)malloc(methodNameLen+1);
    strncpy(tmp, methodName, methodNameLen+1);
    tmp[methodNameLen] = '\0';
    msg->methodName = tmp;
}

static KIARA_ALWAYS_INLINE void clearMessage(KIARA_Message *msg, enum DUMMY_MessageKind newKind)
{
    KIARA_PING();

    msg->kind = newKind;
    free(msg->methodName);
    msg->methodName = NULL;
}

void freeMessage(KIARA_Message *msg)
{
    KIARA_PING();

    free(msg->methodName);
    free(msg);
}

const char * getMimeType(void)
{
    return "application/octet-stream";
}

KIARA_Message * createRequestMessageFromData(const void *data, size_t dataSize)
{
    KIARA_PING();

    KIARA_Message *msg = createNewMessage(DUMMY_REQUEST);

    msg->methodName = xstrndup(((const char *)data)+1, dataSize-1);

    return msg;
}

const char * getMessageMethodName(KIARA_Message *msg)
{
    return msg->methodName;
}

KIARA_Result getMessageData(KIARA_Message *msg, kr_dbuffer_t *dest)
{
    kr_dbuffer_resize_nocopy(dest, DUMMY_MESSAGE_SIZE);

    char *outBuf = kr_dbuffer_data(dest);

    outBuf[0] = (char)msg->kind;
    if (msg->kind == DUMMY_REQUEST)
    {
        KIARA_DEBUGF("Write method name: %s to output buffer\n", msg->methodName);
        strncpy(outBuf+1, msg->methodName, DUMMY_MESSAGE_SIZE-1);
        outBuf[DUMMY_MESSAGE_SIZE-1] = '\0';
    }

    return KIARA_SUCCESS;
}

KIARA_Message * createRequestMessage(KIARA_Connection * KIARA_UNUSED conn, const char *name, size_t name_length)
{
    KIARA_PING();

    KIARA_Message *msg = createNewMessage(DUMMY_REQUEST);
    setMethodName(msg, name, name_length);

    return msg;
}

KIARA_Message * createResponseMessage(KIARA_Connection * KIARA_UNUSED conn, KIARA_Message * KIARA_UNUSED requestMsg)
{
    KIARA_PING();

    KIARA_Message *msg = createNewMessage(DUMMY_RESPONSE);

    KIARA_DEBUGF("LEAVE createResponseMessage\n");

    return msg;
}

void setGenericErrorMessage(KIARA_Message *msg, int errorCode, const char *errorMessage)
{
    KIARA_PING();

    clearMessage(msg, DUMMY_EXCEPTION);
}

KIARA_Result sendMessageSync(KIARA_Connection *conn, KIARA_Message *outMsg, KIARA_Message *inMsg)
{
    KIARA_PING();

    int result = 0;
    kr_dbuffer_t buf;

    if (!outMsg)
        return -1; /* FIXME use proper error code */

    /*KIARA_DEBUGF("sendMessageSync %s to %s\n", msgData, getConnectionURI(conn));*/
    /*KIARA_DEBUGF("sendMessageSync %s\n", msgData);*/

    kr_dbuffer_init(&buf);

    char outBuf[DUMMY_MESSAGE_SIZE];
    outBuf[0] = (char)outMsg->kind;
    if (outMsg->kind == DUMMY_REQUEST)
    {
        KIARA_DEBUGF("Write method name: %s to output buffer\n", outMsg->methodName);
        strncpy(outBuf+1, outMsg->methodName, DUMMY_MESSAGE_SIZE-1);
        outBuf[DUMMY_MESSAGE_SIZE-1] = '\0';

        KIARA_IFDEBUG(kr_dump_data("  Buffer: ", stderr,
                (unsigned char*)outBuf, DUMMY_MESSAGE_SIZE, 0));
    }

    result = sendData(conn, outBuf, DUMMY_MESSAGE_SIZE, &buf);

    inMsg->methodName = NULL;

#if 0
    if (result == KIARA_SUCCESS)
    {
        //inMsg->kind = (DUMMY_MessageKind);
        KIARA_DEBUGF("A\n");
        const char *data = kr_dbuffer_data(&buf);
        KIARA_DEBUGF("B\n");
        inMsg->kind = (enum DUMMY_MessageKind)*data;

        KIARA_DEBUGF("Received buffe size: %i\n", (int)kr_dbuffer_size(&buf));

        if (kr_dbuffer_size(&buf) > 0)
        {
            inMsg->methodName = xstrndup(data+1, kr_dbuffer_size(&buf)-1);
        }
        else
            inMsg->methodName = NULL;
    }
#endif
    kr_dbuffer_destroy(&buf);

    KIARA_DEBUGF("LEAVE sendMessageSync\n");

    return result;
}

KIARA_Result writeStructBegin(KIARA_Message * KIARA_UNUSED msg, const char * KIARA_UNUSED name)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeStructEnd(KIARA_Message * KIARA_UNUSED msg)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeFieldBegin(KIARA_Message * KIARA_UNUSED msg, const char * KIARA_UNUSED name)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeFieldEnd(KIARA_Message * KIARA_UNUSED msg)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeMessage_boolean(KIARA_Message *msg, int value)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeMessage_i8(KIARA_Message *msg, int8_t value)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeMessage_u8(KIARA_Message *msg, uint8_t value)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeMessage_i16(KIARA_Message *msg, int16_t value)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeMessage_u16(KIARA_Message *msg, uint16_t value)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeMessage_i32(KIARA_Message *msg, int32_t value)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeMessage_u32(KIARA_Message *msg, uint32_t value)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeMessage_i64(KIARA_Message *msg, int64_t value)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeMessage_u64(KIARA_Message *msg, uint64_t value)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeMessage_float(KIARA_Message *msg, float value)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeMessage_double(KIARA_Message *msg, double value)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeMessage_string(KIARA_Message *msg, const char * value)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readStructBegin(KIARA_Message * KIARA_UNUSED msg)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readStructEnd(KIARA_Message * KIARA_UNUSED msg)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readFieldBegin(KIARA_Message * KIARA_UNUSED msg, const char * KIARA_UNUSED name)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readFieldEnd(KIARA_Message * KIARA_UNUSED msg)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeArrayBegin(KIARA_Message *msg, size_t size)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeArrayEnd(KIARA_Message * KIARA_UNUSED msg)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readArrayBegin(KIARA_Message *msg, size_t *size)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readArrayEnd(KIARA_Message * KIARA_UNUSED msg)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readMessage_boolean(KIARA_Message *msg, int *value)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readMessage_i8(KIARA_Message *msg, int8_t *value)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readMessage_u8(KIARA_Message *msg, uint8_t *value)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readMessage_i16(KIARA_Message *msg, int16_t *value)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readMessage_u16(KIARA_Message *msg, uint16_t *value)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readMessage_i32(KIARA_Message *msg, int32_t *value)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readMessage_u32(KIARA_Message *msg, uint32_t *value)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readMessage_i64(KIARA_Message *msg, int64_t *value)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readMessage_u64(KIARA_Message *msg, uint64_t *value)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readMessage_float(KIARA_Message *msg, float *value)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readMessage_double(KIARA_Message *msg, double *value)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeMessage_user_string(KIARA_Message *msg, KIARA_UserType *value, KIARA_GetCString getCStringFunc)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readMessage_user_string(KIARA_Message *msg, KIARA_UserType *value, KIARA_SetCString setStringFunc)
{
    KIARA_PING();

    setStringFunc(value, "");

    return KIARA_SUCCESS;
}

KIARA_Result readMessage_string(KIARA_Message *msg, char **value)
{
    KIARA_PING();

    free(*value);
    *value = (char*)malloc(1);
    *value[0] = '\0';

    return KIARA_SUCCESS;
}

KIARA_Bool isErrorResponse(KIARA_Message *msg)
{
    KIARA_PING();

    return KIARA_TO_BOOL(msg->kind == DUMMY_EXCEPTION);
}

KIARA_Result readGenericError(KIARA_Message *msg, KIARA_UserType *userException, KIARA_SetGenericError setGenericErrorFunc)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeGenericError(KIARA_Message *msg, KIARA_UserType *userException, KIARA_GetGenericError getGenericErrorFunc)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeMessage_binary_stream(KIARA_Message *msg, KIARA_BinaryStream *stream)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readMessage_binary_stream(KIARA_Message *msg, KIARA_BinaryStream *stream)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}
