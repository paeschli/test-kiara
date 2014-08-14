/*
 * api.h
 *
 *  Created on: Feb 12, 2014
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMPONENTS_API_H_INCLUDED
#define KIARA_COMPONENTS_API_H_INCLUDED

#include <KIARA/kiara.h>
#include <stdlib.h>
#include <stdint.h>
#include <KIARA/CDT/kr_dbuffer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct KIARA_ConnectionData KIARA_ConnectionData;

/* Network */
extern KIARA_Connection * getConnection(KIARA_FuncObj *funcObj);
extern const char * getConnectionURI(KIARA_Connection *conn);
extern KIARA_ConnectionData * getConnectionData(KIARA_Connection *conn);
extern void setErrorMessage(KIARA_Connection *conn, const char *msg);
extern void setConnectionData(KIARA_Connection *conn, KIARA_ConnectionData *data);
/* Synchronously send data of size dataSize via opened connection. Received response will be stored in the destBuf. */
extern int sendData(KIARA_Connection *conn, const void *data, size_t dataSize, kr_dbuffer_t * destBuf);

/* Returns MIME Type of the protocol */
const char * getMimeType(void) KIARA_ALWAYS_INLINE;

KIARA_Message * createRequestMessageFromData(const void *data, size_t dataSize) KIARA_ALWAYS_INLINE;
const char * getMessageMethodName(KIARA_Message *msg) KIARA_ALWAYS_INLINE;

/* Stores in buffer dest message representation that can be sent over network */
KIARA_Result getMessageData(KIARA_Message *msg, kr_dbuffer_t *dest) KIARA_ALWAYS_INLINE;

/* Create message for calling service method name */
KIARA_Message * createRequestMessage(KIARA_Connection *conn, const char *name, size_t name_length) KIARA_ALWAYS_INLINE;

/** Create response from request message, requestMsg can be NULL */
KIARA_Message * createResponseMessage(KIARA_Connection *conn, KIARA_Message *requestMsg) KIARA_ALWAYS_INLINE;

void setGenericErrorMessage(KIARA_Message *message, int errorCode, const char *errorMessage) KIARA_ALWAYS_INLINE;

/* Deallocate message */
void freeMessage(KIARA_Message *msg) KIARA_ALWAYS_INLINE;

/* Perform synchronous call with outMsg and store response to inMsg
 * inMsg and outMsg can be equal
 */
KIARA_Result sendMessageSync(KIARA_Connection *conn, KIARA_Message *outMsg, KIARA_Message *inMsg) KIARA_ALWAYS_INLINE;


KIARA_Result writeStructBegin(KIARA_Message *msg, const char *name) KIARA_ALWAYS_INLINE;
KIARA_Result writeStructEnd(KIARA_Message *msg) KIARA_ALWAYS_INLINE;
KIARA_Result writeFieldBegin(KIARA_Message *msg, const char *name) KIARA_ALWAYS_INLINE;
KIARA_Result writeFieldEnd(KIARA_Message *msg) KIARA_ALWAYS_INLINE;

KIARA_Result readStructBegin(KIARA_Message *msg) KIARA_ALWAYS_INLINE;
KIARA_Result readStructEnd(KIARA_Message *msg) KIARA_ALWAYS_INLINE;
KIARA_Result readFieldBegin(KIARA_Message *msg, const char *name) KIARA_ALWAYS_INLINE;
KIARA_Result readFieldEnd(KIARA_Message *msg) KIARA_ALWAYS_INLINE;

KIARA_Result writeArrayBegin(KIARA_Message *msg, size_t size) KIARA_ALWAYS_INLINE;
KIARA_Result writeArrayEnd(KIARA_Message *msg) KIARA_ALWAYS_INLINE;

KIARA_Result readArrayBegin(KIARA_Message *msg, size_t *size) KIARA_ALWAYS_INLINE;
KIARA_Result readArrayEnd(KIARA_Message *msg) KIARA_ALWAYS_INLINE;

/* Write boolean type to message */
KIARA_Result writeMessage_boolean(KIARA_Message *msg, int value) KIARA_ALWAYS_INLINE;
/* Read boolean type from message */
KIARA_Result readMessage_boolean(KIARA_Message *msg, int *value) KIARA_ALWAYS_INLINE;

/* Write i8 type to message */
KIARA_Result writeMessage_i8(KIARA_Message *msg, int8_t value) KIARA_ALWAYS_INLINE;
/* Read i8 type from message */
KIARA_Result readMessage_i8(KIARA_Message *msg, int8_t *value) KIARA_ALWAYS_INLINE;

