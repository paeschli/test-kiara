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
 * ortecdr.c
 *
 *  Created on: 21.12.2013
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
#include "orte/cdr.h"

// #define KIARA_DO_DEBUG
#if defined(KIARA_DO_DEBUG) && !defined(NDEBUG)
#define KIARA_PING() KIARA_DEBUGF("CALLED: %s\n", KIARA_FUNCID())
#define KIARA_PINGF(...) KIARA_DEBUGF("CALLED: %s : ", KIARA_FUNCID()); KIARA_DEBUGF(__VA_ARGS__);
#define KIARA_IFDEBUG(code) code
#define KIARA_DEBUGF(...) fprintf(stderr, __VA_ARGS__)
#else
#define KIARA_PING()
#define KIARA_PINGF(...)
#define KIARA_IFDEBUG(code)
#define KIARA_DEBUGF(...)
#endif

KIARA_INFO_BEGIN
KIARA_COMMAND("llvmMarkFunctionsWithAlwaysInline")
KIARA_INFO_END

/*
 * Messaging via ORTE's CDR
 */

enum ORTECDR_MessageKind
{
    ORTECDR_NOT_A_MESSAGE = 0,
    ORTECDR_REQUEST       = 1,
    ORTECDR_RESPONSE      = 2,
    ORTECDR_EXCEPTION     = 3
};

#define METHOD_NAME_BUF_SIZE 255

struct KIARA_Message
{
    enum ORTECDR_MessageKind kind;
    CDR_Codec codec;
    char *methodName;
    char methodNameBuf[METHOD_NAME_BUF_SIZE];
    char methodNameInBuf;
};

static KIARA_ALWAYS_INLINE void dumpMessage(KIARA_Message *msg)
{
    const char *msgType;

    fprintf(stderr, "Message {\n");

    switch (msg->kind)
    {
        case ORTECDR_NOT_A_MESSAGE: msgType = "NOT A MESSAGE"; break;
        case ORTECDR_REQUEST: msgType = "REQUEST"; break;
        case ORTECDR_RESPONSE: msgType = "RESPONSE"; break;
        case ORTECDR_EXCEPTION: msgType = "EXCEPTION"; break;
        default: msgType = "<Unknown Type>"; break;
    }

    fprintf(stderr, "  %s\n", msgType);

    kr_dump_data("  Buffer: ", stderr,
            (unsigned char*)CDR_buffer_data(&msg->codec), CDR_buffer_size(&msg->codec), 0);
    fprintf(stderr, " Position: %u\n", (unsigned int)CDR_buffer_position(&msg->codec));
    fprintf(stderr, " Method name: %s\n", msg->methodName);
    fprintf(stderr, "}\n");
}

static KIARA_ALWAYS_INLINE void initCDRCodec(CDR_Codec *codec)
{
    CDR_codec_init_static(codec);
    CDR_buffer_init(codec, 0);
    codec->data_endian = codec->host_endian;

    KIARA_DEBUGF("initCDRCodec: CDR codec buf_len = %u\n", (unsigned int)CDR_buffer_size(codec));
}

static KIARA_ALWAYS_INLINE void resetCDRCodec(CDR_Codec *codec)
{
    CDR_buffer_reset(codec);
    CDR_buffer_reset_position(codec);
    codec->data_endian = codec->host_endian;
}

static KIARA_ALWAYS_INLINE void destroyCDRCodec(CDR_Codec *codec)
{
    CDR_codec_release_buffer(codec);
}

static KIARA_ALWAYS_INLINE KIARA_Message * createNewMessage(enum ORTECDR_MessageKind kind)
{
   KIARA_Message *msg = malloc(sizeof(KIARA_Message));
   msg->kind = kind;
   initCDRCodec(&msg->codec);
   msg->methodName = NULL;
   msg->methodNameInBuf = 0;
   return msg;
}

