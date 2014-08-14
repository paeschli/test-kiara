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
 * structtest_server.c
 *
 *  Created on: 03.07.2013
 *      Author: Dmitri Rubinstein
 */
#include <KIARA/kiara.h>
#include <KIARA/kiara_macros.h>
#include <KIARA/CDT/kr_dstring.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "c99fmt.h"

static char *xstrdup(const char *str)
{
    size_t len;
    char *newstr;

    if (!str)
        return (char *)NULL;

    len = strlen(str);

    if (len >= ((size_t)-1) / sizeof(char))
        return (char *)NULL;

    newstr = malloc((len+1)*sizeof(char));
    if (!newstr)
        return (char *)NULL;

    memcpy(newstr,str,(len+1)*sizeof(char));

    return newstr;
}

/* Application data structures */

/* Data */

typedef int64_t Integer;

typedef struct Data
{
    Integer ival;
    kr_dstring_t sval;
} Data;

void initData(Data *data)
{
    data->ival = 0;
    kr_dstring_init(&data->sval); /* must be initialized */
}

void destroyData(Data *data)
{
    kr_dstring_destroy(&data->sval);
}

KIARA_UserType * Data_Allocate(void)
{
    Data *data = malloc(sizeof(Data));
    initData(data);
    return (KIARA_UserType *)data;
}

void Data_Deallocate(KIARA_UserType *value)
{
    if (value)
    {
        destroyData((Data*)value);
        free(value);
    }
}

/* Location */

typedef struct Vec3f {
    float x;
    float y;
    float z;
} Vec3f;

typedef struct Quatf {
    float r; /* real part */
    Vec3f v; /* imaginary vector */
} Quatf;

typedef struct Location {
    Vec3f position;
    Quatf rotation;
} Location;

/* Exception */

typedef struct Exception
{
    int errorCode;
    char *errorMessage;
} Exception;

void initException(Exception *exception)
{
    exception->errorCode = 0;
    exception->errorMessage = NULL;
}

void destroyException(Exception *exception)
{
    free(exception->errorMessage);
    exception->errorMessage = NULL;
}

void setException(Exception *exception, int errorCode, const char *errorMessage)
{
    exception->errorCode = errorCode;
    free(exception->errorMessage);
    exception->errorMessage = xstrdup(errorMessage);
}

/* KIARA descriptions */

/* KIARA_DECL_PTR(IntPtr, KIARA_INT) */
/* KIARA_DECL_PTR(FloatPtr, KIARA_FLOAT) */
/* KIARA_DECL_PTR(CharPtrPtr, KIARA_CHAR_PTR) */

/** Describe kr_dstring_t */

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

/** Describe Exception */

int Exception_SetGenericError(KIARA_UserType *uexc, int errorCode, const char *errorMessage)
{
    Exception *exception = (Exception*)uexc;
    setException(exception, errorCode, errorMessage);

    return KIARA_SUCCESS;
}

int Exception_GetGenericError(KIARA_UserType *uexc, int *errorCode, const char **errorMessage)
{
    Exception *exception = (Exception*)uexc;
    *errorCode = exception->errorCode;
    *errorMessage = exception->errorMessage;
    return KIARA_SUCCESS;
}

KIARA_UserType * Exception_Allocate(void)
{
    Exception *exception = malloc(sizeof(Exception));
    initException(exception);
    return (KIARA_UserType *)exception;
}

void Exception_Deallocate(KIARA_UserType *value)
{
    Exception *exception = (Exception*)value;
    if (exception)
    {
        destroyException(exception);
        free(exception);
    }
}

KIARA_DECL_OPAQUE_TYPE(Exception,
       KIARA_USER_API(SetGenericError, Exception_SetGenericError)
       KIARA_USER_API(GetGenericError, Exception_GetGenericError)
       KIARA_USER_API(AllocateType, Exception_Allocate)
       KIARA_USER_API(DeallocateType, Exception_Deallocate))
KIARA_DECL_PTR(ExceptionPtr, Exception)
/* KIARA_DECL_CONST_PTR(ConstExceptionPtr, Exception) */

/** Describe Integer */

KIARA_DECL_DERIVED_BUILTIN(Integer, KIARA_INT64_T)
KIARA_DECL_PTR(IntegerPtr, Integer)

