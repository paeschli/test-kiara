/**
 * @file negotiation.c
 * @author Philipp Aeschlimann <aepp@zhaw.ch>
 * @version
 * @license TBD
 * @brief
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <uthash.h>
#include "reco_engine.h"
#include "registry.h"
#include "KT_Configuration_glob.h"

/**
 * @return
 * @brief
 */
neg_ctx_t *neg_init() {
	return reg_create_context();
}

/**
 * @param neg_ctx
 * @param key
 * @param value
 * @return
 * @brief
 */
int neg_set_local_capability(neg_ctx_t *neg_ctx, char *key, char *value) {
	reg_set_capability(neg_ctx, key, value);
	return 1;
}

/**
 * @param neg_ctx
 * @param key
 * @return
 * @brief
 */
neg_dict_t *neg_get_capability(neg_ctx_t* neg_ctx, char* key) {
	return reg_get_capability(neg_ctx, key);
}

/**
 * @param neg_ctx
 * @param key
 * @brief
 */
void neg_get_final_capability(neg_ctx_t* neg_ctx, char* key) {
	reg_get_final_capability(neg_ctx, key);
}

/**
 * @param neg_ctx
 * @param identifier
 * @param payload
 * @brief
 */
void neg_negotiate_remote(neg_ctx_t* neg_ctx, char* identifier, char* payload) {
	reg_set_remote_capability(neg_ctx, identifier, payload);
	neg_set_final_capabilities(neg_ctx, neg_negotiate(neg_ctx, identifier));
}

/**
 * @param neg_ctx
 * @return
 * @brief
 */
char *neg_send_offer(neg_ctx_t *neg_ctx) {
	return reco_send_offer(neg_ctx->host, neg_ctx);
}

/**
 * @param neg_ctx
 * @return
 * @brief
 */
int neg_run_server(neg_ctx_t *neg_ctx) {
	void* server = init_reco_server(neg_ctx->host, neg_ctx);
	reco_run_server(server);
	return 1;
}

/**
 * @param neg_ctx
 * @param response
 * @return
 * @brief
 */
int neg_set_final_capabilities(neg_ctx_t* neg_ctx, char *response) {
	printf("resp:%s\n", response);
	reg_set_final_capabilities(neg_ctx, response);
	return 1;
}

/**
 * @param neg_ctx
 * @param app
 * @brief
 */
void neg_set_local_profile(neg_ctx_t* neg_ctx, kt_application_type app){
	switch(app) {
		case KT_WEBSERVER:
			neg_set_local_capability(neg_ctx, "network.transport-protocols.tcp.prec", "MUST");
			neg_set_local_capability(neg_ctx, "network.transport-port.*.prec", "9090");
			neg_set_local_capability(neg_ctx, "network.communication-paradigm.req-rep.prec", "MUST");
			neg_set_local_capability(neg_ctx, "application.application-type.stream.prec", "MUST");
			neg_set_local_capability(neg_ctx, "application.application-protocols.http.prec", "MUST");
			break;
	}
}

/**
 * @param neg_ctx
 * @param capability
 * @return
 * @brief
 */
char *neg_get_best_local_capability(neg_ctx_t *neg_ctx, const char *capability) {
	neg_dict_t *current_dict, *tmp;
	char *c = NULL, *key;
	char delimiter[] = ".";
	int prec = -1, i;
	
	HASH_ITER(hh, neg_ctx->hash, current_dict, tmp) {
		if (strncmp(current_dict->id, capability, strlen(capability)) == 0) {
			if(_prec_to_int(current_dict->value) > prec) {
				prec = _prec_to_int(current_dict->value);
				c = current_dict->id;
			}
		}
	}
	char id[strlen(c) + 1];
	strncpy(id, c, sizeof (id));
	key = strtok(id, delimiter);
	i = 1;
	while (key != NULL) {
		if(i == 3){
			c = key;
		}
		key = strtok(NULL, delimiter);
		i++;
	}
	
	char* str = (char*)malloc(sizeof(char)*(strlen(c)+1));
	strncpy(str, c, strlen(c));
	str[strlen(c)] = '\0';
	return str;
}

/**
 * @param neg_ctx
 * @param endpoint
 * @return
 * @brief
 */
