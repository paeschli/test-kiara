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
 * basetypetest_server.c
 *
 *  Created on: 28.11.2013
 *      Author: Dmitri Rubinstein
 */
#include <KIARA/kiara.h>
#include <KIARA/kiara_macros.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* KIARA descriptions */

KIARA_DECL_PTR(IntPtr, KIARA_INT)

KIARA_DECL_PTR(Int8Ptr, KIARA_INT8_T)
KIARA_DECL_PTR(UInt8Ptr, KIARA_UINT8_T)

KIARA_DECL_PTR(Int16Ptr, KIARA_INT16_T)
KIARA_DECL_PTR(UInt16Ptr, KIARA_UINT16_T)

KIARA_DECL_PTR(Int32Ptr, KIARA_INT32_T)
KIARA_DECL_PTR(UInt32Ptr, KIARA_UINT32_T)

KIARA_DECL_PTR(Int64Ptr, KIARA_INT64_T)
KIARA_DECL_PTR(UInt64Ptr, KIARA_UINT64_T)

KIARA_DECL_PTR(FloatPtr, KIARA_FLOAT)
KIARA_DECL_PTR(DoublePtr, KIARA_DOUBLE)
KIARA_DECL_PTR(CharPtrPtr, KIARA_CHAR_PTR)

KIARA_DECL_SERVICE(Basetype_Send_boolean,
    KIARA_SERVICE_RESULT(IntPtr, result)
    KIARA_SERVICE_ARG(KIARA_INT, a))

KIARA_DECL_SERVICE(Basetype_Send_i8,
    KIARA_SERVICE_RESULT_WITH_SEMANTICS(Int8Ptr, result, VI_NONE)
    KIARA_SERVICE_ARG(KIARA_INT8_T, a))

KIARA_DECL_SERVICE(Basetype_Send_u8,
    KIARA_SERVICE_RESULT_WITH_SEMANTICS(UInt8Ptr, result, VI_NONE)
    KIARA_SERVICE_ARG(KIARA_UINT8_T, a))

KIARA_DECL_SERVICE(Basetype_Send_i16,
    KIARA_SERVICE_RESULT(Int16Ptr, result)
    KIARA_SERVICE_ARG(KIARA_INT16_T, a))

KIARA_DECL_SERVICE(Basetype_Send_u16,
    KIARA_SERVICE_RESULT(UInt16Ptr, result)
    KIARA_SERVICE_ARG(KIARA_UINT16_T, a))

KIARA_DECL_SERVICE(Basetype_Send_i32,
    KIARA_SERVICE_RESULT(Int32Ptr, result)
    KIARA_SERVICE_ARG(KIARA_INT32_T, a))

KIARA_DECL_SERVICE(Basetype_Send_u32,
    KIARA_SERVICE_RESULT(UInt32Ptr, result)
    KIARA_SERVICE_ARG(KIARA_UINT32_T, a))

KIARA_DECL_SERVICE(Basetype_Send_i64,
    KIARA_SERVICE_RESULT(Int64Ptr, result)
    KIARA_SERVICE_ARG(KIARA_INT64_T, a))

KIARA_DECL_SERVICE(Basetype_Send_u64,
    KIARA_SERVICE_RESULT(UInt64Ptr, result)
    KIARA_SERVICE_ARG(KIARA_UINT64_T, a))

KIARA_DECL_SERVICE(Basetype_Send_float,
    KIARA_SERVICE_RESULT(FloatPtr, result)
    KIARA_SERVICE_ARG(KIARA_FLOAT, a))

KIARA_DECL_SERVICE(Basetype_Send_double,
    KIARA_SERVICE_RESULT(DoublePtr, result)
    KIARA_SERVICE_ARG(KIARA_DOUBLE, a))

KIARA_DECL_SERVICE(Basetype_Send_string,
    KIARA_SERVICE_RESULT(CharPtrPtr, result)
    KIARA_SERVICE_ARG(KIARA_const_char_ptr, a))

/* Service implementation */

KIARA_Result basetype_send_boolean(KIARA_ServiceFuncObj *kiara_funcobj, int *result, int a)
{
    *result = a;
    return KIARA_SUCCESS;
}

KIARA_Result basetype_send_i8(KIARA_ServiceFuncObj *kiara_funcobj, int8_t *result, int8_t a)
{
    *result = a;
    return KIARA_SUCCESS;
}

KIARA_Result basetype_send_u8(KIARA_ServiceFuncObj *kiara_funcobj, uint8_t *result, uint8_t a)
{
    *result = a;
    return KIARA_SUCCESS;
}

KIARA_Result basetype_send_i16(KIARA_ServiceFuncObj *kiara_funcobj, int16_t *result, int16_t a)
{
    *result = a;
    return KIARA_SUCCESS;
}

KIARA_Result basetype_send_u16(KIARA_ServiceFuncObj *kiara_funcobj, uint16_t *result, uint16_t a)
{
    *result = a;
    return KIARA_SUCCESS;
}

