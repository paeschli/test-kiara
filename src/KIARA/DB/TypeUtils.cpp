/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2013, 2014  German Research Center for Artificial Intelligence (DFKI)
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
 * TypeUtils.cpp
 *
 *  Created on: Feb 7, 2014
 *      Author: Dmitri Rubinstein
 */

#define KIARA_LIB
#include "TypeUtils.hpp"
#include "World.hpp"
#include "TypeSemantics.hpp"
#include "Attributes.hpp"

namespace KIARA
{

bool TypeUtils::isCStringPtrType(const Type::Ptr &type)
{
    World &world = type->getWorld();
    if (type == world.type_c_string_ptr())
        return true;
    if (!canonicallyEqual(type, PtrType::get(world.type_c_char())))
        return false;
    TypeSemantics *tsem = type->getAttributeValuePtr<TypeSemanticsAttr>();
    return tsem && tsem->valueInterp == TypeSemantics::VI_CSTRING_PTR;
}

bool TypeUtils::isCArrayPtrType(const Type::Ptr &type)
{
    if (!getTypeAs<PtrType>(type))
        return false;

    TypeSemantics *tsem = type->getAttributeValuePtr<TypeSemanticsAttr>();
    return tsem && tsem->valueInterp == TypeSemantics::VI_ARRAY_PTR;
}

bool TypeUtils::isPtrToCArrayPtrType(const Type::Ptr &type)
{
    PtrType::Ptr pty = dyn_cast<PtrType>(removeTypedefs(type));
    return pty && isCArrayPtrType(pty->getElementType());
}

bool TypeUtils::isRefToCArrayPtrType(const Type::Ptr &type)
{
    RefType::Ptr rty = dyn_cast<RefType>(removeTypedefs(type));
    return rty && isCArrayPtrType(rty->getElementType());
}

bool TypeUtils::isPtrToCStringPtrType(const Type::Ptr &type)
{
    PtrType::Ptr pty = dyn_cast<PtrType>(removeTypedefs(type));
    return pty && isCStringPtrType(pty->getElementType());
}

Type::Ptr TypeUtils::removeTypedefs(const Type::Ptr &type)
{
    Type::Ptr result = type;
    TypedefType::Ptr tdef;
    while ((tdef = dyn_cast<TypedefType>(result)))
        result = tdef->getDeclType();
    return result;
}

}
