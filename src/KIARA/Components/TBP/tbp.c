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
 * tbp.c
 *
 *  Created on: 21.12.2013
 *      Author: Dmitri Rubinstein
 */

/* Used variable length encoding is from Google Protocol Buffers:
 *
 * Protocol Buffers - Google's data interchange format
 * Copyright 2008 Google Inc.  All rights reserved.
 * http://code.google.com/p/protobuf/
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 * Author: kenton@google.com (Kenton Varda)
 *  Based on original Protocol Buffers design by
 *  Sanjay Ghemawat, Jeff Dean, and others.
 *
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

//#define KIARA_DO_DEBUG
#if defined(KIARA_DO_DEBUG) && !defined(NDEBUG)
#define KIARA_PING() KIARA_DEBUGF("CALLED: %s\n", KIARA_FUNCID())
#define KIARA_IFDEBUG(code) code
#define KIARA_DEBUGF(...) fprintf(stderr, __VA_ARGS__)
#else
#define KIARA_PING()
#define KIARA_IFDEBUG(code)
#define KIARA_DEBUGF(...)
#endif

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

KIARA_INFO_BEGIN
KIARA_COMMAND("llvmMarkFunctionsWithAlwaysInline")
KIARA_INFO_END

/*
 * Messaging via Trivial Binary Protocol (TBP)
 */

enum TBP_MessageKind
{
    TBP_NOT_A_MESSAGE = 0,
    TBP_REQUEST       = 1,
    TBP_RESPONSE      = 2,
    TBP_EXCEPTION     = 3
};

struct KIARA_Message
{
    enum TBP_MessageKind kind;
    kr_dbuffer_t buf;
    size_t offset;
    char *methodName;
};

static void dumpMessage(KIARA_Message *msg)
{
    const char *msgType;

    fprintf(stderr, "Message {\n");

    switch (msg->kind)
    {
        case TBP_NOT_A_MESSAGE: msgType = "NOT A MESSAGE"; break;
        case TBP_REQUEST: msgType = "REQUEST"; break;
        case TBP_RESPONSE: msgType = "RESPONSE"; break;
        case TBP_EXCEPTION: msgType = "EXCEPTION"; break;
        default: msgType = "<Unknown Type>"; break;
    }

    fprintf(stderr, "  %s\n", msgType);
    kr_dump_data("  Buffer: ", stderr,
            (unsigned char*)kr_dbuffer_data(&msg->buf), kr_dbuffer_size(&msg->buf), 0);
    fprintf(stderr, " Offset: %i\n", (int)msg->offset);
    fprintf(stderr, " Method name: %s\n", msg->methodName);
    fprintf(stderr, "}\n");
}

static void initMessage(KIARA_Message *msg) KIARA_ALWAYS_INLINE;
static void initMessage(KIARA_Message *msg)
{
    if (msg)
    {
        msg->kind = TBP_REQUEST;
        kr_dbuffer_init(&msg->buf);
        msg->offset = 0;
        msg->methodName = NULL;
    }
}

static KIARA_Message * createNewMessage(enum TBP_MessageKind kind) KIARA_ALWAYS_INLINE;
static KIARA_Message * createNewMessage(enum TBP_MessageKind kind)
{
   KIARA_Message *msg = malloc(sizeof(KIARA_Message));
   msg->kind = kind;
   kr_dbuffer_init(&msg->buf);
   msg->offset = 0;
   msg->methodName = NULL;
   return msg;
}

static void setMethodName(KIARA_Message *msg, const char *methodName, size_t methodNameLen) KIARA_ALWAYS_INLINE;
static void setMethodName(KIARA_Message *msg, const char *methodName, size_t methodNameLen)
{
    char *tmp = (char*)malloc(methodNameLen+1);
    strncpy(tmp, methodName, methodNameLen+1);
    tmp[methodNameLen] = '\0';
    msg->methodName = tmp;
}

