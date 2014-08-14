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
 * CycleCollector.cpp
 *
 *  Created on: 13.08.2012
 *      Author: Dmitri Rubinstein
 */

#define KIARA_LIB
#include <KIARA/Core/CycleCollector.hpp>
#include <KIARA/Core/Exception.hpp>
#include <KIARA/Utils/VarGuard.hpp>
#include <algorithm>
#include <iostream>

#include <map>
#include <set>

//#define KIARA_GC_DUMP_CYCLEGRAPH 1

namespace KIARA
{

CycleCollector::CycleCollector(size_t maxNumPossibleCycleRoots)
    : numPossibleCycleRoots_(0)
    , maxNumPossibleCycleRoots_(maxNumPossibleCycleRoots)
    , collecting_(false)
{
}

CycleCollector::~CycleCollector()
{
    //dumpMemGraph(); //???DEBUG

    // unlink and free garbage
    GCObject::Ptr ref;
    for (GCList::iterator it = objects_.begin();
            it != objects_.end(); ++it)
    {
        ref = &(*it); // avoid unintentional destruction of the current object
        ref->gcUnlinkRefs();
    }
    ref.reset();

    // check memory leaks
    if (!objects_.empty())
    {
        std::cerr<<"MEMORY LEAKS DETECTED !\n"
                 <<objects_.size()<<" OBJECTS NOT DESTROYED"<<std::endl;
        size_t index = 0;
        for (GCList::iterator it = objects_.begin(),
                end = objects_.end(); it != end; ++it, ++index)
        {
            std::cerr<<"["<<index<<"] = ";
            dump(&(*it));
            std::cerr<<std::endl;
        }

        std::cerr<<std::endl;
        dumpMemGraph(std::cerr);
    }
}

namespace
{

struct GCInfo
{
    unsigned long inEdges;
    bool reachable;

    GCInfo(unsigned long inEdges = 0)
        : inEdges(inEdges)
        , reachable(false)
    { }
};
typedef std::map<GCObject::RawPtr, GCInfo> GCMap;

struct GCData
{
    GCMap gcMap;
    bool externalRef;
    unsigned long numReachables;
#if defined(KIARA_GC_DUMP_CYCLEGRAPH)
    std::set<GCObject::RawPtr> dumpedObjects;
#endif
};

} // unnamed namespace

class CycleCollector::PrivateImpl
{
public:

    static void gcFindRoots(GCObject::RawPtr obj, void *data);

    static void gcFindReachables(GCObject::RawPtr obj, void *data);

    static void memGraphScanChild(GCObject::RawPtr obj, void *data);

    static void memGraphDumpChild(GCObject::RawPtr obj, void *data);

    static void memGraphDumpChildName(GCObject::RawPtr obj, void *data);

#if defined(KIARA_GC_DUMP_CYCLEGRAPH)
    static void gcDumpChild(GCObject::RawPtr obj, void *data);

