/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2012, 2013  German Research Center for Artificial Intelligence (DFKI)
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
 * calctest.c
 *
 *  Created on: 21.11.2012
 *      Author: Dmitri Rubinstein
 */
#include <KIARA/kiara.h>
#include <KIARA/kiara_macros.h>
#include <KIARA/CDT/kr_dstring.h>
#include "calctest_data_access.h"
#include <stdio.h>

KIARA_DECL_PTR(IntPtr, KIARA_INT)
KIARA_DECL_PTR(FloatPtr, KIARA_FLOAT)
KIARA_DECL_PTR(CharPtrPtr, KIARA_CHAR_PTR)


KIARA_DECL_OPAQUE_TYPE(kr_dstring_t,
       KIARA_USER_API(SetCString, dstring_SetCString)
       KIARA_USER_API(GetCString, dstring_GetCString))
KIARA_DECL_PTR(DStringPtr, kr_dstring_t)
KIARA_DECL_CONST_PTR(ConstDStringPtr, kr_dstring_t)

KIARA_DECL_FUNC(Calc_Add,
  KIARA_FUNC_RESULT(IntPtr, result)
  KIARA_FUNC_ARG(KIARA_INT, a)
  KIARA_FUNC_ARG(KIARA_INT, b)
)

KIARA_DECL_FUNC(Calc_Add_Float,
  KIARA_FUNC_RESULT(FloatPtr, result)
  KIARA_FUNC_ARG(KIARA_FLOAT, a)
  KIARA_FUNC_ARG(KIARA_FLOAT, b)
)

KIARA_DECL_FUNC(Calc_String_To_Int32,
  KIARA_FUNC_RESULT(IntPtr, result)
  KIARA_FUNC_ARG(KIARA_const_char_ptr, s)
)

KIARA_DECL_FUNC(Calc_Int32_To_String,
  KIARA_FUNC_RESULT(CharPtrPtr, result)
  KIARA_FUNC_ARG(KIARA_INT, i)
)

KIARA_DECL_FUNC(Calc_DString_To_Int32,
  KIARA_FUNC_RESULT(IntPtr, result)
  KIARA_FUNC_ARG(ConstDStringPtr, s)
)

KIARA_DECL_FUNC(Calc_Int32_To_DString,
  KIARA_FUNC_RESULT(DStringPtr, result)
  KIARA_FUNC_ARG(KIARA_INT, i)
)

int main(int argc, char **argv)
{
    KIARA_Context *ctx;
    KIARA_Connection *conn;
    const char *url = NULL;

    KIARA_FUNC_OBJ(Calc_Add) add;
    KIARA_FUNC_OBJ(Calc_Add_Float) addf;

    KIARA_FUNC_OBJ(Calc_String_To_Int32) strToInt;
    KIARA_FUNC_OBJ(Calc_Int32_To_String) intToStr;

    KIARA_FUNC_OBJ(Calc_DString_To_Int32) dstrToInt;
    KIARA_FUNC_OBJ(Calc_Int32_To_DString) intToDStr;
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
        url = "http://localhost:9090/service";
    }

    kiaraLoadLLVMModule(ctx, "calctest_data_access.bc");

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
        int result;
        /* IDL -> native */
        add = KIARA_GENERATE_CLIENT_FUNC(conn, "calc.add", Calc_Add, "");
        if (add) /* FIXME this should be always true */
        {
            errorCode = KIARA_CALL(add, &result, 21, 32);
            if (errorCode != KIARA_SUCCESS)
            {
                fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
            }
            else
            {
                printf("calc.add: result = %i\n", result);
            }
        }
        else
        {
            fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        }
    }

    {
        float result;
        addf = KIARA_GENERATE_CLIENT_FUNC(conn, "calc.addf", Calc_Add_Float, "");
        if (addf) /* FIXME this should be always true */
        {
            errorCode = KIARA_CALL(addf, &result, 21, 32);
            if (errorCode != KIARA_SUCCESS)
            {
                fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
            }
            else
            {
                printf("calc.addf: result = %f\n", result);
            }
        }
        else
        {
            fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        }
    }

    {
        int result;
        /* IDL -> native */
        strToInt = KIARA_GENERATE_CLIENT_FUNC(conn, "calc.stringToInt32", Calc_String_To_Int32, "");
        if (strToInt) /* FIXME this should be always true */
        {
            errorCode = KIARA_CALL(strToInt, &result, "-125");
            if (errorCode != KIARA_SUCCESS)
            {
                fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
            }
            else
            {
                printf("calc.stringToInt32: result = %i\n", result);
            }
        }
        else
        {
            fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        }
    }

    {
        char *result = NULL; /* must be initialized to NULL or malloc-allocated memory ! */
        /* IDL -> native */
        intToStr = KIARA_GENERATE_CLIENT_FUNC(conn, "calc.int32ToString", Calc_Int32_To_String, "");
        if (intToStr) /* FIXME this should be always true */
        {
            errorCode = KIARA_CALL(intToStr, &result, -42);
            if (errorCode != KIARA_SUCCESS)
            {
                fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
            }
            else
            {
                printf("calc.int32ToString: result = %s\n", result);
            }
        }
        else
        {
            fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        }
        free(result); /* deallocation is needed */
    }

    {
        int result;
        /* IDL -> native */
        dstrToInt = KIARA_GENERATE_CLIENT_FUNC(conn, "calc.stringToInt32", Calc_DString_To_Int32, "");
        if (dstrToInt) /* FIXME this should be always true */
        {
            kr_dstring_t value;
            kr_dstring_init(&value); /* must be initialized */

            kr_dstring_assign_str(&value, "521"); /* value to be passed */

            errorCode = KIARA_CALL(dstrToInt, &result, &value);
            if (errorCode != KIARA_SUCCESS)
            {
                fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
            }
            else
            {
                printf("calc.stringToInt32: result = %i\n", result);
            }
            kr_dstring_destroy(&value);
        }
        else
        {
            fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        }
    }

    {
        kr_dstring_t result;
        kr_dstring_init(&result); /* must be initialized */

        /* IDL -> native */
        intToDStr = KIARA_GENERATE_CLIENT_FUNC(conn, "calc.int32ToString", Calc_Int32_To_DString, "");
        if (intToDStr) /* FIXME this should be always true */
        {
            errorCode = KIARA_CALL(intToDStr, &result, 142);
            if (errorCode != KIARA_SUCCESS)
            {
                fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
            }
            else
            {
                printf("calc.int32ToString: result = %s\n", kr_dstring_str(&result));
            }
        }
        else
        {
            fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        }
        kr_dstring_destroy(&result); /* deallocation is needed */
    }

    kiaraCloseConnection(conn);

    kiaraFreeContext(ctx);

    kiaraFinalize();

    return 0;
}
