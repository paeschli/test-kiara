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
 * Object.hpp
 *
 *  Created on: 16.08.2012
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_DB_OBJECT_HPP_INCLUDED
#define KIARA_DB_OBJECT_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <KIARA/Core/GCObject.hpp>
#include <boost/unordered_set.hpp>
#include <boost/functional/hash.hpp>
#include <set>
#include <sstream>

namespace KIARA
{

class World;

class KIARA_API Object : public GCObject
{
    DFC_DECLARE_ABSTRACT_TYPE(Object, GCObject)
public:

    World &getWorld() const;

    /** Returns true if objects are equal.
     *  Default implementation returns true if objects are identical
     *  by comparing pointers.
     */
    virtual bool equals(const Object::Ptr &other) const;

    virtual size_t hash() const;

    bool operator==(const Object::Ptr &other) const
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

    Object(World &world);
    virtual ~Object();
};

template <class T, class U>
inline bool isa(DFC_OBJECT_SMART_PTR(U) const & p)
{
    return DFC::safe_object_cast<T>(p);
}

template <class T, class U>
inline typename DFC::PointerTraits<T>::Ptr cast(
    DFC_OBJECT_SMART_PTR(U) const & p)
{
    return DFC::object_cast<T>(p);
}

template <class T, class U>
inline typename DFC::PointerTraits<T>::Ptr dyn_cast(
    DFC_OBJECT_SMART_PTR(U) const & p)
{
    return DFC::safe_object_cast<T>(p);
}

template <class T, class U>
inline bool isa(DFC_OBJECT_RAW_PTR(U) const p)
{
    return DFC::safe_object_cast<T>(p);
}

template <class T, class U>
inline typename DFC::PointerTraits<T>::RawPtr cast(
    DFC_OBJECT_RAW_PTR(U) const p)
{
    return DFC::object_cast<T>(p);
}

template <class T, class U>
inline typename DFC::PointerTraits<T>::RawPtr dyn_cast(
    DFC_OBJECT_RAW_PTR(U) const p)
{
    return DFC::safe_object_cast<T>(p);
}

inline std::ostream & operator<<(std::ostream &out, const Object &obj)
{
    obj.print(out);
    return out;
}

inline std::ostream & operator<<(std::ostream &out, const Object::Ptr &obj)
{
    if (obj)
        obj->print(out);
    else
        out<<"NULL";
    return out;
}

struct ObjectHash : std::unary_function<const Object::Ptr &, size_t>
{
    size_t operator () (const Object::Ptr & v) const
    {
        return v->hash();
    }
};

struct ObjectEqual : std::binary_function<const Object::Ptr &, const Object::Ptr &, bool>
{
    bool operator () (const Object::Ptr & v1, const Object::Ptr & v2) const
    {
        return v1->equals(v2);
    }
};

typedef boost::unordered_set<Object::Ptr, ObjectHash, ObjectEqual> ObjectSet;
//typedef std::set<Object::Ptr> ObjectSet; // <- just for testing, will not work correctly

} // namespace KIARA

#endif /* KIARA_IDL_IDLOBJECT_HPP_INCLUDED */
