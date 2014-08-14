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
 * binaryio.c
 *
 *  Created on: Aug 21, 2013
 *      Author: Dmitri Rubinstein
 */
#include "binaryio.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <KIARA/Common/Config.h>
#include <KIARA/CDT/kr_dstring.h>

//#define KIARA_DO_DEBUG

#ifdef KIARA_DO_DEBUG
#include <KIARA/CDT/kr_dumpdata.h>
#endif

#if defined(KIARA_DO_DEBUG) && !defined(NDEBUG)
#define KIARA_PING() KIARA_DEBUGF("CALLED: %s\n", KIARA_FUNCID())
#define KIARA_IFDEBUG(code) code
#define KIARA_DEBUGF(...) fprintf(stderr, __VA_ARGS__)
#else
#define KIARA_PING()
#define KIARA_IFDEBUG(code)
#define KIARA_DEBUGF(...)
#endif


struct KIARA_BinaryStream
{
    kr_dbuffer_t buf;
    size_t offset;
};

static KIARA_BinaryStream * createNewStream(void) KIARA_ALWAYS_INLINE;
static KIARA_BinaryStream * createNewStream(void)
{
    KIARA_BinaryStream *stream = malloc(sizeof(KIARA_BinaryStream));
    kr_dbuffer_init(&stream->buf);
    stream->offset = 0;
    return stream;
}

static KIARA_Result writeData(KIARA_BinaryStream *out, const void *data, size_t size)
{
    size_t requiredSize = out->offset + size;
    if (requiredSize > kr_dbuffer_size(&out->buf))
        if (!kr_dbuffer_resize(&out->buf, requiredSize))
            return KIARA_FAILURE;
    memcpy(kr_dbuffer_data(&out->buf) + out->offset, data, size);
    out->offset += size;
    return KIARA_SUCCESS;
}

static KIARA_Result readData(KIARA_BinaryStream *in, void *data, size_t size)
{
    size_t requiredSize = in->offset + size;
    if (requiredSize > kr_dbuffer_size(&in->buf))
        return KIARA_FAILURE;
    memcpy(data, kr_dbuffer_data(&in->buf) + in->offset, size);
    in->offset += size;
    return KIARA_SUCCESS;
}

size_t getStreamSize(KIARA_BinaryStream *stream)
{
    return kr_dbuffer_size(&stream->buf);
}

void * getStreamData(KIARA_BinaryStream *stream)
{
    return kr_dbuffer_data(&stream->buf);
}

void setStreamData(KIARA_BinaryStream *stream, void *data, size_t dataSize)
{
    /* FIXME: Following will copy passed data to the stream, this can be optimized when we just refer to data) */
    kr_dbuffer_copy_mem(&stream->buf, data, dataSize);
    stream->offset = 0;
}

kr_dbuffer_t * getStreamBuffer(KIARA_BinaryStream *stream)
{
    return &stream->buf;
}

void setStreamBuffer(KIARA_BinaryStream *stream, kr_dbuffer_t *buf)
{
    KIARA_PING();

    if (buf == &stream->buf)
        return;

    /*kr_dbuffer_copy_mem(&stream->buf, kr_dbuffer_data(buf), kr_dbuffer_size(buf));*/
    kr_dbuffer_move(&stream->buf, buf);
    stream->offset = 0;

    KIARA_IFDEBUG(kr_dump_data("setStreamBuffer: ", stderr,
        (unsigned char*)kr_dbuffer_data(&stream->buf), kr_dbuffer_size(&stream->buf), 0));
}

void copyStreamToBuffer(kr_dbuffer_t *dest, KIARA_BinaryStream *src)
{
    KIARA_PING();

    if (dest == &src->buf)
        return;

    KIARA_IFDEBUG(kr_dump_data("copyStreamBuffer: ", stderr,
        (unsigned char*)kr_dbuffer_data(&src->buf), kr_dbuffer_size(&src->buf), 0));

    kr_dbuffer_copy_mem(dest, kr_dbuffer_data(&src->buf), kr_dbuffer_size(&src->buf));
}

