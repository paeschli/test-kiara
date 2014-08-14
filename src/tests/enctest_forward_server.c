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
 * enctest_server.c
 *
 *  Created on: Aug 16, 2013
 *      Author: rubinste
 */
#include <KIARA/kiara.h>
#include <KIARA/kiara_macros.h>
#include <KIARA/CDT/kr_dstring.h>
#include <KIARA/CDT/kr_dumpdata.h>
#include <stdio.h>
#include <stdlib.h>

int dstring_SetCString(KIARA_UserType *ustr, const char *cstr)
{
    int result = kr_dstring_assign_str((kr_dstring_t*)ustr, cstr);
    return result ? 0 : 1;
}

int dstring_GetCString(KIARA_UserType *ustr, const char **cstr)
{
    *cstr = kr_dstring_str((kr_dstring_t*)ustr);
    return 0;
}

KIARA_UserType * dstring_Allocate(void)
{
    return (KIARA_UserType *)kr_dstring_new();
}

void dstring_Deallocate(KIARA_UserType *value)
{
    kr_dstring_delete((kr_dstring_t*)value);
}

KIARA_DECL_OPAQUE_TYPE(kr_dstring_t,
       KIARA_USER_API(SetCString, dstring_SetCString)
       KIARA_USER_API(GetCString, dstring_GetCString)
       KIARA_USER_API(AllocateType, dstring_Allocate)
       KIARA_USER_API(DeallocateType, dstring_Deallocate))
KIARA_DECL_PTR(DStringPtr, kr_dstring_t)
KIARA_DECL_CONST_PTR(ConstDStringPtr, kr_dstring_t)

KIARA_DECL_ENCRYPTED(EncDString, kr_dstring_t)

KIARA_DECL_PTR(IntPtr, KIARA_INT)

KIARA_DECL_FUNC(SendIntFwd,
  KIARA_FUNC_RESULT(IntPtr, result)
  KIARA_FUNC_ARG(KIARA_INT, i)
)

KIARA_DECL_FUNC(SendStringFwd,
  KIARA_FUNC_RESULT(EncDString, result)
  KIARA_FUNC_ARG(EncDString, s)
  KIARA_FUNC_ARG(KIARA_INT, i)
)

KIARA_DECL_SERVICE(SendInt,
  KIARA_SERVICE_RESULT(IntPtr, result)
  KIARA_SERVICE_ARG(KIARA_INT, i))

KIARA_DECL_SERVICE(SendString,
  KIARA_SERVICE_RESULT(EncDString, result)
  KIARA_SERVICE_ARG(EncDString, s)
  KIARA_SERVICE_ARG(KIARA_INT, i))

KIARA_FUNC_OBJ(SendIntFwd) sendIntFwd;
KIARA_FUNC_OBJ(SendStringFwd) sendStringFwd;
KIARA_Context *ctxFwd;
KIARA_Connection *connFwd;

KIARA_Result send_int_impl(KIARA_ServiceFuncObj *kiara_funcobj, int *result, int i)
{
    int errorCode;

    printf("SendInt: Forwarding %i\n", i);

    errorCode = KIARA_CALL(sendIntFwd, result, i);

    printf("SendInt: Received %i\n", *result);

    return errorCode;
}

KIARA_Result send_string_impl(KIARA_ServiceFuncObj *kiara_funcobj, EncDString result, EncDString s, int i)
{
    int errorCode;

    kr_dump_data("SendString: Forwarding encrypted data: ", stderr, (unsigned char *)kr_dbuffer_data(&(s->data)), kr_dbuffer_size(&(s->data)), 0);
    printf("SendString: Forwarding %i\n", i);

    //s->data.data[0] = 0xff;

    errorCode = KIARA_CALL(sendStringFwd, result, s, i);

    kr_dump_data("SendString: Received encrypted data: ", stderr, (unsigned char *)kr_dbuffer_data(&(result->data)), kr_dbuffer_size(&(result->data)), 0);
    printf("SendString: Received %i\n", i);

    // assign_EncDString(result, s);

    return errorCode;
}

int main(int argc, char **argv)
{
    KIARA_Context *ctx;
    KIARA_Service *service;
    KIARA_Server *server;
    KIARA_Result result;
    const char *port = NULL;
    const char *url = NULL;
    const char *protocol = NULL;

    /* This code is required for testing tool when compiled with MS CRT library and valgrind */
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    /* Initialize KIARA */
    kiaraInit(&argc, argv);

    ctx = kiaraNewContext();
    ctxFwd = kiaraNewContext();

    if (argc > 2)
    {
        url = argv[1];
        port = argv[2];
    }
    else
    {
        url = "http://localhost:8080/service";
        port = "8080";
    }

    if (argc > 3)
    {
        protocol = argv[3];
    }
    else
    {
        protocol = "jsonrpc";
    }

    printf("Remote service URL: %s\n", url);
    printf("Server port: %s\n", port);
    printf("Protocol: %s\n", protocol);

    /* Open connection to the service */
    printf("Opening connection to %s...\n", url);
    connFwd = kiaraOpenConnection(ctxFwd, url);

    if (!connFwd)
    {
        fprintf(stderr, "Error: Could not open connection : %s\n", kiaraGetContextError(ctxFwd));
        exit(1);
    }

    {
        /* IDL -> native */
        sendIntFwd = KIARA_GENERATE_CLIENT_FUNC(connFwd, "enc.sendInt", SendIntFwd, "");
        if (!sendIntFwd) /* FIXME this should be always true */
        {
            fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(connFwd));
            exit(1);
        }
    }

    {
        /* IDL -> native */
        sendStringFwd = KIARA_GENERATE_CLIENT_FUNC(connFwd, "enc.sendString", SendStringFwd, "");
        if (!sendStringFwd) /* FIXME this should be always true */
        {
            fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(connFwd));
            exit(1);
        }
    }

    /* Server */

    service = kiaraNewService(ctx);

    result = kiaraLoadServiceIDLFromString(service,
        "KIARA",
        "namespace * enc "
        "service enc { "
        "    i32 sendInt([Encrypted] i32 i) "
        "    string [Encrypted] sendString([Encrypted] string s, i32 i) "
        "} ");
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: could not parse IDL: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    printf("Register enc.sendInt...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "enc.sendInt", SendInt, "", send_int_impl);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    printf("Register enc.sendString...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "enc.sendString", SendString, "", send_string_impl);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    server = kiaraNewServer(ctx, "0.0.0.0", atoi(port), "/service");

    kiaraAddService(server, "/rpc/enc", protocol, service);

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