KIARA_Result basetype_send_i32(KIARA_ServiceFuncObj *kiara_funcobj, int32_t *result, int32_t a)
{
    *result = a;
    return KIARA_SUCCESS;
}

KIARA_Result basetype_send_u32(KIARA_ServiceFuncObj *kiara_funcobj, uint32_t *result, uint32_t a)
{
    *result = a;
    return KIARA_SUCCESS;
}

KIARA_Result basetype_send_i64(KIARA_ServiceFuncObj *kiara_funcobj, int64_t *result, int64_t a)
{
    *result = a;
    return KIARA_SUCCESS;
}

KIARA_Result basetype_send_u64(KIARA_ServiceFuncObj *kiara_funcobj, uint64_t *result, uint64_t a)
{
    *result = a;
    return KIARA_SUCCESS;
}

KIARA_Result basetype_send_float(KIARA_ServiceFuncObj *kiara_funcobj, float *result, float a)
{
    *result = a;
    return KIARA_SUCCESS;
}

KIARA_Result basetype_send_double(KIARA_ServiceFuncObj *kiara_funcobj, double *result, double a)
{
    *result = a;
    return KIARA_SUCCESS;
}

KIARA_Result basetype_send_string(KIARA_ServiceFuncObj *kiara_funcobj, char **result, const char *a)
{
    *result = malloc(strlen(a)+1);
    strcpy(*result, a);
    return KIARA_SUCCESS;
}

int main(int argc, char **argv)
{
    KIARA_Context *ctx;
    KIARA_Service *service;
    KIARA_Server *server;
    KIARA_Result result;
    const char *port = NULL;
    const char *protocol = NULL;

    /* This code is required for testing tool when compiled with MS CRT library and valgrind */
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    /* Initialize KIARA */
    kiaraInit(&argc, argv);

    ctx = kiaraNewContext();

    if (argc > 1)
    {
        port = argv[1];
    }
    else
    {
        port = "8080";
    }

    if (argc > 2)
    {
        protocol = argv[2];
    }
    else
    {
        protocol = "jsonrpc";
    }

    printf("Server port: %s\n", port);
    printf("Protocol: %s\n", protocol);

    service = kiaraNewService(ctx);

    if (!service)
    {
        fprintf(stderr, "Error: could not create service: %i: %s\n",
                (int)kiaraGetContextErrorCode(ctx), kiaraGetContextError(ctx));
        exit(1);
    }

    result = kiaraLoadServiceIDLFromString(service,
        "KIARA",
        "namespace * basetype "
        "service basetype { "
        "    boolean send_boolean(boolean a) "
        "    i8 send_i8(i8 a) "
        "    u8 send_u8(u8 a) "
        "    i16 send_i16(i16 a) "
        "    u16 send_u16(u16 a) "
        "    i32 send_i32(i32 a) "
        "    u32 send_u32(u32 a) "
        "    i64 send_i64(i64 a) "
        "    u64 send_u64(u64 a) "
        "    float send_float(float a) "
        "    double send_double(double a) "
        "    string send_string(string a) "
        "} ");
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: could not parse IDL: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    printf("Register basetype.send_boolean ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "basetype.send_boolean", Basetype_Send_boolean, "", basetype_send_boolean);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    printf("Register basetype.send_i8 ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "basetype.send_i8", Basetype_Send_i8, "", basetype_send_i8);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    printf("Register basetype.send_u8 ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "basetype.send_u8", Basetype_Send_u8, "", basetype_send_u8);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    printf("Register basetype.send_i16 ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "basetype.send_i16", Basetype_Send_i16, "", basetype_send_i16);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    printf("Register basetype.send_u16 ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "basetype.send_u16", Basetype_Send_u16, "", basetype_send_u16);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }


    printf("Register basetype.send_i32 ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "basetype.send_i32", Basetype_Send_i32, "", basetype_send_i32);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    printf("Register basetype.send_u32 ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "basetype.send_u32", Basetype_Send_u32, "", basetype_send_u32);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    printf("Register basetype.send_i64 ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "basetype.send_i64", Basetype_Send_i64, "", basetype_send_i64);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    printf("Register basetype.send_u64 ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "basetype.send_u64", Basetype_Send_u64, "", basetype_send_u64);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    printf("Register basetype.send_float ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "basetype.send_float", Basetype_Send_float, "", basetype_send_float);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    printf("Register basetype.send_double ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "basetype.send_double", Basetype_Send_double, "", basetype_send_double);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    printf("Register basetype.send_string ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "basetype.send_string", Basetype_Send_string, "", basetype_send_string);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    /* Run server */

    server = kiaraNewServer(ctx, "0.0.0.0", atoi(port), "/service");

    result = kiaraAddService(server, "/rpc/basetype", protocol, service);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: could not add service: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServerError(server));
    }

    printf("Starting server...\n");

    result = kiaraRunServer(server);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: could not start server: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServerError(server));
    }

    /* Shutdown server */

    kiaraFreeServer(server);

    kiaraFreeService(service);

    kiaraFreeContext(ctx);

    kiaraFinalize();

    return 0;
}
