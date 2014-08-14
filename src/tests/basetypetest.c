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
 * basetypetest.c
 *
 *  Created on: 28.11.2013
 *      Author: Dmitri Rubinstein
 */
#include <KIARA/kiara.h>
#include <KIARA/kiara_macros.h>
#include <KIARA/CDT/kr_dstring.h>
#include <stdio.h>
#include "c99fmt.h"

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

KIARA_DECL_FUNC(Basetype_Send_boolean,
    KIARA_FUNC_RESULT(IntPtr, result)
    KIARA_FUNC_ARG(KIARA_INT, a))

KIARA_DECL_FUNC(Basetype_Send_i8,
    KIARA_FUNC_RESULT(Int8Ptr, result)
    KIARA_FUNC_ARG(KIARA_INT8_T, a))

KIARA_DECL_FUNC(Basetype_Send_u8,
    KIARA_FUNC_RESULT(UInt8Ptr, result)
    KIARA_FUNC_ARG(KIARA_UINT8_T, a))

KIARA_DECL_FUNC(Basetype_Send_i16,
    KIARA_FUNC_RESULT(Int16Ptr, result)
    KIARA_FUNC_ARG(KIARA_INT16_T, a))

KIARA_DECL_FUNC(Basetype_Send_u16,
    KIARA_FUNC_RESULT(UInt16Ptr, result)
    KIARA_FUNC_ARG(KIARA_UINT16_T, a))

KIARA_DECL_FUNC(Basetype_Send_i32,
    KIARA_FUNC_RESULT(Int32Ptr, result)
    KIARA_FUNC_ARG(KIARA_INT32_T, a))

KIARA_DECL_FUNC(Basetype_Send_u32,
    KIARA_FUNC_RESULT(UInt32Ptr, result)
    KIARA_FUNC_ARG(KIARA_UINT32_T, a))

KIARA_DECL_FUNC(Basetype_Send_i64,
    KIARA_FUNC_RESULT(Int64Ptr, result)
    KIARA_FUNC_ARG(KIARA_INT64_T, a))

KIARA_DECL_FUNC(Basetype_Send_u64,
    KIARA_FUNC_RESULT(UInt64Ptr, result)
    KIARA_FUNC_ARG(KIARA_UINT64_T, a))

KIARA_DECL_FUNC(Basetype_Send_float,
    KIARA_FUNC_RESULT(FloatPtr, result)
    KIARA_FUNC_ARG(KIARA_FLOAT, a))

KIARA_DECL_FUNC(Basetype_Send_double,
    KIARA_FUNC_RESULT(DoublePtr, result)
    KIARA_FUNC_ARG(KIARA_DOUBLE, a))

KIARA_DECL_FUNC(Basetype_Send_string,
      KIARA_FUNC_RESULT(CharPtrPtr, result)
      KIARA_FUNC_ARG(KIARA_CHAR_PTR, a))

KIARA_Context *ctx;
KIARA_Connection *conn;

KIARA_FUNC_OBJ(Basetype_Send_boolean) send_boolean;

KIARA_FUNC_OBJ(Basetype_Send_i8) send_i8;
KIARA_FUNC_OBJ(Basetype_Send_u8) send_u8;

KIARA_FUNC_OBJ(Basetype_Send_i16) send_i16;
KIARA_FUNC_OBJ(Basetype_Send_u16) send_u16;

KIARA_FUNC_OBJ(Basetype_Send_i32) send_i32;
KIARA_FUNC_OBJ(Basetype_Send_u32) send_u32;

KIARA_FUNC_OBJ(Basetype_Send_i64) send_i64;
KIARA_FUNC_OBJ(Basetype_Send_u64) send_u64;

KIARA_FUNC_OBJ(Basetype_Send_float) send_float;
KIARA_FUNC_OBJ(Basetype_Send_double) send_double;

KIARA_FUNC_OBJ(Basetype_Send_string) send_string;

void initConn(const char *url)
{
    /* Create new context */

    ctx = kiaraNewContext();

    /* Open connection to the service */

    printf("Opening connection to %s...\n", url);
    conn = kiaraOpenConnection(ctx, url);

    if (!conn)
    {
        fprintf(stderr, "Error: Could not open connection : %s\n", kiaraGetContextError(ctx));
        exit(1);
    }

    /* Generate remote functions */

    send_boolean = KIARA_GENERATE_CLIENT_FUNC(conn, "basetype.send_boolean", Basetype_Send_boolean, "");
    if (!send_boolean)
    {
        fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        exit(1);
    }

    send_i8 = KIARA_GENERATE_CLIENT_FUNC(conn, "basetype.send_i8", Basetype_Send_i8, "");
    if (!send_i8)
    {
        fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        exit(1);
    }

    send_u8 = KIARA_GENERATE_CLIENT_FUNC(conn, "basetype.send_u8", Basetype_Send_u8, "");
    if (!send_u8)
    {
        fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        exit(1);
    }

    send_i16 = KIARA_GENERATE_CLIENT_FUNC(conn, "basetype.send_i16", Basetype_Send_i16, "");
    if (!send_i16)
    {
        fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        exit(1);
    }

    send_u16 = KIARA_GENERATE_CLIENT_FUNC(conn, "basetype.send_u16", Basetype_Send_u16, "");
    if (!send_u16)
    {
        fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        exit(1);
    }

    send_i32 = KIARA_GENERATE_CLIENT_FUNC(conn, "basetype.send_i32", Basetype_Send_i32, "");
    if (!send_i32)
    {
        fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        exit(1);
    }

    send_u32 = KIARA_GENERATE_CLIENT_FUNC(conn, "basetype.send_u32", Basetype_Send_u32, "");
    if (!send_u32)
    {
        fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        exit(1);
    }

    send_i64 = KIARA_GENERATE_CLIENT_FUNC(conn, "basetype.send_i64", Basetype_Send_i64, "");
    if (!send_i64)
    {
        fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        exit(1);
    }

    send_u64 = KIARA_GENERATE_CLIENT_FUNC(conn, "basetype.send_u64", Basetype_Send_u64, "");
    if (!send_u64)
    {
        fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        exit(1);
    }

    send_float = KIARA_GENERATE_CLIENT_FUNC(conn, "basetype.send_float", Basetype_Send_float, "");
    if (!send_float)
    {
        fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        exit(1);
    }

    send_double = KIARA_GENERATE_CLIENT_FUNC(conn, "basetype.send_double", Basetype_Send_double, "");
    if (!send_double)
    {
        fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        exit(1);
    }

    send_string = KIARA_GENERATE_CLIENT_FUNC(conn, "basetype.send_string", Basetype_Send_string, "");
    if (!send_string)
    {
        fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        exit(1);
    }
}

