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
 * Object.cpp
 *
 *  Created on: 16.08.2012
 *      Author: Dmitri Rubinstein
 */

#define KIARA_LIB
#include <KIARA/Common/Config.hpp>
#include <KIARA/DB/Object.hpp>
#include <KIARA/DB/World.hpp>
#include <boost/functional/hash.hpp>
#include <iostream>

namespace KIARA
{

// RTTI
DFC_DEFINE_ABSTRACT_TYPE(KIARA::Object)

/// Object

Object::Object(World &world)
    : GCObject(world)
{
}

Object::~Object()
{
}

World & Object::getWorld() const
{
    return static_cast<World&>(getCollector());
}

bool Object::equals(const Object::Ptr &other) const
{
    return this == other.get();
}

size_t Object::hash() const
{
    return boost::hash_value(this);
}

void Object::dump() const
{
    printRepr(std::cerr);
}

} // namespace KIARA
