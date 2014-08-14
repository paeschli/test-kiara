#include "fastcdr_protocol.h"

#include "fastcdr/FastCdr.h"
#include "fastcdr/FastBuffer.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <malloc.h>

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


static int dump_to_buffer(const char *buffer, size_t size, void *data)
{
    kr_dbuffer_t *dbuf = (kr_dbuffer_t*)data;
    if (kr_dbuffer_append_mem(dbuf, buffer, size))
        return 0;
    return -1;
}

struct KIARA_Message
{
	char* buffer; /* char array where serialized data will be allocated */
	FastBuffer *fastbuffer; /* buffer where data will be serialized */
	FastCdr *cdr; /* CDR object used to serialize */
	char* protocol;
	char* method;
	char* methodname;
	char* data; /* May be params if request, result if response or errormessage if error */
	int32_t id;
	int32_t errorcode;
};

static void initMessage(KIARA_Message *msg)
{
	//printf("initMessage - %p\n", msg);
    if (msg)
    {
		msg->buffer = NULL;
		msg->errorcode = 0;
		msg->id = 0;
		msg->protocol = NULL;
		msg->method = NULL;
		msg->methodname = NULL;
		msg->data = NULL;
    }
}

static KIARA_Message * createNewMessage(void)
{
   KIARA_Message *msg = (KIARA_Message*) malloc(sizeof(KIARA_Message));
	//printf("createNewMessage - %p\n", msg);
   initMessage(msg);
   return msg;
}

static void clearMessage(KIARA_Message *msg)
{
	//printf("clearMessage - %p\n", msg);
    if (msg)
    {
		if(msg->cdr){
			freeFastCdr(msg->cdr);
			msg->cdr = NULL;
		}
		if(msg->fastbuffer){
			freeFastBuffer(msg->fastbuffer);
			msg->fastbuffer = NULL;
		}
		if(msg->buffer) {
			free(msg->buffer);
		}
		if(msg->data) {
			free(msg->data);
		}
		if(msg->protocol) {
			free(msg->protocol);
		}
		if(msg->method) {
			free(msg->method);
		}
		if(msg->methodname) {
			free(msg->methodname);
		}
    }

}

void freeMessage(KIARA_Message *msg)
{
    //KIARA_PING();
	if (msg)
	{
		clearMessage(msg);
		//printf("freeMessage - %p\n", msg);
		free(msg);
	}
}

const char * getMimeType(void)
{
	//printf("getMimeType\n");
    return "application/octet-stream";
}

KIARA_Message * createRequestMessageFromData(const void *data, size_t dataSize)
{
	//printf("createRequestMessageFromData\n");
	KIARA_Message *msg = createNewMessage();

	msg->fastbuffer = newFastBufferWithBuffer((char*)data, dataSize);
	msg->cdr = newFastCdr(msg->fastbuffer);

	if(deserialize_string_t(msg->cdr, &msg->protocol)!=FASTCDR_SUCCESS) {
	        freeMessage(msg);
		return NULL;
	}
	if(strlen(msg->protocol) == 0){
	        freeMessage(msg);
		return NULL;
	}

	if(deserialize_string_t(msg->cdr, &msg->method)!=FASTCDR_SUCCESS) {
	        freeMessage(msg);
		return NULL;
	}
	if(strlen(msg->method) == 0){
	        freeMessage(msg);
		return NULL;
	}else{
		if(strcmp(msg->method, "request") == 0){
			if(deserialize_string_t(msg->cdr, &msg->methodname)!=FASTCDR_SUCCESS) {
			        freeMessage(msg);
				return NULL;
			}
		}else{
		        freeMessage(msg);
			return NULL;
		}
	}

	if(deserialize_int32_t(msg->cdr, &msg->id)!=FASTCDR_SUCCESS) {
	        freeMessage(msg);
		return NULL;
	}

	return msg;
}

const char * getMessageMethodName(KIARA_Message *msg)
{
	//printf("getMessageMethodName\n");
	return msg->methodname;
}