    static void gcDumpChildName(GCObject::RawPtr obj, void *data);
#endif
};


#if defined(KIARA_GC_DUMP_CYCLEGRAPH)
void CycleCollector::PrivateImpl::gcDumpChildName(GCObject::RawPtr obj, void *rawData)
{
    BOOST_ASSERT(rawData != 0 && obj != 0);
    std::cerr<<" obj_"<<(void*)obj<<";";
}

void CycleCollector::PrivateImpl::gcDumpChild(GCObject::RawPtr obj, void *rawData)
{
    BOOST_ASSERT(rawData != 0 && obj != 0);
    GCData *data = static_cast<GCData*>(rawData);

    if (data->dumpedObjects.find(obj) != data->dumpedObjects.end())
        return;
    data->dumpedObjects.insert(obj);

    GCMap::iterator it = data->gcMap.find(obj);

    std::ostream &out = std::cerr;

    out<<" obj_"<<(void*)obj<<" [shape=box, label=\"";
    obj->getCollector().dumpAsDotLabel(out, obj);
    if (it != data->gcMap.end())
    {
        out<<"\\ninEdges = "<<it->second.inEdges;
        out<<"\\nreachable = "<<(it->second.reachable ? "true" : "false");
    }
    else
    {
        out<<"\\nNOT IN GCMAP : reachable";
    }
    out<<"\"];\n";
    out<<" obj_"<<(void*)obj<<" -> { ";
    obj->gcApplyToChildren(CollectorCallback(gcDumpChildName, data));
    out<<" };\n";
    obj->gcApplyToChildren(CollectorCallback(gcDumpChild, data));
}
#endif

void CycleCollector::collectCycles()
{
    if (collecting_ || numPossibleCycleRoots_ == 0)
        return;
    VarGuard<bool> g(collecting_, true);

    GCData data;
    data.numReachables = 0;


    // detect roots (objects not reachable from other objects, but for example from stack)
    for (GCList::iterator it = objects_.begin(),
            end = objects_.end(); it != end; ++it)
    {
        GCObject::RawPtr obj = &(*it);
        if (obj->possibleCycleRoot_)
        {
            data.externalRef = true;
            PrivateImpl::gcFindRoots(obj, &data);
        }
    }

    // find all from roots reachable objects
    // roots have externalRefs counter greater than zero
    for (GCMap::iterator it = data.gcMap.begin(), end = data.gcMap.end(); it != end; ++it)
    {
        // note special case: when reference count is 0 object is not yet assigned to smart ptr
        if (!it->second.reachable &&
                ((it->first->getNumRefs() > it->second.inEdges) || it->first->getNumRefs() == 0))
        {
            PrivateImpl::gcFindReachables(it->first, &data);
        }
    }

    size_t numGCObjects = data.gcMap.size() - data.numReachables;
    if (numGCObjects == 0)
        return;

#if defined(KIARA_GC_DUMP_CYCLEGRAPH)
    // dump collected information
    std::cerr<<"digraph {\n concentrate=true;\n";
    for (GCList::iterator it = objects_.begin(),
            end = objects_.end(); it != end; ++it)
    {
        GCObject::RawPtr obj = &(*it);
        PrivateImpl::gcDumpChild(obj, &data);
    }
    std::cerr<<"}\n";
#endif

    std::vector<GCObject::Ptr> garbage;
    garbage.reserve(numGCObjects);

    for (GCMap::iterator it = data.gcMap.begin(), end = data.gcMap.end(); it != end; ++it)
    {
        if (!it->second.reachable)
            garbage.push_back(it->first);
    }

    data.gcMap.clear();

    // unlink & remove garbage
    for (std::vector<GCObject::Ptr>::iterator it = garbage.begin(), end = garbage.end(); it != end; ++it)
    {
        (*it)->gcUnlinkRefs();
    }
    garbage.clear();
}

void CycleCollector::PrivateImpl::gcFindRoots(GCObject::RawPtr obj, void *rawData)
{
    BOOST_ASSERT(rawData != 0 && obj != 0);
    GCData *data = static_cast<GCData*>(rawData);

    GCMap::iterator it = data->gcMap.find(obj);
    if (it != data->gcMap.end())
    {
       // if origin of the edge is not root
       // update count of incoming edges
        if (!data->externalRef)
            ++it->second.inEdges;
        return;
    }
    it = data->gcMap.insert(std::make_pair(obj, GCInfo(data->externalRef ? 0 : 1))).first;

    data->externalRef = false;
    obj->gcApplyToChildren(CollectorCallback(gcFindRoots, data));
}

void CycleCollector::PrivateImpl::gcFindReachables(GCObject::RawPtr obj, void *rawData)
{
    BOOST_ASSERT(rawData != 0 && obj != 0);
    GCData *data = static_cast<GCData*>(rawData);

    GCMap::iterator it = data->gcMap.find(obj);
    BOOST_ASSERT(it != data->gcMap.end());

    if (it->second.reachable)
        return;
    it->second.reachable = true;
    ++data->numReachables;
    obj->gcApplyToChildren(CollectorCallback(gcFindReachables, data));
}

void CycleCollector::dump()
{
    std::cerr<<"CycleCollector {\n";
    size_t index = 0;
    for (GCList::iterator it = objects_.begin(),
            end = objects_.end(); it != end; ++it, ++index)
    {
        std::cerr<<"["<<index<<"] = ";
        dump(&(*it));
        std::cerr<<std::endl;
    }
    std::cerr<<"}\n"<<std::flush;
}

void CycleCollector::dump(GCObject::RawPtr s)
{
    dump(std::cerr, s);
}

void CycleCollector::dump(std::ostream &out, GCObject::RawPtr obj)
{
    out<<obj<<" RC = "<<obj->getRefCount()<<" type name = "
            <<(obj->getTypeName() ? obj->getTypeName() : "");
}

void CycleCollector::dumpAsDotLabel(std::ostream &out, GCObject::RawPtr obj)
{
    out<<obj<<"\\n"<<(obj->getTypeName() ? obj->getTypeName() : "")
            <<"\\nRC = "<<obj->getRefCount();
}

void CycleCollector::onNewObject(GCObject::RawPtr object)
{
    BOOST_ASSERT(object != 0);
    objects_.push_back(*object);
}

void CycleCollector::onDestroyObject(GCObject::RawPtr object)
{
    BOOST_ASSERT(object != 0);
    objects_.erase(objects_.iterator_to(*object));
}

void CycleCollector::onObjectAddRef(size_t numRefs)
{
    if (numPossibleCycleRoots_ > 0)
        --numPossibleCycleRoots_;
}

void CycleCollector::onObjectRelease(size_t numRefs)
{
    if (numRefs != 0)
        ++numPossibleCycleRoots_;
    if (maxNumPossibleCycleRoots_ != size_t(-1) && numPossibleCycleRoots_ > maxNumPossibleCycleRoots_)
        collectCycles();
}

namespace
{
    struct GCMemGraphInfo
    {
        unsigned int inEdges;
        unsigned int outEdges;
        bool visited;

