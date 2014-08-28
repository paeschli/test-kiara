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
 * URLLoader.cpp
 *
 *  Created on: 23.04.2013
 *      Author: Dmitri Rubinstein
 */
#define KIARA_LIB
#include "URLLoader.hpp"
#include <DFC/Base/Utils/StaticInit.hpp>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include "../Transport/KT_Zeromq.hpp"
#include "../Transport/KT_Msg.hpp"
#include "../Transport/KT_HTTP_Parser.hpp"
#include "../Transport/KT_HTTP_Responder.hpp"
#include "../Transport/KT_HTTP_Requester.hpp"
#include "../Utils/URL.hpp"
#include <iostream>
#include <iomanip>
#include <unistd.h>


// for debugging
// #define SSL_DEBUG

namespace KIARA
{

// for curl debugging
static size_t writeToString(void *buf, size_t size, size_t nmemb, void *userp)
{
    if (userp)
    {
        std::string *dstr = (std::string *)userp;
        size_t len = size * nmemb;
        dstr->append((char*)buf, len);
        return len;
    }

    return 0;
}


struct data
{
    char trace_ascii; /* 1 or 0 */
};

#ifdef SSL_DEBUG
static void dump(const char *text, FILE *stream, unsigned char *ptr, size_t size, char nohex)
{
    size_t i;
    size_t c;

    unsigned int width = 0x10;

    if (nohex)
        /* without the hex output, we can fit more on screen */
        width = 0x40;

    fprintf(stream, "%s, %10.10ld bytes (0x%8.8lx)\n", text, (long) size, (long) size);

    for (i = 0; i < size; i += width)
    {

        fprintf(stream, "%4.4lx: ", (long) i);

        if (!nohex)
        {
            /* hex not disabled, show it */
            for (c = 0; c < width; c++)
                if (i + c < size)
                    fprintf(stream, "%02x ", ptr[i + c]);
                else
                    fputs("   ", stream);
        }

        for (c = 0; (c < width) && (i + c < size); c++)
        {
            /* check for 0D0A; if found, skip past and start a new line of output */
            if (nohex && (i + c + 1 < size) && ptr[i + c] == 0x0D && ptr[i + c + 1] == 0x0A)
            {
                i += (c + 2 - width);
                break;
            }
            fprintf(stream, "%c", (ptr[i + c] >= 0x20) && (ptr[i + c] < 0x80) ? ptr[i + c] : '.');
            /* check again for 0D0A, to avoid an extra \n if it's at width */
            if (nohex && (i + c + 2 < size) && ptr[i + c + 1] == 0x0D && ptr[i + c + 2] == 0x0A)
            {
                i += (c + 3 - width);
                break;
            }
        }
        fputc('\n', stream); /* newline */
    }
    fflush(stream);
}

static int ssl_debug_func(CURL *handle, curl_infotype type, char *info, size_t size, void *userp)
{
    struct data *config = (struct data *) userp;
    const char *text;
    (void) handle; /* prevent compiler warning */

    switch (type)
    {
        case CURLINFO_TEXT:
            fprintf(stderr, "== Info: %s", info);
        default: /* in case a new one is introduced to shock us */
            return 0;

        case CURLINFO_HEADER_OUT:
            text = "=> Send header";
            break;
        case CURLINFO_DATA_OUT:
            text = "=> Send data";
            break;
        case CURLINFO_SSL_DATA_OUT:
            text = "=> Send SSL data";
            break;
        case CURLINFO_HEADER_IN:
            text = "<= Recv header";
            break;
        case CURLINFO_DATA_IN:
            text = "<= Recv data";
            break;
        case CURLINFO_SSL_DATA_IN:
            text = "<= Recv SSL data";
            break;
    }

    dump(text, stderr, (unsigned char *) info, size, config->trace_ascii);
    return 0;
}
#endif

static void setupSecurityConfiguration(CURL * curlHandle, const SecurityConfiguration &securityConfiguration)
{
    /* set the cert for client authentication */
    curl_easy_setopt(curlHandle,CURLOPT_SSLCERT, securityConfiguration.certFile.c_str());
    /* set the private key file */
    curl_easy_setopt(curlHandle,CURLOPT_SSLKEY, securityConfiguration.keyFile.c_str());
    /* set the file with the certs vaildating the server */
    curl_easy_setopt(curlHandle,CURLOPT_CAINFO, securityConfiguration.caCertFile.c_str());
    /* disconnect if we can't validate server's cert */
    curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYPEER, 1L);
}