static KIARA_ALWAYS_INLINE void setMethodName(KIARA_Message *msg, const char *methodName, size_t methodNameLen)
{
    KIARA_PING();

    if (methodNameLen+1 > METHOD_NAME_BUF_SIZE)
    {
        char *tmp = (char*)malloc(methodNameLen+1);
        strncpy(tmp, methodName, methodNameLen+1);
        tmp[methodNameLen] = '\0';
        msg->methodName = tmp;
        msg->methodNameInBuf = 0;
    }
    else
    {
        strncpy(msg->methodNameBuf, methodName, methodNameLen+1);
        msg->methodNameBuf[methodNameLen] = '\0';
        msg->methodName = msg->methodNameBuf;
        msg->methodNameInBuf = 1;
    }
}

static KIARA_ALWAYS_INLINE void freeMethodName(KIARA_Message *msg)
{
    if (!msg->methodNameInBuf)
    {
        free(msg->methodName);
    }
    msg->methodName = NULL;
    msg->methodNameInBuf = 0;
}

static KIARA_ALWAYS_INLINE void clearMessage(KIARA_Message *msg, enum ORTECDR_MessageKind newKind)
{
    KIARA_PING();

    msg->kind = newKind;
    resetCDRCodec(&msg->codec);
    freeMethodName(msg);
}

/* Set message data for reading */
static KIARA_ALWAYS_INLINE void setMessageData(KIARA_Message *msg, const void *data, size_t dataSize)
{
    CDR_buffer_reset_position(&msg->codec); /* Set position to 0 for writing */
    CDR_buffer_puts(&msg->codec, data, dataSize); /* Write data */
    CDR_buffer_reset_position(&msg->codec); /* Set position to 0 for reading */
}

static KIARA_ALWAYS_INLINE KIARA_Result readMessageHeader(KIARA_Message *msg)
{
    KIARA_PING();

    KIARA_IFDEBUG(dumpMessage(msg));

    int8_t msgCode;
    KIARA_Result result;
    result = readMessage_i8(msg, &msgCode);
    if (result != KIARA_SUCCESS)
        return result;

    if (msgCode != ORTECDR_REQUEST && msgCode != ORTECDR_RESPONSE && msgCode != ORTECDR_EXCEPTION)
    {
       msg->kind = ORTECDR_NOT_A_MESSAGE;
       result = KIARA_FAILURE;
    }
    else
    {
        msg->kind = (enum ORTECDR_MessageKind)msgCode;
    }

    if (msg->kind == ORTECDR_REQUEST)
    {
        freeMethodName(msg);

        if (CDR_get_string_static(&msg->codec, &msg->methodName) == CORBA_FALSE)
            result = KIARA_FAILURE;
        else
        {
            /* CDR_get_string_static references to the data in the buffer */
            msg->methodNameInBuf = 1;
        }

        /* result = readMessage_string(msg, &msg->methodName); */
    }

    KIARA_DEBUGF("After Header:\n");
    KIARA_IFDEBUG(dumpMessage(msg));

    return result;
}

static KIARA_ALWAYS_INLINE KIARA_Result writeMessageHeader(KIARA_Message *msg)
{
    KIARA_PING();

    KIARA_Result result;

    result = writeMessage_i8(msg, msg->kind);
    if (result != KIARA_SUCCESS)
        return result;

    if (msg->kind == ORTECDR_REQUEST)
    {
        result = writeMessage_string(msg, msg->methodName);
    }
    return result;
}

static KIARA_ALWAYS_INLINE KIARA_Result initMessageFromBuffer(KIARA_Message *msg, kr_dbuffer_t *buf)
{
    KIARA_PING();

    KIARA_Result result;

    clearMessage(msg, ORTECDR_NOT_A_MESSAGE);

    setMessageData(msg, kr_dbuffer_data(buf), kr_dbuffer_size(buf));

    KIARA_IFDEBUG(kr_dump_data("setStreamBuffer: ", stderr,
        (unsigned char*)msg->codec.buffer, msg->codec.buf_size, 0));
    result = readMessageHeader(msg);
    if (result != KIARA_SUCCESS)
        return result;

    if (msg->kind == ORTECDR_EXCEPTION)
        return KIARA_EXCEPTION;

    return result;
}

