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
 * ValueIO.cpp
 *
 *  Created on: Jul 16, 2013
 *      Author: Dmitri Rubinstein
 */
#define KIARA_LIB
#include "ValueIO.hpp"
#include "KIARA/Core/Exception.hpp"
#include "jansson.h"

namespace KIARA
{

namespace
{

class ValueToJSONVisitor : public boost::static_visitor<json_t *>
{
public:

    ValueToJSONVisitor() { }

    json_t * operator()(NullValue) const
    {
        return json_null();
    }

    json_t * operator()(const Number &n) const
    {
        if (n.isInteger())
            return json_integer(n.toInt());
        else
            return json_real(n.toDouble());
    }

    json_t * operator()(const bool &b) const
    {
        return json_boolean(b);
    }

    json_t * operator()(const std::string &s) const
    {
        return json_string(s.c_str());
    }

    json_t * operator()(const Object::Ptr &objPtr) const
    {
        if (objPtr)
        {
            std::ostringstream oss;
            oss<<"object<";
            objPtr->printRepr(oss);
            oss<<">";

            return json_string(oss.str().c_str());
        }
        else
            return json_null();
    }

    json_t * operator()(const boost::any &anyVal) const
    {
        std::ostringstream oss;
        oss<<"any<type:"<<anyVal.type().name()<<">";
        return json_string(oss.str().c_str());
    }

    json_t * operator()(const DictValue &d) const
    {
        json_t *object = json_object();

        if (!d.empty())
        {
            DictValue::const_iterator last = d.end();
            --last;

            for (DictValue::const_iterator it = d.begin(), end = d.end();
                 it != end; ++it)
            {
                json_object_set_new(object, it->first.c_str(), it->second.applyVisitor(*this));
            }
        }
        return object;
    }

    json_t * operator()(const ArrayValue &v) const
    {
        json_t *array = json_array();
        for (ArrayValue::const_iterator it = v.begin();
             it != v.end(); ++it)
        {
            json_array_append_new(array, it->applyVisitor(*this));
        }
        return array;
    }

};

} // unnamed namespace


std::string ValueIO::toJSON(const Value &value)
{
    ValueToJSONVisitor jsonVisitor;
    json_t *object = value.applyVisitor(jsonVisitor);
    char *str = json_dumps(object, JSON_ENCODE_ANY | JSON_INDENT(2));
    json_decref(object);
    std::string result(str);
    free(str);
    return result;
}

namespace
{

void createValueFromJSON(json_t *object, Value &dest)
{
    if (json_is_true(object))
        dest.setBool(true);
    else if (json_is_false(object))
        dest.setBool(false);
    else if (json_is_null(object))
        dest.setNull();
    else if (json_is_integer(object))
        dest.set(json_integer_value(object));
    else if (json_is_real(object))
        dest.set(json_real_value(object));
    else if (json_is_string(object))
        dest.setString(json_string_value(object));
    else if (json_is_array(object))
    {
        ArrayValue &destArray = dest.getOrCreateArray();
        destArray.clear();
        for (size_t i = 0, size = json_array_size(object); i < size; ++i)
        {
            Value item;
            createValueFromJSON(json_array_get(object, i), item);
            destArray.push_back(item);
        }
    }
    else if (json_is_object(object))
    {
        DictValue &destDict = dest.getOrCreateDict();
        destDict.clear();

        const char *key;
        json_t *value;

        json_object_foreach(object, key, value) {
            /* block of code that uses key and value */
            Value item;
            createValueFromJSON(value, item);
            destDict[key] = item;
        }
    }
    else
        DFC_THROW_EXCEPTION(Exception, "Internal error: unknown JSON type");
}

} // namespace


bool ValueIO::fromJSON(const std::string &jsonString, Value &result, std::string *errorMsg)
{
    json_error_t error;
    json_t *object = json_loads(jsonString.c_str(), JSON_DECODE_ANY, &error);
    if (!object)
    {
        if (errorMsg)
            *errorMsg = "JSON parsing error: "+std::string(error.text) + " at " +
                boost::lexical_cast<std::string>(error.line) + ":" +
                boost::lexical_cast<std::string>(error.column);
        return false;
    }
    createValueFromJSON(object, result);
    json_decref(object);
    return true;
}

Value ValueIO::fromJSON(const std::string &jsonString)
{
    Value result;
    std::string errorMsg;
    if (!fromJSON(jsonString, result, &errorMsg))
        DFC_THROW_EXCEPTION(Exception, errorMsg);
    return result;
}

} // namespace KIARA