/** Describe Data */

KIARA_DECL_STRUCT_WITH_API(Data,
  KIARA_STRUCT_MEMBER(Integer, ival)
  KIARA_STRUCT_MEMBER(kr_dstring_t, sval),
  KIARA_USER_API(AllocateType, Data_Allocate)
  KIARA_USER_API(DeallocateType, Data_Deallocate)
)
KIARA_DECL_PTR(DataPtr, Data)
KIARA_DECL_CONST_PTR(ConstDataPtr, Data)

/** Describe Location */

KIARA_DECL_STRUCT(Vec3f,
  KIARA_STRUCT_MEMBER(KIARA_FLOAT, x)
  KIARA_STRUCT_MEMBER(KIARA_FLOAT, y)
  KIARA_STRUCT_MEMBER(KIARA_FLOAT, z)
)

KIARA_DECL_STRUCT(Quatf,
  KIARA_STRUCT_MEMBER(KIARA_FLOAT, r)
  KIARA_STRUCT_MEMBER(Vec3f, v)
)

KIARA_DECL_STRUCT(Location,
  KIARA_STRUCT_MEMBER(Vec3f, position)
  KIARA_STRUCT_MEMBER(Quatf, rotation)
)

KIARA_DECL_PTR(LocationPtr, Location)
KIARA_DECL_CONST_PTR(ConstLocationPtr, Location)

/** Describe Service Functions */

KIARA_DECL_SERVICE(StructTest_Pack,
  KIARA_SERVICE_RESULT(DataPtr, result)
  KIARA_SERVICE_ARG(Integer, ival)
  KIARA_SERVICE_ARG(KIARA_const_char_ptr, sval))

KIARA_DECL_SERVICE(StructTest_GetInteger,
  KIARA_SERVICE_RESULT(IntegerPtr, result)
  KIARA_SERVICE_ARG(ConstDataPtr, data))

KIARA_DECL_SERVICE(StructTest_GetString,
  KIARA_SERVICE_RESULT(DStringPtr, result)
  KIARA_SERVICE_ARG(ConstDataPtr, data))


KIARA_DECL_SERVICE(StructTest_GetLocation,
  KIARA_SERVICE_RESULT(LocationPtr, location))

KIARA_DECL_SERVICE(StructTest_SetLocation,
  KIARA_SERVICE_ARG(ConstLocationPtr, location))

KIARA_DECL_SERVICE(StructTest_ThrowException,
    KIARA_SERVICE_ARG(KIARA_INT, code)
    KIARA_SERVICE_ARG(KIARA_const_char_ptr, message)
    KIARA_SERVICE_EXCEPTION(ExceptionPtr, exception))

/** Service implementation */

KIARA_Result structtest_pack_impl(KIARA_ServiceFuncObj *kiara_funcobj, Data *result, Integer ival, const char *sval)
{
    result->ival = ival;
    kr_dstring_assign_str(&result->sval, sval);
    return KIARA_SUCCESS;
}

KIARA_Result structtest_get_integer_impl(KIARA_ServiceFuncObj *kiara_funcobj, Integer *result, const Data *data)
{
    *result = data->ival;
    return KIARA_SUCCESS;
}

KIARA_Result structtest_get_string_impl(KIARA_ServiceFuncObj *kiara_funcobj, kr_dstring_t *result, const Data *data)
{
    kr_dstring_assign_str(result, kr_dstring_str(&data->sval));
    return KIARA_SUCCESS;
}

Location objectLocation;

KIARA_Result structtest_set_location_impl(KIARA_ServiceFuncObj *kiara_funcobj, const Location *location)
{
    printf("Location.position %f %f %f\nLocation.rotation %f %f %f %f\n",
        location->position.x,
        location->position.y,
        location->position.z,
        location->rotation.r,
        location->rotation.v.x,
        location->rotation.v.y,
        location->rotation.v.z);

    memcpy(&objectLocation, location, sizeof(Location));

    return KIARA_SUCCESS;
}

KIARA_Result structtest_get_location_impl(KIARA_ServiceFuncObj *kiara_funcobj, Location *location)
{
    memcpy(location, &objectLocation, sizeof(Location));

    return KIARA_SUCCESS;
}

