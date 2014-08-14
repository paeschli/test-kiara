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
 * CycleCollector.hpp
 *
 *  Created on: 13.08.2012
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_CORE_CYCLECOLLECTOR_HPP_INCLUDED
#define KIARA_CORE_CYCLECOLLECTOR_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <KIARA/Core/GCObject.hpp>
#include <boost/assert.hpp>
#include <boost/scoped_array.hpp>
#include <vector>
#include <iostream>

// This is not MT-safe

namespace KIARA
{

class KIARA_API CycleCollector
{
    friend class GCObject;
public:

    CycleCollector(size_t maxNumPossibleCycleRoots = 10000);
    ~CycleCollector();

    void collectCycles();

    void dump();

    void dump(GCObject::RawPtr obj);

    void dump(std::ostream &out, GCObject::RawPtr obj);

    void dumpAsDotLabel(std::ostream &out, GCObject::RawPtr obj);

    void dumpMemGraph();

    void dumpMemGraph(std::ostream &out);

protected:
    void onNewObject(GCObject::RawPtr object);
    void onDestroyObject(GCObject::RawPtr object);
private:
    GCList objects_;
    size_t numPossibleCycleRoots_;
    size_t maxNumPossibleCycleRoots_;
    bool collecting_;

    class PrivateImpl;
    friend class PrivateImpl;

    void onObjectAddRef(size_t numRefs);
    void onObjectRelease(size_t numRefs);
};

} // namespace KIARA

#endif /* KIARA_CORE_CYCLECOLLECTOR_HPP_INCLUDED */