void finalizeConn()
{
    kiaraCloseConnection(conn);

    kiaraFreeContext(ctx);

    kiaraFinalize();
}

int main(int argc, char **argv)
{
    const char *url = NULL;

    int errorCode;

    /* This code is required for testing tool when compiled with MS CRT library and valgrind */
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    /* Initialize KIARA */
    kiaraInit(&argc, argv);

    if (argc > 1)
    {
        url = argv[1];
    }
    else
    {
        url = "http://localhost:8080/service";
    }

    /* Initialize connection and generate functions */
    initConn(url);

    /* Call remote functions */

    {
        int result;
        errorCode = KIARA_CALL(send_boolean, &result, 1);
        if (errorCode != KIARA_SUCCESS)
        {
            fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
        }
        else
        {
            printf("basetype.send_boolean: result = %i\n", result);
        }
    }

    {
        int8_t result;
        errorCode = KIARA_CALL(send_i8, &result, -128);
        if (errorCode != KIARA_SUCCESS)
        {
            fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
        }
        else
        {
            printf("basetype.send_i8: result = %" PRIi8 "\n", (int)result);
        }
    }

    {
        uint8_t result;
        errorCode = KIARA_CALL(send_u8, &result, 255);
        if (errorCode != KIARA_SUCCESS)
        {
            fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
        }
        else
        {
            printf("basetype.send_u8: result = %" PRIu8 "\n", (int)result);
        }
    }

    {
        int16_t result;
        errorCode = KIARA_CALL(send_i16, &result, -32768);
        if (errorCode != KIARA_SUCCESS)
        {
            fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
        }
        else
        {
            printf("basetype.send_i16: result = %" PRIi16 "\n", (int)result);
        }
    }

    {
        uint16_t result;
        errorCode = KIARA_CALL(send_u16, &result, 65535);
        if (errorCode != KIARA_SUCCESS)
        {
            fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
        }
        else
        {
            printf("basetype.send_u16: result = %" PRIu16 "\n", result);
        }
    }

    {
        int32_t result;
        errorCode = KIARA_CALL(send_i32, &result, -2147483648);
        if (errorCode != KIARA_SUCCESS)
        {
            fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
        }
        else
        {
            printf("basetype.send_i32: result = %" PRIi32 "\n", result);
        }
    }

    {
        uint32_t result;
        errorCode = KIARA_CALL(send_u32, &result, 4294967295);
        if (errorCode != KIARA_SUCCESS)
        {
            fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
        }
        else
        {
            printf("basetype.send_u32: result = %" PRIu32 "\n", result);
        }
    }

    {
        int64_t result;
        errorCode = KIARA_CALL(send_i64, &result, -9223372036854775807LL-1);
        if (errorCode != KIARA_SUCCESS)
        {
            fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
        }
        else
        {
            printf("basetype.send_i64: result = %" PRIi64 "\n", result);
        }
    }

    {
        uint64_t result;
        errorCode = KIARA_CALL(send_u64, &result, 18446744073709551615UL);
        if (errorCode != KIARA_SUCCESS)
        {
            fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
        }
        else
        {
            printf("basetype.send_u64: result = %" PRIu64 "\n", result);
        }
    }

    {
        float result;
        errorCode = KIARA_CALL(send_float, &result, 64.132004);
        if (errorCode != KIARA_SUCCESS)
        {
            fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
        }
        else
        {
            printf("basetype.send_float: result = %f\n", result);
        }
    }

    {
        double result;
        errorCode = KIARA_CALL(send_double, &result, 21164.103021);
        if (errorCode != KIARA_SUCCESS)
        {
            fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
        }
        else
        {
            printf("basetype.send_double: result = %f\n", result);
        }
    }

    {
        char *result = NULL; /* must be initialized to NULL or malloc-allocated memory ! */
        errorCode = KIARA_CALL(send_string, &result, "test string");
        if (errorCode != KIARA_SUCCESS)
        {
            fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
        }
        else
        {
            printf("basetype.send_string: result = %s\n", result);
        }
        free(result); /* deallocation is needed */
    }

    /* Finalize */

    finalizeConn();

    return 0;
}
