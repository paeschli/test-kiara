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
 * StringAttributeHolder.hpp
 *
 *  Created on: 13.04.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_UTILS_STRINGATTRIBUTEHOLDER_HPP_INCLUDED
#define KIARA_UTILS_STRINGATTRIBUTEHOLDER_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <string>
#include <map>
#include <ostream>

namespace KIARA
{

typedef std::map<std::string, std::string> AttributeMap;

/// AttributeHolder -- contains string attributes
class KIARA_API StringAttributeHolder
{
public:
    typedef KIARA::AttributeMap AttributeMap;
    typedef AttributeMap::iterator AttrIterator;
    typedef AttributeMap::const_iterator ConstAttrIterator;

    StringAttributeHolder() : attributeMap_() { }
    ~StringAttributeHolder() { }

    bool hasAttributes() const
    {
        return !attributeMap_.empty();
    }

    void setAttributes(const AttributeMap &attrs)
    {
        attributeMap_ = attrs;
    }

    ConstAttrIterator findAttribute(const std::string &key) const
    {
        return attributeMap_.find(key);
    }

    AttrIterator findAttribute(const std::string &key)
    {
        return attributeMap_.find(key);
    }

    bool hasAttribute(const std::string &key) { return findAttribute(key) != attr_end(); }

    ConstAttrIterator attr_begin() const { return attributeMap_.begin(); }
    ConstAttrIterator attr_end() const { return attributeMap_.end(); }

    AttrIterator attr_begin() { return attributeMap_.begin(); }
    AttrIterator attr_end() { return attributeMap_.end(); }

    void removeAttribute(AttrIterator it) { attributeMap_.erase(it); }
    void removeAttribute(const std::string &key) { attributeMap_.erase(key); }

    void setAttribute(const std::string &key, const std::string &value)
    {
        attributeMap_[key] = value;
    }

    std::string getAttribute(const std::string &key)
    {
        return attributeMap_[key];
    }

    void print(std::ostream &out) const
    {
        if (!attributeMap_.empty())
        {
            out << "[";
            for (ConstAttrIterator it = attributeMap_.begin(), end = attributeMap_.end(); it != end; ++it)
            {
                out << it->first;
                if (it->second != "true" && it->second != "1")
                {
                    out << "(" << it->second << ")";
                }
                ConstAttrIterator tmp = it;
                ++tmp;
                if (tmp != end)
                {
                    out << ", ";
                }
            }
            out << "] ";
        }
    }


private:
    AttributeMap attributeMap_;
};

} // namespace KIARA

#endif /* KIARA_UTILS_STRINGATTRIBUTEHOLDER_HPP_INCLUDED */
