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
 * message.c
 *
 *  Created on: 21.04.2013
 *      Author: Dmitri Rubinstein
 */
#include <KIARA/Components/api.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <KIARA/Common/Config.h>
#include <KIARA/CDT/kr_base64.h>

#include "kiara_module.h"
#include "binaryio.h"
#include "jansson.h"

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

/* Check http://stackoverflow.com/questions/5707957/c-curl-json-rest */

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
KIARA_COMMAND("llvmMarkFunctionsWithNoInline")
KIARA_INFO_END

/*
 * Messaging via JSON-RPC 2.0
 */

/** TODO
 * Fix cursor
 * index is used when value is ARRAY
 * fieldName is used when value is OBJECT
 * index is incremented automatically
 * fieldName is set manually via API
 */

typedef struct KIARA_CursorNode
{
    struct KIARA_CursorNode *prev;
    json_t *value;
    const char *fieldName; /* Not dynamically allocated, valid when value is an object */
    union {
        void *objectIter; /* valid when value is an object */
        size_t index;     /* valid when value is an array or single value */
    } data;
} KIARA_CursorNode;

struct KIARA_Message
{
    json_t *body;
    json_t *params; /* refers to "params" on request
                                 "result" on successful response
                                 "data" of the error on error response */
    json_t *error;  /* refers to "error" on error response */
    KIARA_CursorNode *cursor;
};

#define cursor_at_object(msg) (json_is_object((msg)->cursor->value))
#define cursor_at_array(msg) (json_is_array((msg)->cursor->value))

static void dumpNode(json_t *node)
{
    if (node)
        KIARA_IFDEBUG(json_dumpf(node, stderr, JSON_ENCODE_ANY | JSON_INDENT(2)));
    else
        fprintf(stderr, "NULL");
}

static void dumpCursorInfo(KIARA_Message *msg)
{
    KIARA_DEBUGF("body = \n");
    if (msg->body)
        KIARA_IFDEBUG(json_dumpf(msg->body, stderr, JSON_ENCODE_ANY | JSON_INDENT(2)));
    else
        KIARA_DEBUGF("  NULL");
    KIARA_DEBUGF("\nparams = <%p>\n", msg->params);
    if (msg->params)
        KIARA_IFDEBUG(json_dumpf(msg->params, stderr, JSON_ENCODE_ANY | JSON_INDENT(2)));
    else
        KIARA_DEBUGF("  NULL");
    KIARA_DEBUGF("\nerror = \n");
    if (msg->error)
        KIARA_IFDEBUG(json_dumpf(msg->error, stderr, JSON_ENCODE_ANY | JSON_INDENT(2)));
    else
        KIARA_DEBUGF("  NULL");
    KIARA_DEBUGF("\ncursor->value = \n");
    if (msg->cursor)
    {
        if (msg->cursor->value)
            KIARA_IFDEBUG(json_dumpf(msg->cursor->value, stderr, JSON_ENCODE_ANY | JSON_INDENT(2)));
        else
            KIARA_DEBUGF("  NULL");
        KIARA_DEBUGF("\ncursor->fieldName = \"%s\"\n", msg->cursor->fieldName);
    }
}

static KIARA_CursorNode * createCursorNode()
{
    KIARA_CursorNode *n = malloc(sizeof(KIARA_CursorNode));
    n->value = NULL;
    n->fieldName = NULL;
    n->prev = NULL;
    return n;
}

static void freeCursorNode(KIARA_CursorNode *node)
{
    free(node);
}

static KIARA_CursorNode * pushCursorNode(KIARA_Message *msg, json_t *value)
{
    KIARA_CursorNode *top = createCursorNode();
    top->prev = msg->cursor;
    top->value = value;
    msg->cursor = top;

    if (value)
    {
        switch (json_typeof(value))
        {
            case JSON_OBJECT:
                msg->cursor->data.objectIter = json_object_iter(value);
                break;
            case JSON_ARRAY:
                msg->cursor->data.index = 0;
                break;
            case JSON_STRING:
            case JSON_INTEGER:
            case JSON_REAL:
            case JSON_TRUE:
            case JSON_FALSE:
            case JSON_NULL:
                msg->cursor->data.index = 0;
                break;
        }
    }

    return top;
}

