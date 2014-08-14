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
 * aostest.c
 *
 *  Created on: Dec 2, 2013
 *      Author: Dmitri Rubinstein
 */

/*
 * This file contains client implementation of aostest (Array-Of-Structures Test) example
 * implemented in C with KIARA framework.
 */

#include <KIARA/kiara.h>
#include <KIARA/kiara_macros.h>
#include <KIARA/kiara_pp_annotation.h>

#include "aostest_types.h"
#include "aostest_kiara_client.h"

#include <stdio.h>
#include <string.h>
#include "c99fmt.h"

/*
 * Declare application structures and client functions of the application.
 * Following macros are processed by kiara-preprocessor tool and result
 * is output to the aostest_kiara_client.h file.
 */

/* kiara_declare_struct_with_api(type_name, ...)
 *
 * Declares non-trivial structure type type_name, which require custom behavior via
 * user specified API functions.
 *
 * kiara_struct_array_member(ptr_member_name, size_member_name)
 *
 * Declares member in a structure that represents a C-array composed from
 * pointer to the array field and integer array size field.
 *
 * kiara_user_api(api_name, user_function_name)
 *
 * Registers user-defined API function user_function_name with a predefined KIARA API api_name
 *
 * In our example we need to declare LocationList structure explicitly because it
 * contains C-array composed from locations pointer and num_locations integer and
 * because LocationList requires custom allocation and deallocation functions.
 * Both can't be deduced automatically from the source code.
 */
kiara_declare_struct(LocationList,
    kiara_struct_array_member(locations, num_locations))

/* kiara_declare_func(func_name, ...)
 *
 * Declares remote function that can be called by the client.
 * All arguments after the function name are argument types and names
 * to the function.
 *
 * Usually types used as arguments do not need to be explicitly declared.
 * Only when types require custom handling an explicit declaration is required.
 */
kiara_declare_func(AOSTest_SetLocations, const LocationList * locations)
kiara_declare_func(AOSTest_GetLocations, LocationList * result_value locations)

/*
 * KIARA context and connection variables.
 *
 * KIARA_Context is used for all KIARA operations issued from the single thread.
 * Each separate thread require a separate KIARA_Context instance.
 *
 * KIARA_Connection is a handle to the remote endpoint
 * over which remote calls are performed.
 */
KIARA_Context *ctx;
KIARA_Connection *conn;

/*
 * set_locations and get_locations are handles to the function objects
 * that perform remote call.
 * They are dynamically generated by the KIARA_GENERATE_CLIENT_FUNC macro.
 */
KIARA_FUNC_OBJ(AOSTest_SetLocations) set_locations;
KIARA_FUNC_OBJ(AOSTest_GetLocations) get_locations;

/*
 * Initialization of the connection
 */
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

    /*
     * KIARA_GENERATE_CLIENT_FUNC(connection, idl_method_name, func_type_name, mapping)
     *
     * Generates function that will perform a remote call.
     *
     * connection       - opened and valid KIARA_Connection handle.
     * idl_method_name  - name of the remote service method specified in the IDL.
     * func_type_name   - name of the function object type declared
     *                    with the KIARA_FUNC_OBJ(func_type_name) macro.
     * mapping          - mapping of the IDL types to the application types.
     *                    By default 1:1 mapping by names and types is used.
     *                    Note: mapping is not implemented yet.
     *
     * Note: The IDL of the server application is embedded in aostest_server.c.
     */

    set_locations = KIARA_GENERATE_CLIENT_FUNC(conn, "aostest.setLocations", AOSTest_SetLocations, "");
    if (!set_locations)
        fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));

    get_locations = KIARA_GENERATE_CLIENT_FUNC(conn, "aostest.getLocations", AOSTest_GetLocations, "");
    if (!get_locations)
        fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
}

/*
 * Close connection and finalize KIARA framework
 */
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
        url = argv[1];
    else
        url = "http://localhost:8080/service";

    /* Initialize connection and generate functions */
    initConn(url);

    /* Call remote functions */

    /* Send 10 locations to the server, where they will be stored */
    {
        size_t num, i;
        LocationList loclist;
        num = 10;
        initLocationList(&loclist, num);
        for (i = 0; i < num; ++i)
        {
            loclist.locations[i].position.x = i;
            loclist.locations[i].position.y = i;
            loclist.locations[i].position.z = i;

            loclist.locations[i].rotation.r = 0.707107f;
            loclist.locations[i].rotation.v.x = 0.0f;
            loclist.locations[i].rotation.v.y = 0.0f;
            loclist.locations[i].rotation.v.z = 0.70710701f;
        }

        /*
         * KIARA_CALL(funcobj, ...)
         *
         * Will call remote function via function object generated by KIARA_GENERATE_CLIENT_FUNC macro.
         * All arguments after function objects are input/output arguments to the remote function.
         * KIARA_CALL returns integer value of type KIARA_Result that represent an error code.
         */

        errorCode = KIARA_CALL(set_locations, &loclist);
        if (errorCode != KIARA_SUCCESS)
            fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
        else
            printf("aostest.setLocations: DONE\n");
        destroyLocationList(&loclist);
    }

    /* Receive locations stored on the server, and print them */
    {
        size_t i;
        LocationList loclist;

        initLocationList(&loclist, 0);

        errorCode = KIARA_CALL(get_locations, &loclist);
        if (errorCode != KIARA_SUCCESS)
            fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
        else
        {
            printf("aostest.getLocations: LocationList {\n");
            printf("  locations: [\n");
            for (i = 0; i < loclist.num_locations; ++i)
            {
                printf("    position %f %f %f rotation %f %f %f %f\n",
                    loclist.locations[i].position.x,
                    loclist.locations[i].position.y,
                    loclist.locations[i].position.z,
                    loclist.locations[i].rotation.r,
                    loclist.locations[i].rotation.v.x,
                    loclist.locations[i].rotation.v.y,
                    loclist.locations[i].rotation.v.z);
            }
            printf("  ]\n");
            printf("}\n");
        }

        destroyLocationList(&loclist);
    }

    /* Finalize */

    finalizeConn();

    return 0;
}