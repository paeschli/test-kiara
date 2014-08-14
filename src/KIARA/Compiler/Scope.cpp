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
 * Scope.cpp
 *
 *  Created on: 01.02.2013
 *      Author: Dmitri Rubinstein
 */

#define KIARA_COMPILER_LIB
#include "Scope.hpp"
#include <DFC/Base/Core/ObjectFactory.hpp>
#include <DFC/Base/Core/ObjectMacros.hpp>
#include <DFC/Base/Utils/StaticInit.hpp>
#include <KIARA/Core/Exception.hpp>
#include <KIARA/Utils/IndentingStreambuf.hpp>

namespace KIARA
{

namespace Compiler
{

// -- OverloadedObjectMap --

DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::Compiler::OverloadedObjectMap)

OverloadedObjectMap::OverloadedObjectMap(const OverloadedObjectMap::Ptr &other)
    : Object(other->getWorld())
    , name_(other->name_)
    , objectMap_(other->objectMap_)
{
}

OverloadedObjectMap::OverloadedObjectMap(const OverloadedObjectMap &other)
    : Object(other.getWorld())
    , name_(other.name_)
    , objectMap_(other.objectMap_)
{
}

OverloadedObjectMap::OverloadedObjectMap(World& world, const std::string &name)
    : Object(world)
    , name_(name)
    , objectMap_()
{
}

OverloadedObjectMap::OverloadedObjectMap(World& world, const std::string &name, const ObjectMap &other)
    : Object(world)
    , name_(name)
    , objectMap_(other)
{
}

void OverloadedObjectMap::addObject(const std::string &name, const Object::Ptr &object)
{
    BOOST_ASSERT(object != 0);
    BOOST_ASSERT(&object->getWorld() == &getWorld());

    if (objectMap_.find(name) != objectMap_.end())
        DFC_THROW_EXCEPTION(Exception, "Object '"<<name<<"' already defined.");

    objectMap_[name] = object;
}

bool OverloadedObjectMap::removeObject(const std::string &name)
{
    ObjectMap::iterator it = objectMap_.find(name);
    if (it != objectMap_.end())
    {
        objectMap_.erase(it);
        return true;
    }
    return false;
}

const Object::Ptr OverloadedObjectMap::lookupObject(const std::string &name) const
{
    ObjectMap::const_iterator it = objectMap_.find(name);

    if (it != objectMap_.end())
        return it->second;
    return Object::Ptr();
}

const std::string OverloadedObjectMap::getObjectName(const Object::Ptr &object) const
{
    for (ObjectMap::const_iterator it = objectMap_.begin(), end = objectMap_.end();
            it != end; ++it)
    {
        if (it->second == object)
            return it->first;
    }
    return "";
}

void OverloadedObjectMap::print(std::ostream &out) const
{
    if (objectMap_.size() == 1)
    {
        ObjectMap::const_iterator it = objectMap_.begin();
        if (name_ != it->first)
            out<<" mangled \""<<it->first<<"\" : ";
        if (it->second)
            it->second->print(out);
        else
            out<<"NULL";
    }
    else
    {
        out<<"overloaded \""<<name_<<"\" {\n";
        {
            IndentingStreambuf isb(out, 1);
            for (ObjectMap::const_iterator it = objectMap_.begin(), end = objectMap_.end();
                    it != end; ++it)
            {
                if (it != objectMap_.begin())
                    out<<",\n";
                out<<"\""<<it->first<<"\" : ";
                if (it->second)
                    it->second->print(out);
                else
                    out<<"NULL";
            }
            if (!objectMap_.empty())
                out<<"\n";
        }
        out<<" }";
    }
}

void OverloadedObjectMap::dump() const
{
    print(std::cerr);
}

void OverloadedObjectMap::gcUnlinkRefs()
{
    InheritedType::gcUnlinkRefs();
    gcUnlinkChildren(GCObject::map_values_tag(), objectMap_.begin(), objectMap_.end());
}

void OverloadedObjectMap::gcApplyToChildren(const CollectorCallback &callback)
{
    InheritedType::gcApplyToChildren(callback);
    gcApply(GCObject::map_values_tag(), objectMap_.begin(), objectMap_.end(), callback);
}

// -- Scope --

DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::Compiler::Scope)

Scope::Scope(World& world, const std::string &name)
    : Object(world)
    , name_(name)
    , parent_()
    , objectMap_()
    , subscopes_()
{
}

Scope::Scope(World& world, const std::string &name, const Scope::Ptr &parent)
    : Object(world)
    , name_(name)
    , parent_(parent)
    , objectMap_()
    , subscopes_()
{
}