/* Write u8 type to message */
KIARA_Result writeMessage_u8(KIARA_Message *msg, uint8_t value) KIARA_ALWAYS_INLINE;
/* Read u8 type from message */
KIARA_Result readMessage_u8(KIARA_Message *msg, uint8_t *value) KIARA_ALWAYS_INLINE;

/* Write i16 type to message */
KIARA_Result writeMessage_i16(KIARA_Message *msg, int16_t value) KIARA_ALWAYS_INLINE;
/* Read i16 type from message */
KIARA_Result readMessage_i16(KIARA_Message *msg, int16_t *value) KIARA_ALWAYS_INLINE;

/* Write u16 type to message */
KIARA_Result writeMessage_u16(KIARA_Message *msg, uint16_t value) KIARA_ALWAYS_INLINE;
/* Read u16 type from message */
KIARA_Result readMessage_u16(KIARA_Message *msg, uint16_t *value) KIARA_ALWAYS_INLINE;

/* Write i32 type to message */
KIARA_Result writeMessage_i32(KIARA_Message *msg, int32_t value) KIARA_ALWAYS_INLINE;
/* Read i32 type from message */
KIARA_Result readMessage_i32(KIARA_Message *msg, int32_t *value) KIARA_ALWAYS_INLINE;

/* Write u32 type to message */
KIARA_Result writeMessage_u32(KIARA_Message *msg, uint32_t value) KIARA_ALWAYS_INLINE;
/* Read u32 type from message */
KIARA_Result readMessage_u32(KIARA_Message *msg, uint32_t *value) KIARA_ALWAYS_INLINE;

/* Write i64 type to message */
KIARA_Result writeMessage_i64(KIARA_Message *msg, int64_t value) KIARA_ALWAYS_INLINE;
/* Read i64 type from message */
KIARA_Result readMessage_i64(KIARA_Message *msg, int64_t *value) KIARA_ALWAYS_INLINE;

/* Write i64 type to message */
KIARA_Result writeMessage_u64(KIARA_Message *msg, uint64_t value) KIARA_ALWAYS_INLINE;
/* Read i64 type from message */
KIARA_Result readMessage_u64(KIARA_Message *msg, uint64_t *value) KIARA_ALWAYS_INLINE;

/* Write float type to message */
KIARA_Result writeMessage_float(KIARA_Message *msg, float value) KIARA_ALWAYS_INLINE;
/* Read float type from message */
KIARA_Result readMessage_float(KIARA_Message *msg, float *value) KIARA_ALWAYS_INLINE;

/* Write float type to message */
KIARA_Result writeMessage_double(KIARA_Message *msg, double value) KIARA_ALWAYS_INLINE;
/* Read float type from message */
KIARA_Result readMessage_double(KIARA_Message *msg, double *value) KIARA_ALWAYS_INLINE;

/* Write zero terminated string to message */
KIARA_Result writeMessage_string(KIARA_Message *msg, const char * value) KIARA_ALWAYS_INLINE;

/* Read zero terminated string from message */
KIARA_Result readMessage_string(KIARA_Message *msg, char ** value) KIARA_ALWAYS_INLINE;

/* Write zero terminated string to message with user-defined getter method */
KIARA_Result writeMessage_user_string(KIARA_Message *msg, KIARA_UserType *value, KIARA_GetCString getCStringFunc) KIARA_ALWAYS_INLINE;

/* Read zero terminated string from message with user-defined setter method */
KIARA_Result readMessage_user_string(KIARA_Message *msg, KIARA_UserType *value, KIARA_SetCString setCStringFunc) KIARA_ALWAYS_INLINE;

KIARA_Bool isErrorResponse(KIARA_Message *msg) KIARA_ALWAYS_INLINE;

KIARA_Result readGenericError(KIARA_Message *msg, KIARA_UserType *userException, KIARA_SetGenericError setGenericErrorFunc) KIARA_ALWAYS_INLINE;
KIARA_Result writeGenericError(KIARA_Message *msg, KIARA_UserType *userException, KIARA_GetGenericError getGenericErrorFunc) KIARA_ALWAYS_INLINE;

KIARA_Result writeMessage_binary_stream(KIARA_Message *msg, KIARA_BinaryStream *stream) KIARA_ALWAYS_INLINE;
KIARA_Result readMessage_binary_stream(KIARA_Message *msg, KIARA_BinaryStream *stream) KIARA_ALWAYS_INLINE;

#ifdef __cplusplus
}
#endif

#endif /* KIARA_COMPONENTS_API_H_INCLUDED */
