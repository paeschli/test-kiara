/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2013, 2014  German Research Center for Artificial Intelligence (DFKI)
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
 * KiaraTypedSubscriber.c
 *
 *  Created on: Mar 18, 2014
 *      Author: Dmitri Rubinstein
 */

#include <KIARA/kiara.h>
#include <KIARA/kiara_macros.h>
#include <KIARA/kiara_pp_annotation.h>
#include "kiara_server_decls.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "c99fmt.h"

kiara_declare_service(Benchmark_SendIntImpl, int32_t * result_value result, int32_t value)

/** Service implementation */

/* Receive location list sent by the client and store it to the objectLocations variable */
KIARA_Result benchmark_sendint_impl(KIARA_ServiceFuncObj *kiara_funcobj, int32_t * result, int32_t value)
{
    *result = value;

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
            "namespace * benchmark "
            " "
            "service benchmark { "
            "  i32 sendInt(i32 value); "
            "} ");
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: could not parse IDL: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    printf("Register benchmark.sendInt ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "benchmark.sendInt", Benchmark_SendIntImpl, "", benchmark_sendint_impl);
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

    //result = kiaraAddService(server, "/rpc/benchmark", protocol, service);
    result = kiaraAddService(server, "tcp://0.0.0.0:53212", protocol, service);
    if (result != KIARA_SUCCESS)
        fprintf(stderr, "Error: could not add service: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServerError(server));

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

    return 0;
}
