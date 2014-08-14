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
 * Scope.hpp
 *
 *  Created on: 01.02.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMPILER_SCOPE_HPP_INCLUDED
#define KIARA_COMPILER_SCOPE_HPP_INCLUDED

#include "Config.hpp"
#include <KIARA/Common/Config.hpp>
#include <KIARA/DB/Object.hpp>
#include <map>
#include <string>

namespace KIARA
{

namespace Compiler
{

class KIARA_COMPILER_API OverloadedObjectMap : public Object
{
    friend class Scope;
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(OverloadedObjectMap, Object)
public:

    typedef std::map<std::string, Object::Ptr> ObjectMap;
    typedef ObjectMap::iterator iterator;
    typedef ObjectMap::const_iterator const_iterator;

    OverloadedObjectMap(const OverloadedObjectMap::Ptr &other);

    OverloadedObjectMap(const OverloadedObjectMap &other);

    OverloadedObjectMap(World& world, const std::string &name);

    OverloadedObjectMap(World& world, const std::string &name, const ObjectMap &other);

    const std::string & getName() const { return name_; }

    size_t getNumObjects() const { return objectMap_.size(); }

    void addObject(const std::string &name, const Object::Ptr &object);
    bool removeObject(const std::string &name);
    const Object::Ptr lookupObject(const std::string &name) const;
    const std::string getObjectName(const Object::Ptr &object) const;

    virtual void print(std::ostream &out) const;

    bool empty() const { return objectMap_.empty(); }

    iterator begin() { return objectMap_.begin(); }
    iterator end() { return objectMap_.end(); }

    const_iterator begin() const { return objectMap_.begin(); }
    const_iterator end() const { return objectMap_.end(); }

    // dump scope contents to stderr
    void dump() const;

protected:
    virtual void gcUnlinkRefs();
    virtual void gcApplyToChildren(const CollectorCallback &callback);
private:
    std::string name_;
    ObjectMap objectMap_;
};

// -- Scope --

class KIARA_COMPILER_API Scope : public Object
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(Scope, Object)
    friend class World;
public:

    typedef std::vector<Object::Ptr> ObjectList;
    typedef std::map<std::string, Object::Ptr> ObjectMap;
    typedef ObjectMap::iterator iterator;
    typedef ObjectMap::const_iterator const_iterator;

    Scope(World& world, const std::string &name);

    Scope(World& world, const std::string &name, const Scope::Ptr &parent);

    const std::string & getName() const { return name_; }

    std::string getFullName() const;

    const Ptr & getParent() const { return parent_; }
    void setParent(const Scope::Ptr &parent);

    Ptr getTopScope() const;

    void addObject(const std::string &name, const Object::Ptr &object);
    void removeObject(const std::string &name);
    const Object::Ptr lookupObject(const std::string &name, bool recursive = true) const;

    void lookupObjectsRecursive(const std::string &name, ObjectList &objects) const;

    std::pair<Object::Ptr, const Scope *> lookupObjectAndScope(const std::string &name) const;

    const std::string getObjectName(const Object::Ptr &object, bool recursive = true) const;

    virtual void print(std::ostream &out) const;

    iterator begin() { return objectMap_.begin(); }
    iterator end() { return objectMap_.end(); }

    const_iterator begin() const { return objectMap_.begin(); }
    const_iterator end() const { return objectMap_.end(); }

    // dump scope contents to stderr
    void dump() const;

    static void dump(const ObjectList &objects);

protected:
    virtual void gcUnlinkRefs();
    virtual void gcApplyToChildren(const CollectorCallback &callback);
private:
    std::string name_;
    Scope::Ptr parent_;
    ObjectMap objectMap_;
    std::vector<Scope::Ptr> subscopes_;

    void lookupOverloadedObjects(const std::string &name, ObjectMap &objects) const;
};

} // namespace Compiler

} // namespace KIARA

#endif /* KIARA_COMPILER_SCOPE_HPP_INCLUDED */