static void clearMessage(KIARA_Message *msg, enum TBP_MessageKind newKind) KIARA_ALWAYS_INLINE;
static void clearMessage(KIARA_Message *msg, enum TBP_MessageKind newKind)
{
    KIARA_PING();

    msg->kind = newKind;
    kr_dbuffer_clear(&msg->buf);
    free(msg->methodName);
    msg->methodName = NULL;
    msg->offset = 0;
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

    if (msgCode != TBP_REQUEST && msgCode != TBP_RESPONSE && msgCode != TBP_EXCEPTION)
    {
        msg->kind = TBP_NOT_A_MESSAGE;
        result = KIARA_FAILURE;
    }
    else
    {
        msg->kind = (enum TBP_MessageKind)msgCode;
    }

    if (msg->kind == TBP_REQUEST)
    {
        result = readMessage_string(msg, &msg->methodName);
    }

    KIARA_DEBUGF("After Header:\n");
    KIARA_IFDEBUG(dumpMessage(msg));

    return result;
}

static KIARA_Result writeMessageHeader(KIARA_Message *msg) KIARA_ALWAYS_INLINE;
static KIARA_Result writeMessageHeader(KIARA_Message *msg)
{
    KIARA_PING();

    KIARA_Result result;

    result = writeMessage_i8(msg, msg->kind);
    if (result != KIARA_SUCCESS)
        return result;

    if (msg->kind == TBP_REQUEST)
    {
        result = writeMessage_string(msg, msg->methodName);
    }
    return result;
}

static KIARA_Result initMessageFromBuffer(KIARA_Message *msg, kr_dbuffer_t *buf) KIARA_ALWAYS_INLINE;
static KIARA_Result initMessageFromBuffer(KIARA_Message *msg, kr_dbuffer_t *buf)
{
    KIARA_PING();

    KIARA_Result result;

    if (buf == &msg->buf)
        return KIARA_FAILURE;

    clearMessage(msg, TBP_NOT_A_MESSAGE);

    /*kr_dbuffer_copy_mem(&msg->buf, kr_dbuffer_data(buf), kr_dbuffer_size(buf));*/
    kr_dbuffer_move(&msg->buf, buf);

    KIARA_IFDEBUG(kr_dump_data("setStreamBuffer: ", stderr,
        (unsigned char*)kr_dbuffer_data(&msg->buf), kr_dbuffer_size(&msg->buf), 0));

    result = readMessageHeader(msg);
    if (result != KIARA_SUCCESS)
        return result;

    if (msg->kind == TBP_EXCEPTION)
        return KIARA_EXCEPTION;

    return result;
}

void freeMessage(KIARA_Message *msg)
{
    KIARA_PING();

    kr_dbuffer_destroy(&msg->buf);
    free(msg->methodName);
    free(msg);
}

static void advanceBuffer(KIARA_Message *msg, size_t size) KIARA_ALWAYS_INLINE;
static void advanceBuffer(KIARA_Message *msg, size_t size)
{
    msg->offset += size;
}

static uint8_t * getBufferBegin(KIARA_Message *msg) KIARA_ALWAYS_INLINE;
static uint8_t * getBufferBegin(KIARA_Message *msg)
{
    return (uint8_t*)(kr_dbuffer_data(&msg->buf) + msg->offset);
}

static uint8_t * getBufferEnd(KIARA_Message *msg) KIARA_ALWAYS_INLINE;
static uint8_t * getBufferEnd(KIARA_Message *msg)
{
    return (uint8_t*)(kr_dbuffer_data(&msg->buf) + kr_dbuffer_size(&msg->buf));
}

static size_t getBufferSize(KIARA_Message *msg) KIARA_ALWAYS_INLINE;
static size_t getBufferSize(KIARA_Message *msg)
{
    return kr_dbuffer_size(&msg->buf) - msg->offset;
}

static void writeRaw(KIARA_Message *out, const void *data, size_t size) KIARA_ALWAYS_INLINE;
static void writeRaw(KIARA_Message *out, const void *data, size_t size)
{
    memcpy(getBufferBegin(out), data, size);
    advanceBuffer(out, size);
}

static void readRaw(KIARA_Message *in, void *data, size_t size) KIARA_ALWAYS_INLINE;
static void readRaw(KIARA_Message *in, void *data, size_t size)
{
    memcpy(data, getBufferBegin(in), size);
    advanceBuffer(in, size);
}

static int bufferHasSpace(KIARA_Message *msg, size_t dataSize) KIARA_ALWAYS_INLINE;
static int bufferHasSpace(KIARA_Message *msg, size_t dataSize)
{
    size_t requiredSize = msg->offset + dataSize;
    return requiredSize <= kr_dbuffer_size(&msg->buf);
}

