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
 *  Created on: Mar 9, 2014
 *      Author: Dmitri Rubinstein
 */

#include <KIARA/kiara.h>
#include <KIARA/kiara_macros.h>
#include <KIARA/kiara_pp_annotation.h>

#include "MarketData.h"
#include "QuoteRequest.h"
#include "kiara_server_decls.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "c99fmt.h"

kiara_declare_struct(MarketData,
    kiara_struct_array_member(mdEntries, num_mdEntries))

kiara_declare_struct(QuoteRequest,
    kiara_struct_array_member(related, num_related))

kiara_declare_service(Benchmark_MarketDataImpl, MarketData * result_value result, const MarketData *marketData)
kiara_declare_service(Benchmark_QuoteRequestImpl, QuoteRequest * result_value result, const QuoteRequest *quoteRequest)

/** Service implementation */

const size_t NUM_MESSAGES = 1000;
size_t msg_counter = 0;

/* Receive location list sent by the client and store it to the objectLocations variable */
KIARA_Result benchmark_marketdata_impl(KIARA_ServiceFuncObj *kiara_funcobj, MarketData * result, const MarketData *marketData)
{
    size_t counter = msg_counter % NUM_MESSAGES;
    ++msg_counter;

    MarketData_copy(result, marketData);
    result->isEcho = 1;

    if (counter != result->counter)
        return KIARA_FAILURE;

    return KIARA_SUCCESS;
}

/* Return location list stored in the objectLocations variable back to the client */
KIARA_Result benchmark_quoterequest_impl(KIARA_ServiceFuncObj *kiara_funcobj, QuoteRequest * result, const QuoteRequest *quoteRequest)
{
    size_t counter = msg_counter % NUM_MESSAGES;
    ++msg_counter;

    QuoteRequest_copy(result, quoteRequest);
    result->isEcho = 1;

    if (counter != result->counter)
        return KIARA_FAILURE;

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
            "struct MarketDataEntry { "
            "u32 mdUpdateAction; "
            "u32 mdPriceLevel; "
            "double        mdEntryType; "
            "u32 openCloseSettleFlag; "
            "u32 securityIDSource; "
            "u32 securityID; "
            "u32 rptSeq; "
            "double        mdEntryPx; "
            "u32 mdEntryTime; "
            "u32          mdEntrySize; " // in original i32
            "u32 numberOfOrders; "
            "double        tradingSessionID; "
            "double        netChgPrevDay; "
            "u32 tradeVolume; "
            "double        tradeCondition; "
            "double        tickDirection; "
            "double        quoteCondition; "
            "u32 aggressorSide; "
            "double        matchEventIndicator; "
            ""
            "double dummy1; "
            "float dummy2; "
            "} "
            ""
            "struct MarketData { "
            "boolean    isEcho; "
            "u32        counter; "
            "u32        securityID; "
            "double    applVersionID; "
            "double    messageType; "
            "double    senderCompID; "
            "u32       msgSeqNum; "
            "u32       sendingTime; "
            "u32       tradeDate; "
            "array<MarketDataEntry>  mdEntries; "
            "} "
            ""
            "struct RelatedSym { "
            "double    symbol; "
            "u64       orderQuantity; "
            "u32    side; "
            "u64    transactTime; "
            "u32    quoteType; "
            "u32      securityID; "
            "u32      securityIDSource; "
            "double dummy1; "
            "float dummy2; "
            "} "
            "struct QuoteRequest { "
            "boolean            isEcho; "
            "u32      counter; "
            "u32      securityID; "
            "double             applVersionID; "
            "double             messageType; "
            "double             senderCompID; "
            "u32      msgSeqNum; "
            "u32      sendingTime; "
            "double             quoteReqID; "
            "array<RelatedSym>        related; "
            "} "
            ""
            "service benchmark { "
            "  MarketData marketData(MarketData marketData); "
            "  QuoteRequest quoteRequest(QuoteRequest quoteRequest); "
            "} ");
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: could not parse IDL: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    printf("Register benchmark.marketData ...\n");

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

    result = KIARA_REGISTER_SERVICE_FUNC(service, "benchmark.marketData", Benchmark_MarketDataImpl, "", benchmark_marketdata_impl);
    if (result != KIARA_SUCCESS)
    {
        fprintf(stderr, "Error: registration failed: %s: %s\n",
                kiaraGetErrorName(result), kiaraGetServiceError(service));
        exit(1);
    }

    printf("Register benchmark.quoteRequest ...\n");

    result = KIARA_REGISTER_SERVICE_FUNC(service, "benchmark.quoteRequest", Benchmark_QuoteRequestImpl, "", benchmark_quoterequest_impl);
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