static KIARA_CursorNode * popCursorNode(KIARA_Message *msg)
{
    KIARA_CursorNode *node = msg->cursor;
    if (node)
    {
        msg->cursor = node->prev;
        freeCursorNode(node);
    }
    return msg->cursor;
}

static void freeCursorStack(KIARA_Message *msg)
{
    while (popCursorNode(msg)) ;
}

/*
static void resetMessage(KIARA_Message *msg)
{
    freeCursorStack(msg);
    json_array_clear(msg->params);
}
*/

static void initMessage(KIARA_Message *msg) KIARA_ALWAYS_INLINE;
static void initMessage(KIARA_Message *msg)
{
    if (msg)
    {
        msg->body = NULL;
        msg->params = NULL;
        msg->error = NULL;
        msg->cursor = NULL;
    }
}

static KIARA_Message * createNewMessage(void) KIARA_ALWAYS_INLINE;
static KIARA_Message * createNewMessage(void)
{
   KIARA_Message *msg = malloc(sizeof(KIARA_Message));
   initMessage(msg);
   return msg;
}

static void clearMessage(KIARA_Message *msg)
{
    if (msg)
    {
        freeCursorStack(msg);
        msg->cursor = NULL;
        json_decref(msg->body);
        msg->body = NULL;
        msg->params = NULL;
        msg->error = NULL;
    }
}

void freeMessage(KIARA_Message *msg)
{
    KIARA_PING();
    clearMessage(msg);
    free(msg);
}

const char * getMimeType(void)
{
    return "application/json";
}

KIARA_Message * createRequestMessageFromData(const void *data, size_t dataSize)
{
    json_t *version = NULL;
    KIARA_Message *msg = createNewMessage();

    json_error_t error;
    msg->body = json_loadb(data, dataSize, 0, &error);
    if (!msg->body)
    {
        /* FIXME where to put error message ? */
        printf("json error: %s\nsource: %s\nline: %i\ncolumn: %i\n", error.text, error.source, error.line, error.column);
        freeMessage(msg);
        return NULL;
    }

    version = json_object_get(msg->body, "jsonrpc");
    if (!json_is_string(version) || strcmp(json_string_value(version), "2.0") != 0)
    {
        freeMessage(msg);
        /* FIXME where to put error message ? */
        /* error: invalid or missing jsonrpc version */
        return NULL;
    }
    msg->params = json_object_get(msg->body, "params");
    if (msg->params)
    {
        pushCursorNode(msg, msg->params);
    }

    return msg;
}

const char * getMessageMethodName(KIARA_Message *msg)
{
    json_t *method = json_object_get(msg->body, "method");
    return json_string_value(method);
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
    kr_dbuffer_clear(dest);
    if (json_dump_callback(msg->body, dump_to_buffer, dest, JSON_ENCODE_ANY | JSON_INDENT(2)) == 0)
        return KIARA_SUCCESS;
    return KIARA_FAILURE;
}

KIARA_Message * createRequestMessage(KIARA_Connection *conn, const char *name, size_t name_length)
{
    KIARA_PING();

    KIARA_Message *msg = createNewMessage();

    json_t *body = msg->body = json_object();
    json_object_set_new(body, "jsonrpc", json_string("2.0"));

    /* jansson has no function to set name with specific length
     * we will make temporary copy for safety
     */
    char *tmp = (char*)malloc(name_length+1);
    strncpy(tmp, name, name_length+1);
    tmp[name_length] = '\0';
    json_object_set_new(body, "method", json_string(tmp));
    free(tmp);

    json_object_set_new(body, "id", json_integer(1)); /* FIXME how to generate ID ? */

    msg->params = json_array();
    pushCursorNode(msg, msg->params);
    json_object_set_new(body, "params", msg->params);

    KIARA_IFDEBUG(tmp = json_dumps(body, JSON_ENCODE_ANY | JSON_INDENT(2)));
    KIARA_DEBUGF("LEAVE createRequestMessage -> %s\n", tmp);
    KIARA_IFDEBUG(free(tmp));
    return msg;
}

