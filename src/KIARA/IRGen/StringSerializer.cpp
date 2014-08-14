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
 * StringSerializer.cpp
 *
 *  Created on: Nov 1, 2013
 *      Author: rubinste
 */

#define KIARA_LIB
#include "KIARA/IRGen/IRGen.hpp"
#include "KIARA/DB/Attributes.hpp"
#include "KIARA/Impl/Core.hpp"
#include "KIARA/Compiler/IR.hpp"

// #define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

namespace KIARA
{

KIARA::IR::IRExpr::Ptr IRGen::createStringSerializer(
    IRGenContext &genCtx,
    const IRGen::SerializerConfig &config,
    const NativeExprInfo &natExprInfo,
    const KIARA::Type::Ptr &natElemType,
    bool isReference,
    const TypeInfo &idlTypeInfo,
    const KIARA::IR::IRExpr::Ptr &outObject)
{
    using namespace KIARA::Compiler;

    KIARA::IR::IRExpr::Ptr natArgExpr = natExprInfo.argExpr;
    const Type::Ptr &idlType = idlTypeInfo.type;
    const KIARA::Type::Ptr &natType = natArgExpr->getExprType();
    KIARA::Compiler::IRBuilder &builder = genCtx.builder;
    KIARA::World &world = builder.getWorld();

    Callee addressOf("&", builder);
    Callee to_KIARA_UserType_ptr("to_KIARA_UserType_ptr", builder);

    // serialize custom user string
    if (idlType == world.type_string() &&
            natType != world.type_c_char_ptr() &&
            natElemType->hasAttributeValue<KIARA::GetCStringAPIAttr>())
    {
        DFC_DEBUG("natElemType: "<<natElemType->toString());

        // suffix of the serialization function
        const char *suffix = "user_string";

        NamedGenericFunc *getUserStringFunc = natElemType->getAttributeValuePtr<KIARA::GetCStringAPIAttr>();
        if (!getUserStringFunc->func)
        {
            IRGEN_ERROR(genCtx, KIARA_INVALID_ARGUMENT,
                    "Attribute " KIARA_ATTR_GETCSTRING_API
                    " is not set or is not of type NamedGenericFunc");
        }

        // Following name must be unique !
        std::string getUserStringFuncName = natElemType->getTypeName();
        getUserStringFuncName += "_";
        getUserStringFuncName += KIARA::GetCStringAPIAttr::getAttrName();

        KIARA::IR::FunctionDefinition::Ptr funcDef = builder.lookupFunction(getUserStringFuncName);

        if (!funcDef)
        {
            KIARA::IR::Prototype::Arg args[] = {
                    KIARA::IR::Prototype::Arg("ustr", KIARA::PtrType::get(builder.lookupType("KIARA_UserType"))),
                    KIARA::IR::Prototype::Arg("cstr", KIARA::PtrType::get(builder.getWorld().type_c_char_ptr()))
            };

            KIARA::IR::Prototype::Ptr proto = KIARA::Compiler::createCFuncProto(
                    getUserStringFuncName,
                    builder.getWorld().type_c_int(),
                    args,
                    builder.getWorld());

            DFC_DEBUG("PROTO:\n"); //???DEBUG
            DFC_DEBUG(proto->toString());

            funcDef = genCtx.addExternFunction(proto, FunctionLinkInfo(reinterpret_cast<void*>(getUserStringFunc->func), getUserStringFunc->funcName));
        }

        // Following assertion may fail since list of linked functions is not restored
        // when new IRGenContext is created
        //assert(genCtx.getLinkedFunction(getUserStringFuncName) == reinterpret_cast<void*>(getUserStringFuncPtr));

        Callee serFunc(config.serializerNamePrefix + suffix, builder);

        if (isReference)
        {
            natArgExpr = addressOf(natArgExpr);
        }
        return serFunc(outObject, to_KIARA_UserType_ptr(natArgExpr), addressOf(funcDef));
    }

    return 0;
}

KIARA::IR::IRExpr::Ptr IRGen::createStringDeserializer(
    IRGenContext &genCtx,
    const IRGen::DeserializerConfig &config,
    const NativeExprInfo &natExprInfo,
    const KIARA::Type::Ptr &natElemType,
    bool isReference,
    const TypeInfo &idlTypeInfo,
    const KIARA::IR::IRExpr::Ptr &inObject)
{
    using namespace KIARA::Compiler;

    KIARA::IR::IRExpr::Ptr natArgExpr = natExprInfo.argExpr;
    const Type::Ptr &idlType = idlTypeInfo.type;
    //const KIARA::Type::Ptr &natType = natArgExpr->getExprType();
    KIARA::Compiler::IRBuilder &builder = genCtx.builder;
    KIARA::World &world = builder.getWorld();

    Callee addressOf("&", builder);
    Callee to_KIARA_UserType_ptr("to_KIARA_UserType_ptr", builder);

    // Test for custom user string
    if (idlType == world.type_string() &&
            natElemType->hasAttributeValue<KIARA::SetCStringAPIAttr>())
    {
        DFC_DEBUG("natElemType: "<<natElemType->toString());

        // suffix of the deserialization function
        const char *suffix = "user_string";
        KIARA::Type::Ptr natIdlType = natElemType;

        NamedGenericFunc *setUserStringFunc = natElemType->getAttributeValuePtr<KIARA::SetCStringAPIAttr>();
        if (!setUserStringFunc->func)
        {
            IRGEN_ERROR(genCtx, KIARA_INVALID_ARGUMENT,
                    "Attribute " KIARA_ATTR_SETCSTRING_API
                    " is not set or is not of type NamedGenericFunc");
        }

        // Following name must be unique !
        std::string setUserStringFuncName = natElemType->getTypeName();
        setUserStringFuncName += "_";
        setUserStringFuncName += KIARA::SetCStringAPIAttr::getAttrName();

        KIARA::IR::FunctionDefinition::Ptr funcDef = builder.lookupFunction(setUserStringFuncName);

        if (!funcDef)
        {
            KIARA::IR::Prototype::Arg args[] = {
                    KIARA::IR::Prototype::Arg("ustr", KIARA::PtrType::get(builder.lookupType("KIARA_UserType"))),
                    KIARA::IR::Prototype::Arg("cstr", builder.getWorld().type_c_char_ptr())
            };

            KIARA::IR::Prototype::Ptr proto = KIARA::Compiler::createCFuncProto(
                    setUserStringFuncName,
                    builder.getWorld().type_c_int(),
                    args,
                    builder.getWorld());

            DFC_DEBUG("PROTO: "<<proto->toString());

            funcDef = genCtx.addExternFunction(proto, FunctionLinkInfo(reinterpret_cast<void*>(setUserStringFunc->func), setUserStringFunc->funcName));
        }

        // Following assertion may fail since list of linked functions is not restored
        // when new IRGenContext is created
        // assert(genCtx.functionLinkMap[setUserStringFuncName] == reinterpret_cast<void*>(*setUserStringFuncPtr));

        if (isReference)
        {
            natArgExpr = addressOf(natArgExpr);
        }

        Callee deserFunc(config.deserializerNamePrefix + suffix, builder);

        return deserFunc(inObject, to_KIARA_UserType_ptr(natArgExpr), addressOf(funcDef));
    }

    return 0;
}

} // namespace KIARA
