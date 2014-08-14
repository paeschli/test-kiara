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
 * arraytest_server.c
 *
 *  Created on: 03.07.2013
 *      Author: Dmitri Rubinstein
 */
#include <KIARA/kiara.h>
#include <KIARA/kiara_macros.h>
#include <KIARA/CDT/kr_dstring.h>
#include <stdio.h>
#include <stdlib.h>
#include "arraytest_kiara.h"
#include "c99fmt.h"

/* Service implementation */

KIARA_Result send_data_impl(KIARA_ServiceFuncObj *kiara_funcobj, Data *result, Data *a)
{
    size_t i;

    printf("received\n");

#define PRINT_ARRAY(type, fmt)                                      \
                printf(#type " array size = %i\n", a->size_##type); \
                printf(#type " array = [\n ");                      \
                for (i = 0; i < a->size_##type; ++i)                \
                {                                                   \
                    printf("%" fmt " ", a->array_##type[i]);        \
                }                                                   \
                printf("]\n")

    PRINT_ARRAY(boolean, "i");
    PRINT_ARRAY(i8, PRIi8);
    PRINT_ARRAY(u8, PRIu8);
    PRINT_ARRAY(i16, PRIi16);
    PRINT_ARRAY(u16, PRIu16);
    PRINT_ARRAY(i32, PRIi32);
    PRINT_ARRAY(u32, PRIu32);
    PRINT_ARRAY(i64, PRIi64);
    PRINT_ARRAY(u64, PRIu64);
    PRINT_ARRAY(float, "f");
    PRINT_ARRAY(double, "f");

#undef PRINT_ARRAY

    initData(result, a->size_boolean);

#define MAKE_RESULT(type)                                               \
    for (i = 0; i < a->size_##type; ++i)                                \
    {                                                                   \
        result->array_##type[i] = a->array_##type[a->size_##type-1-i];  \
    }

    MAKE_RESULT(boolean);
    MAKE_RESULT(i8);
    MAKE_RESULT(u8);
    MAKE_RESULT(i16);
    MAKE_RESULT(u16);
    MAKE_RESULT(i32);
    MAKE_RESULT(u32);
    MAKE_RESULT(i64);
    MAKE_RESULT(u64);
    MAKE_RESULT(float);
    MAKE_RESULT(double);

#undef MAKE_RESULT

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

#if 0
    /* Manual test of the service wrapper function */
    {
        int a, b, result, * result_ptr;
        void *args[3];
        KIARA_Result status;
        KIARA_DeclType *ty;
        KIARA_DeclService *s;
        a = 12;
        b = 33;
        result = 0;
        result_ptr = &result;

        args[0] = &result_ptr;
        args[1] = &a;
        args[2] = &b;

        ty = _KR_get_type(Calc_Add)();
        s = ty->typeDecl.serviceDecl;
        status = s->serviceWrapperFunc(/*Connection*/NULL, args, 3);
        printf("status = %i\nresult = %i\n", status, result);
    }
#endif

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

    {
        kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(Data));
    }

    service = kiaraNewService(ctx);

    result = kiaraLoadServiceIDLFromString(service,
        "KIARA",
        "namespace * arraytest "
        "struct Data { "
        "    array<boolean> array_boolean "
        "    array<i8> array_i8 "
        "    array<u8> array_u8 "
        "    array<i16> array_i16 "
        "    array<u16> array_u16 "
        "    array<i32> array_i32 "
        "    array<u32> array_u32 "
        "    array<i64> array_i64 "
        "    array<u64> array_u64 "
        "    array<float> array_float "
        "    array<double> array_double "
        "} "
        "service arraytest { "
        "    Data send(Data a) "
        "} ");
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: could not parse IDL: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    printf("Register arraytest.send ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "arraytest.send", OnSendData, "", send_data_impl);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

#ifdef RUNSIM
    kiaraDbgSimulateCall(service,
                         "{\"jsonrpc\": \"2.0\", \"method\": \"arraytest.send\", \"params\": [42, 23], \"id\": 1}");
#endif

    server = kiaraNewServer(ctx, "0.0.0.0", atoi(port), "/service");

    result = kiaraAddService(server, "/rpc/arraytest", protocol, service);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: could not add service: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServerError(server));
        exit(1);
    }

    printf("Starting server...\n");

    result = kiaraRunServer(server);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: could not start server: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServerError(server));
    }

    kiaraFreeServer(server);

    kiaraFreeService(service);

    kiaraFreeContext(ctx);

    kiaraFinalize();

    return 0;
}