KIARA_Message * createResponseMessage(KIARA_Connection *conn, KIARA_Message *requestMsg)
{
    KIARA_PING();

    KIARA_Message *msg = createNewMessage();

    json_t *body = msg->body = json_object();
    json_object_set_new(body, "jsonrpc", json_string("2.0"));
    if (requestMsg)
        json_object_set(body, "id", json_object_get(requestMsg->body, "id"));
    else
        json_object_set_new(body, "id", json_null());

    pushCursorNode(msg, msg->body);
    json_object_set_new(msg->body, "result", json_null());
    msg->cursor->fieldName = "result";

    return msg;
}

void setGenericErrorMessage(KIARA_Message *msg, int errorCode, const char *errorMessage)
{
    KIARA_PING();

    freeCursorStack(msg);
    msg->params = NULL;
    json_object_del(msg->body, "params");
    json_object_del(msg->body, "error");
    json_object_del(msg->body, "result");

    msg->error = json_object();
    json_object_set_new(msg->error, "code", json_integer(errorCode));
    json_object_set_new(msg->error, "message", json_string(errorMessage));
    json_object_set_new(msg->body, "error", msg->error);
}

static KIARA_Result initMessageFromString(KIARA_Message *msg, const char *str)
{
    clearMessage(msg);
    json_error_t error;
    msg->body = json_loads(str, 0, &error);

    KIARA_DEBUGF("response:\n");
    KIARA_IFDEBUG(dumpCursorInfo(msg));

    if (!msg->body)
    {
        /* FIXME where to put error message ? */
        return KIARA_INVALID_RESPONSE;
    }
    msg->params = json_object_get(msg->body, "result");
    if (msg->params)
    {
        pushCursorNode(msg, msg->params);
        return KIARA_SUCCESS;
    }

    msg->error = json_object_get(msg->body, "error");
    if (!json_is_object(msg->error))
    {
        /* Invalid JSONRPC 2.0 response ! */
        /* FIXME report error message string */
        return KIARA_INVALID_RESPONSE;
    }
    msg->params = json_object_get(msg->error, "data");

    if (msg->params)
        pushCursorNode(msg, msg->params);

    return KIARA_EXCEPTION;
}


KIARA_Result sendMessageSync(KIARA_Connection *conn, KIARA_Message *outMsg, KIARA_Message *inMsg)
{
    int result = 0;
    kr_dbuffer_t buf;

    if (!outMsg)
        return -1; /* FIXME use proper error code */

    char *msgData = json_dumps(outMsg->body, JSON_ENCODE_ANY | JSON_INDENT(2));
    /*KIARA_DEBUGF("sendMessageSync %s to %s\n", msgData, getConnectionURI(conn));*/
    KIARA_DEBUGF("sendMessageSync %s\n", msgData);

    kr_dbuffer_init(&buf);

    result = sendData(conn, msgData, strlen(msgData), &buf);

    free(msgData);

    /* TODO send message via network */
    if (result == KIARA_SUCCESS)
    {
        /* buf must be a null-terminated C string in order to work with initMessageFromString */
        kr_dbuffer_make_cstr(&buf);

        /* result = initMessageFromString(inMsg, "{\"jsonrpc\": \"2.0\", \"result\": 202, \"id\": 1}"); */
        result = initMessageFromString(inMsg, kr_dbuffer_data(&buf));
    }
    kr_dbuffer_destroy(&buf);

    return result;
}

static KIARA_Result writeValue(KIARA_Message *msg, json_t *value)
{
    int result;
    if (cursor_at_array(msg))
        result = json_array_append_new(msg->cursor->value, value);
    else if (cursor_at_object(msg))
        result = json_object_set_new(msg->cursor->value, msg->cursor->fieldName, value);
    else
    {
        json_decref(value);
        result = 0;
    }
    return result == 0 ? KIARA_SUCCESS : -1; /* FIXME use proper error code */
}