KIARA_Result getMessageData(KIARA_Message *msg, kr_dbuffer_t *dest)
{
	//printf("getMessageData\n");
	kr_dbuffer_clear(dest);
	if (dump_to_buffer(msg->fastbuffer->m_buffer, getSerializedDataLength(msg->cdr), dest) == 0){
		return KIARA_SUCCESS;
	}
	return KIARA_FAILURE;
}

KIARA_Message * createRequestMessage(KIARA_Connection *conn, const char *name, size_t name_length)
{
	//printf("createRequestMessage\n");
	//KIARA_PING();
	KIARA_Message *msg = createNewMessage();

	msg->fastbuffer = newFastBuffer();
	msg->cdr = newFastCdr(msg->fastbuffer);

	msg->protocol = (char*) calloc(4, sizeof(char));
	strcpy(msg->protocol, "cdr");
	if(serialize_string_t(msg->cdr, msg->protocol)!=FASTCDR_SUCCESS) {
		freeMessage(msg);
		return NULL;
	}

	msg->method = (char*) calloc(8, sizeof(char));
	strcpy(msg->method, "request");
	if(serialize_string_t(msg->cdr, msg->method)!=FASTCDR_SUCCESS) {
		freeMessage(msg);
		return NULL;
	}

	msg->methodname = (char*) calloc(name_length+1, sizeof(char));
	strcpy(msg->methodname, name);
	if(serialize_string_t(msg->cdr, msg->methodname)!=FASTCDR_SUCCESS) {
		freeMessage(msg);
		return NULL;
	}

	msg->id = 1; /* Generate new id */
	if(serialize_int32_t(msg->cdr, msg->id)!=FASTCDR_SUCCESS) {
		freeMessage(msg);
		return NULL;
	}

	// Parameters for the request must be introduced in auto-generated

    return msg;
}

KIARA_Message * createResponseMessage(KIARA_Connection *conn, KIARA_Message *requestMsg)
{
	//printf("createResponseMessage\n");
    //KIARA_PING();

    KIARA_Message *msg = createNewMessage();

    msg->fastbuffer = newFastBuffer(/*msg->buffer, BUFFER_SIZE*/);
	msg->cdr = newFastCdr(msg->fastbuffer);

	msg->protocol = (char*) calloc(4, sizeof(char));
	strcpy(msg->protocol, "cdr");
	if(serialize_string_t(msg->cdr, msg->protocol)!=FASTCDR_SUCCESS)
		return NULL;

	msg->method = (char*) calloc(9, sizeof(char));
	strcpy(msg->method, "response");
	if(serialize_string_t(msg->cdr, msg->method)!=FASTCDR_SUCCESS)
		return NULL;

	int32_t id;
	if (requestMsg){
		id = requestMsg->id;
	}else{
		id = -1;
	}
	if(serialize_int32_t(msg->cdr, id)!=FASTCDR_SUCCESS)
		return NULL;

	// Result must be set by auto-generated code

    return msg;
}

void setGenericErrorMessage(KIARA_Message *msg, int errorCode, const char *errorMessage)
{
	//printf("setGenericErrorMessage\n");
    //KIARA_PING();

	clearMessage(msg);
	initMessage(msg);

    msg->fastbuffer = newFastBuffer(/*msg->buffer, BUFFER_SIZE*/);
	msg->cdr = newFastCdr(msg->fastbuffer);

	msg->protocol = (char*) calloc(4, sizeof(char));
	strcpy(msg->protocol, "cdr");
	if(serialize_string_t(msg->cdr, msg->protocol)!=FASTCDR_SUCCESS)
		return;

	msg->method = (char*) calloc(6, sizeof(char));
	strcpy(msg->method, "error");
	if(serialize_string_t(msg->cdr, msg->method)!=FASTCDR_SUCCESS)
		return;

	msg->errorcode = errorCode;
	if(serialize_int32_t(msg->cdr, msg->errorcode)!=FASTCDR_SUCCESS)
		return;

	msg->data = (char*) calloc(strlen(errorMessage)+1, sizeof(char));
	strcpy(msg->data, errorMessage);
	if(serialize_string_t(msg->cdr, msg->data)!=FASTCDR_SUCCESS)
		return;
}

