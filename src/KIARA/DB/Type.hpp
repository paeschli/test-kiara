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
 * Type.hpp
 *
 *  Created on: 02.08.2012
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_DB_TYPE_HPP_INCLUDED
#define KIARA_DB_TYPE_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <boost/assert.hpp>
#include <KIARA/kiara.h>
#include <KIARA/DB/Object.hpp>
#include <KIARA/DB/Value.hpp>
#include <KIARA/DB/AttributeHolder.hpp>
#include <KIARA/Utils/ArrayRef.hpp>
#include <KIARA/Utils/TypedBox.hpp>
#include <KIARA/Common/stdint.h>
#include "Enums.hpp"
#include <map>

namespace KIARA
{

class World;
class Type;
typedef DFC::PointerTraits<Type>::Ptr TypePtr;
typedef DFC::PointerTraits<Type>::RawPtr TypeRawPtr;
typedef std::map<std::string, TypePtr> TypeMap;
typedef std::set<TypePtr> TypeSet;
typedef std::vector<TypePtr> TypeList;

class KIARA_API Namespace : public Object
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(Namespace, Object)
    friend class World;
public:

    typedef TypeMap::iterator typemap_iterator;
    typedef TypeMap::const_iterator typemap_const_iterator;

    static Namespace::Ptr create(World &world, const std::string &name);

    const std::string & getName() const { return name_; }

    std::string getFullName() const;

    const Ptr & getParent() const { return parent_; }
    void setParent(const Namespace::Ptr &parent);

    void bindType(const std::string &name, const TypePtr &type, bool takeOwnership = true);
    const TypePtr lookupType(const std::string &name) const;
    const std::string getTypeName(const TypePtr &type) const;

    virtual void print(std::ostream &out) const;

    typemap_iterator typemap_begin() { return typeMap_.begin(); }
    typemap_iterator typemap_end() { return typeMap_.end(); }

    typemap_const_iterator typemap_begin() const { return typeMap_.begin(); }
    typemap_const_iterator typemap_end() const { return typeMap_.end(); }

protected:

    virtual void gcUnlinkRefs();

    virtual void gcApplyToChildren(const CollectorCallback &callback);

private:

    Namespace(World& world, const std::string &name);

    std::string name_;
    Namespace::Ptr parent_;
    TypeMap typeMap_;
    std::vector<TypePtr> subnamespaces_;
};

class KIARA_API Type : public Object, public AttributeHolder
{
    DFC_DECLARE_ABSTRACT_TYPE(Type, Object)
public:

    const Namespace::Ptr & getNamespace() const { return namespace_; }
    void setNamespace(const Namespace::Ptr &newNamespace);

    const std::string & getTypeName() const { return name_; }

    std::string getFullTypeName() const;

    int kind() const { return kind_; }

    const std::vector<Object::Ptr> & getElements() const { return elements_; }

    size_t getNumElements() const { return elements_.size(); }

    template <class T>
    typename DFC::PointerTraits<T>::Ptr getAs() const
    {
        typename DFC::PointerTraits<T>::Ptr ty = dyn_cast<T>(this);
        if (!ty)
            ty = dyn_cast<T>(getCanonicalType());
        return ty;
    }

    template <class T>
    typename DFC::PointerTraits<T>::Ptr getAs()
    {
        typename DFC::PointerTraits<T>::Ptr ty = dyn_cast<T>(this);
        if (!ty)
            ty = dyn_cast<T>(getCanonicalType());
        return ty;
    }

    template <class T>
    typename DFC::PointerTraits<T>::Ptr getElementAs(size_t index) const
    {
        return cast<T>(elements_[index]);
    }

    // FIXME: getSafeAs is deprecated
    template <class T>
    typename DFC::PointerTraits<T>::Ptr getSafeElementAs(size_t index) const
    {
        return dyn_cast<T>(elements_[index]);
    }

    const Type::Ptr & getCanonicalType() const { return canonicalType_; }

    bool isCanonicalType() const { return this == canonicalType_; }

    virtual bool equals(const Object::Ptr &other) const;

    virtual size_t hash() const;

    void print(std::ostream &out) const;

    virtual void print(std::ostream &out, std::set<const Type*> &visited) const;

protected:

    Type(World& world, const std::string &name, int kind, size_t num);
    Type(World& world, const std::string &name, int kind, ArrayRef<Object::Ptr> elems);
    Type(World& world, const std::string &name, int kind, size_t num, const Type::Ptr &canonicalType);
    Type(World& world, const std::string &name, int kind,  ArrayRef<Object::Ptr> elems, const Type::Ptr &canonicalType);

    void resizeElements(size_t newSize);

    void setCanonicalTypeUnsafe(const Type::Ptr &type);

    virtual void gcUnlinkRefs();

    virtual void gcApplyToChildren(const CollectorCallback &callback);

    std::vector<Object::Ptr> elements_;

private:
    std::string name_;
    int kind_;
    Namespace::Ptr namespace_;
    Type::Ptr canonicalType_;
};

inline bool canonicallyEqual(const Type::Ptr &a, const Type::Ptr &b)
{
    if (a == b)
        return true;
    if (a && b)
        return a->getCanonicalType() == b->getCanonicalType();
    return false;
}

template <class T>
inline typename DFC::PointerTraits<T>::Ptr getTypeAs(const Type::Ptr &ty)
{
    if (!ty)
        return 0;
    return ty->getAs<T>();
}

} // namespace KIARA

#endif /* KIARA_TYPE_HPP_INCLUDED */
