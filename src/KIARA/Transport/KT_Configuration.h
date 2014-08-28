/**
 * @file KT_Configuration.h
 * @author Mathias Hablützel <habl@zhaw.ch>
 * @version 1.0
 * @license TBD
 *
 * @brief C99 bindings for KT_Configuration class.
 */

#ifndef SRC_CORE_KT_CONFIGURATION_H_
#define SRC_CORE_KT_CONFIGURATION_H_

#include "KT_Configuration_glob.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kt_configuration kt_configuration_t;

kt_configuration_t* kt_configuration_create ();

kt_network_layer kt_configuration_get_network_layer (kt_configuration_t* config);
void kt_configuration_set_network_layer (kt_configuration_t* config,
        kt_network_layer network_layer);

kt_transport_layer kt_configuration_get_transport_layer (kt_configuration_t* config);
void kt_configuration_set_transport_layer (kt_configuration_t* config,
        kt_transport_layer transport_layer);

kt_crypto_layer kt_configuration_get_crypto_layer (kt_configuration_t* config);
void kt_configuration_set_crypto_layer (kt_configuration_t* config,
        kt_crypto_layer crypto_layer);

kt_application_layer kt_configuration_get_application_layer (kt_configuration_t* config);
void kt_configuration_set_application_layer (kt_configuration_t* config,
        kt_application_layer application_layer);

kt_application_type kt_configuration_get_application_type (kt_configuration_t* config);
void kt_configuration_set_application_type (kt_configuration_t* config,
        kt_application_type application_type);

char* kt_configuration_get_hostname (kt_configuration_t* config);
void kt_configuration_set_hostname (kt_configuration_t* config, char* hostname);

unsigned int kt_configuration_get_port (kt_configuration_t* config);
void kt_configuration_set_port (kt_configuration_t* config, unsigned int port);

#ifdef __cplusplus
}
#endif

#endif  // SRC_CORE_KT_CONFIGURATION_H_