static KIARA_Result initMessageFromString(KIARA_Message *msg, const char *str, size_t strSize)
{
	//printf("initMessageFromString, strSize=%u\n", strSize);

	clearMessage(msg);
	initMessage(msg);
	msg->buffer = (char*) calloc(strSize, sizeof(char));
	memcpy(msg->buffer, str, strSize);
	msg->fastbuffer = newFastBufferWithBuffer(msg->buffer, strSize);
	msg->cdr = newFastCdr(msg->fastbuffer);

	if(deserialize_string_t(msg->cdr, &msg->protocol)!=FASTCDR_SUCCESS)
		return KIARA_EXCEPTION;
	if(strlen(msg->protocol) == 0){
		return KIARA_INVALID_RESPONSE;
	}
	if( strcmp(msg->protocol, "cdr") != 0){
		return KIARA_INVALID_RESPONSE;
	}

	if(deserialize_string_t(msg->cdr, &msg->method)!=FASTCDR_SUCCESS)
		return KIARA_EXCEPTION;
	if(strlen(msg->method) == 0){
		return KIARA_INVALID_RESPONSE;
	}else{
		if(strcmp(msg->method, "request") == 0){
			return KIARA_INVALID_RESPONSE;
		}else{
			if(strcmp(msg->method, "response") == 0){
				// Id from method
				if(deserialize_int32_t(msg->cdr, &msg->id)!=FASTCDR_SUCCESS)
					return KIARA_EXCEPTION;

				// Response will have to be serialized from outside
				return KIARA_SUCCESS;
			}else{
				// Error code
				if(deserialize_int32_t(msg->cdr, &msg->errorcode)!=FASTCDR_SUCCESS)
					return KIARA_EXCEPTION;

				// Error message
				if(deserialize_string_t(msg->cdr, &msg->data)!=FASTCDR_SUCCESS)
					return KIARA_INVALID_RESPONSE;

				return KIARA_EXCEPTION;
			}
		}
	}

	return KIARA_EXCEPTION;
}

KIARA_Result sendMessageSync(KIARA_Connection *conn, KIARA_Message *outMsg, KIARA_Message *inMsg)
{
	//printf("sendMessageSync\n");
	kr_dbuffer_t buf;

	if (!outMsg){
		return KIARA_FAILURE;
	}

	//char *msgData = outMsg->buffer;
	char *msgData = outMsg->fastbuffer->m_buffer;
	size_t dataSize = getSerializedDataLength(outMsg->cdr);

	kr_dbuffer_init(&buf);

	int result = sendData(conn, msgData, dataSize, &buf);

	//free(msgData);

	if (result == KIARA_SUCCESS)
	{
		kr_dbuffer_make_cstr(&buf);
		result = initMessageFromString(inMsg, buf.data, buf.size);
	}
	kr_dbuffer_destroy(&buf);

	if(result == KIARA_SUCCESS){
		return KIARA_SUCCESS;
	}

	return result;
}

KIARA_Result writeStructBegin(KIARA_Message *msg, const char *name)
{
	// Not implemented for CDR serialization
	return KIARA_SUCCESS;
}

KIARA_Result writeStructEnd(KIARA_Message *msg)
{
	// Not implemented for CDR serialization
	return KIARA_SUCCESS;
}

KIARA_Result writeFieldBegin(KIARA_Message *msg, const char *name)
{
    // Not implemented for CDR serialization
    return KIARA_SUCCESS;
}

KIARA_Result writeFieldEnd(KIARA_Message *msg)
{
    // Not implemented for CDR serialization
    return KIARA_SUCCESS;
}

