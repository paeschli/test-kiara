/**
 * @file KT_HTTP_Parser.cpp
 * @author Mathias Hablützel <habl@zhaw.ch>
 * @version 1.0
 * @license TBD
 *
 * @brief Simple wrapper for http_parser.c with ostream support.
 */

#include "KT_HTTP_Parser.hpp"

namespace KIARA
{
namespace Transport
{

/**
 * @brief Instantiate the parser and configure it to extract the body.
 * @param msg The message to parse.
 */
KT_HTTP_Parser::KT_HTTP_Parser (KT_Msg& msg)
{
    http_parser_settings settings;
	
	tmp_parser_fields *parser_fields;
	parser_fields = new tmp_parser_fields;
	parser_fields->host = new std::string("0");
	parser_fields->body = new std::string;
	parser_fields->query_string = new std::string;

    // seems that we have to 0 all unused callback hooks
    // srsly ... save initializing? Anyone?
    settings.on_message_complete = 0;
    settings.on_status = 0;
    settings.on_message_begin = 0;
    settings.on_headers_complete = 0;
    settings.on_header_field = 0;
    settings.on_header_value = 0;

    settings.on_body = body_cb;
	settings.on_url = url_cb;
	settings.on_header_field = header_field_cb;
	settings.on_header_value = header_value_cb;

    parser = (http_parser*) malloc ( sizeof ( http_parser ) );
    http_parser_init ( parser, HTTP_BOTH );

    parser->data = parser_fields;
    http_parser_execute ( parser, &settings, msg.get_payload().data(), \
        msg.get_payload().size() );
	
    payload = ((tmp_parser_fields*)parser->data)->body;
	query_string = ((tmp_parser_fields*)parser->data)->query_string;
	method = parser->method;
	status_code = parser->status_code;
	host = ((tmp_parser_fields*)parser->data)->host;
	identifier = ((tmp_parser_fields*)parser->data)->host;
	*identifier += *((tmp_parser_fields*)parser->data)->query_string;
}

/**
 * @destructor
 */
KT_HTTP_Parser::~KT_HTTP_Parser()
{
    //delete (static_cast<std::string*>(parser->data));
    free (parser);
}

/**
 * @brief Internal method to get the payload.
 * @return String with the payload.
 */
std::string KT_HTTP_Parser::get_payload()
{
	return NULL != payload ? *payload : std::string("");
}

std::string KT_HTTP_Parser::get_url()
{
	return NULL != query_string ? *query_string : std::string("");
}

std::string KT_HTTP_Parser::get_host()
{
	return NULL != host ? *host : std::string("");
}

std::string KT_HTTP_Parser::get_identifier()
{
	return NULL != identifier ? *identifier : std::string("");
}

int KT_HTTP_Parser::get_status_code()
{
	return NULL != status_code ? status_code : 0;
}

/**
 * @brief Overloaded << stream operator to return the actual HTTP Body
 * @param lhs Next ostream object, lhs.
 * @param rhs The parser reference
 * @return Ostream object.
 * @note Use it like that: std::cout << KIARA::Transport::KT_HTTP_Parser(msg) << std::endl;
 */
std::ostream& operator<< (std::ostream& lhs, KT_HTTP_Parser& rhs)
{
	lhs << rhs.get_payload();
	return lhs;
}

/**
 * @brief Callback called when the HTTP body is found.
 */
int body_cb (http_parser* p, char const* at, size_t len)
{
	tmp_parser_fields *parser_fields;
	parser_fields = (tmp_parser_fields*) p->data;
	parser_fields->body->resize(len);
    parser_fields->body->insert (0, at, len);
    return 0;
}

int url_cb (http_parser* p, char const* at, size_t len)
{
	tmp_parser_fields *parser_fields;
	parser_fields = (tmp_parser_fields*) p->data;
    parser_fields->query_string->resize (len);
    parser_fields->query_string->insert (0, at, len);
    return 0;
}

int header_field_cb (http_parser* p, char const* at, size_t len)
{
	std::string field;
	field.resize (len);
    field.insert (0, at, len);
	if(field.compare(0, 4, "Host") == 0) {
		tmp_parser_fields *parser_fields;
		parser_fields = (tmp_parser_fields*) p->data;
		parser_fields->host = new std::string("1");;
	}
    return 0;
}

int header_value_cb (http_parser* p, char const* at, size_t len)
{
	tmp_parser_fields *parser_fields;
	parser_fields = (tmp_parser_fields*) p->data;
	if(parser_fields->host->compare(0, 4, "1") == 0) {
		parser_fields->host->clear();
		parser_fields->host->resize (len);
		parser_fields->host->insert (0, at, len);
	}
    return 0;
}

} /* namespace Transport */
} /* namespace KIARA */
