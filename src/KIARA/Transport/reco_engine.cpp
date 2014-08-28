/**
 * @file reco_engine.cpp
 * @author Philipp Aeschlimann <aepp@zhaw.ch>
 * @version
 * @license TBD
 * @brief
 *
 */

#include "KT_Zeromq.hpp"
#include "KT_HTTP_Parser.hpp"
#include "KT_HTTP_Responder.hpp"
#include "KT_HTTP_Requester.hpp"
#include "reco_engine.h"
#include "registry.h"
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <cstring>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

using namespace KIARA::Transport;

/**
 * @param
 * @param
 * @param
 * @brief
 */
void callback_handler_reco(KT_Msg&, KT_Session*, KT_Connection*);

/**
 * @param endpoint
 * @param neg_ctx
 * @brief
 */
RecoServer::RecoServer(char* endpoint, neg_ctx_t* neg_ctx) {
	host = endpoint;
	ctx = neg_ctx;
	//url = std::string(endpoint);
}

RecoServer::~RecoServer() {
	//delete[];
}

/**
 * @return
 * @brief
 */
int RecoServer::RunServer() {
	KT_Configuration config;
	config.set_application_type(KT_STREAM);
	config.set_transport_layer(KT_TCP);
	config.set_hostname("*");
	config.set_port_number(ctx->port);

	KT_Connection* connection = new KT_Zeromq();
	connection->set_configuration(config);

	connection->register_callback(&callback_handler_reco);
	connection->bind();

	connection->get_session()->begin()->second->set_k_user_data(ctx);

	sleep(600);

	connection->unbind();

	return 0;
}

/**
 * @param msg
 * @param sess
 * @param obj
 * @brief
 */
void callback_handler_reco(KT_Msg& msg, KT_Session* sess, KT_Connection* obj) {
	KT_HTTP_Parser parser(msg);
	neg_ctx_t *neg_ctx = (neg_ctx_t*) sess->get_k_user_data();
	std::cout << neg_ctx->host << std::endl;
	std::string payload("");
	if (parser.get_url().compare(0, 12, "/negotiation") != 0) {
		payload.append("Not a negotiation endpoint!");
		payload = KT_HTTP_Responder::generate_400_BAD_REQUEST(std::vector<char>(payload.begin(), payload.end()));
	} else {
		switch (parser.method) {
				//GET Request
			case 1:
				std::cout << "GET request" << std::endl;
				payload.append(reg_get_local_capability_json(neg_ctx));
				payload = KT_HTTP_Responder::generate_200_OK(std::vector<char>(payload.begin(), payload.end()));
				break;
				//POST Request
			case 4:
				std::cout << "PUT request" << std::endl;
				if(neg_ctx->client_nego){
					payload.append("Bad Request!");
					payload = KT_HTTP_Responder::generate_400_BAD_REQUEST(std::vector<char>(payload.begin(), payload.end()));
					break;
				}
				std::cout << parser.get_payload() << std::endl;
				reg_set_remote_capability(neg_ctx, parser.get_identifier().c_str(), parser.get_payload().c_str());
				payload.append(neg_negotiate(neg_ctx, parser.get_identifier().c_str()));
				payload = KT_HTTP_Responder::generate_200_OK(std::vector<char>(payload.begin(), payload.end()));
				break;
				//Anything else
			default:
				payload.append("Request type is not supported");
				payload = KT_HTTP_Responder::generate_400_BAD_REQUEST(std::vector<char>(payload.begin(), payload.end()));
		}
	}
	//DEBUG Only
	//std::cout << parser.get_payload() << std::endl;
	//std::cout << parser.get_identifier() << std::endl;

	KT_Msg message;
	message.set_payload(payload);

	obj->send(message, (*sess), 0);
}

/**
 * @param serverhost
 * @param neg_ctx
 * @brief
 */
RecoClient::RecoClient(char* serverhost, neg_ctx_t* neg_ctx) {

	KT_Configuration config;
	config.set_application_type(KT_STREAM);

	config.set_host(KT_TCP, neg_ctx->host, neg_ctx->port);

	KT_Connection* connection = new KT_Zeromq();
	connection->set_configuration(config);

	KT_Session* session = nullptr;

	if (0 != connection->connect(&session)) {
		remote_endpoint = 25;
	} else {

		if (nullptr == session) {
			remote_endpoint = 30;
		}

		KT_Msg request;
		std::string payload(reg_get_local_capability_json(neg_ctx));
		payload = KT_HTTP_Requester::generate_request("PUT", "localhost:5555", "/negotiation", std::vector<char>(payload.begin(), payload.end()));

		request.set_payload(payload);

		if (0 != connection->send(request, *session, 0)) {
			remote_endpoint = 35;
		}

		KT_Msg reply;
		if (0 != connection->recv(*session, reply, 0))
			remote_endpoint = 40;

		KT_HTTP_Parser parser(reply);
		if(parser.get_status_code() < 200 || parser.get_status_code() > 300) {
			//Try GET here
			payload = KT_HTTP_Requester::generate_request("GET", "localhost:5555", "/negotiation", std::vector<char>(payload.begin(), payload.end()));

			KT_Msg request;
			request.set_payload(payload);

			if (0 != connection->send(request, *session, 0)) {
				remote_endpoint = 35;
			}

			KT_Msg reply;
			if (0 != connection->recv(*session, reply, 0))
				remote_endpoint = 40;

			KT_HTTP_Parser parser(reply);
			printf("done so\n");
			neg_ctx->client_nego = 1;
			response = parser.get_payload();
		}
		else {
			response = parser.get_payload();
		}
	}
}

/**
 * @return
 * @brief
 */
char *RecoClient::GetPayload() {
	return (char *) response.c_str();
}

#ifdef	__cplusplus
extern "C" {
#endif

        /**
         * @param endpoint
         * @param neg_ctx
         * @return
         * @brief
         */
	void* init_reco_server(char *endpoint, neg_ctx_t *neg_ctx) {
		RecoServer *out = new RecoServer(endpoint, neg_ctx);
		return ((void*) out);
	}

        /**
         * @param reco_server
         * @brief
         */
	void reco_run_server(void *reco_server) {
		RecoServer *tmp_reco_server = ((RecoServer*) reco_server);
		tmp_reco_server->RunServer();
	}

        /**
         * @param endpoint
         * @param neg_ctx
         * @return
         * @brief
         */
	char* reco_send_offer(char *endpoint, neg_ctx_t* neg_ctx) {
		if(_check_remote_endpoint(endpoint, neg_ctx->port) == 0){
			neg_ctx->kiara_endpoint = 0;
			return "";
		}
		else {
			//Try PUT first
			RecoClient *out = new RecoClient(endpoint, neg_ctx);
			return out->GetPayload();
		}
	}

        /**
         * @param hostname
         * @param port
         * @return
         * @brief
         */
	int _check_remote_endpoint(char* hostname, int port) {
		int sockfd, ret = 1;
		struct addrinfo hints, *servinfo, *p;
		int rv;
		char s[INET6_ADDRSTRLEN], sport[20];

		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		sprintf(sport, "%d", port);

		if ((rv = getaddrinfo(hostname, sport, &hints, &servinfo)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			return 1;
		}

		// loop through all the results and connect to the first we can
		for(p = servinfo; p != NULL; p = p->ai_next) {
			if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
				continue;
			}
			if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
				close(sockfd);
				continue;
			}
			break;
		}

		if (p == NULL) {
			ret = 0;
		}

		close(sockfd);

		return ret;
	}

#ifdef	__cplusplus
}
#endif
