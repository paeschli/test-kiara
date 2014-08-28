/**
 * @file registry.h
 * @author Philipp Aeschlimann <aepp@zhaw.ch>
 * @version
 * @license TBD
 * @brief
 *
 */

#include <uthash.h>
#include "negotiation.h"

#ifndef REGISTRY_H
#define	REGISTRY_H

#ifdef	__cplusplus
extern "C" {
#endif

    /**
     * @enum dict_struct
     * @brief
     */
	enum dict_struct {
		CATEGORY,
		GROUP,
		TYPE,
		VALUE
	};

	neg_ctx_t *reg_create_context(void);
	int reg_set_remote_capability(neg_ctx_t *neg_ctx, const char *endpoint, const char *remote_body);
	neg_dict_remote_collection_t* reg_get_remote_dict(neg_ctx_t *neg_ctx, const char *endpoint);
	int reg_set_capability(neg_ctx_t *neg_ctx, char *key, char *value);
	neg_dict_t *reg_get_capability(neg_ctx_t* neg_ctx, char* key);
	void reg_get_final_capability(neg_ctx_t* neg_ctx, char* key);
	int reg_set_final_capabilities(neg_ctx_t* neg_ctx, char *response);
	int _parse_final_capabilities(json_t *value, neg_ctx_t* neg_ctx, char *nego_key, enum dict_struct level);
	char** _reg_str_split(char* a_str, const char a_delim);
	char* reg_get_local_capability_json(neg_ctx_t* neg_ctx);
	char* _reg_get_local_capability_json(neg_dict_t *hash, json_t *root);
	int _reg_parse_dict(json_t* value, neg_dict_remote_collection_t* remote_dict, const char* nego_key);

#ifdef	__cplusplus
}
#endif

#endif	/* REGISTRY_H */