void copyBufferToStream(KIARA_BinaryStream *dest, kr_dbuffer_t *src)
{
    KIARA_PING();

    if (src == &dest->buf)
        return;

    kr_dbuffer_copy_mem(&dest->buf, kr_dbuffer_data(src), kr_dbuffer_size(src));

    dest->offset = 0;

    KIARA_IFDEBUG(kr_dump_data("copyBufferToStream: ", stderr,
        (unsigned char*)kr_dbuffer_data(&dest->buf), kr_dbuffer_size(&dest->buf), 0));
}

KIARA_BinaryStream * createOutputStream(KIARA_Connection *conn)
{
    KIARA_PING();
    return createNewStream();
}

KIARA_BinaryStream * createInputStream(KIARA_Connection *conn)
{
    KIARA_PING();
    return createNewStream();
}

void freeStream(KIARA_BinaryStream *stream)
{
    KIARA_PING();

    KIARA_IFDEBUG(kr_dump_data("freeStream: ", stderr, (unsigned char*)kr_dbuffer_data(&stream->buf), stream->offset, 0));

    kr_dbuffer_destroy(&stream->buf);
    free(stream);
}

KIARA_Result writeStructBeginAsBinary(KIARA_BinaryStream *out, const char *name)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeStructEndAsBinary(KIARA_BinaryStream *out)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeFieldBeginAsBinary(KIARA_BinaryStream *out, const char *name)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeFieldEndAsBinary(KIARA_BinaryStream *out)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readStructBeginAsBinary(KIARA_BinaryStream *in)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readStructEndAsBinary(KIARA_BinaryStream *in)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readFieldBeginAsBinary(KIARA_BinaryStream *in, const char *name)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readFieldEndAsBinary(KIARA_BinaryStream *in)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeArrayBeginAsBinary(KIARA_BinaryStream *out, size_t size)
{
    KIARA_PING();

    uint64_t sz = size;

    return writeData(out, &sz, sizeof(sz));
}

KIARA_Result writeArrayEndAsBinary(KIARA_BinaryStream *out)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result readArrayBeginAsBinary(KIARA_BinaryStream *in, size_t *size)
{
    KIARA_PING();

    KIARA_Result result;
    uint64_t sz;

    result = readData(in, &sz, sizeof(sz));
    if (result == KIARA_SUCCESS && size)
    {
        *size = sz;
    }

    return result;
}

KIARA_Result readArrayEndAsBinary(KIARA_BinaryStream *in)
{
    KIARA_PING();

    return KIARA_SUCCESS;
}

KIARA_Result writeTypeAsBinary_boolean(KIARA_BinaryStream *out, int value)
{
    KIARA_PING();

    /* Use a single byte for boolean */
    int8_t b = value ? 1 : 0;

    return writeData(out, &b, sizeof(b));
}

KIARA_Result readTypeAsBinary_boolean(KIARA_BinaryStream *in, int *value)
{
    KIARA_PING();

    int8_t b;
    KIARA_Result result;

    result = readData(in, &b, sizeof(b));
    if (result == KIARA_SUCCESS && value)
        *value = b ? 1 : 0;

    return result;
}

KIARA_Result writeTypeAsBinary_i8(KIARA_BinaryStream *out, int8_t value)
{
    KIARA_PING();

    return writeData(out, &value, sizeof(value));
}

KIARA_Result readTypeAsBinary_i8(KIARA_BinaryStream *in, int8_t *value)
{
    KIARA_PING();

    return readData(in, value, sizeof(int8_t));
}

KIARA_Result writeTypeAsBinary_u8(KIARA_BinaryStream *out, uint8_t value)
{
    KIARA_PING();

    return writeData(out, &value, sizeof(value));
}

KIARA_Result readTypeAsBinary_u8(KIARA_BinaryStream *in, uint8_t *value)
{
    KIARA_PING();

    return readData(in, value, sizeof(*value));
}

KIARA_Result writeTypeAsBinary_i16(KIARA_BinaryStream *out, int16_t value)
{
    KIARA_PING();

    return writeData(out, &value, sizeof(value));
}

KIARA_Result readTypeAsBinary_i16(KIARA_BinaryStream *in, int16_t *value)
{
    KIARA_PING();

    return readData(in, value, sizeof(*value));
}