KIARA_Result writeStructBegin(KIARA_Message *msg, const char *name)
{
    KIARA_PING();
    int result;
    json_t *obj = json_object();
    result = writeValue(msg, obj);
    if (result != 0)
        return result;

    pushCursorNode(msg, obj);

    KIARA_DEBUGF("writeStructBegin\n");
    return KIARA_SUCCESS;
}

KIARA_Result writeStructEnd(KIARA_Message *msg)
{
    KIARA_PING();
    if (!cursor_at_object(msg))
        return -1; /*FIXME use proper error code */
    popCursorNode(msg);
    return KIARA_SUCCESS;
}

KIARA_Result writeFieldBegin(KIARA_Message *msg, const char *name)
{
    KIARA_PING();
    KIARA_DEBUGF("field name: %s\n", name);
    if (!cursor_at_object(msg))
        return -1; /*FIXME use proper error code */
    msg->cursor->fieldName = name;
    return KIARA_SUCCESS;
}

KIARA_Result writeFieldEnd(KIARA_Message *msg)
{
    KIARA_PING();
    if (!cursor_at_object(msg))
        return -1; /*FIXME use proper error code */
    msg->cursor->fieldName = NULL;
    return KIARA_SUCCESS;
}

KIARA_Result writeMessage_boolean(KIARA_Message *msg, int value)
{
    KIARA_PING();
    int result = writeValue(msg, json_boolean(value));
    KIARA_DEBUGF("writeMessage_boolean(%i)\n", value);
    return result;
}

KIARA_Result writeMessage_i8(KIARA_Message *msg, int8_t value)
{
    KIARA_PING();
    int result = writeValue(msg, json_integer(value));
    KIARA_DEBUGF("writeMessage_i8(%i)\n", (int)value);
    return result;
}

KIARA_Result writeMessage_u8(KIARA_Message *msg, uint8_t value)
{
    KIARA_PING();
    KIARA_Result result = writeValue(msg, json_integer(value));
    KIARA_DEBUGF("writeMessage_u8(%i)\n", (int)value);
    return result;
}

KIARA_Result writeMessage_i16(KIARA_Message *msg, int16_t value)
{
    KIARA_PING();
    int result = writeValue(msg, json_integer(value));
    KIARA_DEBUGF("writeMessage_i16(%i)\n", (int)value);
    return result;
}

KIARA_Result writeMessage_u16(KIARA_Message *msg, uint16_t value)
{
    KIARA_PING();
    KIARA_Result result = writeValue(msg, json_integer(value));
    KIARA_DEBUGF("writeMessage_u16(%i)\n", (int)value);
    return result;
}

KIARA_Result writeMessage_i32(KIARA_Message *msg, int32_t value)
{
    KIARA_PING();
    KIARA_Result result = writeValue(msg, json_integer(value));
    KIARA_DEBUGF("writeMessage_i32(%i)\n", (int)value);
    return result;
}

KIARA_Result writeMessage_u32(KIARA_Message *msg, uint32_t value)
{
    KIARA_PING();
    KIARA_Result result = writeValue(msg, json_integer(value));
    KIARA_DEBUGF("writeMessage_u32(%i)\n", (int)value);
    return result;
}

KIARA_Result writeMessage_i64(KIARA_Message *msg, int64_t value)
{
    KIARA_PING();
    KIARA_Result result = writeValue(msg, json_integer(value));
    KIARA_DEBUGF("writeMessage_i64(%i)\n", (int)value);
    return result;
}

KIARA_Result writeMessage_u64(KIARA_Message *msg, uint64_t value)
{
    KIARA_PING();
    KIARA_Result result = writeValue(msg, json_integer(value));
    KIARA_DEBUGF("writeMessage_u64(%i)\n", (int)value);
    return result;
}

KIARA_Result writeMessage_float(KIARA_Message *msg, float value)
{
    KIARA_PING();
    KIARA_Result result = writeValue(msg, json_real(value));
    KIARA_DEBUGF("writeMessage_float(%f)\n", value);
    return result;
}

