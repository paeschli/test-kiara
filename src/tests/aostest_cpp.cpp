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
 * aostest_cpp.cpp
 *
 *  Created on: Dec 21, 2013
 *      Author: Dmitri Rubinstein
 */

/*
 * This file contains client implementation of aostest (Array-Of-Structures Test) example
 * implemented in C with KIARA framework.
 */

#include <KIARA/kiara.h>
#include <KIARA/kiara_cxx_macros.hpp>

#include <stdio.h>
#include <string.h>
#include "c99fmt.h"

#include "aostest_types.h"

//KIARA_CXX_DECL_ANNOTATED(Vec3fA, Vec3f, NONE, NONE)

KIARA_CXX_DECL_STRUCT(Vec3f,
  KIARA_CXX_STRUCT_MEMBER(x)
  KIARA_CXX_STRUCT_MEMBER(y)
  KIARA_CXX_STRUCT_MEMBER(z)
)
KIARA_CXX_DECL_STRUCT(Quatf,
  KIARA_CXX_STRUCT_MEMBER(r)
  KIARA_CXX_STRUCT_MEMBER(v)
)
KIARA_CXX_DECL_STRUCT(Location,
  KIARA_CXX_STRUCT_MEMBER(position)
  KIARA_CXX_STRUCT_MEMBER(rotation)
)

KIARA_CXX_DECL_STRUCT(LocationList,
  KIARA_CXX_STRUCT_ARRAY_MEMBER(locations, num_locations)
)

KIARA_CXX_DECL_FUNC(AOSTest_SetLocations,
  KIARA_CXX_FUNC_ARG(const LocationList *, locations)
)

KIARA_CXX_DECL_FUNC(AOSTest_GetLocations,
  KIARA_CXX_FUNC_RESULT(LocationList *, locations)
)

/*
 * KIARA context and connection variables.
 */

KIARA_Context *ctx;
KIARA_Connection *conn;

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

    set_locations = KIARA_GENERATE_CLIENT_FUNC(conn, "aostest.setLocations", AOSTest_SetLocations, "");
    if (!set_locations)
    {
        fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
    }

    get_locations = KIARA_GENERATE_CLIENT_FUNC(conn, "aostest.getLocations", AOSTest_GetLocations, "");
    if (!get_locations)
    {
        fprintf(stderr, "Error: code generation failed: %s\n", kiaraGetConnectionError(conn));
    }
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

        errorCode = KIARA_CALL(set_locations, &loclist);
        if (errorCode != KIARA_SUCCESS)
        {
            fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
        }
        else
        {
            printf("aostest.setLocations: DONE\n");
        }
        destroyLocationList(&loclist);
    }

    /* Receive locations stored on the server, and print them */
    {
        size_t i;
        LocationList loclist;

        initLocationList(&loclist, 0);

        errorCode = KIARA_CALL(get_locations, &loclist);
        if (errorCode != KIARA_SUCCESS)
        {
            fprintf(stderr, "Error: call failed: %s\n", kiaraGetConnectionError(conn));
        }
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