static KIARA_Result writeData(KIARA_Message *out, const void *data, size_t size)
{
    size_t requiredSize = out->offset + size;
    if (requiredSize > kr_dbuffer_size(&out->buf))
        if (!kr_dbuffer_resize(&out->buf, requiredSize))
            return KIARA_FAILURE;
    writeRaw(out, data, size);
    return KIARA_SUCCESS;
}

static KIARA_Result readData(KIARA_Message *in, void *data, size_t size)
{
    size_t requiredSize = in->offset + size;
    if (requiredSize > kr_dbuffer_size(&in->buf))
        return KIARA_FAILURE;
    readRaw(in, data, size);
    return KIARA_SUCCESS;
}

#if 1

#define MAX_VARINT_BYTES    10
#define MAX_VARINT32_BYTES  5

static KIARA_Result KIARA_ALWAYS_INLINE readVarint64Slow(KIARA_Message *in, uint64_t * value)
{
    /* Slow path:  This read might cross the end of the buffer, so we
     * need to check and refresh the buffer if and when it does.
     */

    uint64_t result = 0;
    int count = 0;
    uint32_t b;

    do
    {
        if (count == MAX_VARINT_BYTES)
            return KIARA_FAILURE;
        if (in->offset + 1 > kr_dbuffer_size(&in->buf))
            return KIARA_FAILURE;

        b = *(uint8_t*) getBufferBegin(in);
        result |= (uint64_t) (b & 0x7F) << (7 * count);
        advanceBuffer(in, 1);
        ++count;
    } while (b & 0x80);

    *value = result;
    return KIARA_SUCCESS;
}

static KIARA_Result KIARA_ALWAYS_INLINE readVarint64Fallback(KIARA_Message *in, uint64_t *value)
{
    if (getBufferSize(in) >= MAX_VARINT_BYTES)
    {
        // Fast path:  We have enough bytes left in the buffer to guarantee that
        // this read won't cross the end, so we can skip the checks.

        const uint8_t * ptr = getBufferBegin(in);
        uint32_t b;

        // Splitting into 32-bit pieces gives better performance on 32-bit
        // processors.
        uint32_t part0 = 0, part1 = 0, part2 = 0;

        b = *(ptr++); part0  = (b & 0x7F)      ; if (!(b & 0x80)) goto done;
        b = *(ptr++); part0 |= (b & 0x7F) <<  7; if (!(b & 0x80)) goto done;
        b = *(ptr++); part0 |= (b & 0x7F) << 14; if (!(b & 0x80)) goto done;
        b = *(ptr++); part0 |= (b & 0x7F) << 21; if (!(b & 0x80)) goto done;
        b = *(ptr++); part1  = (b & 0x7F)      ; if (!(b & 0x80)) goto done;
        b = *(ptr++); part1 |= (b & 0x7F) <<  7; if (!(b & 0x80)) goto done;
        b = *(ptr++); part1 |= (b & 0x7F) << 14; if (!(b & 0x80)) goto done;
        b = *(ptr++); part1 |= (b & 0x7F) << 21; if (!(b & 0x80)) goto done;
        b = *(ptr++); part2  = (b & 0x7F)      ; if (!(b & 0x80)) goto done;
        b = *(ptr++); part2 |= (b & 0x7F) <<  7; if (!(b & 0x80)) goto done;

        // We have overrun the maximum size of a varint (10 bytes).  The data
        // must be corrupt.
        return KIARA_FAILURE;

    done:
        advanceBuffer(in, ptr - getBufferBegin(in));
        *value = ((uint64_t)(part0)      ) |
                 ((uint64_t)(part1) << 28) |
                 ((uint64_t)(part2) << 56);
        return KIARA_SUCCESS;
    }
    else
        return readVarint64Slow(in, value);
}

static KIARA_Result KIARA_ALWAYS_INLINE readVarint64(KIARA_Message *in, uint64_t *value)
{
    if ((getBufferBegin(in) < getBufferEnd(in)) && *getBufferBegin(in) < 0x80)
    {
          *value = *getBufferBegin(in);
          advanceBuffer(in, 1);
          return KIARA_SUCCESS;
    }
    else
    {
        return readVarint64Fallback(in, value);
    }
}

