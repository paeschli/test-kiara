/**
 * @file reco_engine.h
 * @author Philipp Aeschlimann <aepp@zhaw.ch>
 * @version
 * @license TBD
 * @brief
 *
 */

#include "negotiation.h"
#ifdef	__cplusplus
	#include <iostream>
#endif
#include <czmq.h>

#ifndef RECO_ENGINE_H
#define	RECO_ENGINE_H

#ifdef	__cplusplus
/**
 * @class RecoServer
 */
class RecoServer {
public:
	RecoServer(char *endpoint, neg_ctx_t* neg_ctx);
	~RecoServer();
	int RunServer();
private:
	std::string host;
	neg_ctx_t* ctx;
};

/**
 * @class RecoClient
 */
class RecoClient {
public:
	RecoClient(char *serverhost, neg_ctx_t* neg_ctx);
	~RecoClient();
	char *GetPayload();
	
	int remote_endpoint = 0;
private:
	std::string host;
	std::string response;
};
#endif

#ifdef	__cplusplus
extern "C" {
#endif
	
void* init_reco_server(char* endpoint, neg_ctx_t *neg_ctx);
void reco_run_server(void *reco_server);
char* reco_send_offer (char* endpoint, neg_ctx_t *neg_ctx);
int _check_remote_endpoint(char* hostname, int port);

#ifdef	__cplusplus
}
#endif

#endif	/* RECO_ENGINE_H */

