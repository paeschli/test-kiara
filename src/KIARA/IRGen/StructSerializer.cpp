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
 * StructSerializer.cpp
 *
 *  Created on: Nov 1, 2013
 *      Author: rubinste
 */

#define KIARA_LIB
#include "KIARA/IRGen/IRGen.hpp"
#include "KIARA/DB/Attributes.hpp"
#include "KIARA/Impl/Core.hpp"
#include "KIARA/Compiler/IR.hpp"
#include "KIARA/Compiler/IRUtils.hpp"

// #define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

namespace KIARA
{

KIARA::IR::IRExpr::Ptr IRGen::createStructSerializer(
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

    //KIARA::World &world = builder.getWorld();

    Callee derefPtr("*", builder);
    Callee assign("=", builder);
    Callee notEqual("!=", builder);
    Callee getField(".", builder);

    if (KIARA::StructType::Ptr idlStructType = KIARA::dyn_cast<KIARA::StructType>(idlType))
        if (KIARA::StructType::Ptr natStructType = KIARA::dyn_cast<KIARA::StructType>(natElemType))
        {
            DFC_DEBUG("MAP IDL TYPE: "<<idlStructType->toString());
            DFC_DEBUG("TO NATIVE TYPE: "<<natStructType->toString());

            builder.createStructCode(natStructType, genCtx.expressions, genCtx.topScope);

            // we need a reference to a structure
            if (!isReference)
            {
                natArgExpr = derefPtr(natArgExpr);
                DFC_DEBUG(natArgExpr->toReprString());
            }

            KIARA::IR::Prototype::Arg args[] = {
                KIARA::IR::Prototype::Arg("$msg", outObject->getExprType()),
                KIARA::IR::Prototype::Arg("$value", natArgExpr->getExprType())
            };
            KIARA::IR::Prototype::Ptr proto = KIARA::Compiler::createMangledFuncProto(
                    config.makeWriteUserTypeName(idlStructType),
                    natType->getWorld().c_type<KIARA_Result>(),
                    args);
            proto->setAttribute("always_inline", "true");

            Callee writeStructBegin(config.writeStructBeginName, builder);
            Callee writeFieldBegin(config.writeFieldBeginName, builder);
            Callee writeFieldEnd(config.writeFieldEndName, builder);
            Callee writeStructEnd(config.writeStructEndName, builder);
            Callee writeUserType(config.makeWriteUserTypeName(idlStructType), builder);

            KIARA::IR::FunctionDefinition::Ptr funcDef = builder.getFunctionFromScope(proto, genCtx.topScope);

            if (!funcDef)
            {
                TFunction func = Function(proto);
                KIARA::Compiler::IRBuilder::ScopeGuard g(
                        new KIARA::Compiler::Scope(builder.getWorld(), "func", genCtx.topScope),
                        builder);
                builder.initFunctionScope(func);

                TLiteral successVal = Literal<KIARA_Result>(KIARA_SUCCESS, builder);
                TLiteral structNameVal = Literal<std::string>(idlStructType->getTypeName(), builder);

                TVar statusVar = Var("$status", builder.getWorld().type_c_int(), builder);
                TLiteral resultVal = successVal;
                TVar msgVar = Arg(func, 0);
                TVar valueVar = Arg(func, 1);

                TBlock serBlock = NamedBlock("serBlock", builder.getWorld());

                // iterate over all members in IDL and create serialization
                // for corresponding members in native struct

                // call begin struct
                TExpr expr = Block(
                        assign(statusVar, writeStructBegin(msgVar, structNameVal)),
                        If(notEqual(statusVar, successVal),
                            Break(serBlock)));

                serBlock->addExpr(expr);

                NativeExprInfo natMemberInfo;

                for (size_t i = 0, numElems = idlStructType->getNumElements();
                        i < numElems; ++i)
                {
                    KIARA::Type::Ptr elemType = idlStructType->getElementAt(i);
                    const KIARA::ElementData &elemData = idlStructType->getElementDataAt(i);
                    const std::string &elemName = elemData.getName();
                    size_t natElemIndex = natStructType->getElementIndexByName(elemName);

                    DFC_DEBUG("Processing "<<i<<"-th element '"<<elemName<<"', native elem index = "<<natElemIndex);

                    if (natElemIndex == KIARA::StructType::npos)
                        IRGEN_ERROR(genCtx, KIARA_INVALID_ARGUMENT,
                                "Could not create serializer for native struct type '"
                                <<natStructType->getTypeName()
                                <<"', no element '"<<elemName
                                <<"' found required by the IDL struct type '"
                                <<idlStructType->getTypeName()<<"'");

                    DFC_DEBUG("serialize element "<<elemName<<" of type "<<elemType->toString());
                    const KIARA::ElementData &natElemData = natStructType->getElementDataAt(natElemIndex);

                    // if member has a main member annotation, we do not serialize it
                    if (natElemData.hasAttributeValue<MainMemberAttr>())
                    {
                        DFC_DEBUG("Element with name '"<<elemName<<"' does have a main member annotation: "
                            <<elemData.getAttributeValue<MainMemberAttr>());
                        continue;
                    }

                    natMemberInfo.clear();


                    // check if a member contains a list of dependent members
                    if (const std::vector<std::string> *dependentMembers = natElemData.getAttributeValuePtr<DependentMembersAttr>())
                    {
                        for (std::vector<std::string>::const_iterator it = dependentMembers->begin(),
                            end = dependentMembers->end(); it != end; ++it)
                        {
                            size_t natElemIndex = natStructType->getElementIndexByName(*it);

                            if (natElemIndex == KIARA::StructType::npos)
                                IRGEN_ERROR(genCtx, KIARA_INVALID_ARGUMENT,
                                        "Could not create deserializer for native struct type '"
                                        <<natStructType->getTypeName()
                                        <<"', no dependent element '"<<*it
                                        <<"' found in the native struct type '"
                                        <<natStructType->getTypeName()<<"'");

                            TExpr elemAccess = getField(valueVar, Symbol(*it, builder));
                            natMemberInfo.dependentExprs.push_back(elemAccess);
                        }
                    }

                    // create begin field

                    expr = Block(
                        assign(statusVar,
                            writeFieldBegin(msgVar,
                                Literal<std::string>(elemName, builder))),
                        If(notEqual(statusVar, successVal),
                            Break(serBlock)));

                    serBlock->addExpr(expr);

                    // serialize field

                    TExpr elemAccess = getField(valueVar, Symbol(elemName, builder));
                    DFC_DEBUG("access element code: "<<elemAccess->toString());

                    natMemberInfo.argExpr = elemAccess;
                    TExpr ser = createSerializer(genCtx, config, natMemberInfo, TypeInfo(elemType, elemData), msgVar);
                    if (!ser)
                        return 0;

                    DFC_DEBUG("serializer "<<ser->toString()<<" of type "<<ser->getExprType()->toString());

                    expr = Block(
                            assign(statusVar, ser),
                            If(notEqual(statusVar, successVal),
                                Break(serBlock)));

                    serBlock->addExpr(expr);

                    // call end field

                    expr = Block(
                            assign(statusVar, writeFieldEnd(msgVar)),
                            If(notEqual(statusVar, successVal),
                                Break(serBlock)));

                    serBlock->addExpr(expr);
                }

                // call end struct

                expr = Block(
                        assign(statusVar, writeStructEnd(msgVar)),
                        If(notEqual(statusVar, successVal),
                            Break(serBlock)));

                serBlock->addExpr(expr);

                TExpr body = Let(statusVar, resultVal,
                    Block(serBlock, statusVar));
                func->setBody(body);

                genCtx.addGlobalFunction(func);

                DFC_DEBUG("func : "<<func->toString());

                funcDef = func;
            }

            return writeUserType(outObject, natArgExpr);
        }

    return 0;
}

KIARA::IR::IRExpr::Ptr IRGen::createStructDeserializer(
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
    const KIARA::Type::Ptr &natType = natArgExpr->getExprType();
    KIARA::Compiler::IRBuilder &builder = genCtx.builder;
    //KIARA::World &world = builder.getWorld();

    Callee derefPtr("*", builder);
    Callee assign("=", builder);
    Callee notEqual("!=", builder);
    Callee getField(".", builder);

    if (KIARA::StructType::Ptr idlStructType = KIARA::dyn_cast<KIARA::StructType>(idlType))
    {
        if (KIARA::StructType::Ptr natStructType = KIARA::dyn_cast<KIARA::StructType>(natElemType))
        {
            DFC_DEBUG("MAP IDL TYPE: "<<idlStructType->toString());
            DFC_DEBUG("TO NATIVE TYPE: "<<natStructType->toString());

            builder.createStructCode(natStructType, genCtx.expressions, genCtx.topScope);

            // we need a reference to a structure
            if (!isReference)
            {
                natArgExpr = derefPtr(natArgExpr);
                DFC_DEBUG(natArgExpr->toReprString());
            }

            KIARA::IR::Prototype::Arg args[] = {
                KIARA::IR::Prototype::Arg("$msg", inObject->getExprType()),
                KIARA::IR::Prototype::Arg("$value", natArgExpr->getExprType())
            };
            KIARA::IR::Prototype::Ptr proto = KIARA::Compiler::createMangledFuncProto(
                    config.makeReadUserTypeName(idlStructType),
                    natType->getWorld().c_type<int>(),
                    args);
            proto->setAttribute("always_inline", "true");

            Callee readStructBegin(config.readStructBeginName, builder);
            Callee readFieldBegin(config.readFieldBeginName, builder);
            Callee readFieldEnd(config.readFieldEndName, builder);
            Callee readStructEnd(config.readStructEndName, builder);
            Callee readUserType(config.makeReadUserTypeName(idlStructType), builder);

            KIARA::IR::FunctionDefinition::Ptr funcDef = builder.getFunctionFromScope(proto, genCtx.topScope);

            if (!funcDef)
            {
                TFunction func = Function(proto);
                KIARA::Compiler::IRBuilder::ScopeGuard g(
                        new KIARA::Compiler::Scope(builder.getWorld(), "func", genCtx.topScope),
                        builder);
                builder.initFunctionScope(func);

                TLiteral successVal = Literal<KIARA_Result>(KIARA_SUCCESS, builder);
                TLiteral structNameVal = Literal<std::string>(idlStructType->getTypeName(), builder);

                TVar statusVar = Var("$status", builder.getWorld().type_c_int(), builder);
                TLiteral resultVal = successVal;
                TVar msgVar = Arg(func, 0);
                TVar valueVar = Arg(func, 1);

                TBlock deserBlock = NamedBlock("deserBlock", builder.getWorld());

                // iterate over all members in IDL and create deserialization
                // for corresponding members in native struct

                // call begin struct
                TExpr expr = Block(
                        assign(statusVar, readStructBegin(msgVar)),
                        If(notEqual(statusVar, successVal),
                            Break(deserBlock)));

                deserBlock->addExpr(expr);

                NativeExprInfo natMemberInfo;

                for (size_t i = 0, numElems = idlStructType->getNumElements();
                        i < numElems; ++i)
                {
                    KIARA::Type::Ptr elemType = idlStructType->getElementAt(i);
                    const KIARA::ElementData &elemData = idlStructType->getElementDataAt(i);
                    const std::string &elemName = elemData.getName();
                    size_t natElemIndex = natStructType->getElementIndexByName(elemName);

                    DFC_DEBUG("Processing "<<i<<"-th element '"<<elemName<<"', native elem index = "<<natElemIndex);

                    if (natElemIndex == KIARA::StructType::npos)
                        IRGEN_ERROR(genCtx, KIARA_INVALID_ARGUMENT,
                                "Could not create deserializer for native struct type '"
                                <<natStructType->getTypeName()
                                <<"', no element '"<<elemName
                                <<"' found required by the IDL struct type '"
                                <<idlStructType->getTypeName()<<"'");

                    DFC_DEBUG("deserialize element "<<elemName<<" of type "<<elemType->toString());

                    const KIARA::ElementData &natElemData = natStructType->getElementDataAt(natElemIndex);

                    // if member has a main member annotation, we do not serialize it
                    if (natElemData.hasAttributeValue<MainMemberAttr>())
                    {
                        DFC_DEBUG("Element with name '"<<elemName<<"' does have a main member annotation: "
                            <<elemData.getAttributeValue<MainMemberAttr>());
                        continue;
                    }

                    natMemberInfo.clear();

                    // check if a member contains a list of dependent members
                    if (const std::vector<std::string> *dependentMembers = natElemData.getAttributeValuePtr<DependentMembersAttr>())
                    {
                        for (std::vector<std::string>::const_iterator it = dependentMembers->begin(),
                            end = dependentMembers->end(); it != end; ++it)
                        {
                            size_t natElemIndex = natStructType->getElementIndexByName(*it);

                            if (natElemIndex == KIARA::StructType::npos)
                                IRGEN_ERROR(genCtx, KIARA_INVALID_ARGUMENT,
                                        "Could not create deserializer for native struct type '"
                                        <<natStructType->getTypeName()
                                        <<"', no dependent element '"<<*it
                                        <<"' found in the native struct type '"
                                        <<natStructType->getTypeName()<<"'");

                            TExpr elemAccess = getField(valueVar, Symbol(*it, builder));
                            natMemberInfo.dependentExprs.push_back(elemAccess);
                        }
                    }

                    // create begin field

                    expr = Block(
                            assign(statusVar, readFieldBegin(msgVar, Literal<std::string>(elemName, builder))),
                            If(notEqual(statusVar, successVal),
                                Break(deserBlock)));

                    deserBlock->addExpr(expr);

                    // deserialize field

                    TExpr elemAccess = getField(valueVar, Symbol(elemName, builder));
                    DFC_DEBUG("access element code: "<<elemAccess->toString());

                    natMemberInfo.argExpr = elemAccess;
                    TExpr deser = createDeserializer(genCtx, config, natMemberInfo, TypeInfo(elemType, elemData), msgVar);
                    if (!deser)
                        return 0;

                    DFC_DEBUG("deserializer "<<deser->toString()<<" of type "<<deser->getExprType()->toString());

                    expr = Block(
                            assign(statusVar, deser),
                            If(notEqual(statusVar, successVal),
                                Break(deserBlock)));

                    deserBlock->addExpr(expr);

                    // call end field

                    expr = Block(
                        assign(statusVar, readFieldEnd(msgVar)),
                        If(notEqual(statusVar, successVal),
                            Break(deserBlock)));

                    deserBlock->addExpr(expr);
                }

                // call end struct

                expr = Block(
                    assign(statusVar, readStructEnd(msgVar)),
                    If(notEqual(statusVar, successVal),
                        Break(deserBlock)));

                deserBlock->addExpr(expr);

                TExpr body = Let(statusVar, resultVal, Block(deserBlock, statusVar));
                func->setBody(body);

                genCtx.addGlobalFunction(func);

                DFC_DEBUG("func : "<<func->toString());

                funcDef = func;
            }

            return readUserType(inObject, natArgExpr);
        }
        else
        {
            IRGEN_ERROR(genCtx, KIARA_INVALID_OPERATION,
                        "type mismatch in deserializer: IDL struct type '"
                        <<KIARA::IR::IRUtils::getTypeName(idlType)
                        <<"' cannot be mapped to '"
                        <<KIARA::IR::IRUtils::getTypeName(natElemType)
                        <<"' native type");
            return 0;
        }
    }

    return 0;
}

} // namespace KIARA
