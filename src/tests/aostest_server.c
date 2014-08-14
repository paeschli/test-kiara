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
 * aostest_server.c
 *
 *  Created on: Dec 2, 2013
 *      Author: Dmitri Rubinstein
 */

/*
 * This file contains server implementation of aostest (Array-Of-Structures Test) example
 * implemented in C with KIARA framework.
 */

#include <KIARA/kiara.h>
#include <KIARA/kiara_macros.h>
#include <KIARA/kiara_pp_annotation.h>

#include "aostest_types.h"
#include "aostest_kiara_server.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "c99fmt.h"

/*
 * Declare application structures and service functions of the application.
 * Following macros are processed by kiara-preprocessor tool and result
 * is output to the aostest_kiara_server.h file.
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

/* kiara_declare_service(service_name, ...)
 *
 * Declares service function type that can be called by the client.
 * All arguments after the function name are argument types and names
 * to the function.
 *
 * Usually types used as arguments do not need to be explicitly declared.
 * Only when types require custom handling an explicit declaration is required.
 */
kiara_declare_service(AOSTest_GetLocationsImpl, LocationList * result_value locations)
kiara_declare_service(AOSTest_SetLocationsImpl, const LocationList * locations)

/** Service implementation */

/* In objectLocations list are stored locations sent by the client */
LocationList objectLocations = {0, NULL};

void copyLocationList(LocationList *dest, const LocationList *src)
{
    if (dest->num_locations != src->num_locations)
    {
        destroyLocationList(dest);
        initLocationList(dest, src->num_locations);
    }
    memcpy(dest->locations, src->locations, sizeof(src->locations[0]) * src->num_locations);
}

/* Receive location list sent by the client and store it to the objectLocations variable */
KIARA_Result aostest_set_locations_impl(KIARA_ServiceFuncObj *kiara_funcobj, const LocationList *locations)
{
    size_t i;
    size_t num = locations->num_locations;
    for (i = 0; i < num; ++i)
    {
        printf("Location.position %f %f %f\nLocation.rotation %f %f %f %f\n",
                locations->locations[i].position.x,
                locations->locations[i].position.y,
                locations->locations[i].position.z,
                locations->locations[i].rotation.r,
                locations->locations[i].rotation.v.x,
                locations->locations[i].rotation.v.y,
                locations->locations[i].rotation.v.z);
    }

    copyLocationList(&objectLocations, locations);

    return KIARA_SUCCESS;
}

/* Return location list stored in the objectLocations variable back to the client */
KIARA_Result aostest_get_locations_impl(KIARA_ServiceFuncObj *kiara_funcobj, LocationList *locations)
{
    copyLocationList(locations, &objectLocations);

    return KIARA_SUCCESS;
}

int main(int argc, char **argv)
{
    /*
     * KIARA context and connection variables.
     *
     * KIARA_Context is used for all KIARA operations issued from the single thread.
     * Each separate thread require a separate KIARA_Context instance.
     *
     * KIARA_Service is a handle to the service which provides implementation
     * of service methods specified in the IDL.
     *
     * KIARA_Server is a handle to the server which can host multiple services.
     */

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

    if (argc > 1)
        port = argv[1];
    else
        port = "8080";

    if (argc > 2)
        protocol = argv[2];
    else
        protocol = "jsonrpc";

    printf("Server port: %s\n", port);
    printf("Protocol: %s\n", protocol);

    /* Create new context */

    ctx = kiaraNewContext();

    /* Create a new service */

    service = kiaraNewService(ctx);

    /* Add IDL to the service */

    result = kiaraLoadServiceIDLFromString(service,
            "KIARA",
            "namespace * aostest "
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
            "struct LocationList { "
            " array<Location> locations "
            "} "
            "service aostest { "
            "  void setLocations(LocationList locations); "
            "  LocationList getLocations(); "
            "} ");
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: could not parse IDL: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    printf("Register aostest.setLocations ...\n");

    /*
     * KIARA_REGISTER_SERVICE_FUNC(service, idl_method_name,
     *                             service_type_name, mapping, service_func_impl)
     *
     * Registers service function implementation with a specified IDL service method.
     *
     * service           - valid KIARA_Service handle.
     * idl_method_name   - name of the remote service method specified in the IDL.
     * service_type_name - name of the service type declared with the KIARA_DECL_SERVICE macro.
     * mapping           - mapping of the IDL types to the application types.
     *                     By default 1:1 mapping by names and types is used.
     *                     Note: mapping is not implemented yet.
     * service_func_impl - user function that implements service method.
     */

    result = KIARA_REGISTER_SERVICE_FUNC(service, "aostest.setLocations", AOSTest_SetLocationsImpl, "", aostest_set_locations_impl);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    printf("Register aostest.getLocations ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "aostest.getLocations", AOSTest_GetLocationsImpl, "", aostest_get_locations_impl);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    /*
     * Create new server and register service
     */

    server = kiaraNewServer(ctx, "0.0.0.0", atoi(port), "/service");

    kiaraAddService(server, "/rpc/aostest", protocol, service);

    printf("Starting server...\n");

    /* Run server */

    result = kiaraRunServer(server);
    if (result != KIARA_SUCCESS)
        fprintf(stderr, "Error: could not start server: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServerError(server));

    /* Free everything */

    kiaraFreeServer(server);
    kiaraFreeService(service);
    kiaraFreeContext(ctx);
    kiaraFinalize();

    /* Free temporary copy of location list */
    destroyLocationList(&objectLocations);

    return 0;
}