KIARA_Result writeMessage_double(KIARA_Message *msg, double value)
{
    KIARA_PING();
    KIARA_Result result = writeValue(msg, json_real(value));
    KIARA_DEBUGF("writeMessage_double(%f)\n", value);
    return result;
}

KIARA_Result writeMessage_string(KIARA_Message *msg, const char * value)
{
    KIARA_PING();
    KIARA_Result result = writeValue(msg, json_string(value));
    return result;
}

/*
 * FIXME this logic will not work with serialization / deserialization of structs
 *       because order is not specified.
 *       Modify readMessage_i32 to have additional index parameter (char * index ?)
 */

static json_t * readNextMessageItem(KIARA_Message *msg) KIARA_ALWAYS_INLINE;
static json_t * readNextMessageItem(KIARA_Message *msg)
{
    KIARA_PING();
    json_t *item = NULL;
    json_t *value = msg->cursor->value;

    /*???DEBUG*/
    KIARA_DEBUGF("readNextMessageItem:\n");
    KIARA_IFDEBUG(dumpCursorInfo(msg));

    switch (json_typeof(value))
    {
        case JSON_OBJECT:
            if (msg->cursor->fieldName)
            {
                /* Read item in the object */
                msg->cursor->data.objectIter = json_object_iter_at(value, msg->cursor->fieldName);
                item = json_object_iter_value(msg->cursor->data.objectIter);
            }
            else
            {
                /* Start read object */
                msg->cursor->data.objectIter = json_object_iter(value);
                item = value;
            }
            break;
        case JSON_ARRAY:
            KIARA_DEBUGF("value is array\n");
            KIARA_DEBUGF("array: index is %i\n", (int)msg->cursor->data.index);
            if (msg->cursor->data.index >= json_array_size(value))
                return NULL; /* No items */
            item = json_array_get(value, msg->cursor->data.index);
            KIARA_DEBUGF("array: item at index %i is ", (int)msg->cursor->data.index);
            KIARA_IFDEBUG(dumpNode(item));
            KIARA_DEBUGF("\n");
            if (item)
                ++msg->cursor->data.index;
            KIARA_DEBUGF("array: new index is %i\n", (int)msg->cursor->data.index);
            break;
        case JSON_STRING:
        case JSON_INTEGER:
        case JSON_REAL:
        case JSON_TRUE:
        case JSON_FALSE:
        case JSON_NULL:
            if (msg->cursor->data.index != 0)
                return NULL; /* No items */
            item = value;
            ++msg->cursor->data.index;
            break;
    }
    return item;
}

static KIARA_Result readIntegerValue(KIARA_Message *msg, json_int_t *value) KIARA_ALWAYS_INLINE;
static KIARA_Result readIntegerValue(KIARA_Message *msg, json_int_t *value)
{
    KIARA_PING();
    if (!msg->cursor->value)
        return -1; /* FIXME return proper error code */

    json_t *item = readNextMessageItem(msg);
    if (!item)
        return -1; /* FIXME return proper error code */

    KIARA_DEBUGF("readIntegerValue: read item : ");
    KIARA_IFDEBUG(dumpNode(item));
    KIARA_DEBUGF("\n");

    assert(value && "value pointer is NULL");
    KIARA_DEBUGF("readIntegerValue: value pointer is %p\n", value);

    if (json_is_integer(item))
        *value = json_integer_value(item);
    else if (json_is_real(item))
        *value = json_real_value(item);
    else
        return -1; /* FIXME return proper error code */
    KIARA_DEBUGF("LEAVE readIntegerValue -> %i\n", (int)*value); /*???DEBUG*/
    return KIARA_SUCCESS;
}

