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
 * structtest.c
 *
 *  Created on: 13.06.2013
 *      Author: Dmitri Rubinstein
 */
#include <KIARA/kiara.h>
#include <KIARA/kiara_macros.h>
#include <KIARA/CDT/kr_dstring.h>
#include <stdio.h>
#include <string.h>
#include "c99fmt.h"

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

KIARA_DECL_OPAQUE_TYPE(kr_dstring_t,
       KIARA_USER_API(SetCString, dstring_SetCString)
       KIARA_USER_API(GetCString, dstring_GetCString))
KIARA_DECL_PTR(DStringPtr, kr_dstring_t)
KIARA_DECL_CONST_PTR(ConstDStringPtr, kr_dstring_t)

/** Describe Exception */

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

int Exception_SetGenericError(KIARA_UserType *uexc, int errorCode, const char *errorMessage)
{
    Exception *exception = (Exception*)uexc;
    exception->errorCode = errorCode;
    free(exception->errorMessage);
    exception->errorMessage = xstrdup(errorMessage);

    return KIARA_SUCCESS;
}

KIARA_DECL_OPAQUE_TYPE(Exception,
       KIARA_USER_API(SetGenericError, Exception_SetGenericError))
KIARA_DECL_PTR(ExceptionPtr, Exception)
KIARA_DECL_CONST_PTR(ConstExceptionPtr, Exception)

/** Describe Integer */

KIARA_DECL_DERIVED_BUILTIN(Integer, KIARA_INT64_T)
KIARA_DECL_PTR(IntegerPtr, Integer)

/** Describe Data */

