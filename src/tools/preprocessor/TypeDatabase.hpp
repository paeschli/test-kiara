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
 * TypeDatabase.hpp
 *
 *  Created on: Nov 27, 2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_TYPEDATABASE_HPP_INCLUDED
#define KIARA_TYPEDATABASE_HPP_INCLUDED

#include <sstream>
#include <iostream>
#include <vector>
#include <set>
#include <map>
#include "llvm/Support/Casting.h"
#include "KIARA/Utils/ArrayRef.hpp"

namespace KIARA
{

namespace TDB
{

class Database;

class DBObject
{
    friend class Database;
public:

    enum Kind
    {
        KIND_NAMESPACE,
        KIND_PRIMTYPE,
        KIND_PRIMVALUETYPE,
        KIND_VOIDTYPE,
        KIND_TYPETYPE,
        KIND_UNRESOLVEDSYMBOLTYPE,
        KIND_ANYTYPE,
        KIND_ENUMTYPE,
        KIND_STRUCTTYPE,
        KIND_PTRTYPE,
        KIND_REFTYPE,
        KIND_ARRAYTYPE,
        KIND_FIXEDARRAYTYPE,
        KIND_SYMBOLTYPE,
        KIND_FUNCTIONTYPE,
        KIND_SERVICETYPE,
        KIND_FIRST_TYPE = KIND_PRIMTYPE,
        KIND_LAST_TYPE = KIND_SERVICETYPE,
    };

    Database &getDatabase() const { return database_; }

    Kind getKind() const { return kind_; }

    DBObject * getParent() const { return parent_; }

    void setParent(DBObject *parent);

    /** Returns true if objects are equal.
     *  Default implementation returns true if objects are identical
     *  by comparing pointers.
     */
    virtual bool equals(const DBObject *other) const;

    virtual size_t hash() const;

    bool operator==(const DBObject *other) const
    {
        return equals(other);
    }

    virtual void print(std::ostream &out) const = 0;

    virtual void printRepr(std::ostream &out) const
    {
        print(out);
    }

    void dump() const;

    std::string toString() const
    {
        std::ostringstream oss;
        print(oss);
        return oss.str();
    }

    std::string toReprString() const
    {
        std::ostringstream oss;
        printRepr(oss);
        return oss.str();
    }

protected:

    DBObject(Database &db, Kind kind, DBObject *parent = 0);
    virtual ~DBObject();

private:
    Database &database_;
    const Kind kind_;
    DBObject *parent_;
};

class Type;
typedef std::map<std::string, Type*> TypeMap;
typedef std::set<Type*> TypeSet;
typedef std::vector<Type*> TypeList;

class Namespace : public DBObject
{
    friend class Database;
public:

    typedef TypeMap::iterator typemap_iterator;
    typedef TypeMap::const_iterator typemap_const_iterator;

    static Namespace * create(Database &db, const std::string &name);

    const std::string & getName() const { return name_; }

    Namespace * getParentNamespace() const
    {
        return (getParent() && getParent()->getKind() == KIND_NAMESPACE) ? static_cast<Namespace*>(getParent()) : 0;
    }

    std::string getFullName() const;

    bool bindType(const std::string &name, Type *type, bool takeOwnership = true);
    Type * lookupType(const std::string &name) const;
    const std::string getTypeName(Type *type) const;

    virtual void print(std::ostream &out) const;

    typemap_iterator typemap_begin() { return typeMap_.begin(); }
    typemap_iterator typemap_end() { return typeMap_.end(); }

    typemap_const_iterator typemap_begin() const { return typeMap_.begin(); }
    typemap_const_iterator typemap_end() const { return typeMap_.end(); }

    static bool classof(const DBObject *obj)
    {
        return obj->getKind() == KIND_NAMESPACE;
    }

private:

    Namespace(Database& db, const std::string &name);

    std::string name_;
    TypeMap typeMap_;
    TypeList subnamespaces_;
};

class Type : public DBObject
{
public:

    const std::string & getTypeName() const { return name_; }

    std::string getFullTypeName() const;

    const std::vector<DBObject*> & getElements() const { return elements_; }

    size_t getNumElements() const { return elements_.size(); }

    template <typename T>
    T * getAs(size_t index) const
    {
        return llvm::cast<T>(elements_[index]);
    }

    template <typename T>
    T * getSafeAs(size_t index) const
    {
        return llvm::dyn_cast_or_null<T>(elements_[index]);
    }

    virtual bool equals(const DBObject *other) const;

    virtual size_t hash() const;

    virtual void print(std::ostream &out) const;

    static inline bool classof(const DBObject *obj)
    {
        return obj->getKind() >= KIND_FIRST_TYPE ||
            obj->getKind() <= KIND_LAST_TYPE;
    }

protected:

    Type(Database& db, const std::string &name, Kind kind, size_t num);
    Type(Database& db, const std::string &name, Kind kind, ArrayRef<DBObject*> elems);

    void resizeElements(size_t newSize);

    std::vector<DBObject*> elements_;

private:
    std::string name_;
};

class Database
{
    friend class Namespace;
public:

    Database();
    ~Database();

    void clear();

protected:

    template <class T> T * newObject(T *object)
    {
        objects_.push_back(object);
        return object;
    }

private:
    std::vector<DBObject*> objects_;
};

} // namespace TDB

} // namespace KIARA

#endif /* KIARA_TYPEDATABASE_HPP_INCLUDED */
