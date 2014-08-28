/**
 * @file KT_HTTP_Responder.hpp
 * @author Mathias Hablützel <habl@zhaw.ch>
 * @version 1.0
 * @license TBD
 *
 * @brief Generate HTTP responses
 */

#include "KT_HTTP_Responder.hpp"

namespace KIARA
{
namespace Transport
{
namespace KT_HTTP_Responder
{
std::string generate_200_OK(std::vector<char> payload)
{
	std::string response(HTTP_Code::_200);
	response += Content_Type::_text_plain;
	response += content_length(payload);
	response += header_delimiter;
	response.append(payload.data(), payload.size());
	response += "\r\n";
	return response;
}

std::string generate_400_BAD_REQUEST(
		std::vector<char> payload)
{
	std::string response(HTTP_Code::_400);
	response += Content_Type::_text_plain;
	response += content_length(payload);
	response += header_delimiter;
	response.append(payload.data(), payload.size());
	response += "\r\n";
	return response;
}

std::string generate_401_UNAUTHORIZED(
		std::vector<char> payload)
{
	std::string response(HTTP_Code::_401);
	response += Content_Type::_text_plain;
	response += content_length(payload);
	response += header_delimiter;
	response.append(payload.data(), payload.size());
	response += "\r\n";
	return response;
}

std::string generate_404_NOT_FOUND(std::vector<char> payload)
{
	std::string response(HTTP_Code::_404);
	response += Content_Type::_text_plain;
	response += content_length(payload);
	response += header_delimiter;
	response.append(payload.data(), payload.size());
	response += "\r\n";
	return response;
}

std::string generate_500_INTERNAL_SERVER_ERROR(
		std::vector<char> payload)
{
	std::string response(HTTP_Code::_500);
	response += Content_Type::_text_plain;
	response += content_length(payload);
	response += header_delimiter;
	response.append(payload.data(), payload.size());
	response += "\r\n";
	return response;
}

std::string generate_418_IM_A_TEAPOT()
{
	// https://tools.ietf.org/html/rfc2324
	// Simple response method for testing
	std::string response ("            _\n"
			"         _,(_)._\n"
			"    ___,(_______).      ____\n"
			"  ,'__.           \\    /\\___\\-.\n"
			" /,' /             \\  /  /     \\\n"
			"| | |              |,'  /       \\\n"
			" \\`.|                  /       _|_\n"
			"  `. :           :    /        MMM\n"
			"    `.            :.,'        ZHAW\n"
			"      `-.________,-'          KIARA\r\n");
	return std::string("HTTP/1.0 418 IM A TEAPOT\r\nContent-Type: text/plain\r\nContent-Length: 300\r\n\r\n" + response);
}

std::string content_length(std::vector<char> payload)
{
	return std::string ("Content-Length: " + std::to_string(payload.size()));
}

const char* header_delimiter = "\r\n\r\n";

namespace HTTP_Code {
	const char* _200 = "HTTP/1.0 200 OK\r\n";
	const char* _400 = "HTTP/1.0 400 BAD REQUEST";
	const char* _401 = "HTTP/1.0 401 UNAUTHORIZED";
	const char* _404 = "HTTP/1.0 404 NOT FOUND";
	const char* _500 = "HTTP/1.0 500 INTERNAL SERVER ERROR";
}

namespace Content_Type {
	const char* _text_plain  = "Content-Type: text/plain\r\n";
}

} /* namespace KT_HTTP_Responder */
} /* namespace Transport */
} /* namespace KIARA */
