/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2013  German Research Center for Artificial Intelligence (DFKI)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
 * ServerConfiguration.cpp
 *
 *  Created on: 24.04.2013
 *      Author: Dmitri Rubinstein
 */

#define KIARA_LIB
#include "ServerConfiguration.hpp"
#include "jansson.h"
#include <sstream>

namespace KIARA
{

void ServerConfiguration::clear()
{
    info.clear();
    idlURL.clear();
    idlContents.clear();
    servers.clear();
}

namespace
{

template <class T>
inline bool extractFromJSON(json_t *object, T &value)
{
    return false;
}

template <class T>
inline json_t * convertToJSON(const T &value)
{
    return 0;
}

template <>
inline bool extractFromJSON(json_t *object, std::string &dest)
{
    if (object && json_is_string(object))
        if (const char *value = json_string_value(object))
        {
            dest.assign(value);
            return true;
        }
    return false;
}

template <>
inline json_t * convertToJSON(const std::string &value)
{
    return json_string(value.c_str());
}

template <class T>
inline bool extractFromJSON(json_t *object, std::vector<T> &dest)
{
    if (!object || !json_is_array(object))
        return false;

    size_t numElements = json_array_size(object);
    size_t index = 0;
    dest.clear();
    dest.reserve(numElements);
    for (size_t i = 0; i < numElements; ++i)
    {
        dest.resize(index+1);
        if (!extractFromJSON(json_array_get(object, i), dest[index]))
            return false;
        ++index;
    }
    return true;
}

template <class T>
inline json_t * convertToJSON(const std::vector<T> &src)
{
    json_t *array = json_array();
    for (typename std::vector<T>::const_iterator it = src.begin(), end = src.end(); it != end; ++it)
    {
        json_array_append_new(array, convertToJSON<T>(*it));
    }
    return array;
}

template <>
inline bool extractFromJSON(json_t *object, ProtocolInfo &dest)
{
    if (!json_is_object(object))
        return false;
    dest.clear();

    if (!extractFromJSON(json_object_get(object, "name"), dest.name))
        return false;

    return true;
}

template <>
inline json_t * convertToJSON(const ProtocolInfo &src)
{
    json_t *object = json_object();
    json_object_set_new(object, "name", convertToJSON(src.name));
    return object;
}

template <>
inline bool extractFromJSON(json_t *object, TransportInfo &dest)
{
    if (!json_is_object(object))
        return false;
    dest.clear();

    if (!extractFromJSON(json_object_get(object, "name"), dest.name))
        return false;
    if (!extractFromJSON(json_object_get(object, "url"), dest.url))
        return false;

    return true;
}

template <>
inline json_t * convertToJSON(const TransportInfo &src)
{
    json_t *object = json_object();
    json_object_set_new(object, "name", convertToJSON(src.name));
    json_object_set_new(object, "url", convertToJSON(src.url));
    return object;
}

template <>
inline bool extractFromJSON(json_t *object, ServerInfo &dest)
{
    if (!json_is_object(object))
        return false;
    dest.clear();

    std::string tmpServices;
    json_t *jservices = json_object_get(object, "services");

    if (extractFromJSON(jservices, tmpServices))
        dest.services.assign(1, tmpServices);
    else
        if (!extractFromJSON(jservices, dest.services))
            return false;

    if (!extractFromJSON(json_object_get(object, "protocol"), dest.protocol))
        return false;
    if (!extractFromJSON(json_object_get(object, "transport"), dest.transport))
        return false;

    return true;
}

template <>
inline json_t * convertToJSON(const ServerInfo &src)
{
    json_t *object = json_object();

    json_object_set_new(object, "services", convertToJSON(src.services));
    json_object_set_new(object, "protocol", convertToJSON(src.protocol));
    json_object_set_new(object, "transport", convertToJSON(src.transport));
    return object;
}

template <>
inline bool extractFromJSON(json_t *object, ServerConfiguration &dest)
{
    if (!json_is_object(object))
        return false;
    dest.clear();

    extractFromJSON(json_object_get(object, "info"), dest.info);
    extractFromJSON(json_object_get(object, "idlURL"), dest.idlURL);
    extractFromJSON(json_object_get(object, "idlContents"), dest.idlContents);
    extractFromJSON(json_object_get(object, "servers"), dest.servers);

    return true;
}

template <>
inline json_t * convertToJSON(const ServerConfiguration &src)
{
    json_t *object = json_object();

    json_object_set_new(object, "info", convertToJSON(src.info));
    if (!src.idlURL.empty())
        json_object_set_new(object, "idlURL", convertToJSON(src.idlURL));
    if (!src.idlContents.empty())
        json_object_set_new(object, "idlContents", convertToJSON(src.idlContents));
    json_object_set_new(object, "servers", convertToJSON(src.servers));
    return object;
}

} // unnamed namespace

bool ServerConfiguration::fromJSON(const std::string &jsonStr, std::string *errorMsg)
{
    json_error_t error;
    json_t *data = json_loads(jsonStr.c_str(), 0, &error);
    if (!data)
    {
        if (errorMsg)
        {
            std::ostringstream oss;
            oss<<"JSON parsing error: "<<error.text<<" at "<<error.line<<":"<<error.column;
            *errorMsg = oss.str();
        }
        return false;
    }

    if (!extractFromJSON(data, *this))
    {
        json_decref(data);
        if (errorMsg)
        {
            *errorMsg = "Invalid JSON data";
        }
        return false;
    }

    json_decref(data);
    return true;
}

std::string ServerConfiguration::toJSON() const
{
    json_t *object = convertToJSON(*this);

    char *str = json_dumps(object, JSON_ENCODE_ANY | JSON_INDENT(2));
    json_decref(object);
    std::string result(str);
    free(str);
    return result;
}

} // namespace KIARA
