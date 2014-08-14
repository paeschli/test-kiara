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
 * arraytest.c
 *
 *  Created on: 21.11.2012
 *      Author: Dmitri Rubinstein
 */
#include <KIARA/kiara.h>
#include <KIARA/kiara_macros.h>
#include <KIARA/CDT/kr_dstring.h>
#include <stdio.h>
#include "arraytest_kiara.h"
#include "c99fmt.h"

int main(int argc, char **argv)
{
    KIARA_Context *ctx;
    KIARA_Connection *conn;
    const char *url = NULL;

    KIARA_FUNC_OBJ(SendData) send_data;
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

    {
        KIARA_Type *ty = kiaraGetTypeFromDecl(conn, KIARA_TYPE(Data));
        kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(Data));
        kiaraDbgDumpType(ty);
    }

    {
        size_t num, i;
        Data data;
        num = 50;
        initData(&data, num);
        for (i = 0; i < num; ++i)
        {
            data.array_boolean[i] = i % 2;
            data.array_i8[i] = i;
            data.array_u8[i] = i;
            data.array_i16[i] = i;
            data.array_u16[i] = i;
            data.array_i32[i] = i;
            data.array_u32[i] = i;
            data.array_i64[i] = i;
            data.array_u64[i] = i;
            data.array_float[i] = -(float)i;
            data.array_double[i] = -(double)i;
        }

        /* IDL -> native */
        send_data = KIARA_GENERATE_CLIENT_FUNC(conn, "arraytest.send", SendData, "");
        if (send_data) /* FIXME this should be always true */
        {
            errorCode = KIARA_CALL(send_data, &data, &data);
            if (errorCode != KIARA_SUCCESS)
            {
                fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
            }
            else
            {
                printf("arraytest.send: result = Data {\n");

#define PRINT_ARRAY(type, fmt)                                  \
                printf("array_" #type ": [ ");                  \
                for (i = 0; i < data.size_##type; ++i)          \
                {                                               \
                    printf("%" fmt " ", data.array_##type[i]);  \
                }                                               \
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

                printf("}\n");
            }
        }
        else
        {
            fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
        }
        destroyData(&data);
    }

    kiaraCloseConnection(conn);

    kiaraFreeContext(ctx);

    kiaraFinalize();

    return 0;
}
