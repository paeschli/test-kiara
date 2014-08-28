/*
 * KT_HTTP_Requester.hpp
 *
 *  Created on: Mar 19, 2014
 *      Author: aepp
 */

#ifndef KT_HTTP_REQUESTER_HPP_
#define KT_HTTP_REQUESTER_HPP_

#include <vector>
#include <string>
#include <iostream>

namespace KIARA
{
namespace Transport
{

namespace KT_HTTP_Requester
{
	std::string generate_request(std::string method, std::string host, std::string path, std::vector<char> payload);
	static std::string make_header(std::string method, std::string path);

	static std::string content_length(std::vector<char> payload);

	/*namespace Content_Type
	{
		extern const char* _text_plain;
	}*/
} /* namespace KT_HTTP_Requester */

} /* namespace Transport */
} /* namespace KIARA */

#endif /* KT_HTTP_REQUESTER_HPP_ */
