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
 * Allocator.cpp
 *
 *  Created on: Oct 31, 2013
 *      Author: rubinste
 */

#define KIARA_LIB
#include "KIARA/IRGen/IRGen.hpp"
#include "KIARA/DB/Attributes.hpp"
#include "KIARA/DB/TypeUtils.hpp"
#include "KIARA/Impl/Core.hpp"
#include <iostream>

// #define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

namespace KIARA
{

KIARA::IR::IRExpr::Ptr IRGen::createAllocator(
        IRGenContext &genCtx,
        const TypeInfo &natTypeInfo,
        const TypeInfo *destNatTypeInfo)
{
    using namespace KIARA::Compiler;

    const KIARA::Type::Ptr natType = natTypeInfo.type;

    // Passed natType is C-type to be allocated.
    // Resulting expression must be of pointer type to the natType.
    // By default malloc(sizeof(natType)) is called.

    KIARA::Compiler::IRBuilder &builder = genCtx.builder;
    KIARA::World &world = builder.getWorld();

    Callee k_malloc("malloc", builder);
    Callee k_sizeof("sizeof", builder);

    if (destNatTypeInfo && TypeUtils::isCStringPtrType(destNatTypeInfo->type))
    {
        // handle C-strings
        // return null pointer, since C-strings are allocated by the
        // deserialization function.
        return KIARA::IR::PrimLiteral::getNullPtr(world);
    }

    if (natType->hasAttributeValue<KIARA::AllocateTypeAPIAttr>())
    {
        NamedGenericFunc *allocFunc = natType->getAttributeValuePtr<KIARA::AllocateTypeAPIAttr>();
        if (!allocFunc->func)
        {
            IRGEN_ERROR(genCtx, KIARA_INVALID_ARGUMENT,
                    "Attribute " KIARA_ATTR_ALLOCATETYPE_API
                    " is not set or is not of type NamedGenericFunc");
        }

        // Following name must be unique !
        std::string funcName = natType->getTypeName();
        funcName += "_";
        funcName += KIARA::AllocateTypeAPIAttr::getAttrName();

        KIARA::IR::FunctionDefinition::Ptr funcDef = builder.lookupFunction(funcName);

        if (!funcDef)
        {
            KIARA::IR::Prototype::Ptr proto = KIARA::Compiler::createCFuncProto(
                    funcName,
                    KIARA::PtrType::get(builder.lookupType("KIARA_UserType")),
                    ArrayRef<KIARA::IR::Prototype::Arg>(),
                    builder.getWorld());

            DFC_DEBUG("PROTO: "<<proto->toString());

            funcDef = genCtx.addExternFunction(proto, FunctionLinkInfo(reinterpret_cast<void*>(allocFunc->func), allocFunc->funcName));
        }

        std::string convFuncName;
        builder.createCastCode(
                builder.getWorld().type_c_void_ptr(),
                KIARA::PtrType::get(natType),
                convFuncName,
                genCtx.expressions,
                genCtx.topScope);

        Callee convFunc(convFuncName, builder);
        Callee func(funcName, builder);

        return convFunc(func());
    }

    std::string convFuncName;
    builder.createCastCode(
            builder.getWorld().type_c_void_ptr(),
            KIARA::PtrType::get(natType),
            convFuncName,
            genCtx.expressions,
            genCtx.topScope);

    Callee convFunc(convFuncName, builder);

    return convFunc(k_malloc(k_sizeof(EType(natType))));
}

KIARA::IR::IRExpr::Ptr IRGen::createDeallocator(
        IRGenContext &genCtx,
        const KIARA::IR::IRExpr::Ptr &value,
        const TypeInfo &natTypeInfo)
{
    using namespace KIARA::Compiler;

    KIARA::Compiler::IRBuilder &builder = genCtx.builder;
    KIARA::World &world = builder.getWorld();
    const KIARA::Type::Ptr &natType = value->getExprType();

    assert(natType == natTypeInfo.type);

    Callee k_free("free", builder);
    Callee derefPtr("*", builder);

    // value must be of type pointer.
    if (TypeUtils::isPtrToCStringPtrType(natTypeInfo.type))
    {
        // handle strings:
        // if we have char**, dereference first and free a C-string and then free char*.
        assert(natType == PtrType::get(world.type_c_char_ptr()));

        {
            builder.createDereferenceCode(PtrType::get(world.type_c_char_ptr()), genCtx.expressions);
            builder.createDereferenceCode(RefType::get(world.type_c_char_ptr()), genCtx.expressions);
            return Block(
                k_free(derefPtr(value)),
                k_free(value));
        }
    }

    KIARA::Type::Ptr elemType;
    if (KIARA::PtrType::Ptr pty = dyn_cast<KIARA::PtrType>(natType))
        elemType = pty->getElementType();

    if (elemType && elemType->hasAttributeValue<KIARA::DeallocateTypeAPIAttr>())
    {
        NamedGenericFunc *deallocFunc = elemType->getAttributeValuePtr<KIARA::DeallocateTypeAPIAttr>();
        if (!deallocFunc->func)
        {
            IRGEN_ERROR(genCtx, KIARA_INVALID_ARGUMENT,
                    "Attribute " KIARA_ATTR_DEALLOCATETYPE_API
                    " is not set or is not of type KIARA_GenericFunc");
        }
        // Following name must be unique !
        std::string funcName = elemType->getTypeName();
        funcName += "_";
        funcName += KIARA::DeallocateTypeAPIAttr::getAttrName();

        KIARA::IR::FunctionDefinition::Ptr funcDef = builder.lookupFunction(funcName);

        if (!funcDef)
        {
            KIARA::IR::Prototype::Arg args[] = {
                    KIARA::IR::Prototype::Arg("arg", KIARA::PtrType::get(builder.lookupType("KIARA_UserType")))
            };

            KIARA::IR::Prototype::Ptr proto = KIARA::Compiler::createCFuncProto(
                    funcName,
                    builder.getWorld().type_void(),
                    args,
                    builder.getWorld());

            DFC_DEBUG("PROTO: "<<proto->toString());

            funcDef = genCtx.addExternFunction(proto, FunctionLinkInfo(reinterpret_cast<void*>(deallocFunc->func), deallocFunc->funcName));
        }

        std::string convFuncName;
        builder.createCastCode(
                natType,
                KIARA::PtrType::get(builder.lookupType("KIARA_UserType")),
                convFuncName,
                genCtx.expressions,
                genCtx.topScope);

        Callee convFunc(convFuncName, builder);
        Callee func(funcName, builder);

        return func(convFunc(value));
    }

    return k_free(value);
}

} // namespace KIARA
