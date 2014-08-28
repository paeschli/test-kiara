/**
 * @file KT_Configuration.cpp
 * @author Mathias Hablützel <habl@zhaw.ch>
 * @version 1.0
 * @license TBD
 *
 * @brief Configuration class for network stack.
 *
 * Lazy way to have C99 wrapper and C++ class implementation in one file.
 */
#include "KT_Configuration.hpp"    // class declaration
#include "KT_Configuration.h"      // wrapper declaration
#include "negotiation.h"
#include <stdlib.h>

namespace KIARA {
namespace Transport {

KT_Configuration::KT_Configuration ( ) { }

KT_Configuration::~KT_Configuration ( ) { }

/**
 * @brief
 * @param neg_ctx
 */
void KT_Configuration::negotiation ( neg_ctx_t* neg_ctx )   {
	char *local;
	//Check if the negotiation was successful
	neg_get_final_capability(neg_ctx, "network.transport-protocols");
	if(neg_ctx->neg_dict == NULL) {
		local = neg_get_best_local_capability(neg_ctx, "network.transport-protocols");
		this->set_transport_layer(neg_capability_to_int(local));
	} else {
		this->set_transport_layer(neg_capability_to_int(neg_ctx->neg_dict->value));
	}
	
	neg_get_final_capability(neg_ctx, "network.transport-port");
	if(neg_ctx->neg_dict == NULL) {
		local = neg_get_best_local_capability(neg_ctx, "network.transport-port");
		this->set_port_number((unsigned int)strtoull(local, NULL, 0));
	} else {
		this->set_port_number((unsigned int)strtoull(neg_ctx->neg_dict->value, NULL, 0));
	}
	
	neg_get_final_capability(neg_ctx, "application.application-type");
	if(neg_ctx->neg_dict == NULL) {
		local = neg_get_best_local_capability(neg_ctx, "application.application-type");
		this->set_application_type(neg_capability_to_int(local));
	} else {
		this->set_application_type(neg_capability_to_int(neg_ctx->neg_dict->value));
	}
	
	this->set_hostname( neg_ctx->host );
	//free(local);
}
        /**
         * @brief Set the value of _network_layer Layer 3 in the networking stack
         * @param network_layer the new value of _network_layer
         */
        void KT_Configuration::set_network_layer(unsigned int network_layer) {
            _network_layer = network_layer;
        }

        /**
         * @brief Get the value of _network_layer Layer 3 in the networking stack
         * @return the value of _network_layer
         */
        unsigned int KT_Configuration::get_network_layer()const {
            return _network_layer;
        }

        /**
         * @brief Set the value of _transport_layer Layer 4 in the networking stack
         * @param transport_layer the new value of _transport_layer
         */
        void KT_Configuration::set_transport_layer(unsigned int transport_layer) {
            _transport_layer = transport_layer;
        }

        /**
         * @brief Get the value of _transport_layer Layer 4 in the networking stack
         * @return the value of _transport_layer
         */
        unsigned int KT_Configuration::get_transport_layer() const {
            return _transport_layer;
        }

        /**
         * @brief Set the value of _crypto_layer Optional crypto layer, based on OpenSSL
         * @param crypto_layer the new value of _crypto_layer
         */
        void KT_Configuration::set_crypto_layer(unsigned int crypto_layer) {
            _crypto_layer = crypto_layer;
        }

        /**
         * @brief Get the value of _crypto_layer Optional crypto layer, based on OpenSSL
         * @return the value of _crypto_layer
         */
        unsigned int KT_Configuration::get_crypto_layer() const {
            return _crypto_layer;
        }

        /**
         * @brief Set the value of _application_layer
         * @param application_layer the new value of _application_layer
         */
        void KT_Configuration::set_application_layer(unsigned int application_layer) {
            _application_layer = application_layer;
        }

        /**
         * @brief Get the value of _application_layer
         * @return the value of _application_layer
         */
        unsigned int KT_Configuration::get_application_layer() const {
            return _application_layer;
        }

