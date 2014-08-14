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
 * TypeDatabase.cpp
 *
 *  Created on: Nov 27, 2013
 *      Author: Dmitri Rubinstein
 */

#include "TypeDatabase.hpp"
#include <boost/functional/hash.hpp>
#include <algorithm>

namespace KIARA
{

namespace TDB
{

/// DBObject

DBObject::DBObject(Database &db, Kind kind, DBObject *parent)
    : database_(db), kind_(kind), parent_(parent)
{
}

DBObject::~DBObject()
{

}

void DBObject::setParent(DBObject *parent)
{
    parent_ = parent;
}

bool DBObject::equals(const DBObject* other) const
{
    return this == other;
}

size_t DBObject::hash() const
{
    return boost::hash_value(this);
}

void DBObject::dump() const
{
    printRepr(std::cerr);
}

/// Namespace

Namespace::Namespace(Database& db, const std::string &name)
    : DBObject(db, KIND_NAMESPACE)
    , name_(name)
    , typeMap_()
    , subnamespaces_()
{
}

Namespace* Namespace::create(Database &db, const std::string &name)
{
    return db.newObject(new Namespace(db, name));
}

std::string Namespace::getFullName() const
{
    std::string fullName;
    std::vector<const std::string *> names;
    Namespace* ns = getParentNamespace();
    while (ns)
    {
        names.push_back(&ns->getName());
        ns = ns->getParentNamespace();
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

bool Namespace::bindType(const std::string &name, Type *type, bool takeOwnership)

{
    BOOST_ASSERT(type != 0);
    BOOST_ASSERT(&type->getDatabase() == &getDatabase());

    if (typeMap_.find(name) != typeMap_.end())
        return false;

    typeMap_[name] = type;
    if (takeOwnership)
    {
        type->setParent(this);
    }

    return true;
}

Type * Namespace::lookupType(const std::string &name) const
{
    TypeMap::const_iterator it = typeMap_.find(name);
    return it != typeMap_.end() ? it->second : 0;
}

const std::string Namespace::getTypeName(Type *type) const
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

/// Type

Type::Type(Database& db, const std::string &name, Kind kind, size_t num)
    : DBObject(db, kind), elements_(num), name_(name)
{ }

Type::Type(Database& db, const std::string &name, Kind kind, ArrayRef<DBObject*> elems)
    : DBObject(db, kind), elements_(elems.begin(), elems.end()), name_(name)
{ }

std::string Type::getFullTypeName() const
{
    if (getParent() && llvm::isa<Namespace>(getParent()))
        return static_cast<Namespace*>(getParent())->getFullName() + "::" + getTypeName();
    else
        return getTypeName();
}

bool Type::equals(const DBObject * other) const
{
    if (getKind() != other->getKind())
        return false;
    const Type* otherTy = llvm::cast<Type>(other);
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
    boost::hash_combine(seed, getKind());
    boost::hash_combine(seed, elements_);
    return seed;
}

void Type::print(std::ostream & p) const
{
    p << getFullTypeName();
}

void Type::resizeElements(size_t newSize)
{
    elements_.resize(newSize);
}

/// Database

Database::Database()
{

}

Database::~Database()
{

}

void Database::clear()
{
    for (std::vector<DBObject*>::iterator it = objects_.begin(), end = objects_.end();
        it != end; ++it)
    {
        delete *it;
        *it = 0;
    }
    objects_.clear();
}

} // namespace TDB

} // namespace KIARA
