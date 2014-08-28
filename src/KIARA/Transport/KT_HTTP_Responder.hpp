/**
 * @file KT_HTTP_Responder.hpp
 * @author Mathias Hablützel <habl@zhaw.ch>
 * @version 1.0
 * @license TBD
 *
 * @brief Generate HTTP responses
 */

#ifndef KT_HTTP_RESPONDER_HPP_
#define KT_HTTP_RESPONDER_HPP_

#include <vector>
#include <string>
#include <iostream>

namespace KIARA
{
namespace Transport
{

namespace KT_HTTP_Responder
{
    /**
     * @brief Generate a HTTP 200 response with payload.
     * @param payload The payload or HTTP Body.
     * @return The generated response.
     */
	std::string generate_200_OK (std::vector<char> payload);
    /**
     * @brief Generate a HTTP 400 response with payload.
     * @param payload The payload or HTTP Body.
     * @return The generated response.
     */
	std::string generate_400_BAD_REQUEST(std::vector<char> payload);
    /**
     * @brief Generate a HTTP 401 response with payload.
     * @param payload The payload or HTTP Body.
     * @return The generated response.
     */
	std::string generate_401_UNAUTHORIZED(std::vector<char> payload);
    /**
     * @brief Generate a HTTP 404 response with payload.
     * @param payload The payload or HTTP Body.
     * @return The generated response.
     */
	std::string generate_404_NOT_FOUND(std::vector<char> payload);
    /**
     * @brief Generate a HTTP 500 response with payload.
     * @param payload The payload or HTTP Body.
     * @return The generated response.
     */
	std::string generate_500_INTERNAL_SERVER_ERROR(std::vector<char> payload);
    /**
     * @brief Generate a HTTP 418 response.
     * @return The generated response.
     * @note This is a test method and purely for demonstration purposes.
     */
	std::string generate_418_IM_A_TEAPOT();

    /**
     * @brief Calculate the HTTP Body content length.
     * @param payload The passed payload.
     * @return Content-length string for HTTP use.
     * @note Only used internaly.
     */
	static std::string content_length(std::vector<char> payload);

    /**
     * @brief HTTP Header delimiter, basically '\r\n\r\n'
     */
	extern const char* header_delimiter;

    /**
     * @namespace For the HTTP static status codes.
     */
	namespace HTTP_Code
	{
		extern const char* _200;
		extern const char* _400;
		extern const char* _401;
		extern const char* _404;
		extern const char* _500;
	}

    /**
     * @namespace For the HTTP static content type.
     */
	namespace Content_Type
	{
		extern const char* _text_plain;
	}
} /* namespace KT_HTTP_Responder */

} /* namespace Transport */
} /* namespace KIARA */

#endif /* KT_HTTP_RESPONDER_HPP_ */
