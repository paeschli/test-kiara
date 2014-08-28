/**
 * @file KT_Client.hpp
 * @author Mathias Hablützel <habl@zhaw.ch>
 * @version 1.0
 * @license TBD
 *
 * @brief Client class.
 */
#ifndef KT_CLIENT_HPP
#define KT_CLIENT_HPP

#include <string>


namespace KIARA {
namespace Transport {

/**
 * class KT_Client
 *
 */

class KT_Client
{
private:
	void* _socket;
	std::string _endpoint_identifier;

public:
	KT_Client();
	virtual ~KT_Client();

	/**
     * @brief Set Endpoint.
	 * @param endpoint to be set
	 */
	void set_endpoint(std::string endpoint)
	{
		_endpoint_identifier = endpoint;
	}

	/**
     * @brief Get reference of the endpoint.
	 * @return Reference to std::string.
	 */
	std::string& get_endpoint()
	{
		return _endpoint_identifier;
	}

	/**
	 * @brief Set the value of _endpoint_identifier.
	 * @param identifier the new value of _endpoint_identifier.
	 */
	void set_endpoint_identifier(std::string identifier) {
		_endpoint_identifier = identifier;
	}

	/**
	 * @brief Get the value of _endpoint_identifier.
	 * @return the value of _endpoint_identifier.
	 */
	std::string get_endpoint_identifier() {
		return _endpoint_identifier;
	}

    /**
     * @brief Get the socket pointer.
     * @return Pointer to the socket.
     */
	void* get_socket() const	{
		return _socket;
	}

    /**
     * @brief Set the socket pointer.
     * @param socket Pointer to the socket.
     */
	void set_socket(void* socket) {
		_socket = socket;
	}
};
} // end of Transport namespace
} // end of KIARA namespace

#endif // KT_CLIENT_HPP
