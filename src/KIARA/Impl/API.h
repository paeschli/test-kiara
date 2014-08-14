/*
 * API.h
 *
 *  Created on: Feb 15, 2014
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_IMPL_API_H_INCLUDED
#define KIARA_IMPL_API_H_INCLUDED

#include <KIARA/kiara.h>

KIARA_BEGIN_EXTERN_C

typedef struct KIARA_ConnectionData KIARA_ConnectionData;
typedef int (*KIARA_InitNetworkFunc)(KIARA_Connection *);
typedef int (*KIARA_FinalizeNetworkFunc)(KIARA_Connection *);
typedef KIARA_Message * (*KIARA_CreateRequestMessageFromData)(const void *data, size_t dataSize);
typedef KIARA_Message * (*KIARA_CreateResponseMessage)(KIARA_Connection *conn, KIARA_Message *requestMsg);
typedef const char * (*KIARA_GetMessageMethodName)(KIARA_Message *msg);
typedef void (*KIARA_FreeMessage)(KIARA_Message *msg);
typedef KIARA_Result (*KIARA_GetMessageData)(KIARA_Message *msg, kr_dbuffer_t *dest);
typedef void (*KIARA_SetGenericErrorMessage)(KIARA_Message *msg, int errorCode, const char *errorMessage);
typedef const char * (*KIARA_GetMimeType)(void);

typedef KIARA_Connection * (*KIARA_GetConnection)(KIARA_FuncObj *funcObj);
typedef KIARA_Connection * (*KIARA_GetServiceConnection)(KIARA_ServiceFuncObj *funcObj);
typedef KIARA_Result (*KIARA_SendData)(KIARA_Connection *conn, const void *data, size_t dataSize, kr_dbuffer_t *destBuf);

KIARA_END_EXTERN_C

#endif /* KIARA_IMPL_API_H_INCLUDED */