static KIARA_Result readRealValue(KIARA_Message *msg, double *value) KIARA_ALWAYS_INLINE;
static KIARA_Result readRealValue(KIARA_Message *msg, double *value)
{
    KIARA_PING();
    if (!msg->cursor->value)
        return -1; /* FIXME return proper error code */

    json_t *item = readNextMessageItem(msg);
    if (!item)
        return -1; /* FIXME return proper error code */

    if (json_is_integer(item))
        *value = json_integer_value(item);
    else if (json_is_real(item))
        *value = json_real_value(item);
    else
        return -1; /* FIXME return proper error code */
    KIARA_DEBUGF("readRealValue\n");
    return KIARA_SUCCESS;
}

static KIARA_Result readBooleanValue(KIARA_Message *msg, int *value) KIARA_ALWAYS_INLINE;
static KIARA_Result readBooleanValue(KIARA_Message *msg, int *value)
{
    KIARA_PING();
    if (!msg->cursor->value)
        return -1; /* FIXME return proper error code */

    json_t *item = readNextMessageItem(msg);
    if (!item)
        return -1; /* FIXME return proper error code */

    if (json_is_true(item))
        *value = 1;
    else if (json_is_false(item))
        *value = 0;
    else
        return -1; /* FIXME return proper error code */
    KIARA_DEBUGF("readBooleanValue\n");
    return KIARA_SUCCESS;
}

KIARA_Result readStructBegin(KIARA_Message *msg)
{
    KIARA_PING();
    KIARA_DEBUGF("readStructBegin CALLED\n");
    json_t *item = readNextMessageItem(msg);
    if (!item)
        return -1; /* FIXME return proper error code */

    KIARA_DEBUGF("readStructBegin B\n");
    if (!json_is_object(item))
        return -1;

    pushCursorNode(msg, item);

    KIARA_DEBUGF("readStructBegin\n");
    return KIARA_SUCCESS;
}

KIARA_Result readStructEnd(KIARA_Message *msg)
{
    KIARA_PING();
    if (!cursor_at_object(msg))
        return -1; /* FIXME return proper error code */
    popCursorNode(msg);
    KIARA_DEBUGF("readStructEnd\n");
    return KIARA_SUCCESS;
}

KIARA_Result readFieldBegin(KIARA_Message *msg, const char *name)
{
    KIARA_PING();
    KIARA_DEBUGF("readFieldBegin CALLED\n");
    dumpCursorInfo(msg);

    if (!cursor_at_object(msg))
        return -1; /* FIXME return proper error code */
    KIARA_DEBUGF("readFieldBegin A\n");
    msg->cursor->fieldName = name;
    KIARA_DEBUGF("readFieldBegin\n");
    return KIARA_SUCCESS;
}

KIARA_Result readFieldEnd(KIARA_Message *msg)
{
    KIARA_PING();
    if (!cursor_at_object(msg))
        return -1; /* FIXME return proper error code */
    msg->cursor->fieldName = NULL;
    KIARA_DEBUGF("readFieldEnd\n");
    return KIARA_SUCCESS;
}

KIARA_Result writeArrayBegin(KIARA_Message *msg, size_t size)
{
    KIARA_PING();
    KIARA_DEBUGF("writeArrayBegin CALLED\n");

    int result;
    json_t *obj = json_array();
    result = writeValue(msg, obj);
    if (result != 0)
        return result;

    pushCursorNode(msg, obj);

    KIARA_DEBUGF("LEAVE writeArrayBegin\n");
    return KIARA_SUCCESS;
}

KIARA_Result writeArrayEnd(KIARA_Message *msg)
{
    KIARA_PING();
    if (!cursor_at_array(msg))
        return -1; /*FIXME use proper error code */
    popCursorNode(msg);
    KIARA_DEBUGF("LEAVE writeArrayEnd\n");
    return KIARA_SUCCESS;
}

KIARA_Result readArrayBegin(KIARA_Message *msg, size_t *size)
{
    KIARA_PING();
    KIARA_DEBUGF("readArrayBegin CALLED\n");
    json_t *item = readNextMessageItem(msg);
    if (!item)
        return -1; /* FIXME return proper error code */

    if (!json_is_array(item))
        return -1;

    if (size)
    {
        *size = json_array_size(item);
        KIARA_DEBUGF("readArrayBegin array size = %i\n", (int)*size);
    }

    pushCursorNode(msg, item);

    KIARA_DEBUGF("LEAVE readArrayBegin\n");
    return KIARA_SUCCESS;
}

