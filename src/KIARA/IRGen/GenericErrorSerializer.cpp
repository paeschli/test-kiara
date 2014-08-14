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
 * GenericErrorSerializer.cpp
 *
 *  Created on: Oct 31, 2013
 *      Author: rubinste
 */

#define KIARA_LIB
#include "KIARA/IRGen/IRGen.hpp"
#include "KIARA/DB/Attributes.hpp"
#include "KIARA/Impl/Core.hpp"

// #define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

namespace KIARA
{

//#undef DFC_DO_DEBUG
//#include <DFC/Utils/Debug.hpp>

KIARA::IR::IRExpr::Ptr IRGen::createGenericErrorSerializer(
        IRGenContext &genCtx,
        const NativeExprInfo &natExprInfo, // argument to the serialization function
        const KIARA::IR::IRExpr::Ptr &outMessage)  // message
{
    using namespace KIARA::Compiler;

    KIARA::IR::IRExpr::Ptr natArgExpr = natExprInfo.argExpr;

    const KIARA::Type::Ptr &natType = natArgExpr->getExprType();

    //KIARA_Base *baseCtx = genCtx.baseCtx;
    KIARA::Compiler::IRBuilder &builder = genCtx.builder;

    Callee addressOf("&", builder);
    Callee writeGenericError("writeGenericError", builder);
    Callee to_KIARA_UserType_ptr("to_KIARA_UserType_ptr", builder);

    // native type can be only pointer or reference
    KIARA::Type::Ptr natElemType;
    bool isReference = false;
    if (KIARA::PtrType::Ptr pty = KIARA::dyn_cast<KIARA::PtrType>(natType))
    {
        natElemType = pty->getElementType();
    }
    else if (KIARA::RefType::Ptr rty = KIARA::dyn_cast<KIARA::RefType>(natType))
    {
        isReference = true;
        builder.createAddressOfCode(natType, genCtx.expressions, genCtx.topScope);
        natElemType = rty->getElementType();
    }
    else
    {
        IRGEN_ERROR(genCtx, KIARA_INVALID_ARGUMENT, "result type must be a pointer or reference");
    }

    if (!natElemType->hasAttributeValue<KIARA::GetGenericErrorAPIAttr>())
        IRGEN_ERROR(genCtx, KIARA_INVALID_OPERATION, "Exception type cannot be serialized, no GetGenericErrorAPI attribute");

    DFC_DEBUG("natElemType: "<<natElemType->toString());

    KIARA_GenericFunc funcPtr = natElemType->getAttributeValuePtr<KIARA::GetGenericErrorAPIAttr>()->func;
    if (!funcPtr)
    {
        IRGEN_ERROR(genCtx, KIARA_INVALID_ARGUMENT,
                "Attribute " KIARA_ATTR_GETGENERICERROR_API
                " is not set or is not of type KIARA_GenericFunc");
    }

    // Following name must be unique !
    std::string funcName = natElemType->getTypeName();
    funcName += "_";
    funcName += KIARA::GetGenericErrorAPIAttr::getAttrName();

    KIARA::IR::FunctionDefinition::Ptr funcDef = builder.lookupFunction(funcName);

    if (!funcDef)
    {
        // KIARA_Message *msg, KIARA_UserType *userException, KIARA_GetGenericError getGenericErrorFunc
        KIARA::IR::Prototype::Arg args[] = {
                KIARA::IR::Prototype::Arg("uexc", KIARA::PtrType::get(builder.lookupType("KIARA_UserType"))),
                KIARA::IR::Prototype::Arg("errorCode", KIARA::PtrType::get(builder.getWorld().type_c_int())),
                KIARA::IR::Prototype::Arg("errorMessage", KIARA::PtrType::get(builder.getWorld().type_c_char_ptr()))
        };

        KIARA::IR::Prototype::Ptr proto = KIARA::Compiler::createCFuncProto(
                funcName,
                builder.getWorld().type_c_int(),
                args,
                builder.getWorld());

        DFC_DEBUG("PROTO: "<<proto->toString());

        funcDef = genCtx.addExternFunction(proto, reinterpret_cast<void*>(*funcPtr));
    }

    // Following assertion may fail since list of linked functions is not restored
    // when new IRGenContext is created
    // assert(genCtx.functionLinkMap[funcName] == reinterpret_cast<void*>(*funcPtr));

    if (isReference)
    {
        natArgExpr = addressOf(natArgExpr);
    }

    DFC_DEBUG("Dump funcDef:");
    DFC_IFDEBUG(funcDef->dump());
    DFC_DEBUG("Dump type of funcDef:");
    DFC_IFDEBUG(funcDef->getExprType()->dump());

    return writeGenericError(
            outMessage,
            to_KIARA_UserType_ptr(natArgExpr),
            addressOf(funcDef));
}

KIARA::IR::IRExpr::Ptr IRGen::createGenericErrorDeserializer(
        IRGenContext &genCtx,
        const NativeExprInfo &natExprInfo, // argument to the serialization function
        const KIARA::IR::IRExpr::Ptr &inMessage)  // message
{
    using namespace KIARA::Compiler;

    KIARA::IR::IRExpr::Ptr natArgExpr = natExprInfo.argExpr;

    const KIARA::Type::Ptr &natType = natArgExpr->getExprType();

    //KIARA_Base *baseCtx = genCtx.baseCtx;
    KIARA::Compiler::IRBuilder &builder = genCtx.builder;

    Callee addressOf("&", builder);
    Callee to_KIARA_UserType_ptr("to_KIARA_UserType_ptr", builder);

    Callee readGenericError("readGenericError", builder);

    // native type can be only pointer or reference
    KIARA::Type::Ptr natElemType;
    bool isReference = false;
    if (KIARA::PtrType::Ptr pty = KIARA::dyn_cast<KIARA::PtrType>(natType))
    {
        natElemType = pty->getElementType();
    }
    else if (KIARA::RefType::Ptr rty = KIARA::dyn_cast<KIARA::RefType>(natType))
    {
        isReference = true;
        builder.createAddressOfCode(natType, genCtx.expressions, genCtx.topScope);
        natElemType = rty->getElementType();
    }
    else
    {
        IRGEN_ERROR(genCtx, KIARA_INVALID_ARGUMENT, "result type must be a pointer or reference");
    }

    if (natElemType->hasAttributeValue<KIARA::SetGenericErrorAPIAttr>())
    {
        DFC_DEBUG("natElemType: "<<natElemType->toString());

        KIARA_GenericFunc funcPtr = natElemType->getAttributeValuePtr<KIARA::SetGenericErrorAPIAttr>()->func;
        if (!funcPtr)
        {
            IRGEN_ERROR(genCtx, KIARA_INVALID_ARGUMENT,
                    "Attribute " KIARA_ATTR_SETGENERICERROR_API
                    " is not set or is not of type KIARA_GenericFunc");
        }

        // Following name must be unique !
        std::string funcName = natElemType->getTypeName();
        funcName += "_";
        funcName += KIARA::SetGenericErrorAPIAttr::getAttrName();

        KIARA::IR::FunctionDefinition::Ptr funcDef = builder.lookupFunction(funcName);

        if (!funcDef)
        {
            //KIARA_Message *msg, KIARA_UserType *userException, KIARA_SetGenericError setGenericErrorFunc
            KIARA::IR::Prototype::Arg args[] = {
                    KIARA::IR::Prototype::Arg("uexc", KIARA::PtrType::get(builder.lookupType("KIARA_UserType"))),
                    KIARA::IR::Prototype::Arg("errorCode", builder.getWorld().type_c_int()),
                    KIARA::IR::Prototype::Arg("errorMessage", builder.getWorld().type_c_char_ptr())
            };

            KIARA::IR::Prototype::Ptr proto = KIARA::Compiler::createCFuncProto(
                    funcName,
                    builder.getWorld().type_c_int(),
                    args,
                    builder.getWorld());

            DFC_DEBUG("PROTO: "<<proto->toString());

            funcDef = genCtx.addExternFunction(proto, reinterpret_cast<void*>(*funcPtr));
        }

        // Following assertion may fail since list of linked functions is not restored
        // when new IRGenContext is created
        // assert(genCtx.getLinkedFunction(funcName) == reinterpret_cast<void*>(*funcPtr));

        if (isReference)
        {
            natArgExpr = addressOf(natArgExpr);
        }

        DFC_DEBUG("Dump funcDef:");
        DFC_IFDEBUG(funcDef->dump());
        DFC_DEBUG("Dump type of funcDef:");
        DFC_IFDEBUG(funcDef->getExprType()->dump());

        return readGenericError(
                inMessage,
                to_KIARA_UserType_ptr(natArgExpr),
                addressOf(funcDef));
    }

    return 0;
}

} // namespace KIARA
