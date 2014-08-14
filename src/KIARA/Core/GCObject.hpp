/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2012, 2013  German Research Center for Artificial Intelligence (DFKI)
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
 * GCObject.hpp
 *
 *  Created on: 13.08.2012
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_CORE_GCOBJECT_HPP_INCLUDED
#define KIARA_CORE_GCOBJECT_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <DFC/Base/Core/Object.hpp>
#include <KIARA/Utils/IntrusiveList.hpp>
#include <algorithm>

namespace KIARA
{

class CycleCollector;
class GCObject;
class CollectorCallback;

class CollectorCallback
{
public:
    typedef void (*Handler)(DFC::PointerTraits<GCObject>::RawPtr, void *data);

    CollectorCallback(Handler handler, void *data = 0)
        : handler_(handler), data_(data)
    {
        BOOST_ASSERT(handler != 0);
    }

    void operator()(DFC::PointerTraits<GCObject>::RawPtr object) const
    {
        BOOST_ASSERT(object != 0);
        handler_(object, data_);
    }

private:
    Handler handler_;
    void *data_;
};

typedef IntrusiveList<GCObject> GCList;

class KIARA_API GCObject : public DFC::Object, public GCList::Node
{
    friend class CycleCollector;
    DFC_DECLARE_ABSTRACT_TYPE(GCObject, DFC::Object)
public:

    GCObject(CycleCollector &collector);
    virtual ~GCObject();

    CycleCollector & getCollector() const { return collector_; }

    virtual size_t addRef();

    virtual size_t release();

protected:

    virtual void destroy();

    /** Unlink all children references in this object.
     *  This method needs to be overridden in order to support cycle collection.
     */
    virtual void gcUnlinkRefs() { }

    /** Apply passed callback function to all children.
     *  This method needs to be overridden in order to support cycle collection.
     */
    virtual void gcApplyToChildren(const CollectorCallback &callback) { }

    template <class T>
    void gcApply(const T &childRef, const CollectorCallback &callback)
    {
        if (childRef)
            callback(boost::get_pointer(childRef));
    }

    template <class T>
    void gcApply(T *childRef, const CollectorCallback &callback)
    {
        if (childRef)
            callback(childRef);
    }

    struct map_values_tag { };
    struct map_keys_tag { };

    template <typename It>
    void gcApply(It begin, It end, const CollectorCallback &callback)
    {
        for (; begin != end; ++begin)
            gcApply(*begin, callback);
    }

    template <typename It>
    void gcApply(map_keys_tag, It begin, It end, const CollectorCallback &callback)
    {
        for (; begin != end; ++begin)
            gcApply(begin->first, callback);
    }

    template <typename It>
    void gcApply(map_values_tag, It begin, It end, const CollectorCallback &callback)
    {
        for (; begin != end; ++begin)
            gcApply(begin->second, callback);
    }

    template <typename T>
    static void gcUnlinkChild(T &child)
    {
        child = 0;
    }

    template <typename It>
    static void gcUnlinkChildren(It begin, It end)
    {
        for (; begin != end; ++begin)
            gcUnlinkChild(*begin);
//        std::for_each(begin, end, unlinkChild);
    }

    template <typename It>
    static void gcUnlinkChildren(map_values_tag, It begin, It end)
    {
        for (; begin != end; ++begin)
            gcUnlinkChild(begin->second);
    }

private:
    CycleCollector &collector_;
    bool possibleCycleRoot_;
};


} // namespace KIARA

#endif /* KIARA_CORE_GCOBJECT_HPP_INCLUDED */