KIARA_Result readArrayEnd(KIARA_Message *msg)
{
    KIARA_PING();
    if (!cursor_at_array(msg))
        return -1; /* FIXME return proper error code */
    /* FIXME should we check that array was completely read ? */
    popCursorNode(msg);
    KIARA_DEBUGF("LEAVE readArrayEnd\n");
    return KIARA_SUCCESS;
}

KIARA_Result readMessage_boolean(KIARA_Message *msg, int *value)
{
    KIARA_Result result;

    KIARA_PING();

    return readBooleanValue(msg, value);
}

KIARA_Result readMessage_i8(KIARA_Message *msg, int8_t *value)
{
    json_int_t tmp;
    KIARA_Result result;

    KIARA_PING();

    result = readIntegerValue(msg, &tmp);
    if (result == KIARA_SUCCESS)
        *value = tmp;
    return result;
}

KIARA_Result readMessage_u8(KIARA_Message *msg, uint8_t *value)
{
    json_int_t tmp;
    KIARA_Result result;

    KIARA_PING();

    result = readIntegerValue(msg, &tmp);
    if (result == KIARA_SUCCESS)
        *value = tmp;
    return result;
}

KIARA_Result readMessage_i16(KIARA_Message *msg, int16_t *value)
{
    json_int_t tmp;
    KIARA_Result result;

    KIARA_PING();

    result = readIntegerValue(msg, &tmp);
    if (result == KIARA_SUCCESS)
        *value = tmp;
    return result;
}

KIARA_Result readMessage_u16(KIARA_Message *msg, uint16_t *value)
{
    json_int_t tmp;
    KIARA_Result result;

    KIARA_PING();

    result = readIntegerValue(msg, &tmp);
    if (result == KIARA_SUCCESS)
        *value = tmp;
    return result;
}

KIARA_Result readMessage_i32(KIARA_Message *msg, int32_t *value)
{
    json_int_t tmp;
    KIARA_Result result;

    KIARA_PING();

    result = readIntegerValue(msg, &tmp);
    if (result == KIARA_SUCCESS)
        *value = tmp;
    return result;
}

KIARA_Result readMessage_u32(KIARA_Message *msg, uint32_t *value)
{
    json_int_t tmp;
    KIARA_Result result;

    KIARA_PING();

    result = readIntegerValue(msg, &tmp);
    if (result == KIARA_SUCCESS)
        *value = tmp;
    return result;
}

KIARA_Result readMessage_i64(KIARA_Message *msg, int64_t *value)
{
    json_int_t tmp;
    KIARA_Result result;

    KIARA_PING();

    result = readIntegerValue(msg, &tmp);
    if (result == KIARA_SUCCESS)
        *value = tmp;
    return result;
}

KIARA_Result readMessage_u64(KIARA_Message *msg, uint64_t *value)
{
    json_int_t tmp;
    KIARA_Result result;

    KIARA_PING();

    result = readIntegerValue(msg, &tmp);
    if (result == KIARA_SUCCESS)
        *value = tmp;
    return result;
}

KIARA_Result readMessage_float(KIARA_Message *msg, float *value)
{
    double tmp;
    KIARA_Result result;

    KIARA_PING();

    result = readRealValue(msg, &tmp);
    if (result == KIARA_SUCCESS)
        *value = tmp;

    return result;
}

KIARA_Result readMessage_double(KIARA_Message *msg, double *value)
{
    KIARA_PING();

    return readRealValue(msg, value);
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
    int result = getCStringFunc(value, &cstr);
    if (result != 0)
        return result;
    result = writeValue(msg, json_string(cstr));
    return result;
}