static uint8_t * KIARA_ALWAYS_INLINE writeVarint64ToArray(uint64_t value, uint8_t * target)
{
    /* Splitting into 32-bit pieces gives better performance on 32-bit
     * processors.
     */
    uint32_t part0 = (uint32_t) (value);
    uint32_t part1 = (uint32_t) (value >> 28);
    uint32_t part2 = (uint32_t) (value >> 56);

    int size;

    /* Here we can't really optimize for small numbers, since the value is
     * split into three parts.  Cheking for numbers < 128, for instance,
     * would require three comparisons, since you'd have to make sure part1
     * and part2 are zero.  However, if the caller is using 64-bit integers,
     * it is likely that they expect the numbers to often be very large, so
     * we probably don't want to optimize for small numbers anyway.  Thus,
     * we end up with a hardcoded binary search tree...
     */
    if (part2 == 0)
    {
        if (part1 == 0)
        {
            if (part0 < (1 << 14))
            {
                if (part0 < (1 << 7))
                {
                    size = 1;
                    goto size1;
                }
                else
                {
                    size = 2;
                    goto size2;
                }
            }
            else
            {
                if (part0 < (1 << 21))
                {
                    size = 3;
                    goto size3;
                }
                else
                {
                    size = 4;
                    goto size4;
                }
            }
        }
        else
        {
            if (part1 < (1 << 14))
            {
                if (part1 < (1 << 7))
                {
                    size = 5;
                    goto size5;
                }
                else
                {
                    size = 6;
                    goto size6;
                }
            }
            else
            {
                if (part1 < (1 << 21))
                {
                    size = 7;
                    goto size7;
                }
                else
                {
                    size = 8;
                    goto size8;
                }
            }
        }
    }
    else
    {
        if (part2 < (1 << 7))
        {
            size = 9;
            goto size9;
        }
        else
        {
            size = 10;
            goto size10;
        }
    }

    assert(0 && "Can't get here.");

    size10: target[9] = ( uint8_t ) ((part2 >> 7) | 0x80);
    size9: target[8] = ( uint8_t ) ((part2) | 0x80);
    size8: target[7] = ( uint8_t ) ((part1 >> 21) | 0x80);
    size7: target[6] = ( uint8_t ) ((part1 >> 14) | 0x80);
    size6: target[5] = ( uint8_t ) ((part1 >> 7) | 0x80);
    size5: target[4] = ( uint8_t ) ((part1) | 0x80);
    size4: target[3] = ( uint8_t ) ((part0 >> 21) | 0x80);
    size3: target[2] = ( uint8_t ) ((part0 >> 14) | 0x80);
    size2: target[1] = ( uint8_t ) ((part0 >> 7) | 0x80);
    size1: target[0] = ( uint8_t ) ((part0) | 0x80);

    target[size - 1] &= 0x7F;
    return target + size;
}

static KIARA_Result KIARA_ALWAYS_INLINE writeVarint64(KIARA_Message *out, uint64_t value)
{
    if (getBufferSize(out) >= MAX_VARINT_BYTES)
    {
        /* Fast path:  We have enough bytes left in the buffer to guarantee that
           this write won't cross the end, so we can skip the checks.
         */
        uint8_t * target = (uint8_t*)getBufferBegin(out);

        uint8_t * end = writeVarint64ToArray(value, target);
        ptrdiff_t size = end - target;
        advanceBuffer(out, size);
    }
    else
    {
        /*
         * Slow path:  This write might cross the end of the buffer, so we
         * compose the bytes first then use WriteRaw().
         */
        uint8_t bytes[MAX_VARINT_BYTES];
        size_t size = 0;
        while (value > 0x7F)
        {
            bytes[size++] = ((uint8_t)(value) & 0x7F) | 0x80;
            value >>= 7;
        }
        bytes[size++] = ((uint8_t)(value)) & 0x7F;
        return writeData(out, bytes, size);
    }
    return KIARA_SUCCESS;
}
#endif

static size_t KIARA_ALWAYS_INLINE getMessageSize(KIARA_Message *stream)
{
    return kr_dbuffer_size(&stream->buf);
}