KIARA_Result writeMessage_boolean(KIARA_Message *msg, int value)
{
    //KIARA_PING();
	//printf("writeMessage_boolean: %d\n", value);
	FastCdr_Result result = serialize_int32_t(msg->cdr, value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}


KIARA_Result writeMessage_i8(KIARA_Message *msg, int8_t value)
{
	FastCdr_Result result = serialize_char_t(msg->cdr, value);
	//printf("writeMessage_i8: %d\n", value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result writeMessage_u8(KIARA_Message *msg, uint8_t value)
{
	FastCdr_Result result = serialize_uint8_t(msg->cdr, value);
	//printf("writeMessage_u8: %d\n", value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result writeMessage_i16(KIARA_Message *msg, int16_t value)
{
	FastCdr_Result result = serialize_int16_t(msg->cdr, value);
	//printf("readMessage_i16: %d\n", value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result writeMessage_u16(KIARA_Message *msg, uint16_t value)
{
	FastCdr_Result result = serialize_uint16_t(msg->cdr, value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result writeMessage_i32(KIARA_Message *msg, int32_t value)
{
	FastCdr_Result result = serialize_int32_t(msg->cdr, value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result writeMessage_u32(KIARA_Message *msg, uint32_t value)
{
	FastCdr_Result result = serialize_uint32_t(msg->cdr, value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result writeMessage_i64(KIARA_Message *msg, int64_t value)
{
	FastCdr_Result result = serialize_int64_t(msg->cdr, value);
	//printf("writeMessage_i64: '%ld'\n", value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result writeMessage_u64(KIARA_Message *msg, uint64_t value)
{
	//printf("writeMessage_u64\n");
    //KIARA_PING();
	FastCdr_Result result = serialize_uint64_t(msg->cdr, value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result writeMessage_float(KIARA_Message *msg, float value)
{
	KIARA_PING();
	FastCdr_Result result = serialize_float_t(msg->cdr, value);
	//printf("writeMessage_float: %f\n", value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result writeMessage_double(KIARA_Message *msg, double value)
{
    //KIARA_PING();
	FastCdr_Result result = serialize_double_t(msg->cdr, value);
	//printf("writeMessage_double: %lf\n", value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result writeMessage_string(KIARA_Message *msg, const char * value)
{
	//printf("writeMessage_string\n");
	//printf("\twrite... '%s'\n", value);
    //KIARA_PING();
	FastCdr_Result result = serialize_string_t(msg->cdr, value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result readStructBegin(KIARA_Message *msg)
{
    // Not implemented for CDR serialization
    return KIARA_SUCCESS;
}

KIARA_Result readStructEnd(KIARA_Message *msg)
{
    // Not implemented for CDR serialization
    return KIARA_SUCCESS;
}

KIARA_Result readFieldBegin(KIARA_Message *msg, const char *name)
{
    // Not implemented for CDR serialization
    return KIARA_SUCCESS;
}

KIARA_Result readFieldEnd(KIARA_Message *msg)
{
    // Not implemented for CDR serialization
    return KIARA_SUCCESS;
}

KIARA_Result writeArrayBegin(KIARA_Message *msg, size_t size)
{
	int32_t size32 = (int32_t)size;
	FastCdr_Result result = serialize_int32_t(msg->cdr, size32);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result writeArrayEnd(KIARA_Message *msg)
{
	return KIARA_SUCCESS;
}

KIARA_Result readArrayBegin(KIARA_Message *msg, size_t *size)
{
	int32_t size32 = 0;
	FastCdr_Result result = deserialize_int32_t(msg->cdr, &size32);
	*size = (size_t)size32;
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result readArrayEnd(KIARA_Message *msg)
{
	return KIARA_SUCCESS;
}

KIARA_Result readMessage_boolean(KIARA_Message *msg, int *value)
{
    //KIARA_PING();
	FastCdr_Result result = deserialize_int32_t(msg->cdr, value);
	//printf("readMessage_boolean: %d\n", *value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}


KIARA_Result readMessage_i8(KIARA_Message *msg, int8_t *value)
{
	FastCdr_Result result = deserialize_char_t(msg->cdr, (char*)value);
	//printf("readMessage_i8: %d\n", *value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result readMessage_u8(KIARA_Message *msg, uint8_t *value)
{
	FastCdr_Result result = deserialize_uint8_t(msg->cdr, value);
	//printf("readMessage_u8: %d\n", *value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result readMessage_i16(KIARA_Message *msg, int16_t *value)
{
	FastCdr_Result result = deserialize_int16_t(msg->cdr, value);
	//printf("readMessage_i16: %d\n", *value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result readMessage_u16(KIARA_Message *msg, uint16_t *value)
{
	FastCdr_Result result = deserialize_uint16_t(msg->cdr, value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result readMessage_i32(KIARA_Message *msg, int32_t *value)
{
	FastCdr_Result result = deserialize_int32_t(msg->cdr, value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result readMessage_u32(KIARA_Message *msg, uint32_t *value)
{
	FastCdr_Result result = deserialize_uint32_t(msg->cdr, value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result readMessage_i64(KIARA_Message *msg, int64_t *value)
{
	FastCdr_Result result = deserialize_int64_t(msg->cdr, value);
	//printf("readMessage_i64: '%ld'\n", *value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result readMessage_u64(KIARA_Message *msg, uint64_t *value)
{
	FastCdr_Result result = deserialize_uint64_t(msg->cdr, value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result readMessage_float(KIARA_Message *msg, float *value)
{
	FastCdr_Result result = deserialize_float_t(msg->cdr, value);
//	printf("readMessage_float %f\n", *value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result readMessage_double(KIARA_Message *msg, double *value)
{
	FastCdr_Result result = deserialize_double_t(msg->cdr, value);
	//printf("readMessage_double: %lf\n", *value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result readMessage_string(KIARA_Message *msg, char **value)
{
    //KIARA_PING();
	FastCdr_Result result = deserialize_string_t(msg->cdr, value);
	//printf("readMessage_string: %s\n", *value);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}


KIARA_Result writeMessage_user_string(KIARA_Message *msg, KIARA_UserType *value, KIARA_GetCString getCStringFunc)
{
	//printf("writeMessage_user_string\n");
    //KIARA_PING();
    const char *cstr;
    int result = getCStringFunc(value, &cstr);

    if (result != 0){
		return KIARA_FAILURE;
	}
	return writeMessage_string(msg, cstr);
}

KIARA_Result readMessage_user_string(KIARA_Message *msg, KIARA_UserType *value, KIARA_SetCString setStringFunc)
{
    //KIARA_PING();
	char* string = NULL;
	FastCdr_Result result = deserialize_string_t(msg->cdr, &string);
	//printf("readMessage_user_string: %s\n", string);

	int res = setStringFunc(value, string);
	if(res != 0){
		free(string);
		return KIARA_FAILURE;
	}
	free(string);
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Bool isErrorResponse(KIARA_Message *msg)
{
	//printf("isErrorResponse\n");
    //KIARA_PING();

	if(strcmp(msg->method, "error") == 0)
		return KIARA_TRUE;

    return KIARA_FALSE;
}

KIARA_Result readGenericError(KIARA_Message *msg, KIARA_UserType *userException, KIARA_SetGenericError setGenericErrorFunc)
{
	//printf("readGenericError\n");
    //KIARA_PING();

	return setGenericErrorFunc(userException, msg->errorcode, msg->data);
}

KIARA_Result writeGenericError(KIARA_Message *msg, KIARA_UserType *userException, KIARA_GetGenericError getGenericErrorFunc)
{
	//printf("writeGenericError\n");
    //KIARA_PING();
    int errorCode = 0;
    const char *errorMessage = 0;

    int result = getGenericErrorFunc(userException, &errorCode, &errorMessage);
    if (result != KIARA_SUCCESS){
        return KIARA_FAILURE;
	}

    setGenericErrorMessage(msg, errorCode, errorMessage);


    return KIARA_SUCCESS;
}


KIARA_Result writeMessage_binary_stream(KIARA_Message *msg, KIARA_BinaryStream *stream)
{
	FastCdr_Result result = serializeSequence_char_t(msg->cdr, (char*)getStreamData(stream), getStreamSize(stream));
	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}

KIARA_Result readMessage_binary_stream(KIARA_Message *msg, KIARA_BinaryStream *stream)
{
	kr_dbuffer_t buf;
	kr_dbuffer_init(&buf);
	size_t *size = &buf.size;
	FastCdr_Result result = deserializeSequence_char_t(msg->cdr, &kr_dbuffer_data(&buf), size);

	setStreamBuffer(stream, &buf);
	kr_dbuffer_destroy(&buf);

	return (result == FASTCDR_SUCCESS) ? KIARA_SUCCESS : KIARA_FAILURE;
}