KIARA_Result writeTypeAsBinary_u16(KIARA_BinaryStream *out, uint16_t value)
{
    KIARA_PING();

    return writeData(out, &value, sizeof(value));
}

KIARA_Result readTypeAsBinary_u16(KIARA_BinaryStream *in, uint16_t *value)
{
    KIARA_PING();

    return readData(in, value, sizeof(*value));
}

KIARA_Result writeTypeAsBinary_i32(KIARA_BinaryStream *out, int32_t value)
{
    KIARA_PING();

    return writeData(out, &value, sizeof(value));
}

KIARA_Result readTypeAsBinary_i32(KIARA_BinaryStream *in, int32_t *value)
{
    KIARA_PING();

    return readData(in, value, sizeof(*value));
}

KIARA_Result writeTypeAsBinary_u32(KIARA_BinaryStream *out, uint32_t value)
{
    KIARA_PING();

    return writeData(out, &value, sizeof(value));
}

KIARA_Result readTypeAsBinary_u32(KIARA_BinaryStream *in, uint32_t *value)
{
    KIARA_PING();

    return readData(in, value, sizeof(*value));
}

KIARA_Result writeTypeAsBinary_i64(KIARA_BinaryStream *out, int64_t value)
{
    KIARA_PING();

    return writeData(out, &value, sizeof(value));
}

KIARA_Result readTypeAsBinary_i64(KIARA_BinaryStream *in, int64_t *value)
{
    KIARA_PING();

    return readData(in, value, sizeof(int64_t));
}


KIARA_Result writeTypeAsBinary_u64(KIARA_BinaryStream *out, uint64_t value)
{
    KIARA_PING();

    return writeData(out, &value, sizeof(value));
}

KIARA_Result readTypeAsBinary_u64(KIARA_BinaryStream *in, uint64_t *value)
{
    KIARA_PING();

    return readData(in, value, sizeof(*value));
}

KIARA_Result writeTypeAsBinary_float(KIARA_BinaryStream *out, float value)
{
    KIARA_PING();

    return writeData(out, &value, sizeof(value));
}

KIARA_Result readTypeAsBinary_float(KIARA_BinaryStream *in, float *value)
{
    KIARA_PING();

    return readData(in, value, sizeof(float));
}


KIARA_Result writeTypeAsBinary_double(KIARA_BinaryStream *out, double value)
{
    KIARA_PING();

    return writeData(out, &value, sizeof(value));
}

KIARA_Result readTypeAsBinary_double(KIARA_BinaryStream *in, double *value)
{
    KIARA_PING();

    return readData(in, value, sizeof(*value));
}

KIARA_Result writeTypeAsBinary_string(KIARA_BinaryStream *out, const char * value)
{
    KIARA_PING();

    int32_t len = strlen(value);

    if (writeData(out, &len, sizeof(len)) != KIARA_SUCCESS)
        return KIARA_FAILURE;

    if (writeData(out, value, len) != KIARA_SUCCESS)
        return KIARA_FAILURE;

    return KIARA_SUCCESS;
}

