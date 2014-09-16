/**
 * @file KT_Msg.hpp
 * @author Mathias Hablützel <habl@zhaw.ch>
 * @version 1.0
 * @license TBD
 *
 * @brief Message object containing payload and metadata.
 */

#ifndef KT_MSG_HPP
#define KT_MSG_HPP

#include <string>
#include <map>
#include <vector>

namespace KIARA {
namespace Transport {


/**
 * @class KT_Msg
 * Since payload is handled as a pointer to a binary memory allocation
 * *free_payload() is responsible to destroy/deallocate the beforementioned
 * memory.
 *
 * According to settings in kt_conn_session_t defined it will generate valid
 * header and body structures from the metadata and payload when sent. The
 * same applies the other way around when receiving messages.
 */

class KT_Msg
{
private:
	std::map< std::string, std::string > _metadata;
	std::vector< char > _payload;
	const void *_payload_binary;
	size_t _payload_size;
	int _binary_transport = 0;

public:
	KT_Msg();
	virtual ~KT_Msg();
	KT_Msg(std::vector<char>& payload);

	void add_metadata(std::string &key, std::string &value);
	std::string get_serialized_metadata(const std::string delim = ":");

	/**
	 * @brief Set the value of _metadata.
	 * @param metadata the new value of _metadata.
	 */
	void set_metadata(std::map<std::string, std::string> &metadata) {
		_metadata = metadata;
	}

	/**
	 * @brief Get the value of _metadata.
	 * @return the value of _metadata.
	 */
	std::map<std::string, std::string>get_metadata() {
		return _metadata;
	}

	/**
	 * @brief Set the value of _payload.
	 * @param payload The new value as std::vector<char>.
	 */
	void set_payload(std::vector<char> payload) {
		_payload = payload;
	}
	/**
	 * @brief Set the value of _payload.
	 * @param payload The new value as std::string.
	 */
	void set_payload(std::string &payload) {
		_payload.resize (payload.size());
		_payload.assign (payload.begin(), payload.end());
	}
	void set_payload(const void *payload) {
		_payload_binary = payload;
		_binary_transport = 1;
	}
	void set_size(size_t payload_size) {
		_payload_size = payload_size;
	}
	size_t get_size() {
		return _payload_size;
	}
	int is_binary_transport() {
		return _binary_transport;
	}
	/**
	 * @brief Get the value of _payload.
	 * @return The value of _payload.
     * @note Take ownership.
	 */
	std::vector<char> get_payload() {
		return _payload;
	}
	const void* get_payload_binary() {
		return _payload_binary;
	}
    /**
     * @brief Get the pointer to the payload.
     * @return std::vector<char> pointer to the payload.
     * @note Don't take ownership.
     */
	std::vector<char>* get_payload_ptr() {
	    return &_payload;
	}

    /**
     * @brief Convert the payload to a std::string.
     * @return An std::string.
     */
	std::string get_payload_as_string() {
	    return std::string(_payload.begin(), _payload.end());
	}


};
} // end of Transport namespace
} // end of KIARA namespace

#endif // KT_MSG_HPP