KIARA_Result readMessage_user_string(KIARA_Message *msg, KIARA_UserType *value, KIARA_SetCString setStringFunc)
{
    KIARA_PING();

    if (!msg->cursor->value)
        return -1; /* FIXME return proper error code */

    json_t *item = readNextMessageItem(msg);
    if (!item)
        return -1; /* FIXME return proper error code */

    if (json_is_string(item))
    {
        setStringFunc(value, json_string_value(item));
    }
    else
        return -1; /* FIXME return proper error code */
    KIARA_DEBUGF("LEAVE readMessage_user_string -> \"%s\"\n", json_string_value(item));
    return KIARA_SUCCESS;
}

KIARA_Result readMessage_string(KIARA_Message *msg, char **value)
{
    KIARA_PING();
    return readMessage_user_string(msg, (KIARA_UserType*)value, defaultSetString);
}

KIARA_Bool isErrorResponse(KIARA_Message *msg)
{
    KIARA_PING();
    return KIARA_TO_BOOL(msg->error);
}

KIARA_Result readGenericError(KIARA_Message *msg, KIARA_UserType *userException, KIARA_SetGenericError setGenericErrorFunc)
{
    KIARA_PING();
    KIARA_DEBUGF("readGenericError\n");
    KIARA_IFDEBUG(dumpCursorInfo(msg));

    json_t *code, *message;
    if (!msg->error)
        return KIARA_RESPONSE_ERROR;
    code = json_object_get(msg->error, "code");
    if (!json_is_integer(code))
        return KIARA_INVALID_RESPONSE;
    json_int_t errorCode = json_integer_value(code);

    // message is optional
    message = json_object_get(msg->error, "message");
    const char *errorMessage = "";
    if (message)
    {
        if (!json_is_string(message))
            return KIARA_INVALID_RESPONSE;
        errorMessage = json_string_value(message);
    }

    KIARA_DEBUGF("CALL setGenericErrorFunc %i %s\n", (int)errorCode, errorMessage);

    return setGenericErrorFunc(userException, errorCode, errorMessage);
}

KIARA_Result writeGenericError(KIARA_Message *msg, KIARA_UserType *userException, KIARA_GetGenericError getGenericErrorFunc)
{
    KIARA_PING();
    KIARA_IFDEBUG(dumpCursorInfo(msg));
    KIARA_DEBUGF("START\n");
    int errorCode = 0;
    const char *errorMessage = 0;
    KIARA_Result result;

    result = getGenericErrorFunc(userException, &errorCode, &errorMessage);
    if (result != KIARA_SUCCESS)
        return result;

    setGenericErrorMessage(msg, errorCode, errorMessage);

    return KIARA_SUCCESS;
}

KIARA_Result writeMessage_binary_stream(KIARA_Message *msg, KIARA_BinaryStream *stream)
{
    KIARA_PING();

    KIARA_Result result;

    kr_dbuffer_t buf;
    kr_dbuffer_init(&buf);
    kr_base64_encode(getStreamData(stream), getStreamSize(stream), &buf, 0);
    kr_dbuffer_make_cstr(&buf);

    result = writeValue(msg, json_string(kr_dbuffer_data(&buf)));

    kr_dbuffer_destroy(&buf);

    return KIARA_SUCCESS;
}

KIARA_Result readMessage_binary_stream(KIARA_Message *msg, KIARA_BinaryStream *stream)
{
    KIARA_PING();

    KIARA_Result result;

    if (!msg->cursor->value)
        return -1; /* FIXME return proper error code */

    json_t *item = readNextMessageItem(msg);
    if (!item)
        return -1; /* FIXME return proper error code */

    if (json_is_string(item))
    {
        kr_dbuffer_t buf;
        int decodeResult;

        kr_dbuffer_init(&buf);
        decodeResult = kr_base64_decode(json_string_value(item), strlen(json_string_value(item)),  &buf);
        if (decodeResult)
            setStreamBuffer(stream, &buf);
        kr_dbuffer_destroy(&buf);
        if (!decodeResult)
            return -1;

        KIARA_DEBUGF("LEAVE readMessage_binary_stream -> \"%s\"\n", json_string_value(item));
    }
    else
        return -1; /* FIXME return proper error code */

    return KIARA_SUCCESS;
}