static void KIARA_ALWAYS_INLINE setMessageData(KIARA_Message *stream, void *data, size_t dataSize)
{
    /* FIXME: Following will copy passed data to the stream, this can be optimized when we just refer to data) */
    kr_dbuffer_copy_mem(&stream->buf, data, dataSize);
    stream->offset = 0;
}

kr_dbuffer_t * KIARA_ALWAYS_INLINE getMessageBuffer(KIARA_Message *stream)
{
    return &stream->buf;
}

void KIARA_ALWAYS_INLINE setMessageBuffer(KIARA_Message *stream, kr_dbuffer_t *buf)
{
    KIARA_PING();

    if (buf == &stream->buf)
        return;

    /*kr_dbuffer_copy_mem(&stream->buf, kr_dbuffer_data(buf), kr_dbuffer_size(buf));*/
    kr_dbuffer_move(&stream->buf, buf);
    stream->offset = 0;

    KIARA_IFDEBUG(kr_dump_data("setMessageBuffer: ", stderr,
        (unsigned char*)kr_dbuffer_data(&stream->buf), kr_dbuffer_size(&stream->buf), 0));
}

void KIARA_ALWAYS_INLINE copyMessageToBuffer(kr_dbuffer_t *dest, KIARA_Message *src)
{
    KIARA_PING();

    if (dest == &src->buf)
        return;

    KIARA_IFDEBUG(kr_dump_data("copyStreamBuffer: ", stderr,
        (unsigned char*)kr_dbuffer_data(&src->buf), kr_dbuffer_size(&src->buf), 0));

    kr_dbuffer_copy_mem(dest, kr_dbuffer_data(&src->buf), kr_dbuffer_size(&src->buf));
}

void KIARA_ALWAYS_INLINE copyBufferToMessage(KIARA_Message *dest, kr_dbuffer_t *src)
{
    KIARA_PING();

    if (src == &dest->buf)
        return;

    kr_dbuffer_copy_mem(&dest->buf, kr_dbuffer_data(src), kr_dbuffer_size(src));

    dest->offset = 0;

    KIARA_IFDEBUG(kr_dump_data("copyBufferToStream: ", stderr,
        (unsigned char*)kr_dbuffer_data(&dest->buf), kr_dbuffer_size(&dest->buf), 0));
}

const char * getMimeType(void)
{
    return "application/octet-stream";
}