KIARA_Result readTypeAsBinary_string(KIARA_BinaryStream *in, char ** value)
{
    KIARA_PING();

    int32_t len;
    char *stmp;

    if (readData(in, &len, sizeof(len)) != KIARA_SUCCESS)
        return KIARA_FAILURE;

    KIARA_DEBUGF("readTypeAsBinary_string: read string: len = %i\n", len);

    stmp = (char*)malloc(len+1);
    if (!stmp)
        return KIARA_FAILURE;
    if (readData(in, stmp, len) != KIARA_SUCCESS)
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

/* Write zero terminated string to message with user-defined getter method */
KIARA_Result writeTypeAsBinary_user_string(KIARA_BinaryStream *out, KIARA_UserType *value, KIARA_GetCString getCStringFunc)
{
    KIARA_PING();

    const char *cstr;
    KIARA_Result result = getCStringFunc(value, &cstr);
    if (result != KIARA_SUCCESS)
        return result;
    return writeTypeAsBinary_string(out, cstr);
}

/* Read zero terminated string from message with user-defined setter method */
KIARA_Result readTypeAsBinary_user_string(KIARA_BinaryStream *in, KIARA_UserType *value, KIARA_SetCString setCStringFunc)
{
    KIARA_PING();

    char *cstr = NULL;
    KIARA_Result result = readTypeAsBinary_string(in, &cstr);

    if (result != KIARA_SUCCESS)
        return result;

    result = setCStringFunc(value, cstr);

    KIARA_DEBUGF("readTypeAsBinary_user_string: string was set to %s\n", cstr);

    free(cstr);

    return result;
}

KIARA_Result encryptStream(KIARA_Connection *conn, KIARA_BinaryStream *stream, const char *keyName)
{
    KIARA_Result result = KIARA_SUCCESS;
    KIARA_SymmetricKey *key = kiaraNewSymmetricKey(kiaraGetCipher(NULL));
    if (key)
    {
        const char *keyText = kiaraGetSecretKeyText(conn, keyName);
        if (keyText)
        {
            result = kiaraInitSymmetricKeyFromText(key, keyText);
            if (result == KIARA_SUCCESS)
            {
                KIARA_CipherContext *ec = kiaraNewCipherContext();
                if (ec)
                {
                    result = kiaraInitEncryption(ec, key);
                    if (result == KIARA_SUCCESS)
                    {
                        kr_dbuffer_t tmp;
                        kr_dbuffer_init(&tmp);

                        result = kiaraEncrypt(ec, &tmp, getStreamBuffer(stream));
                        if (result == KIARA_SUCCESS)
                            setStreamBuffer(stream, &tmp);

                        kr_dbuffer_destroy(&tmp);
                    }

                    kiaraFreeCipherContext(ec);
                }
                else
                    result = KIARA_FAILURE; /* Could not create cipher context */
            }
        }
        else
            result = KIARA_FAILURE; /* Could not get key text */
        kiaraFreeSymmetricKey(key);
    }
    else
        result = KIARA_FAILURE; /* Could not create key */
    return result;
}

KIARA_Result decryptStream(KIARA_Connection *conn, KIARA_BinaryStream *stream, const char *keyName)
{
    KIARA_Result result = KIARA_SUCCESS;
    KIARA_SymmetricKey *key = kiaraNewSymmetricKey(kiaraGetCipher(NULL));
    if (key)
    {
        const char *keyText = kiaraGetSecretKeyText(conn, keyName);
        if (keyText)
        {
            result = kiaraInitSymmetricKeyFromText(key, keyText);
            if (result == KIARA_SUCCESS)
            {
                KIARA_CipherContext *dc = kiaraNewCipherContext();
                if (dc)
                {
                    result = kiaraInitDecryption(dc, key);
                    if (result == KIARA_SUCCESS)
                    {
                        kr_dbuffer_t tmp;
                        kr_dbuffer_init(&tmp);

                        result = kiaraDecrypt(dc, &tmp, getStreamBuffer(stream));
                        if (result == KIARA_SUCCESS)
                            setStreamBuffer(stream, &tmp);

                        kr_dbuffer_destroy(&tmp);
                    }

                    kiaraFreeCipherContext(dc);
                }
                else
                    result = KIARA_FAILURE; /* Could not create cipher context */
            }
        }
        else
            result = KIARA_FAILURE; /* Could not get key text */
        kiaraFreeSymmetricKey(key);
    }
    else
        result = KIARA_FAILURE; /* Could not create key */

    KIARA_DEBUGF("LEAVE decryptStream -> %i\n", (int)result);

    return result;
}

/*
KIARA_Bool isErrorResponse(KIARA_BinaryStream *dest) KIARA_ALWAYS_INLINE;

KIARA_Result readGenericError(KIARA_BinaryStream *dest, KIARA_UserType *userException, KIARA_SetGenericError setGenericErrorFunc) KIARA_ALWAYS_INLINE;
KIARA_Result writeGenericError(KIARA_BinaryStream *dest, KIARA_UserType *userException, KIARA_GetGenericError getGenericErrorFunc) KIARA_ALWAYS_INLINE;
*/
