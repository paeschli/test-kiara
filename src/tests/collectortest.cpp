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
 * collectortest.cpp
 *
 *  Created on: 13.08.2012
 *      Author: Dmitri Rubinstein
 */
#include <boost/test/minimal.hpp>
#include <KIARA/Core/LibraryInit.hpp>
#include <KIARA/Core/GCObject.hpp>
#include <KIARA/Core/CycleCollector.hpp>
#include <DFC/Base/Core/Object.hpp>
#include <DFC/Base/Core/ObjectFactory.hpp>
#include <DFC/Base/Utils/Chronometer.hpp>
#include <limits>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

//#define GC_VALUE_DEBUG

class GCValue : public KIARA::GCObject
{
    DFC_DECLARE_TYPE(GCValue, KIARA::GCObject)
public:

    static size_t constructedObjects;
    static size_t destroyedObjects;

    int value;
    KIARA::GCObject::Ptr children[2];

    GCValue(int value, KIARA::CycleCollector &c)
        : KIARA::GCObject(c)
        , value(value)
    {
        ++constructedObjects;
#ifdef GC_VALUE_DEBUG
        std::cout<<"GCValue("<<value<<") this="<<this<<std::endl;
#endif
    }

    ~GCValue()
    {
        ++destroyedObjects;
#ifdef GC_VALUE_DEBUG
        std::cout<<"~GCValue() this="<<this<<" value = "<<value<<std::endl;
#endif
    }

protected:

    virtual void gcUnlinkRefs()
    {
        gcUnlinkChild(children[0]);
        gcUnlinkChild(children[1]);
    }

    virtual void gcApplyToChildren(const KIARA::CollectorCallback &callback)
    {
        gcApply(children[0], callback);
        gcApply(children[1], callback);
    }

};

DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(GCValue)
size_t GCValue::constructedObjects = 0;
size_t GCValue::destroyedObjects = 0;


double minTime = std::numeric_limits<double>::max();
double maxTime = -1.0;
size_t maxTimeRoots = 0;
size_t minTimeRoots = 0;

void check(size_t maxNumRoots = 1)
{
    KIARA::CycleCollector collector(maxNumRoots);
    GCValue::constructedObjects = 0;
    GCValue::destroyedObjects = 0;

#if 1
    {
        DFC::Object::Ptr obj1 = new GCValue(1, collector);
        DFC::Object::Ptr obj2 = new GCValue(2, collector);
        DFC::Object::Ptr obj3 = new GCValue(3, collector);
    }
#endif

    {
        GCValue::Ptr obj1 = new GCValue(4, collector);
        GCValue::Ptr obj2 = new GCValue(5, collector);

        obj1->children[0] = obj1;
        obj1->children[1] = obj2;
        obj2->children[0] = obj1;
        obj2->children[1] = obj2;
    }
    //collector.dump();
    //collector.collectCycles();

#if 1
    {
        GCValue::Ptr obj1 = new GCValue(6, collector);
        GCValue::Ptr obj2 = new GCValue(7, collector);
        GCValue::Ptr obj3 = new GCValue(8, collector);


        obj1->children[0] = obj2;
        obj2->children[0] = obj3;
        obj3->children[0] = obj1;
        //collector.dump();
        //collector.collectCycles();
    }
    //collector.dump();
    //collector.collectCycles();
#endif

#if 1
    for (int i = 0; i < 1000; ++i)
    {
        {
            GCValue::Ptr obj1 = new GCValue(i+1, collector);
            GCValue::Ptr obj2 = new GCValue(i+2, collector);

            obj1->children[0] = obj1;
            obj1->children[1] = obj2;
            obj2->children[0] = obj1;
            obj2->children[1] = obj2;
        }

        {
            GCValue::Ptr obj1 = new GCValue(i+3, collector);
            GCValue::Ptr obj2 = new GCValue(i+4, collector);
            GCValue::Ptr obj3 = new GCValue(i+5, collector);

            obj1->children[0] = obj2;
            obj2->children[0] = obj3;
            obj3->children[0] = obj1;
        }
    }

    //collector.dump();
    //collector.dumpMemGraph();
#endif

    DFC::Chronometer timer;
    timer.start();
    collector.collectCycles();
    timer.stop();

    BOOST_CHECK(GCValue::constructedObjects == GCValue::destroyedObjects);

    double time = timer.getElapsedTimeInMilliseconds();

    if (GCValue::constructedObjects != GCValue::destroyedObjects)
    {
        std::cerr<<"TEST WITH "<<maxNumRoots<<" ROOTS FAILED: Constructed objects: "<<GCValue::constructedObjects
                 <<" Destroyed objects: "<<GCValue::destroyedObjects
                 <<" TIME : "<<time<<" ms"<<std::endl;
    }
    else
    {
        std::cout<<"TEST WITH "<<maxNumRoots<<" ROOTS SUCCEED: Constructed objects: "<<GCValue::constructedObjects
                 <<" Destroyed objects: "<<GCValue::destroyedObjects
                 <<" TIME : "<<time<<" ms"<<std::endl;
    }

    // record collection time only when number of roots > number of constructed objects

    if (maxNumRoots > GCValue::constructedObjects)
    {
        if (time > maxTime)
        {
            maxTime = time;
            maxTimeRoots = maxNumRoots;
        }
        else if (time < minTime)
        {
            minTime = time;
            minTimeRoots = maxNumRoots;
        }
    }
}

int test_main (int argc, char **argv)
{
    KIARA::LibraryInit init;

    DFC_REGISTER_TYPE(GCValue);

    check(1);
    for (unsigned int i = 10; i <= 10000; i += 10)
    {
        check(i);
    }

    std::cout << "Maximum GC time : "<<maxTime<<" ms num roots = "<<maxTimeRoots<<std::endl;
    std::cout << "Minimum GC time : "<<minTime<<" ms num roots = "<<minTimeRoots<<std::endl;

    return 0;
}
