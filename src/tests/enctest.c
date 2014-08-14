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
 * enctest.c
 *
 *  Created on: Aug 14, 2013
 *      Author: Dmitri Rubinstein
 */
#include <KIARA/kiara.h>
#include <KIARA/kiara_macros.h>
#include <KIARA/CDT/kr_dstring.h>
#include <stdio.h>

KIARA_DECL_PTR(IntPtr, KIARA_INT)

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

KIARA_DECL_ENCRYPTED(EncDString, kr_dstring_t)

KIARA_DECL_FUNC(SendInt,
  KIARA_FUNC_RESULT(IntPtr, result)
  KIARA_FUNC_ARG(KIARA_INT, i)
)

KIARA_DECL_FUNC(SendString,
  KIARA_FUNC_RESULT(DStringPtr, result)
  KIARA_FUNC_ARG(ConstDStringPtr, s)
  KIARA_FUNC_ARG(KIARA_INT, i)
)

int main(int argc, char **argv)
{
    KIARA_Context *ctx;
    KIARA_Connection *conn;
    const char *url = NULL;

    KIARA_FUNC_OBJ(SendInt) sendInt;
    KIARA_FUNC_OBJ(SendString) sendString;

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
        url = "http://localhost:8080/service";
    }

    /* Open connection to the service */
    printf("Opening connection to %s...\n", url);
    conn = kiaraOpenConnection(ctx, url);

    if (!conn)
    {
        fprintf(stderr, "Error: Could not open connection : %s\n", kiaraGetContextError(ctx));
        exit(1);
    }

#if 0
    {
        KIARA_Type *ty = kiaraGetTypeFromDecl(conn, KIARA_TYPE(SendEncString));
        kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(SendEncString));
        kiaraDbgDumpType(ty);
    }
#endif

    {
        int result;
        /* IDL -> native */
        sendInt = KIARA_GENERATE_CLIENT_FUNC(conn, "enc.sendInt", SendInt, "");
        if (sendInt) /* FIXME this should be always true */
        {
            errorCode = KIARA_CALL(sendInt, &result, 321);
            if (errorCode != KIARA_SUCCESS)
            {
                fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
            }
            else
            {
                printf("enc.sendInt: result = %i\n", result);
            }
        }
        else
        {
            fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        }
    }

    {
        kr_dstring_t result;
        kr_dstring_init(&result);

        /* IDL -> native */
        sendString = KIARA_GENERATE_CLIENT_FUNC(conn, "enc.sendString", SendString, "");
        if (sendString) /* FIXME this should be always true */
        {
            kr_dstring_t value;
            kr_dstring_init(&value); /* must be initialized */

            kr_dstring_assign_str(&value, "data321"); /* value to be passed */

            errorCode = KIARA_CALL(sendString, &result, &value, 321);
            if (errorCode != KIARA_SUCCESS)
            {
                fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
            }
            else
            {
                printf("enc.sendString: result = %s\n", kr_dstring_str(&result));
            }
            kr_dstring_destroy(&value);
        }
        else
        {
            fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        }
        kr_dstring_destroy(&result);
    }

    kiaraCloseConnection(conn);

    kiaraFreeContext(ctx);

    kiaraFinalize();

    return 0;
}