std::string Scope::getFullName() const
{
    std::string fullName;
    std::vector<const std::string *> names;
    Scope::Ptr ns = parent_;
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

void Scope::setParent(const Scope::Ptr &parent)
{
    parent_ = parent;
}

Scope::Ptr Scope::getTopScope() const
{
    const Scope* scope = this;
    while (scope->parent_ != 0)
    {
        scope = scope->parent_.get();
    }
    return const_cast<Scope*>(scope);
}

void Scope::addObject(const std::string &name, const Object::Ptr &object)
{
    BOOST_ASSERT(object != 0);
    BOOST_ASSERT(&object->getWorld() == &getWorld());

    if (objectMap_.find(name) != objectMap_.end())
        DFC_THROW_EXCEPTION(Exception, "Object '"<<name<<"' already defined.");

    objectMap_[name] = object;
}

void Scope::removeObject(const std::string &name)
{
    ObjectMap::iterator it = objectMap_.find(name);
    if (it != objectMap_.end())
        objectMap_.erase(it);
}

const Object::Ptr Scope::lookupObject(const std::string &name, bool recursive) const
{
    if (recursive)
    {
        OverloadedObjectMap::Ptr ovm(new OverloadedObjectMap(getWorld(), name));
        lookupOverloadedObjects(name, ovm->objectMap_);
        if (!ovm->empty())
        {
            return ovm;
        }
    }

    ObjectMap::const_iterator it = objectMap_.find(name);

    if (it != objectMap_.end())
        return it->second;
    else
        return (recursive && parent_) ? parent_->lookupObject(name, recursive) : Object::Ptr();
}

void Scope::lookupObjectsRecursive(const std::string &name, ObjectList &objects) const
{
    if (parent_)
        parent_->lookupObjectsRecursive(name, objects);

    ObjectMap::const_iterator it = objectMap_.find(name);
    if (it != objectMap_.end())
    {
        if (OverloadedObjectMap::Ptr overloadedObjects = dyn_cast<OverloadedObjectMap>(it->second))
        {
            for (OverloadedObjectMap::const_iterator j = overloadedObjects->begin(), end = overloadedObjects->end();
                j != end; ++j)
                objects.push_back(j->second);
        }
    }
}

void Scope::lookupOverloadedObjects(const std::string &name, ObjectMap &objects) const
{
    if (parent_)
        parent_->lookupOverloadedObjects(name, objects);

    ObjectMap::const_iterator it = objectMap_.find(name);
    if (it != objectMap_.end())
    {
        if (OverloadedObjectMap::Ptr overloadedObjects = dyn_cast<OverloadedObjectMap>(it->second))
        {
            for (OverloadedObjectMap::const_iterator it = overloadedObjects->begin(), end = overloadedObjects->end();
                it != end; ++it)
                objects[it->first] = it->second;
        }
    }
}

std::pair<Object::Ptr, const Scope *> Scope::lookupObjectAndScope(const std::string &name) const
{
    ObjectMap::const_iterator it = objectMap_.find(name);

    if (it != objectMap_.end())
        return std::make_pair(it->second, this);
    else if (parent_)
        return parent_->lookupObjectAndScope(name);
    else
        return std::make_pair(Object::Ptr(), static_cast<const Scope *>(0));
}

const std::string Scope::getObjectName(const Object::Ptr &object, bool recursive) const
{
    for (ObjectMap::const_iterator it = objectMap_.begin(), end = objectMap_.end();
            it != end; ++it)
    {
        if (it->second == object)
            return it->first;
    }
    if (recursive && parent_)
        return parent_->getObjectName(object, recursive);
    return "";
}

void Scope::print(std::ostream &out) const
{
    out<<"Scope("<<name_<<") = {\n";
    {
        IndentingStreambuf isb(out, 1);
        if (parent_)
        {
            out<<"parent = ";
            parent_->print(out);
        }

        for (ObjectMap::const_iterator it = objectMap_.begin(), end = objectMap_.end();
                it != end; ++it)
        {
            out<<"\""<<it->first<<"\" : ";
            if (it->second)
                it->second->print(out);
            out<<"\n";
        }
    }
    out<<"} // Scope ("<<name_<<")"<<std::endl;
}

void Scope::dump() const
{
    print(std::cerr);
}

void Scope::dump(const ObjectList &objects)
{
    std::cerr<<"ObjectList = [\n";
    {
        IndentingStreambuf isb(std::cerr, 1);

        for (ObjectList::const_iterator it = objects.begin(), end = objects.end();
                it != end; ++it)
        {
            if (*it)
                (*it)->print(std::cerr);
            std::cerr<<"\n";
        }
    }
    std::cerr<<"] // ObjectList"<<std::endl;
}

void Scope::gcUnlinkRefs()
{
    InheritedType::gcUnlinkRefs();
    gcUnlinkChild(parent_);
    gcUnlinkChildren(GCObject::map_values_tag(), objectMap_.begin(), objectMap_.end());
    gcUnlinkChildren(subscopes_.begin(), subscopes_.end());
}

void Scope::gcApplyToChildren(const CollectorCallback &callback)
{
    InheritedType::gcApplyToChildren(callback);
    gcApply(parent_, callback);
    gcApply(GCObject::map_values_tag(), objectMap_.begin(), objectMap_.end(), callback);
    gcApply(subscopes_.begin(), subscopes_.end(), callback);
}

DFC_STATIC_INIT_FUNC {
    DFC_REGISTER_TYPE(OverloadedObjectMap);
    DFC_REGISTER_TYPE(Scope);
}

} // namespace Compiler

} // namespace KIARA