struct URLLoader::Connection
{
    char * curlErrStr, *url;
    CURL * curlHandle;
    struct curl_slist * headers;

    Connection() : curlErrStr(0), url(0), curlHandle(0), headers(0)
    {
        curlErrStr = (char*)malloc(CURL_ERROR_SIZE);
        /* Get a curl handle */
        curlHandle = curl_easy_init();
        //headers = curl_slist_append(data->headers, "content-type: application/json");
    }

    ~Connection()
    {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curlHandle);
        free(curlErrStr);
    }
};

URLLoader::Connection * URLLoader::createConnection()
{
    Connection *conn = new Connection;
    if (!conn->curlHandle)
    {
        delete conn;
        return 0;
    }
    return conn;
}

void URLLoader::deleteConnection(Connection *connection)
{
    delete connection;
}

static size_t data_write(void* buf, size_t size, size_t nmemb, void* userp)
{
    size_t len = size * nmemb;
    if (userp)
    {
        kr_dbuffer_t *dbuf = (kr_dbuffer_t *)userp;
        kr_dbuffer_append_mem(dbuf, buf, len);
    }
    return len;
}

bool URLLoader::sendData(Connection *connection, const std::string &url, const std::string &contentType,
                         const void *data, size_t dataSize, kr_dbuffer_t *destBuf,
                         const SecurityConfiguration *securityConfiguration, std::string *errorMsg)
{
	URL *kt_url = new URL(url, true);

	KIARA::Transport::KT_Configuration config;
	config.set_application_type ( KT_STREAM );

	config.set_host( KT_TCP, kt_url->host, atoi(kt_url->port.c_str()));

	KIARA::Transport::KT_Connection* kt_connection = new KIARA::Transport::KT_Zeromq ();
	kt_connection->set_configuration (config);

	KIARA::Transport::KT_Session* session = nullptr;

	if ( 0 != kt_connection->connect(&session) )
	{
		std::cerr << "Failed to connect" << std::endl;
	}

	if (nullptr == session)
	{
		std::cerr << "Session object was not set" << std::endl;
	}

	KIARA::Transport::KT_Msg request;
	
	std::string kt_data = "";
	kt_data.append((const char*)data);
	
	std::string payload = KIARA::Transport::KT_HTTP_Requester::generate_request("POST", kt_url->host, "/"+kt_url->path, std::vector<char>(kt_data.begin(), kt_data.end()));
	request.set_payload(payload);

	if (0 != kt_connection->send(request, *session, 0))
	{
		std::cerr << "Failed to send payload" << std::endl;
	}

	KIARA::Transport::KT_Msg reply;
	if (0 != kt_connection->recv(*session, reply, 0))
		std::cerr << "Receive failed" << std::endl;

	KIARA::Transport::KT_HTTP_Parser parser (reply);
	
	kr_dbuffer_append_mem(destBuf, parser.get_payload().c_str(), (size_t) parser.get_payload().length());
	
	/*CURLcode curlErr;
	
    curl_easy_setopt(connection->curlHandle, CURLOPT_POST, 1);
    curl_easy_setopt(connection->curlHandle, CURLOPT_CUSTOMREQUEST, "POST");

    curl_slist_free_all(connection->headers);
    connection->headers = 0;
    connection->headers = curl_slist_append(connection->headers, ("content-type: "+contentType).c_str());

    curl_easy_setopt(connection->curlHandle, CURLOPT_HTTPHEADER, connection->headers);
    curl_easy_setopt(connection->curlHandle, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(connection->curlHandle, CURLOPT_POSTFIELDSIZE, dataSize);
    curl_easy_setopt(connection->curlHandle, CURLOPT_ERRORBUFFER, connection->curlErrStr);

     curl_easy_setopt(connection->curlHandle, CURLOPT_WRITEFUNCTION, &data_write);
     curl_easy_setopt(connection->curlHandle, CURLOPT_WRITEDATA, destBuf);

     // Get resouce from URL and send to STDOUT
     curl_easy_setopt(connection->curlHandle, CURLOPT_URL, url.c_str());

     curlErr = curl_easy_perform(connection->curlHandle);

     if (curlErr)
     {
         if (errorMsg)
             *errorMsg = curl_easy_strerror(curlErr);
         return false;
     }

     // check for response code
     long respcode = 0;
     curl_easy_getinfo(connection->curlHandle, CURLINFO_RESPONSE_CODE, &respcode);
     if (respcode != 0 && respcode != 200)
     {
         if (errorMsg)
             errorMsg->assign(kr_dbuffer_data(destBuf), kr_dbuffer_data(destBuf)+kr_dbuffer_size(destBuf));
         return false;
     }
	 printf("%s\n", destBuf->data);
	 printf("%zu\n", destBuf->capacity);
	 printf("%zu\n", destBuf->size);*/

     return true;
}

