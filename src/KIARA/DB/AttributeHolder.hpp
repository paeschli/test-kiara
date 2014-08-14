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
 * AttributeHolder.hpp
 *
 *  Created on: 29.05.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_DB_ATTRIBUTEHOLDER_HPP_INCLUDED
#define KIARA_DB_ATTRIBUTEHOLDER_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <KIARA/DB/Value.hpp>

namespace KIARA
{

/// AttributeHolder -- contains attributes
class KIARA_API AttributeHolder
{
public:
    typedef DictValue::iterator AttrIterator;
    typedef DictValue::const_iterator ConstAttrIterator;

    AttributeHolder() : attributeDict_() { }
    ~AttributeHolder() { }

    bool hasAttributes() const
    {
        return !attributeDict_.empty();
    }

    void setAttributes(const DictValue &attrs)
    {
        attributeDict_ = attrs;
    }

    ConstAttrIterator findAttribute(const std::string &key) const
    {
        return attributeDict_.find(key);
    }

    AttrIterator findAttribute(const std::string &key)
    {
        return attributeDict_.find(key);
    }

    bool hasAttribute(const std::string &key) { return findAttribute(key) != attr_end(); }

    ConstAttrIterator attr_begin() const { return attributeDict_.begin(); }
    ConstAttrIterator attr_end() const { return attributeDict_.end(); }

    AttrIterator attr_begin() { return attributeDict_.begin(); }
    AttrIterator attr_end() { return attributeDict_.end(); }

    void removeAttribute(AttrIterator it) { attributeDict_.erase(it); }
    void removeAttribute(const std::string &key) { attributeDict_.erase(key); }

    template <typename T>
    void setAttribute(const std::string &key, const T &value)
    {
        attributeDict_[key] = value;
    }

    void setAttributeAsAny(const std::string &key, const boost::any &value)
    {
        attributeDict_[key] = value;
    }

    Value & getAttribute(const std::string &key)
    {
        return attributeDict_[key];
    }

    template <typename Tag>
    bool hasAttributeValue(const std::string &name) const
    {
        ConstAttrIterator it = findAttribute(name);
        if (it != attr_end())
            return Tag::isValidValue(it->second);
        return false;
    }

    template <typename Tag>
    bool hasAttributeValue() const
    {
        return hasAttributeValue<Tag>(Tag::getAttrName());
    }

    template <typename Tag>
    typename Tag::type getAttributeValue(const std::string &name) const
    {
        ConstAttrIterator it = findAttribute(name);
        if (it != attr_end())
        {
            if (Tag::isValidValue(it->second))
            {
                return Tag::get(it->second);
            }
        }
        return Tag::getDefaultValue();
    }

    template <typename Tag>
    typename Tag::type getAttributeValue() const
    {
        return getAttributeValue<Tag>(Tag::getAttrName());
    }

    template <typename Tag>
    typename Tag::type * getAttributeValuePtr(const std::string &name)
    {
        AttrIterator it = findAttribute(name);
        if (it == attr_end())
            return 0;
        return Tag::get_ptr(it->second);
    }

    template <typename Tag>
    typename Tag::type & getOrCreateAttributeValueRef(const std::string &name)
    {
        AttrIterator it = findAttribute(name);
        if (it == attr_end())
        {
            it = attributeDict_.insert(std::make_pair(name, Value())).first;
            Tag::set(it->second, Tag::getDefaultValue());
        }
        return *Tag::get_ptr(it->second);
    }

    template <typename Tag>
    const typename Tag::type * getAttributeValuePtr(const std::string &name) const
    {
        ConstAttrIterator it = findAttribute(name);
        if (it == attr_end())
            return 0;
        return Tag::get_const_ptr(it->second);
    }

    template <typename Tag>
    typename Tag::type * getAttributeValuePtr()
    {
        return getAttributeValuePtr<Tag>(Tag::getAttrName());
    }

    template <typename Tag>
    typename Tag::type & getOrCreateAttributeValueRef()
    {
        return getOrCreateAttributeValueRef<Tag>(Tag::getAttrName());
    }

    template <typename Tag>
    const typename Tag::type * getAttributeValuePtr() const
    {
        return getAttributeValuePtr<Tag>(Tag::getAttrName());
    }

    template <typename Tag>
    void setAttributeValue(const std::string &name, typename Tag::arg_type value)
    {
        Tag::set(attributeDict_[name], value);
    }

    template <typename Tag>
    void setAttributeValue(typename Tag::arg_type value)
    {
        setAttributeValue<Tag>(Tag::getAttrName(), value);
    }

    std::string getAttributeAsString(const std::string &key) const
    {
        ConstAttrIterator it = findAttribute(key);
        if (it != attr_end())
        {
            if (it->second.isString())
                return it->second.getString();
            return it->second.toString();
        }
        return "";
    }

    boost::any getAttributeAsAny(const std::string &key)
    {
        ConstAttrIterator it = findAttribute(key);
        if (it != attr_end())
        {
            if (it->second.isAny())
                return it->second.getAny();
        }
        return boost::any();
    }

    void print(std::ostream &out) const
    {
        out<<attributeDict_;
    }

private:
    DictValue attributeDict_;
};

} // namespace KIARA

#endif /* KIARA_ATTRIBUTEHOLDER_HPP_INCLUDED */
