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
 * GCObject.cpp
 *
 *  Created on: 13.08.2012
 *      Author: Dmitri Rubinstein
 */

#define KIARA_LIB
#include <KIARA/Common/Config.hpp>
#include <KIARA/Core/GCObject.hpp>
#include <KIARA/Core/CycleCollector.hpp>
#include <iostream>

namespace KIARA
{

// RTTI
DFC_DEFINE_ABSTRACT_TYPE(KIARA::GCObject)

GCObject::GCObject(CycleCollector &collector)
    : collector_(collector)
    , possibleCycleRoot_(false)
{
    collector_.onNewObject(this);
}

GCObject::~GCObject()
{
}

void GCObject::destroy()
{
    collector_.onDestroyObject(this);
    InheritedType::destroy();
}

size_t GCObject::addRef()
{
    possibleCycleRoot_ = false;
    size_t result = InheritedType::addRef();
    collector_.onObjectAddRef(result);
    return result;
}

size_t GCObject::release()
{
    possibleCycleRoot_ = true;
    CycleCollector &collector = collector_;
    size_t result = InheritedType::release();
    collector.onObjectRelease(result);
    return result;
}

} // namespace KIARA