bool URLLoader::load(Connection *connection, const std::string &url, std::string &dest, const SecurityConfiguration *securityConfiguration, std::string *errorMsg)
{
    if (!connection || !connection->curlHandle)
        return false;

    CURLcode curlErr;
    bool result = true;

    if (securityConfiguration)
    {
        setupSecurityConfiguration(connection->curlHandle, *securityConfiguration);
    }

    curl_easy_setopt(connection->curlHandle, CURLOPT_ERRORBUFFER, connection->curlErrStr);
    curl_easy_setopt(connection->curlHandle, CURLOPT_WRITEFUNCTION, &writeToString);
    curl_easy_setopt(connection->curlHandle, CURLOPT_WRITEDATA, &dest);

#ifdef SSL_DEBUG
    // for debugging
    struct data config;
    config.trace_ascii = 1; /* enable ascii tracing */

    curl_easy_setopt(connection->curlHandle, CURLOPT_DEBUGFUNCTION, ssl_debug_func);
    curl_easy_setopt(connection->curlHandle, CURLOPT_DEBUGDATA, &config);

    /* the DEBUGFUNCTION has no effect until we enable VERBOSE */
    curl_easy_setopt(connection->curlHandle, CURLOPT_VERBOSE, 1L);
#endif

    /* Get resouce from URL and send to STDOUT */
    curl_easy_setopt(connection->curlHandle, CURLOPT_URL, url.c_str());

    curlErr = curl_easy_perform(connection->curlHandle);

    if (curlErr)
    {
        if (errorMsg)
        {
            // FIXME add curlErrStr to the error message ?
            errorMsg->assign(curl_easy_strerror(curlErr));
        }
        result = false;
    }

    // check for response code
    long respcode = 0;
    curl_easy_getinfo(connection->curlHandle, CURLINFO_RESPONSE_CODE, &respcode);
    if (respcode != 0 && respcode != 200)
    {
        if (errorMsg)
            *errorMsg = dest;
        result = false;
    }

    return result;
}

bool URLLoader::load(const std::string &url, std::string &dest, std::string *errorMsg)
{
    Connection *conn = createConnection();

    bool result = load(conn, url, dest, /*SecurityConfiguration*/0, errorMsg);

    deleteConnection(conn);

    return result;
}

DFC_STATIC_INIT_FUNC
{
    curl_global_init(CURL_GLOBAL_ALL);
}

DFC_STATIC_SHUTDOWN_FUNC
{
    curl_global_cleanup();
}

} // namespace KIARA