        /**
         * @brief Set the value of _application_type
         * @param application_type the new value of _application_type
         */

        void KT_Configuration::set_application_type(unsigned int application_type) {
            _application_type = application_type;
        }

        /**
         * @brief Get the value of _application_type
         * @return the value of _application_type
         */
        unsigned int KT_Configuration::get_application_type() const {
            return _application_type;
        }

        /**
         * @brief Set the hostname.
         * @param transport_layer Communication protocol.
         * @param hostname Hostname.
         * @param port_number Port number.
         */
        void KIARA::Transport::KT_Configuration::set_host(unsigned int transport_layer,
                std::string hostname, unsigned int port_number) {

            this->set_transport_layer(transport_layer);
            this->set_hostname(hostname);
            this->set_port_number(port_number);
        }

} // end of namespace Transport
} // end of namespace KIARA


/*
 * Wrapper C99 function around KT_Configuration class
 */
extern "C" {
using namespace KIARA::Transport;
/*
 * A simple preprocessor trick to deduplicate the casting.
 *
 * Takes a parameter which pointer is to be casted to a KT_Configuration* named config_obj.
 * Since this is done by the preprocessor there is no runtime overhead by calling a function
 * that casts the pointer to said object. It may also be done by a inline function but both
 * cases however would not reduce the size of the binary.
 */
#define CASTER(obj) KT_Configuration* config_obj = reinterpret_cast<KT_Configuration*> (obj);

kt_configuration_t* kt_configuration_create () {
    return reinterpret_cast<kt_configuration_t*> (new KT_Configuration());
}

kt_network_layer kt_configuration_get_network_layer (kt_configuration_t* config) {
	CASTER(config)
	return config_obj->get_network_layer ();
}
void kt_configuration_set_network_layer (kt_configuration_t* config, kt_network_layer network_layer) {
	CASTER(config)
	config_obj->set_network_layer (network_layer);
}

kt_transport_layer kt_configuration_get_transport_layer (kt_configuration_t* config) {
	CASTER(config)
	return config_obj->get_transport_layer ();
}
void kt_configuration_set_transport_layer (kt_configuration_t* config, kt_transport_layer transport_layer) {
	CASTER(config)
	config_obj->set_transport_layer (transport_layer);
}

kt_crypto_layer kt_configuration_get_crypto_layer (kt_configuration_t* config) {
	CASTER(config)
	return config_obj->get_crypto_layer ();
}
void kt_configuration_set_crypto_layer (kt_configuration_t* config, kt_crypto_layer crypto_layer) {
	CASTER(config)
	config_obj->set_crypto_layer (crypto_layer);
}

kt_application_layer kt_configuration_get_application_layer (kt_configuration_t* config) {
	CASTER(config)
	return config_obj->get_application_layer ();
}
void kt_configuration_set_application_layer (kt_configuration_t* config, kt_application_layer application_layer) {
	CASTER(config)
	config_obj->set_application_layer (application_layer);
}

kt_application_type kt_configuration_get_application_type (kt_configuration_t* config) {
	CASTER(config)
	return config_obj->get_application_type ();
}
void kt_configuration_set_application_type (kt_configuration_t* config, kt_application_type application_type) {
	CASTER(config)
	config_obj->set_application_type (application_type);
}

char* kt_configuration_get_hostname (kt_configuration_t* config) {
    // TODO: implement kt_configuration_get_hostname
    return nullptr;
}
void kt_configuration_set_hostname (kt_configuration_t* config, char* hostname) {
    CASTER(config)
    config_obj->set_hostname(hostname);
}

unsigned int kt_configuration_get_port (kt_configuration_t* config) {
    // TODO: implement kt_configuration_get_port
    return 0;
}
void kt_configuration_set_port (kt_configuration_t* config, unsigned int port) {
    CASTER(config)
    config_obj->set_port_number(port);
}

/*
 * undef CASTER to prevent any accidents in further code
 */
#undef CASTER
}