KIARA_DECL_STRUCT(Data,
  KIARA_STRUCT_MEMBER(Integer, ival)
  KIARA_STRUCT_MEMBER(kr_dstring_t, sval)
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

KIARA_DECL_FUNC(StructTest_Pack,
  KIARA_FUNC_RESULT(DataPtr, result)
  KIARA_FUNC_ARG(Integer, ival)
  KIARA_FUNC_ARG(KIARA_const_char_ptr, sval)
)

KIARA_DECL_FUNC(StructTest_GetInteger,
  KIARA_FUNC_RESULT(IntegerPtr, result)
  KIARA_FUNC_ARG(ConstDataPtr, data)
)

KIARA_DECL_FUNC(StructTest_GetString,
  KIARA_FUNC_RESULT(DStringPtr, result)
  KIARA_FUNC_ARG(ConstDataPtr, data)
)

KIARA_DECL_FUNC(StructTest_GetLocation,
  KIARA_FUNC_RESULT(LocationPtr, location)
)

KIARA_DECL_FUNC(StructTest_SetLocation,
  KIARA_FUNC_ARG(ConstLocationPtr, location)
)

KIARA_DECL_FUNC(StructTest_ThrowException,
    KIARA_FUNC_ARG(KIARA_INT, code)
    KIARA_FUNC_ARG(KIARA_const_char_ptr, message)
    KIARA_FUNC_EXCEPTION(ExceptionPtr, exception)
)

int main(int argc, char **argv)
{
    KIARA_Context *ctx;
    KIARA_Connection *conn;
    const char *url = NULL;

    KIARA_FUNC_OBJ(StructTest_Pack) pack;
    KIARA_FUNC_OBJ(StructTest_GetInteger) getInteger;
    KIARA_FUNC_OBJ(StructTest_GetString) getString;
    KIARA_FUNC_OBJ(StructTest_GetLocation) getLocation;
    KIARA_FUNC_OBJ(StructTest_SetLocation) setLocation;
    KIARA_FUNC_OBJ(StructTest_ThrowException) throwException;

    int errorCode;

    /* This code is required for testing tool when compiled with MS CRT library and valgrind */
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    /* Initialize KIARA */
    kiaraInit(&argc, argv);

    ctx = kiaraNewContext();

    if (argc > 1)
    {
        url = argv[1];
    }
    else
    {
        url = "http://localhost:8080/service2";
    }

    /* Open connection to the service */
    printf("Opening connection to %s...\n", url);
    conn = kiaraOpenConnection(ctx, url);

    if (!conn)
    {
        fprintf(stderr, "Error: Could not open connection : %s\n", kiaraGetContextError(ctx));
        exit(1);
    }

    {
        KIARA_Type *ty = kiaraGetTypeFromDecl(conn, KIARA_TYPE(kr_dstring_t));
        kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(kr_dstring_t));
        kiaraDbgDumpType(ty);
    }

    {
        Data result;
        initData(&result);

        /* IDL -> native */
        pack = KIARA_GENERATE_CLIENT_FUNC(conn, "StructTest.pack", StructTest_Pack, "");
        if (pack) /* FIXME this should be always true */
        {
            errorCode = KIARA_CALL(pack, &result, 21, "test21");
            if (errorCode != KIARA_SUCCESS)
            {
                fprintf(stderr, "Error: call failed: (%i) %s: %s\n", errorCode, kiaraGetErrorName(errorCode), kiaraGetConnectionError(conn));
            }
            else
            {
                printf("StructTest.pack: result = { ival : %" PRIi64 ", sval : \"%s\" }\n",
                        result.ival,
                        kr_dstring_str(&result.sval));
            }
        }
        else
        {
            fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        }
        destroyData(&result);
    }

    {
        Data data;
        Integer result;

        initData(&data);
        data.ival = 25;
        kr_dstring_assign_str(&data.sval, "test25");

        /* IDL -> native */
        getInteger = KIARA_GENERATE_CLIENT_FUNC(conn, "StructTest.getInteger", StructTest_GetInteger, "");
        if (getInteger) /* FIXME this should be always true */
        {
            errorCode = KIARA_CALL(getInteger, &result, &data);
            if (errorCode != KIARA_SUCCESS)
            {
                fprintf(stderr, "Error: call failed: (%i) %s: %s\n", errorCode, kiaraGetErrorName(errorCode), kiaraGetConnectionError(conn));
            }
            else
            {
                printf("StructTest.getInteger: result = %" PRIi64 "\n", result);
            }
        }
        else
        {
            fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        }
        destroyData(&data);
    }

    {
        Data data;
        kr_dstring_t result;

        initData(&data);
        kr_dstring_init(&result);

        data.ival = 72;
        kr_dstring_assign_str(&data.sval, "test72");

        /* IDL -> native */
        getString = KIARA_GENERATE_CLIENT_FUNC(conn, "StructTest.getString", StructTest_GetString, "");
        if (getString) /* FIXME this should be always true */
        {
            errorCode = KIARA_CALL(getString, &result, &data);
            if (errorCode != KIARA_SUCCESS)
            {
                fprintf(stderr, "Error: call failed: (%i) %s: %s\n", errorCode, kiaraGetErrorName(errorCode), kiaraGetConnectionError(conn));
            }
            else
            {
                printf("StructTest.getString: result = %s\n", kr_dstring_str(&result));
            }
        }
        else
        {
            fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        }

        destroyData(&data);
        kr_dstring_destroy(&result);
    }

    {
        Location location;

        location.position.x = 0.5f;
        location.position.y = 10.5f;
        location.position.z = 8.0f;

        location.rotation.r = 0.707107f;
        location.rotation.v.x = 0.0f;
        location.rotation.v.y = 0.0f;
        location.rotation.v.z = 0.70710701f;

        /* IDL -> native */
        setLocation = KIARA_GENERATE_CLIENT_FUNC(conn, "StructTest.setLocation", StructTest_SetLocation, "");
        if (setLocation) /* FIXME this should be always true */
        {
            errorCode = KIARA_CALL(setLocation, &location);
            if (errorCode != KIARA_SUCCESS)
            {
                fprintf(stderr, "Error: call failed: (%i) %s: %s\n", errorCode, kiaraGetErrorName(errorCode), kiaraGetConnectionError(conn));
            }
            else
            {
                printf("StructTest.setLocation: DONE\n");
            }
        }
        else
        {
            fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        }
    }

    {
        Location location;
        memset(&location, 0, sizeof(location));

        /* IDL -> native */
        getLocation = KIARA_GENERATE_CLIENT_FUNC(conn, "StructTest.getLocation", StructTest_GetLocation, "");
        if (getLocation) /* FIXME this should be always true */
        {
            errorCode = KIARA_CALL(getLocation, &location);
            if (errorCode != KIARA_SUCCESS)
            {
                fprintf(stderr, "Error: call failed: (%i) %s: %s\n", errorCode, kiaraGetErrorName(errorCode), kiaraGetConnectionError(conn));
            }
            else
            {
                printf("StructTest.getLocation: result = position %f %f %f rotation %f %f %f %f\n",
                    location.position.x,
                    location.position.y,
                    location.position.z,
                    location.rotation.r,
                    location.rotation.v.x,
                    location.rotation.v.y,
                    location.rotation.v.z);
            }
        }
        else
        {
            fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        }
    }

    {
        Exception exc;
        initException(&exc);

        /* IDL -> native */
        throwException = KIARA_GENERATE_CLIENT_FUNC(conn, "StructTest.throwException", StructTest_ThrowException, "");
        if (throwException) /* FIXME this should be always true */
        {
            errorCode = KIARA_CALL(throwException, 1234, "error", &exc);
            if (errorCode == KIARA_EXCEPTION)
            {
                fprintf(stderr, "Server exception raised: %i %s\n", exc.errorCode, exc.errorMessage);
            }
            else if (errorCode != KIARA_SUCCESS)
            {
                fprintf(stderr, "Error: call failed: (%i) %s: %s\n", errorCode, kiaraGetErrorName(errorCode), kiaraGetConnectionError(conn));
            }
            else
            {
                printf("StructTest.throwException: succeed\n");
            }
        }
        else
        {
            fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        }

        destroyException(&exc);
    }

    kiaraCloseConnection(conn);

    kiaraFreeContext(ctx);

    kiaraFinalize();

    return 0;
}
