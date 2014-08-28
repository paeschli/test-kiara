/*
 * KT_HTTP_Requester.cpp
 *
 *  Created on: Mar 19, 2014
 *      Author: aepp
 */

#include "KT_HTTP_Requester.hpp"

namespace KIARA
{
namespace Transport
{
namespace KT_HTTP_Requester
{
std::string generate_request(std::string method, std::string host, std::string path, std::vector<char> payload)
{
	std::string request(make_header(method, path));
	if(method.compare("GET") != 0 && method.compare("DELETE")){
		request += content_length(payload);
		request += "\r\n";
	}
	request += "User-Agent: KIARA\r\n";
	request += "Host: ";
	request += host;
	request += "\r\n\r\n";
	if(method.compare("GET") != 0 && method.compare("DELETE")){
		request.append(payload.data(), payload.size());
		request += "\r\n";
	}
	return request;
}

std::string make_header(std::string method, std::string path)
{
	std::string header(method);
	header += " ";
	header += path;
	header += " HTTP/1.1\r\n";
	return header;
}

std::string content_length(std::vector<char> payload)
{
	return std::string ("Content-Length: " + std::to_string(payload.size()));
}

} /* namespace KT_HTTP_Requester */
} /* namespace Transport */
} /* namespace KIARA */