KIARA_Message * createRequestMessageFromData(const void *data, size_t dataSize)
{
    KIARA_Message *msg = createNewMessage(TBP_REQUEST);
    kr_dbuffer_copy_mem(&msg->buf, data, dataSize);

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

static int dump_to_buffer(const char *buffer, size_t size, void *data)
{
    kr_dbuffer_t *dbuf = (kr_dbuffer_t*)data;
    if (kr_dbuffer_append_mem(dbuf, buffer, size))
        return 0;
    return -1;
}

KIARA_Result getMessageData(KIARA_Message *msg, kr_dbuffer_t *dest)
{
    int result = kr_dbuffer_assign(dest, &msg->buf);
    return result ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Message * createRequestMessage(KIARA_Connection * KIARA_UNUSED conn, const char *name, size_t name_length)
{
    KIARA_PING();

    KIARA_Message *msg = createNewMessage(TBP_REQUEST);
    setMethodName(msg, name, name_length);
    writeMessageHeader(msg);

    return msg;
}

KIARA_Message * createResponseMessage(KIARA_Connection * KIARA_UNUSED conn, KIARA_Message * KIARA_UNUSED requestMsg)
{
    KIARA_PING();

    KIARA_Message *msg = createNewMessage(TBP_RESPONSE);
    writeMessageHeader(msg);

    KIARA_DEBUGF("LEAVE createResponseMessage\n");

    return msg;
}

void setGenericErrorMessage(KIARA_Message *msg, int errorCode, const char *errorMessage)
{
    KIARA_PING();

    clearMessage(msg, TBP_EXCEPTION);
    writeMessageHeader(msg);
    writeMessage_i32(msg, errorCode);
    writeMessage_string(msg, errorMessage);
}

KIARA_Result sendMessageSync(KIARA_Connection *conn, KIARA_Message *outMsg, KIARA_Message *inMsg)
{
    int result = 0;
    kr_dbuffer_t buf;

    if (!outMsg)
        return -1; /* FIXME use proper error code */

    /*KIARA_DEBUGF("sendMessageSync %s to %s\n", msgData, getConnectionURI(conn));*/
    /*KIARA_DEBUGF("sendMessageSync %s\n", msgData);*/

    kr_dbuffer_init(&buf);

    result = sendData(conn, kr_dbuffer_data(&outMsg->buf), kr_dbuffer_size(&outMsg->buf), &buf);

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

    /* Use a single byte for boolean */
    int8_t b = value ? 1 : 0;

    return writeData(msg, &b, sizeof(b));
}

KIARA_Result writeMessage_i8(KIARA_Message *msg, int8_t value)
{
    KIARA_PING();

    return writeData(msg, &value, sizeof(value));
}

KIARA_Result writeMessage_u8(KIARA_Message *msg, uint8_t value)
{
    KIARA_PING();

    return writeData(msg, &value, sizeof(value));
}

KIARA_Result writeMessage_i16(KIARA_Message *msg, int16_t value)
{
    KIARA_PING();

    KIARA_DEBUGF("writeMessage_i16: write message: value = %i\n", value);

    return writeData(msg, &value, sizeof(value));
}

KIARA_Result writeMessage_u16(KIARA_Message *msg, uint16_t value)
{
    KIARA_PING();

    return writeData(msg, &value, sizeof(value));
}

KIARA_Result writeMessage_i32(KIARA_Message *msg, int32_t value)
{
    KIARA_PING();

    return writeData(msg, &value, sizeof(value));
}

KIARA_Result writeMessage_u32(KIARA_Message *msg, uint32_t value)
{
    KIARA_PING();

    return writeData(msg, &value, sizeof(value));
}

KIARA_Result writeMessage_i64(KIARA_Message *msg, int64_t value)
{
    KIARA_PING();

    return writeData(msg, &value, sizeof(value));
}

KIARA_Result writeMessage_u64(KIARA_Message *msg, uint64_t value)
{
    KIARA_PING();

    return writeData(msg, &value, sizeof(value));
}

KIARA_Result writeMessage_float(KIARA_Message *msg, float value)
{
    KIARA_PING();

    return writeData(msg, &value, sizeof(value));
}

KIARA_Result writeMessage_double(KIARA_Message *msg, double value)
{
    KIARA_PING();

    return writeData(msg, &value, sizeof(value));
}

KIARA_Result writeMessage_string(KIARA_Message *msg, const char * value)
{
    KIARA_PING();

#if 0
    int32_t len = strlen(value);

    if (writeData(msg, &len, sizeof(len)) != KIARA_SUCCESS)
        return KIARA_FAILURE;
#else
    uint64_t len = strlen(value);
    if (writeVarint64(msg, len) != KIARA_SUCCESS)
        return KIARA_FAILURE;
#endif

    if (writeData(msg, value, len) != KIARA_SUCCESS)
        return KIARA_FAILURE;

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

    uint64_t sz = size;

    return writeData(msg, &sz, sizeof(sz));
}

KIARA_Result writeArrayEnd(KIARA_Message * KIARA_UNUSED msg)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readArrayBegin(KIARA_Message *msg, size_t *size)
{
    KIARA_PING();

    KIARA_Result result;
    uint64_t sz;

    result = readData(msg, &sz, sizeof(sz));
    if (result == KIARA_SUCCESS && size)
    {
        *size = sz;
    }

    return result;
}

KIARA_Result readArrayEnd(KIARA_Message * KIARA_UNUSED msg)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readMessage_boolean(KIARA_Message *msg, int *value)
{
    KIARA_PING();

    int8_t b;
    KIARA_Result result;

    result = readData(msg, &b, sizeof(b));
    if (result == KIARA_SUCCESS && value)
        *value = b ? 1 : 0;

    return result;
}

KIARA_Result readMessage_i8(KIARA_Message *msg, int8_t *value)
{
    KIARA_PING();

    return readData(msg, value, sizeof(*value));
}

KIARA_Result readMessage_u8(KIARA_Message *msg, uint8_t *value)
{
    KIARA_PING();

    return readData(msg, value, sizeof(*value));
}

KIARA_Result readMessage_i16(KIARA_Message *msg, int16_t *value)
{
    KIARA_PING();

    KIARA_IFDEBUG(dumpMessage(msg));

    KIARA_Result result;

    result = readData(msg, value, sizeof(*value));

    KIARA_DEBUGF("readMessage_i16 -> %i\n", *value);

    return result;
}

KIARA_Result readMessage_u16(KIARA_Message *msg, uint16_t *value)
{
    KIARA_PING();

    return readData(msg, value, sizeof(*value));
}

KIARA_Result readMessage_i32(KIARA_Message *msg, int32_t *value)
{
    KIARA_PING();

    return readData(msg, value, sizeof(*value));
}

KIARA_Result readMessage_u32(KIARA_Message *msg, uint32_t *value)
{
    KIARA_PING();

    return readData(msg, value, sizeof(*value));
}

KIARA_Result readMessage_i64(KIARA_Message *msg, int64_t *value)
{
    KIARA_PING();

    return readData(msg, value, sizeof(*value));
}

KIARA_Result readMessage_u64(KIARA_Message *msg, uint64_t *value)
{
    KIARA_PING();

    return readData(msg, value, sizeof(*value));
}

KIARA_Result readMessage_float(KIARA_Message *msg, float *value)
{
    KIARA_PING();

    return readData(msg, value, sizeof(*value));
}

KIARA_Result readMessage_double(KIARA_Message *msg, double *value)
{
    KIARA_PING();

    return readData(msg, value, sizeof(*value));
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

    uint64_t len;
    char *stmp;

#if 0
    if (readData(msg, &len, sizeof(len)) != KIARA_SUCCESS)
        return KIARA_FAILURE;
#else
    if (readVarint64(msg, &len) != KIARA_SUCCESS)
        return KIARA_FAILURE;
#endif

    KIARA_DEBUGF("readMessage_string: read string: len = %i\n", len);

    stmp = (char*)malloc(len+1);
    if (!stmp)
        return KIARA_FAILURE;
    if (readData(msg, stmp, len) != KIARA_SUCCESS)
    {
        KIARA_DEBUGF("readTypeAsBinary_string: read string failed\n");
        free(stmp);
        return KIARA_FAILURE;
    }
    stmp[len] = '\0';
    KIARA_DEBUGF("read string: str = %s\n", stmp);

    free(*value);
    *value = stmp;

    return KIARA_SUCCESS;
}

KIARA_Bool isErrorResponse(KIARA_Message *msg)
{
    KIARA_PING();

    return KIARA_TO_BOOL(msg->kind == TBP_EXCEPTION);
}

KIARA_Result readGenericError(KIARA_Message *msg, KIARA_UserType *userException, KIARA_SetGenericError setGenericErrorFunc)
{
    KIARA_PING();

    int32_t errorCode;
    KIARA_Result result;

    if (msg->kind != TBP_EXCEPTION)
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

    KIARA_Result result;
    uint64_t size;

    size = getStreamSize(stream);

    KIARA_DEBUGF("Binary Stream Size = %i\n", (int)size);

    result = writeVarint64(msg, size);

    if (result != KIARA_SUCCESS)
        return result;

    KIARA_DEBUGF("writeData of size = %i\n", (int)size);

    result = writeData(msg, getStreamData(stream), size);

    KIARA_IFDEBUG(dumpMessage(msg));

    if (result != KIARA_SUCCESS)
        return result;

    return result;
}

KIARA_Result readMessage_binary_stream(KIARA_Message *msg, KIARA_BinaryStream *stream)
{
    KIARA_PING();

    KIARA_Result result;
    uint64_t size;
    kr_dbuffer_t buf;

    KIARA_IFDEBUG(dumpMessage(msg));

    result = readVarint64(msg, &size);

    KIARA_DEBUGF("readMessage_i16 result -> %i\n", result);
    KIARA_DEBUGF("readMessage_i16 value -> %i\n", (int)size);

    if (result != KIARA_SUCCESS)
        return result;

    kr_dbuffer_init(&buf);
    kr_dbuffer_resize_nocopy(&buf, size);

    result = readData(msg, kr_dbuffer_data(&buf), size);

    KIARA_DEBUGF("readData result -> %i\n", (int)result);

    if (result == KIARA_SUCCESS)
    {
        setStreamBuffer(stream, &buf);
    }

    kr_dbuffer_destroy(&buf);

    KIARA_DEBUGF("LEAVE readMessage_binary_stream -> %i\n", (int)result);

    return result;
}
