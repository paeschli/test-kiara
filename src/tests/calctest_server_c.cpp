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
 * calctest_server.c
 *
 *  Created on: 03.07.2013
 *      Author: Dmitri Rubinstein
 */
#include <KIARA/kiara.h>
#include <KIARA/kiara_macros.h>
#include <KIARA/CDT/kr_dstring.h>
#include <stdio.h>
#include <stdlib.h>

/* MSVC Compatibility */
#ifdef _MSC_VER
#define snprintf _snprintf
#endif

/* KIARA descriptions */

KIARA_DECL_PTR(IntPtr, KIARA_INT)
KIARA_DECL_PTR(FloatPtr, KIARA_FLOAT)
KIARA_DECL_PTR(CharPtrPtr, KIARA_CHAR_PTR)

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

KIARA_DECL_SERVICE(Calc_Add,
    KIARA_SERVICE_RESULT(IntPtr, result)
    KIARA_SERVICE_ARG(KIARA_INT, a)
    KIARA_SERVICE_ARG(KIARA_INT, b))

KIARA_DECL_SERVICE(Calc_Add_Float,
    KIARA_SERVICE_RESULT(FloatPtr, result)
    KIARA_SERVICE_ARG(KIARA_FLOAT, a)
    KIARA_SERVICE_ARG(KIARA_FLOAT, b))

KIARA_DECL_SERVICE(Calc_String_To_Int32,
    KIARA_SERVICE_RESULT(IntPtr, result)
    KIARA_SERVICE_ARG(KIARA_const_char_ptr, s))

KIARA_DECL_SERVICE(Calc_Int32_To_String,
    KIARA_SERVICE_RESULT(CharPtrPtr, result)
    KIARA_SERVICE_ARG(KIARA_INT, i))

KIARA_DECL_SERVICE(Calc_DString_To_Int32,
    KIARA_SERVICE_RESULT(IntPtr, result)
    KIARA_SERVICE_ARG(ConstDStringPtr, s))

KIARA_DECL_SERVICE(Calc_Int32_To_DString,
    KIARA_SERVICE_RESULT(DStringPtr, result)
    KIARA_SERVICE_ARG(KIARA_INT, i))

/* Service implementation */

KIARA_Result calc_add_impl(KIARA_ServiceFuncObj *kiara_funcobj, int *result, int a, int b)
{
    *result = a + b;
    return KIARA_SUCCESS;
}

KIARA_Result calc_add_float_impl(KIARA_ServiceFuncObj *kiara_funcobj, float *result, float a, float b)
{
    *result = a + b;
    return KIARA_SUCCESS;
}

KIARA_Result calc_string_to_int32_impl(KIARA_ServiceFuncObj *kiara_funcobj, int *result, const char *s)
{
    *result = atoi(s);
    return KIARA_SUCCESS;
}

KIARA_Result calc_int32_to_string_impl(KIARA_ServiceFuncObj *kiara_funcobj, char **result, int i)
{
    *result = (char*)malloc(15);
    snprintf(*result, 15, "%d", i);
    return KIARA_SUCCESS;
}

KIARA_Result calc_dstring_to_int32_impl(KIARA_ServiceFuncObj *kiara_funcobj, int *result, const kr_dstring_t *s)
{
    *result = atoi(kr_dstring_str(s));
    return KIARA_SUCCESS;
}

KIARA_Result calc_int32_to_dstring_impl(KIARA_ServiceFuncObj *kiara_funcobj, kr_dstring_t *result, int i)
{
    char *s = (char*)malloc(15);
    snprintf(s, 15, "%d", i);
    kr_dstring_assign_str(result, s);
    free(s);
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

    service = kiaraNewService(ctx);

    result = kiaraLoadServiceIDLFromString(service,
        "KIARA",
        "namespace * calc "
        "service calc { "
        "    i32 add(i32 a, i32 b) "
        "    float addf(float a, float b) "
        "    i32 stringToInt32(string s) "
        "    string int32ToString(i32 i) "
        "} ");
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: could not parse IDL: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    printf("Register calc.add ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "calc.add", Calc_Add, "", calc_add_impl);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

#ifdef RUNSIM
    kiaraDbgSimulateCall(service,
                         "{\"jsonrpc\": \"2.0\", \"method\": \"calc.add\", \"params\": [42, 23], \"id\": 1}");
#endif

    printf("Register calc.addf ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "calc.addf", Calc_Add_Float, "", calc_add_float_impl);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

#ifdef RUNSIM
    kiaraDbgSimulateCall(service,
                         "{\"jsonrpc\": \"2.0\", \"method\": \"calc.addf\", \"params\": [21.1, 32.2], \"id\": 2}");
#endif

    printf("Register calc.stringToInt32...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "calc.stringToInt32", Calc_String_To_Int32, "", calc_string_to_int32_impl);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }
#ifdef RUNSIM
    kiaraDbgSimulateCall(service,
                         "{\"jsonrpc\": \"2.0\", \"method\": \"calc.stringToInt32\", \"params\": [\"45\"], \"id\": 3}");
#endif

    printf("Register calc.int32ToString...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "calc.int32ToString", Calc_Int32_To_String, "", calc_int32_to_string_impl);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

#ifdef RUNSIM
    kiaraDbgSimulateCall(service,
                         "{\"jsonrpc\": \"2.0\", \"method\": \"calc.int32ToString\", \"params\": [132], \"id\": 4}");
#endif

    printf("Register calc.stringToInt32 (2nd variant)...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "calc.stringToInt32", Calc_DString_To_Int32, "", calc_dstring_to_int32_impl);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

#ifdef RUNSIM
    kiaraDbgSimulateCall(service,
                         "{\"jsonrpc\": \"2.0\", \"method\": \"calc.stringToInt32\", \"params\": [\"45\"], \"id\": 3}");
#endif

    printf("Register calc.int32ToString (2nd variant)...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "calc.int32ToString", Calc_Int32_To_DString, "", calc_int32_to_dstring_impl);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

#ifdef RUNSIM
    kiaraDbgSimulateCall(service,
                         "{\"jsonrpc\": \"2.0\", \"method\": \"calc.int32ToString\", \"params\": [132], \"id\": 4}");
#endif

    server = kiaraNewServer(ctx, "0.0.0.0", atoi(port), "/service");

    kiaraAddService(server, "/rpc/calc", protocol, service);

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
