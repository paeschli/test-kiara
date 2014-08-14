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
 * TypeUtils.hpp
 *
 *  Created on: Feb 7, 2014
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_DB_TYPEUTILS_HPP_INCLUDED
#define KIARA_DB_TYPEUTILS_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <KIARA/DB/DerivedTypes.hpp>
#include <KIARA/DB/TypeSemantics.hpp>
#include <KIARA/DB/Attributes.hpp>

namespace KIARA
{

class KIARA_API TypeUtils
{
public:


    static TypeSemantics & getOrCreateTypeSemantics(const Type::Ptr &type)
    {
        return type->getOrCreateAttributeValueRef<KIARA::TypeSemanticsAttr>();
    }

    static const TypeSemantics * getTypeSemantics(const Type::Ptr &type)
    {
        return type ? type->getAttributeValuePtr<KIARA::TypeSemanticsAttr>() : 0;
    }

    static bool isCStringPtrType(const Type::Ptr &type);

    static bool isCArrayPtrType(const Type::Ptr &type);

    static bool isPtrToCArrayPtrType(const Type::Ptr &type);

    static bool isRefToCArrayPtrType(const Type::Ptr &type);

    static bool isPtrToCStringPtrType(const Type::Ptr &type);

    static bool isOpaqueType(const Type::Ptr &type)
    {
        KIARA::CompositeType::Ptr cty = dyn_cast<KIARA::CompositeType>(removeTypedefs(type));
        return cty && cty->isOpaque();
    }

    /* If type is a reference, return element type otherwise return passed type */
    static Type::Ptr getDereferencedType(const Type::Ptr &type)
    {
        RefType::Ptr refTy = dyn_cast<RefType>(removeTypedefs(type));
        return refTy ? refTy->getElementType() : type;
    }

    static Type::Ptr getElementType(const Type::Ptr &type)
    {
        Type::Ptr ty = removeTypedefs(type);
        PtrType::Ptr pt = dyn_cast<PtrType>(ty);
        if (pt)
            return pt->getElementType();
        RefType::Ptr rt = dyn_cast<RefType>(ty);
        if (rt)
            return rt->getElementType();
        ArrayType::Ptr at = dyn_cast<ArrayType>(ty);
        if (at)
            return at->getElementType();
        FixedArrayType::Ptr fat = dyn_cast<FixedArrayType>(ty);
        if (fat)
            return fat->getElementType();
        return Type::Ptr();
    }

    static Type::Ptr getMemberType(const Type::Ptr &type, unsigned int index)
    {
        StructType::Ptr st = dyn_cast<StructType>(removeTypedefs(type));
        if (st && st->getNumElements() < index)
            return st->getElementAt(index);
        return Type::Ptr();
    }

    static Type::Ptr removeTypedefs(const Type::Ptr &type);

private:
    TypeUtils();
    TypeUtils(const TypeUtils &);
    TypeUtils & operator=(const TypeUtils &);
};

} // namespace KIARA

#endif /* KIARA_DB_TYPEUTILS_HPP_INCLUDED */