void freeMessage(KIARA_Message *msg)
{
    KIARA_PING();

    destroyCDRCodec(&msg->codec);
    freeMethodName(msg);
    free(msg);
}

const char * getMimeType(void)
{
    return "application/octet-stream";
}

KIARA_Message * createRequestMessageFromData(const void *data, size_t dataSize)
{
    KIARA_PING();

    KIARA_DEBUGF("createRequestMessageFromData: size = %u\n", (unsigned int)dataSize);

    KIARA_Message *msg = createNewMessage(ORTECDR_REQUEST);
    setMessageData(msg, data, dataSize);

    KIARA_IFDEBUG(dumpMessage(msg));

    if (readMessageHeader(msg) != KIARA_SUCCESS)
    {
        freeMessage(msg);
        return NULL;
    }

    return msg;
}

const char * getMessageMethodName(KIARA_Message *msg)
{
    return msg->methodName;
}

KIARA_Result getMessageData(KIARA_Message *msg, kr_dbuffer_t *dest)
{
    int result = kr_dbuffer_copy_mem(dest, CDR_buffer_data(&msg->codec), CDR_buffer_size(&msg->codec));
    return result ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Message * createRequestMessage(KIARA_Connection * KIARA_UNUSED conn, const char *name, size_t name_length)
{
    KIARA_PING();

    KIARA_Message *msg = createNewMessage(ORTECDR_REQUEST);

    CDR_buffer_ensure_capacity_nocopy(&msg->codec, 1+(sizeof(int32_t)+name_length+1)+1024);

    setMethodName(msg, name, name_length);
    writeMessageHeader(msg);

    return msg;
}

KIARA_Message * createResponseMessage(KIARA_Connection * KIARA_UNUSED conn, KIARA_Message * KIARA_UNUSED requestMsg)
{
    KIARA_PING();

    KIARA_Message *msg = createNewMessage(ORTECDR_RESPONSE);

    CDR_buffer_ensure_capacity_nocopy(&msg->codec, 1024);

    writeMessageHeader(msg);

    KIARA_IFDEBUG(dumpMessage(msg));

    KIARA_DEBUGF("LEAVE createResponseMessage\n");

    return msg;
}

void setGenericErrorMessage(KIARA_Message *msg, int errorCode, const char *errorMessage)
{
    KIARA_PINGF("msg = %p, errorCode = %i, errorMessage = %s\n", msg, errorCode, errorMessage);

    clearMessage(msg, ORTECDR_EXCEPTION);

    CDR_buffer_ensure_capacity_nocopy(&msg->codec, 1+sizeof(int32_t)+(sizeof(int32_t)+strlen(errorMessage)+1));

    writeMessageHeader(msg);
    writeMessage_i32(msg, errorCode);
    writeMessage_string(msg, errorMessage);

    KIARA_DEBUGF("LEAVE setGenericErrorMessage\n");
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

    result = sendData(conn, CDR_buffer_data(&outMsg->codec), CDR_buffer_size(&outMsg->codec), &buf);

    if (result == KIARA_SUCCESS)
        result = initMessageFromBuffer(inMsg, &buf);

    kr_dbuffer_destroy(&buf);

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

    if (CDR_put_boolean(&msg->codec, value ? 1 : 0) == CORBA_TRUE)
        return KIARA_SUCCESS;

    return KIARA_FAILURE;
}

KIARA_Result writeMessage_i8(KIARA_Message *msg, int8_t value)
{
    KIARA_PING();

    if (CDR_put_octet(&msg->codec, (CORBA_octet)value) == CORBA_TRUE)
        return KIARA_SUCCESS;

    return KIARA_FAILURE;
}

KIARA_Result writeMessage_u8(KIARA_Message *msg, uint8_t value)
{
    KIARA_PING();

    if (CDR_put_octet(&msg->codec, value) == CORBA_TRUE)
        return KIARA_SUCCESS;

    return KIARA_FAILURE;
}

KIARA_Result writeMessage_i16(KIARA_Message *msg, int16_t value)
{
    KIARA_PING();

    KIARA_DEBUGF("writeMessage_i16: write message: value = %i\n", value);

    if (CDR_put_short(&msg->codec, value) == CORBA_TRUE)
        return KIARA_SUCCESS;

    return KIARA_FAILURE;
}

KIARA_Result writeMessage_u16(KIARA_Message *msg, uint16_t value)
{
    KIARA_PING();

    if (CDR_put_ushort(&msg->codec, value) == CORBA_TRUE)
        return KIARA_SUCCESS;

    return KIARA_FAILURE;
}

KIARA_Result writeMessage_i32(KIARA_Message *msg, int32_t value)
{
    KIARA_PING();

    if (CDR_put_long(&msg->codec, value) == CORBA_TRUE)
        return KIARA_SUCCESS;

    return KIARA_FAILURE;
}

KIARA_Result writeMessage_u32(KIARA_Message *msg, uint32_t value)
{
    KIARA_PING();

    if (CDR_put_ulong(&msg->codec, value) == CORBA_TRUE)
        return KIARA_SUCCESS;

    return KIARA_FAILURE;
}

KIARA_Result writeMessage_i64(KIARA_Message *msg, int64_t value)
{
    KIARA_PING();

    if (CDR_put_long_long(&msg->codec, value) == CORBA_TRUE)
        return KIARA_SUCCESS;

    return KIARA_FAILURE;
}

KIARA_Result writeMessage_u64(KIARA_Message *msg, uint64_t value)
{
    KIARA_PING();

    if (CDR_put_ulong_long(&msg->codec, value) == CORBA_TRUE)
        return KIARA_SUCCESS;

    return KIARA_FAILURE;
}

KIARA_Result writeMessage_float(KIARA_Message *msg, float value)
{
    KIARA_PING();

    if (CDR_put_float(&msg->codec, value) == CORBA_TRUE)
        return KIARA_SUCCESS;

    return KIARA_FAILURE;
}

KIARA_Result writeMessage_double(KIARA_Message *msg, double value)
{
    KIARA_PING();

    if (CDR_put_double(&msg->codec, value) == CORBA_TRUE)
        return KIARA_SUCCESS;

    return KIARA_FAILURE;
}

KIARA_Result writeMessage_string(KIARA_Message *msg, const char * value)
{
    KIARA_PING();

    if (CDR_put_string(&msg->codec, value) == CORBA_FALSE)
        return KIARA_FAILURE;

    KIARA_IFDEBUG(dumpMessage(msg));

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

    if (CDR_put_seq_begin(&msg->codec, (CORBA_unsigned_long)size) == CORBA_FALSE)
        return KIARA_FAILURE;

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

    CORBA_unsigned_long sz;

    if (CDR_get_seq_begin(&msg->codec, &sz) == CORBA_FALSE)
        return KIARA_FAILURE;

    *size = (size_t)sz;
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

    CORBA_boolean b;
    if (CDR_get_boolean(&msg->codec, &b) == CORBA_FALSE)
        return KIARA_FAILURE;

    *value = b ? 1 : 0;

    return KIARA_SUCCESS;
}

KIARA_Result readMessage_i8(KIARA_Message *msg, int8_t *value)
{
    KIARA_PING();

    CORBA_octet tmp;

    if (CDR_get_octet(&msg->codec, &tmp) == CORBA_FALSE)
        return KIARA_FAILURE;

    *value = (int8_t)tmp;

    return KIARA_SUCCESS;
}

KIARA_Result readMessage_u8(KIARA_Message *msg, uint8_t *value)
{
    KIARA_PING();

    if (CDR_get_octet(&msg->codec, value) == CORBA_TRUE)
        return KIARA_SUCCESS;

    return KIARA_FAILURE;
}

KIARA_Result readMessage_i16(KIARA_Message *msg, int16_t *value)
{
    KIARA_PING();

    KIARA_IFDEBUG(dumpMessage(msg));

    if (CDR_get_short(&msg->codec, value) == CORBA_TRUE)
        return KIARA_SUCCESS;

    return KIARA_FAILURE;
}

KIARA_Result readMessage_u16(KIARA_Message *msg, uint16_t *value)
{
    KIARA_PING();

    if (CDR_get_ushort(&msg->codec, value) == CORBA_TRUE)
        return KIARA_SUCCESS;

    return KIARA_FAILURE;
}

KIARA_Result readMessage_i32(KIARA_Message *msg, int32_t *value)
{
    KIARA_PING();

    if (CDR_get_long(&msg->codec, value) == CORBA_TRUE)
        return KIARA_SUCCESS;

    return KIARA_FAILURE;
}

KIARA_Result readMessage_u32(KIARA_Message *msg, uint32_t *value)
{
    KIARA_PING();

    if (CDR_get_ulong(&msg->codec, value) == CORBA_TRUE)
        return KIARA_SUCCESS;

    return KIARA_FAILURE;
}

KIARA_Result readMessage_i64(KIARA_Message *msg, int64_t *value)
{
    KIARA_PING();

    if (CDR_get_long_long(&msg->codec, value) == CORBA_TRUE)
        return KIARA_SUCCESS;

    return KIARA_FAILURE;
}

KIARA_Result readMessage_u64(KIARA_Message *msg, uint64_t *value)
{
    KIARA_PING();

    if (CDR_get_ulong_long(&msg->codec, value) == CORBA_TRUE)
        return KIARA_SUCCESS;

    return KIARA_FAILURE;
}

KIARA_Result readMessage_float(KIARA_Message *msg, float *value)
{
    KIARA_PING();

    if (CDR_get_float(&msg->codec, value) == CORBA_TRUE)
        return KIARA_SUCCESS;

    return KIARA_FAILURE;
}

KIARA_Result readMessage_double(KIARA_Message *msg, double *value)
{
    KIARA_PING();

    if (CDR_get_double(&msg->codec, value) == CORBA_TRUE)
        return KIARA_SUCCESS;

    return KIARA_FAILURE;
}

static inline KIARA_Result defaultSetString(KIARA_UserType *ustr, const char *cstr)
{
    // default reallocation
    char **dest = (char**)ustr;
    free(*dest);
    *dest = malloc(strlen(cstr)+1);
    strcpy(*dest, cstr);
    return KIARA_SUCCESS;
}

KIARA_Result writeMessage_user_string(KIARA_Message *msg, KIARA_UserType *value, KIARA_GetCString getCStringFunc)
{
    KIARA_PING();
    const char *cstr;
    KIARA_Result result = getCStringFunc(value, &cstr);
    if (result != KIARA_SUCCESS)
        return result;

    result = writeMessage_string(msg, cstr);

    return result;
}

KIARA_Result readMessage_user_string(KIARA_Message *msg, KIARA_UserType *value, KIARA_SetCString setStringFunc)
{
    KIARA_PING();

    char *cstr;
    KIARA_Result result;

    result = readMessage_string(msg, &cstr);
    if (result != KIARA_SUCCESS)
        return result;

    result = setStringFunc(value, cstr);

    free(cstr);

    return result;
}

KIARA_Result readMessage_string(KIARA_Message *msg, char **value)
{
    KIARA_PING();

    char *stmp;

    if (CDR_get_string(&msg->codec, &stmp) == CORBA_FALSE)
        return KIARA_FAILURE;

    free(*value);
    *value = stmp;

    return KIARA_SUCCESS;
}

KIARA_Bool isErrorResponse(KIARA_Message *msg)
{
    KIARA_PING();

    return KIARA_TO_BOOL(msg->kind == ORTECDR_EXCEPTION);
}

KIARA_Result readGenericError(KIARA_Message *msg, KIARA_UserType *userException, KIARA_SetGenericError setGenericErrorFunc)
{
    KIARA_PING();

    int32_t errorCode;
    KIARA_Result result;

    if (msg->kind != ORTECDR_EXCEPTION)
        return KIARA_RESPONSE_ERROR;

    if (readMessage_i32(msg, &errorCode) != KIARA_SUCCESS)
        return KIARA_INVALID_RESPONSE;

    char *errorMessage = NULL;
    if (readMessage_string(msg, &errorMessage) != KIARA_SUCCESS)
        return KIARA_INVALID_RESPONSE;

    KIARA_DEBUGF("CALL setGenericErrorFunc %i %s\n", (int)errorCode, errorMessage);

    result = setGenericErrorFunc(userException, errorCode, errorMessage);

    free(errorMessage);

    return result;
}

KIARA_Result writeGenericError(KIARA_Message *msg, KIARA_UserType *userException, KIARA_GetGenericError getGenericErrorFunc)
{
    KIARA_PING();

    KIARA_DEBUGF("START\n");
    int errorCode = 0;
    const char *errorMessage = NULL;
    KIARA_Result result;

    result = getGenericErrorFunc(userException, &errorCode, &errorMessage);
    if (result != KIARA_SUCCESS)
        return result;

    setGenericErrorMessage(msg, errorCode, errorMessage);

    KIARA_IFDEBUG(dumpMessage(msg));

    return KIARA_SUCCESS;
}

KIARA_Result writeMessage_binary_stream(KIARA_Message *msg, KIARA_BinaryStream *stream)
{
    KIARA_PING();

    CORBA_unsigned_long size;

    size = (CORBA_unsigned_long)getStreamSize(stream);

    KIARA_DEBUGF("Binary Stream Size = %u\n", (unsigned int)size);

    if (CDR_put_ulong(&msg->codec, size) == CORBA_FALSE)
        return KIARA_FAILURE;

    KIARA_DEBUGF("writeData of size = %u\n", (unsigned int)size);

    if (CDR_put_octets(&msg->codec, getStreamData(stream), size) == CORBA_FALSE)
        return KIARA_FAILURE;

    KIARA_IFDEBUG(dumpMessage(msg));

    return KIARA_SUCCESS;
}

KIARA_Result readMessage_binary_stream(KIARA_Message *msg, KIARA_BinaryStream *stream)
{
    KIARA_PING();

    KIARA_Result result;
    CORBA_unsigned_long size;
    kr_dbuffer_t buf;

    KIARA_IFDEBUG(dumpMessage(msg));

    if (CDR_get_ulong(&msg->codec, &size) == CORBA_FALSE)
        return KIARA_FAILURE;

    KIARA_DEBUGF("read size -> %u\n", (unsigned int)size);

    kr_dbuffer_init(&buf);
    kr_dbuffer_resize_nocopy(&buf, size);

    result = CDR_buffer_gets(&msg->codec, kr_dbuffer_data(&buf), size) == CORBA_TRUE ? KIARA_SUCCESS : KIARA_FAILURE;

    KIARA_DEBUGF("CDR_buffer_gets result -> %i\n", (int)result);

    if (result == KIARA_SUCCESS)
    {
        setStreamBuffer(stream, &buf);
    }

    kr_dbuffer_destroy(&buf);

    KIARA_DEBUGF("LEAVE readMessage_binary_stream -> %i\n", (int)result);

    return result;
}