        GCMemGraphInfo(unsigned int inEdges = 0, unsigned int outEdges = 0)
            : inEdges(inEdges)
            , outEdges(outEdges)
            , visited(false)
        { }
    };
    typedef std::map<GCObject::RawPtr, GCMemGraphInfo> MemGraphSet;

    struct MemGraphData
    {
        std::ostream *memGraphOut;
        MemGraphSet *memGraphSet;
        GCMemGraphInfo *memGraphEdgeOrigin;
    };

} // unnamed namespace

void CycleCollector::dumpMemGraph(std::ostream &out)
{
    std::map<GCObject::RawPtr, GCMemGraphInfo> graphSet;

    MemGraphData data;
    data.memGraphOut = &out;
    data.memGraphSet = &graphSet;
    data.memGraphEdgeOrigin = 0;

    for (GCList::iterator it = objects_.begin(),
            end = objects_.end(); it != end; ++it)
    {
        GCObject::RawPtr obj = &(*it);
        PrivateImpl::memGraphScanChild(obj, &data);
    }

    out<<"digraph {\n concentrate=true;\n";
    for (GCList::iterator it = objects_.begin(),
            end = objects_.end(); it != end; ++it)
    {
        GCObject::RawPtr obj = &(*it);
        PrivateImpl::memGraphDumpChild(obj, &data);
    }
    out<<"}\n";
}

void CycleCollector::dumpMemGraph()
{
    dumpMemGraph(std::cerr);
}

void CycleCollector::PrivateImpl::memGraphScanChild(GCObject::RawPtr obj, void *rawData)
{
    BOOST_ASSERT(rawData != 0 && obj != 0);
    MemGraphData *data = static_cast<MemGraphData*>(rawData);

    if (data->memGraphEdgeOrigin)
        ++data->memGraphEdgeOrigin->outEdges;

    MemGraphSet::iterator it = data->memGraphSet->find(obj);
    if (it != data->memGraphSet->end())
    {
        // update incoming edge count
        if (data->memGraphEdgeOrigin)
            ++it->second.inEdges;
        return;
    }
    it = data->memGraphSet->insert(
            std::make_pair(obj, GCMemGraphInfo(data->memGraphEdgeOrigin ? 1 : 0))).first;

    GCMemGraphInfo *tmp = data->memGraphEdgeOrigin;
    data->memGraphEdgeOrigin = &it->second;

    obj->gcApplyToChildren(CollectorCallback(memGraphScanChild, data));

    data->memGraphEdgeOrigin = tmp;
}

void CycleCollector::PrivateImpl::memGraphDumpChild(GCObject::RawPtr obj, void *rawData)
{
    BOOST_ASSERT(rawData != 0 && obj != 0);
    MemGraphData *data = static_cast<MemGraphData*>(rawData);

    std::ostream &out = *data->memGraphOut;

    MemGraphSet::iterator it = data->memGraphSet->find(obj);
    BOOST_ASSERT(it != data->memGraphSet->end() && "Not scanned memory graph node");

    if (it->second.visited)
        return;
    it->second.visited = true;

    out<<" obj_"<<(void*)obj<<" [shape=box, label=\"";
    obj->getCollector().dumpAsDotLabel(out, obj);
    out<<"\\ninEdges = "<<it->second.inEdges;
    out<<"\\noutEdges = "<<it->second.outEdges;
    out<<"\"];\n";
    out<<" obj_"<<(void*)obj<<" -> { ";
    obj->gcApplyToChildren(CollectorCallback(memGraphDumpChildName, data));
    out<<" };\n";
    obj->gcApplyToChildren(CollectorCallback(memGraphDumpChild, data));
}

void CycleCollector::PrivateImpl::memGraphDumpChildName(GCObject::RawPtr obj, void *rawData)
{
    BOOST_ASSERT(rawData != 0 && obj != 0);
    MemGraphData *data = static_cast<MemGraphData*>(rawData);
    *(data->memGraphOut)<<" obj_"<<(void*)obj<<";";
}

} // namespace KIARA
