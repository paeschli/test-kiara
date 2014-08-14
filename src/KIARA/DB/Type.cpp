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
 * Type.cpp
 *
 *  Created on: 02.08.2012
 *      Author: Dmitri Rubinstein
 */

#define KIARA_LIB
#include "Type.hpp"
#include "World.hpp"
#include <KIARA/Utils/Error.hpp>
#include <KIARA/Core/Exception.hpp>

namespace KIARA
{

/// Namespace

DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::Namespace)

Namespace::Namespace(World& world, const std::string &name)
    : Object(world)
    , name_(name)
    , parent_()
    , typeMap_()
    , subnamespaces_()
{
}

Namespace::Ptr Namespace::create(World &world, const std::string &name)
{
    return new Namespace(world, name);
}

std::string Namespace::getFullName() const
{
    std::string fullName;
    std::vector<const std::string *> names;
    Namespace::Ptr ns = parent_;
    while (ns)
    {
        names.push_back(&ns->getName());
        ns = ns->getParent();
    }
    for (std::vector<const std::string *>::reverse_iterator it = names.rbegin(),
            end = names.rend(); it != end; ++it)
    {
        fullName += *(*it);
        fullName += '.';
    }
    fullName += getName();

    return fullName;
}

void Namespace::setParent(const Namespace::Ptr &parent)
{
    parent_ = parent;
}

void Namespace::bindType(const std::string &name, const Type::Ptr &type, bool takeOwnership)
{
    BOOST_ASSERT(type != 0);
    BOOST_ASSERT(&type->getWorld() == &getWorld());

    if (typeMap_.find(name) != typeMap_.end())
        DFC_THROW_EXCEPTION(Exception, "Type '"<<name<<"' already defined.");

    typeMap_[name] = type;
    if (takeOwnership)
    {
        type->setNamespace(this);
    }
}

const Type::Ptr Namespace::lookupType(const std::string &name) const
{
    TypeMap::const_iterator it = typeMap_.find(name);
    return it != typeMap_.end() ? it->second : Type::Ptr();
}

const std::string Namespace::getTypeName(const Type::Ptr &type) const
{
    for (TypeMap::const_iterator it = typeMap_.begin(), end = typeMap_.end();
            it != end; ++it)
    {
        if (it->second == type)
            return it->first;
    }
    return "";
}

void Namespace::print(std::ostream &out) const
{
    out<<"Namespace("<<name_<<")";
}

void Namespace::gcUnlinkRefs()
{
    InheritedType::gcUnlinkRefs();
    gcUnlinkChild(parent_);
    gcUnlinkChildren(GCObject::map_values_tag(), typeMap_.begin(), typeMap_.end());
    gcUnlinkChildren(subnamespaces_.begin(), subnamespaces_.end());
}

void Namespace::gcApplyToChildren(const CollectorCallback &callback)
{
    InheritedType::gcApplyToChildren(callback);
    gcApply(parent_, callback);
    gcApply(GCObject::map_values_tag(), typeMap_.begin(), typeMap_.end(), callback);
    gcApply(subnamespaces_.begin(), subnamespaces_.end(), callback);
}

/// Type

DFC_DEFINE_ABSTRACT_TYPE(KIARA::Type)

Type::Type(World& world, const std::string &name, int kind, size_t num)
    : Object(world), elements_(num), name_(name), kind_(kind),
      namespace_(), canonicalType_(this)
{ }

Type::Type(World& world, const std::string &name, int kind, ArrayRef<Object::Ptr> elems)
    : Object(world), elements_(elems.begin(), elems.end()), name_(name), kind_(kind),
      namespace_(), canonicalType_(this)
{ }

Type::Type(World& world, const std::string &name, int kind, size_t num, const Type::Ptr &canonicalType)
    : Object(world), elements_(num), name_(name), kind_(kind),
      namespace_(), canonicalType_(canonicalType)
{ }

Type::Type(World& world, const std::string &name, int kind, ArrayRef<Object::Ptr> elems, const Type::Ptr &canonicalType)
    : Object(world), elements_(elems.begin(), elems.end()), name_(name), kind_(kind),
      namespace_(), canonicalType_(canonicalType)
{ }

void Type::setNamespace(const Namespace::Ptr &newNamespace)
{
    namespace_ = newNamespace;
}

std::string Type::getFullTypeName() const
{
    if (namespace_)
        return namespace_->getFullName() + "." + getTypeName();
    else
        return getTypeName();
}


bool Type::equals(const Object::Ptr &other) const
{
    Type::Ptr otherTy = DFC::safe_object_cast<Type>(other);
    if (!otherTy)
        return false;
    if (kind() != otherTy->kind())
        return false;
    if (elements_.size() != otherTy->elements_.size())
        return false;
    for (size_t i = 0; i < elements_.size(); ++i)
    {
        if (elements_[i] == otherTy->elements_[i])
            continue;

        if (elements_[i] != 0 && elements_[i]->equals(otherTy->elements_[i]))
            continue;

        return false;
    }
    return true;
}

size_t Type::hash() const
{
    size_t seed = 0;
    boost::hash_combine(seed, kind());
    boost::hash_combine(seed, elements_);
    return seed;
}

void Type::print(std::ostream &out) const
{
    std::set<const Type*> visited;
    print(out, visited);
}

void Type::print(std::ostream & p, std::set<const Type*> &visited) const
{
    p << getFullTypeName();
}

void Type::resizeElements(size_t newSize)
{
    elements_.resize(newSize);
}

void Type::setCanonicalTypeUnsafe(const Type::Ptr &type)
{
    canonicalType_ = type;
}

void Type::gcUnlinkRefs()
{
    InheritedType::gcUnlinkRefs();
    gcUnlinkChildren(elements_.begin(), elements_.end());
    gcUnlinkChild(namespace_);
    gcUnlinkChild(canonicalType_);
}

void Type::gcApplyToChildren(const CollectorCallback &callback)
{
    InheritedType::gcApplyToChildren(callback);
    gcApply(elements_.begin(), elements_.end(), callback);
    gcApply(namespace_, callback);
    gcApply(canonicalType_, callback);
}

} // namespace KIARA