char *neg_negotiate(neg_ctx_t *neg_ctx, const char *endpoint) {
	neg_dict_remote_collection_t *s = malloc(sizeof (*s));
	neg_dict_remote_collection_t* remote_dict = reg_get_remote_dict(neg_ctx, endpoint);
	neg_dict_t *current_dict, *tmp;
	dec_dict_t *dec_dict, *dec_tmp, *dec_it, *dec, *d_tmp;

	dec_dict = NULL;
	char delimiter[] = ".";
	char *key;

	HASH_ITER(hh, neg_ctx->hash, current_dict, tmp) {
		s = malloc(sizeof (struct neg_dict_remote_collection_t));
		HASH_FIND_STR(remote_dict->sub, current_dict->id, s);
		if (s == NULL) {
			printf("No coresponding value found in dict %s\n", current_dict->id);
		} else {
			//Found value, negotiate
			int i;
			char *nego_key, *tmp_id;
			char id[strlen(current_dict->id) + 1];

			strncpy(id, current_dict->id, sizeof (id));
			key = strtok(id, delimiter);
			i = 1;
			while (key != NULL) {
				if (i <= 2) {
					if (i == 2) {
						asprintf(&nego_key, "%s.%s", nego_key, key);
					} else {
						asprintf(&nego_key, "%s", key);
					}
				} else if (i == 3) {
					HASH_FIND_STR(dec_dict, nego_key, dec_tmp);
					if (dec_tmp == NULL) {
						dec_tmp = malloc(sizeof (*dec_tmp));
						dec_tmp->id = nego_key;
						dec_tmp->sub = NULL;
						dec_tmp->value = 0;
						HASH_ADD_KEYPTR(hh, dec_dict, nego_key, strlen(nego_key), dec_tmp);
					}
					tmp_id = (char*) malloc(sizeof (char) * (strlen(key) + 1));
					strncpy(tmp_id, key, strlen(key));
					tmp_id[strlen(key)] = '\0';
				} else if (i == 4) {
					int l_prec = _prec_to_int(current_dict->value);
					int r_prec = _prec_to_int(s->value);
					int neg_value = (l_prec * r_prec) + (l_prec - r_prec);
					HASH_FIND_STR(dec_dict, nego_key, dec_it);

					HASH_ITER(hh, dec_it->sub, dec, d_tmp) {
						if (neg_value > dec->value) {
							HASH_DEL(dec_it->sub, dec);
						} else {
							neg_value = 0;
						}
					}
					if (neg_value > 0) {
						dec = malloc(sizeof (*dec));
						if(strcmp(tmp_id, "*") == 0) {
							if(atoi(current_dict->value) > 0) {
								dec->id = current_dict->value;
							}
							else {
								dec->id = s->value;
							}
						}
						else {
							dec->id = tmp_id;
						}
						dec->sub = NULL;
						dec->value = neg_value;
						HASH_ADD_KEYPTR(hh, dec_it->sub, dec->id, strlen(dec->id), dec);
					}
				}
				key = strtok(NULL, delimiter);
				i++;
			}
		}
	}
	neg_dict_t *neg_dict, *nego_tmp;
	neg_dict = NULL;

	HASH_ITER(hh, dec_dict, dec_it, dec_tmp) {

		HASH_ITER(hh, dec_it->sub, dec, d_tmp) {
			char *response_key;
			nego_tmp = malloc(sizeof (*nego_tmp));
			asprintf(&response_key, "%s.%s.prec", dec_it->id, dec->id);
			nego_tmp->id = response_key;
			nego_tmp->value = "1";
			HASH_ADD_KEYPTR(hh, neg_dict, response_key, strlen(response_key), nego_tmp);
		}
	}
	char *json = _reg_get_local_capability_json(neg_dict, neg_ctx->root_response);
	return json;
}

/**
 * @param prec
 * @return
 * @brief
 */
int _prec_to_int(char *prec) {
	int ret;

	if (strcmp(prec, "MUST") == 0) {
		ret = 4;
	} else if (strcmp(prec, "SHOULD") == 0) {
		ret = 3;
	} else if (strcmp(prec, "SHOULD NOT") == 0) {
		ret = 2;
	} else if (strcmp(prec, "MUST NOT") == 0) {
		ret = 1;
	} else if (strcmp(prec, "NONE") == 0) {
		ret = 0;
	} else {
		ret = 4;
	}
	return ret;
}

/**
 * @param capability
 * @return
 * @brief
 */
int neg_capability_to_int(char *capability) {
	if(strcmp(capability, "tcp") == 0){
		return KT_TCP;
	} else if (strcmp(capability, "udp") == 0) {
		return KT_UDP;
	} else if (strcmp(capability, "stream") == 0) {
		return KT_STREAM;
	} else if (strcmp(capability, "tcp") == 0) {
		
	} else if (strcmp(capability, "tcp") == 0) {
		
	} else if (strcmp(capability, "tcp") == 0) {
		
	} else if (strcmp(capability, "tcp") == 0) {
		
	} else if (strcmp(capability, "tcp") == 0) {
		
	} else if (strcmp(capability, "tcp") == 0) {
		
	} else if (strcmp(capability, "tcp") == 0) {
		
	}
	return NULL;
}