KIARA_Result structtest_throw_exception_impl(KIARA_ServiceFuncObj *kiara_funcobj, int code, const char *message, Exception *exception)
{
    setException(exception, code, message);
    return KIARA_EXCEPTION;
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
        Data result, * result_ptr;
        Integer ival;
        KIARA_const_char_ptr sval;

        void *args[3];
        KIARA_Result status;
        KIARA_DeclType *ty;
        KIARA_DeclService *s;

        initData(&result);
        ival = 23;
        sval = "TEST";
        result_ptr = &result;

        args[0] = &result_ptr;
        args[1] = &ival;
        args[2] = &sval;

        ty = _KR_get_type(StructTest_Pack)();
        s = ty->typeDecl.serviceDecl;
        status = s->serviceWrapperFunc(/*Connection*/NULL, args, 3);
        printf("status = %i\nresult = {%i, '%s'}\n",
               status,
               (int)result.ival,
               kr_dstring_str(&result.sval));
        destroyData(&result);
    }
#endif

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

    ctx = kiaraNewContext();

    service = kiaraNewService(ctx);

    result = kiaraLoadServiceIDLFromString(service,
        "KIARA",
        "namespace * struct_test "
        "typedef i64 Integer "
        "struct Data { "
        " Integer ival, "
        "  string  sval "
        "} "
        "struct Vec3f {"
        " float x, "
        " float y, "
        " float z "
        "} "
        "struct Quatf {"
        " float r, "
        " Vec3f v  "
        "} "
        "struct Location {"
        " Vec3f position, "
        " Quatf rotation  "
        "} "
        "exception Exception {"
        " i32 code, "
        " string message "
        "} "
        "service StructTest { "
        "  Data pack(Integer ival, string sval); "
        "  Integer getInteger(Data data); "
        "  string getString(Data data); "
        "  void setLocation(Location location);"
        "  Location getLocation();"
        "  void throwException(i32 code, string message) throws (Exception error); "
        "} ");
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: could not parse IDL: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    printf("Register StructTest.pack ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "StructTest.pack", StructTest_Pack, "", structtest_pack_impl);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

#ifdef RUNSIM
    kiaraDbgSimulateCall(service,
                         "{\"jsonrpc\": \"2.0\", \"method\": \"StructTest.pack\", \"params\": [23, \"TEST\"], \"id\": 1}");
#endif

    printf("Register StructTest.getInteger ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "StructTest.getInteger", StructTest_GetInteger, "", structtest_get_integer_impl);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

#ifdef RUNSIM
    kiaraDbgSimulateCall(service,
                         "{\"jsonrpc\": \"2.0\", \"method\": \"StructTest.getInteger\", \"params\": [{\"ival\" : 45, \"sval\" : \"BLAH\"}], \"id\": 2}");
#endif

    printf("Register StructTest.getString ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "StructTest.getString", StructTest_GetString, "", structtest_get_string_impl);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }
    printf("Register StructTest.setLocation ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "StructTest.setLocation", StructTest_SetLocation, "", structtest_set_location_impl);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

#ifdef RUNSIM
    kiaraDbgSimulateCall(service,
                         "{\"jsonrpc\": \"2.0\", \"method\": \"StructTest.getString\", \"params\": [{\"ival\" : 45, \"sval\" : \"BLAH\"}], \"id\": 3}");
#endif

    printf("Register StructTest.getLocation ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "StructTest.getLocation", StructTest_GetLocation, "", structtest_get_location_impl);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

#ifdef RUNSIM
    kiaraDbgSimulateCall(service,
                         "{\"jsonrpc\": \"2.0\", \"method\": \"StructTest.getString\", \"params\": [{\"ival\" : 45, \"sval\" : \"BLAH\"}], \"id\": 3}");
#endif

    printf("Register StructTest.throwException ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "StructTest.throwException", StructTest_ThrowException, "", structtest_throw_exception_impl);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

#ifdef RUNSIM
    kiaraDbgSimulateCall(service,
                         "{\"jsonrpc\": \"2.0\", \"method\": \"StructTest.throwException\", \"params\": [202, \"NOT FOUND\"], \"id\": 4}");
#endif

    server = kiaraNewServer(ctx, "0.0.0.0", atoi(port), "/service2");

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
