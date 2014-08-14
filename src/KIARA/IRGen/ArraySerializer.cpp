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
 * ArraySerializer.cpp
 *
 *  Created on: Nov 1, 2013
 *      Author: rubinste
 */

#define KIARA_LIB
#include "KIARA/IRGen/IRGen.hpp"
#include "KIARA/DB/Attributes.hpp"
#include "KIARA/DB/TypeUtils.hpp"
#include "KIARA/Impl/Core.hpp"
#include "KIARA/Compiler/IR.hpp"

// #define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

namespace KIARA
{

KIARA::IR::IRExpr::Ptr IRGen::createArraySerializer(
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

    Callee assign("=", builder);
    Callee notEqual("!=", builder);

    if (KIARA::ArrayType::Ptr idlArrayType = KIARA::dyn_cast<KIARA::ArrayType>(idlType))
    {
        if (KIARA::PtrType::Ptr natArrayType = KIARA::dyn_cast<KIARA::PtrType>(TypeUtils::removeTypedefs(natElemType)))
        {
            DFC_DEBUG("MAP IDL TYPE: "<<idlArrayType->toString());
            DFC_DEBUG("TO NATIVE TYPE: "<<natArrayType->toString());

            // we should have size expression
            if (natExprInfo.dependentExprs.size() < 1)
            {
                IRGEN_ERROR(genCtx, KIARA_INVALID_ARGUMENT,
                           "array representation as a pointer require dependent expression with the array size: "
                            <<natArrayType->toString());
                return 0;
            }

            KIARA::IR::Prototype::Arg args[] = {
                KIARA::IR::Prototype::Arg("$msg", outObject->getExprType()),
                KIARA::IR::Prototype::Arg("$value", natArgExpr->getExprType()),
                KIARA::IR::Prototype::Arg("$size", natType->getWorld().c_type<size_t>())
            };
            KIARA::IR::Prototype::Ptr proto = KIARA::Compiler::createMangledFuncProto(
                config.makeWriteArrayTypeName(idlArrayType),
                natType->getWorld().c_type<KIARA_Result>(),
                args);
            proto->setAttribute("always_inline", "true");

            Callee writeArrayBegin(config.writeArrayBeginName, builder);
            Callee writeArrayEnd(config.writeArrayEndName, builder);
            Callee writeArrayType(config.makeWriteArrayTypeName(idlArrayType), builder);
            Callee arrayIndex("__index__", builder);
            Callee k_print("print", builder);
            Callee lessThan("<", builder);
            Callee plus("+", builder);

            KIARA::IR::FunctionDefinition::Ptr funcDef = builder.getFunctionFromScope(proto, genCtx.topScope);

            if (!funcDef)
            {
                TFunction func = Function(proto);
                KIARA::Compiler::IRBuilder::ScopeGuard g(
                        new KIARA::Compiler::Scope(builder.getWorld(), "func", genCtx.topScope),
                        builder);
                builder.initFunctionScope(func);

                TLiteral successVal = Literal<KIARA_Result>(KIARA_SUCCESS, builder);
                TLiteral zeroVal = Literal<size_t>(0, builder);
                TLiteral oneVal = Literal<size_t>(1, builder);

                TVar statusVar = Var("$status", builder.getWorld().type_c_int(), builder);
                TVar indexVar = Var("$i", builder.getWorld().type_c_size_t(), builder);
                TLiteral resultVal = successVal;
                TVar msgVar = Arg(func, 0);
                TVar valueVar = Arg(func, 1);
                TVar sizeVar = Arg(func, 2);

                TBlock serBlock = NamedBlock("serBlock", builder.getWorld());
                DFC_DEBUG("serBlock PTR "<<serBlock.get());

                //builder.createAddressOfCode(sizeVar->getExprType(), genCtx.expressions, builder.getScope()->getTopScope());
                builder.createDereferenceCode(RefType::get(natArrayType), genCtx.expressions, builder.getScope()->getTopScope());
                builder.createArrayIndexCode(natArrayType, indexVar->getExprType(), genCtx.expressions, builder.getScope()->getTopScope());
                builder.createAssignCode(
                    valueVar->getExprType(),
                    TypeUtils::getDereferencedType(valueVar->getExprType()),
                    genCtx.expressions,
                    builder.getScope()->getTopScope());

                Type::Ptr arrayElementType = TypeUtils::getElementType(TypeUtils::getDereferencedType(valueVar->getExprType()));

                TExpr expr = Block(
                        assign(statusVar, writeArrayBegin(msgVar, sizeVar)),
                        If(notEqual(statusVar, successVal),
                            Break(serBlock)));

                serBlock->addExpr(expr);

                TExpr arrayElem = arrayIndex(valueVar, indexVar);

                NativeExprInfo natMemberInfo;
                natMemberInfo.argExpr = arrayElem;

                TExpr ser = createSerializer(genCtx, config, natMemberInfo, TypeInfo(idlArrayType->getElementType()), msgVar);
                if (!ser)
                    return 0;

                TBlock loopBlock = NamedBlock("loopBlock", builder.getWorld());
                loopBlock->addExpr(Loop(
                    If(lessThan(indexVar, sizeVar),
                        Block(assign(statusVar, ser),
                            If(notEqual(statusVar, successVal),
                                Break(serBlock)),
                            assign(indexVar, plus(indexVar, oneVal))),
                        Break(loopBlock))));

                serBlock->addExpr(Let(indexVar, zeroVal, loopBlock));

                serBlock->addExpr(Block(assign(statusVar, writeArrayEnd(msgVar)),
                                     If(notEqual(statusVar, successVal),
                                         Break(serBlock))));

                //Callee to_size_t("to_size_t", builder);

                //TExpr arraySize = to_size_t(natExprInfo.dependentExprs[0]);

                //DFC_DEBUG("Array size : "<<arraySize->toString()<<" type "<<arraySize->getExprType()->toString());

                TExpr body = Let(statusVar, resultVal,
                            Block(serBlock, statusVar));
                func->setBody(body);

                genCtx.addGlobalFunction(func);

                DFC_DEBUG("func : "<<func->toString());
                funcDef = func;
            }

            if (natArgExpr->getExprType() != RefType::get(world.c_type<size_t>()))
            {
                Type::Ptr sizeType = natExprInfo.dependentExprs[0]->getExprType();

                KIARA::IR::Prototype::Arg args[] = {
                    KIARA::IR::Prototype::Arg("$msg", outObject->getExprType()),
                    KIARA::IR::Prototype::Arg("$value", natArgExpr->getExprType()),
                    KIARA::IR::Prototype::Arg("$size", sizeType)
                };
                KIARA::IR::Prototype::Ptr proto = KIARA::Compiler::createMangledFuncProto(
                    config.makeWriteArrayTypeName(idlArrayType),
                    natType->getWorld().c_type<KIARA_Result>(),
                    args);
                proto->setAttribute("always_inline", "true");

                KIARA::IR::FunctionDefinition::Ptr funcDef = builder.getFunctionFromScope(proto, genCtx.topScope);
                if (!funcDef)
                {
                    TFunction func = Function(proto);
                    KIARA::Compiler::IRBuilder::ScopeGuard g(
                            new KIARA::Compiler::Scope(builder.getWorld(), "func", genCtx.topScope),
                            builder);
                    builder.initFunctionScope(func);

                    TLiteral successVal = Literal<KIARA_Result>(KIARA_SUCCESS, builder);
                    TLiteral zeroVal = Literal<size_t>(0, builder);
                    TLiteral oneVal = Literal<size_t>(1, builder);

                    TVar statusVar = Var("$status", builder.getWorld().type_c_int(), builder);
                    TLiteral resultVal = successVal;
                    TVar msgVar = Arg(func, 0);
                    TVar valueVar = Arg(func, 1);
                    TVar sizeVar = Arg(func, 2);

                    std::string castFuncName;
                    builder.createCastCode(
                        TypeUtils::getDereferencedType(sizeVar->getExprType()),
                        world.c_type<size_t>(),
                        castFuncName,
                        genCtx.expressions,
                        builder.getScope()->getTopScope());
                    Callee convFunc(castFuncName, builder);

                    TExpr body = Let(statusVar, resultVal,
                        Block(writeArrayType(outObject, valueVar, convFunc(sizeVar)), statusVar));
                    func->setBody(body);

                    genCtx.addGlobalFunction(func);

                    DFC_DEBUG("func : "<<func->toString());
                    funcDef = func;
                }
            }

            return writeArrayType(outObject, natArgExpr, natExprInfo.dependentExprs[0]);
        }
    }

    return 0;
}

KIARA::IR::IRExpr::Ptr IRGen::createArrayDeserializer(
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

    KIARA::World &world = builder.getWorld();

    Callee assign("=", builder);
    Callee addressOf("&", builder);
    Callee notEqual("!=", builder);
    Callee lessThan("<", builder);
    Callee plus("+", builder);

    if (KIARA::ArrayType::Ptr idlArrayType = KIARA::dyn_cast<KIARA::ArrayType>(idlType))
    {
        if (KIARA::PtrType::Ptr natArrayType = KIARA::dyn_cast<KIARA::PtrType>(TypeUtils::removeTypedefs(natElemType)))
        {
            DFC_DEBUG("MAP IDL TYPE: "<<idlArrayType->toString());
            DFC_DEBUG("TO NATIVE TYPE: "<<natArrayType->toString());

            // we should have size expression
            if (natExprInfo.dependentExprs.size() < 1)
            {
                IRGEN_ERROR(genCtx, KIARA_INVALID_ARGUMENT,
                           "array representation as a pointer require dependent expression with the array size: "
                            <<natArrayType->toString());
                return 0;
            }

            KIARA::IR::Prototype::Arg args[] = {
                KIARA::IR::Prototype::Arg("$msg", inObject->getExprType()),
                KIARA::IR::Prototype::Arg("$value", natArgExpr->getExprType()),
                KIARA::IR::Prototype::Arg("$size", KIARA::RefType::get(natType->getWorld().c_type<size_t>()))
            };
            KIARA::IR::Prototype::Ptr proto = KIARA::Compiler::createMangledFuncProto(
                config.makeReadArrayTypeName(idlArrayType),
                natType->getWorld().c_type<int>(),
                args);
            proto->setAttribute("always_inline", "true");

            Callee readArrayBegin(config.readArrayBeginName, builder);
            Callee readArrayEnd(config.readArrayEndName, builder);
            Callee readArrayType(config.makeReadArrayTypeName(idlArrayType), builder);
            Callee arrayIndex("__index__", builder);
            Callee k_malloc("malloc", builder);
            Callee k_free("free", builder);
            Callee k_sizeof("sizeof", builder);
            Callee mul("*", builder);
            Callee k_print("print", builder);

            KIARA::IR::FunctionDefinition::Ptr funcDef = builder.getFunctionFromScope(proto, genCtx.topScope);
            if (!funcDef)
            {
                TFunction func = Function(proto);
                KIARA::Compiler::IRBuilder::ScopeGuard g(
                        new KIARA::Compiler::Scope(builder.getWorld(), "func", genCtx.topScope),
                        builder);
                builder.initFunctionScope(func);

                TLiteral successVal = Literal<KIARA_Result>(KIARA_SUCCESS, builder);
                TLiteral zeroVal = Literal<size_t>(0, builder);
                TLiteral oneVal = Literal<size_t>(1, builder);

                TVar statusVar = Var("$status", builder.getWorld().type_c_int(), builder);
                TVar indexVar = Var("$i", builder.getWorld().type_c_size_t(), builder);
                TLiteral resultVal = successVal;
                TVar msgVar = Arg(func, 0);
                TVar valueVar = Arg(func, 1);
                TVar sizeVar = Arg(func, 2);

                TBlock deserBlock = NamedBlock("deserBlock", builder.getWorld());
                DFC_DEBUG("deserBlock PTR "<<deserBlock.get());

                builder.createAddressOfCode(sizeVar->getExprType(), genCtx.expressions, builder.getScope()->getTopScope());
                builder.createDereferenceCode(RefType::get(natArrayType), genCtx.expressions, builder.getScope()->getTopScope());
                builder.createArrayIndexCode(natArrayType, indexVar->getExprType(), genCtx.expressions, builder.getScope()->getTopScope());
                builder.createAssignCode(
                    valueVar->getExprType(),
                    TypeUtils::getDereferencedType(valueVar->getExprType()),
                    genCtx.expressions,
                    builder.getScope()->getTopScope());

                std::string convToArrayPtrName;
                builder.createCastCode(
                        builder.getWorld().type_c_void_ptr(),
                        TypeUtils::getDereferencedType(valueVar->getExprType()),
                        convToArrayPtrName,
                        genCtx.expressions,
                        genCtx.topScope);

                Callee convToArrayPtr(convToArrayPtrName, builder);

                Type::Ptr arrayElementType = TypeUtils::getElementType(TypeUtils::getDereferencedType(valueVar->getExprType()));


                TExpr alloc = convToArrayPtr(k_malloc(mul(k_sizeof(EType(arrayElementType)), sizeVar)));

                TExpr expr = Block(
                        assign(statusVar, readArrayBegin(msgVar, addressOf(sizeVar))),
                        If(notEqual(statusVar, successVal),
                            Break(deserBlock)),
                        assign(valueVar, alloc));

                deserBlock->addExpr(expr);

                TExpr arrayElem = arrayIndex(valueVar, indexVar);

                NativeExprInfo natMemberInfo;
                natMemberInfo.argExpr = arrayElem;

                TExpr deser = createDeserializer(genCtx, config, natMemberInfo, TypeInfo(idlArrayType->getElementType()), msgVar);
                if (!deser)
                    return 0;

                TBlock loopBlock = NamedBlock("loopBlock", builder.getWorld());
                loopBlock->addExpr(Loop(
                    If(lessThan(indexVar, sizeVar),
                        Block(assign(statusVar, deser),
                            If(notEqual(statusVar, successVal),
                                Break(deserBlock)),
                            assign(indexVar, plus(indexVar, oneVal))),
                        Break(loopBlock))));

                deserBlock->addExpr(Let(indexVar, zeroVal, loopBlock));

                deserBlock->addExpr(Block(assign(statusVar, readArrayEnd(msgVar)),
                                     If(notEqual(statusVar, successVal),
                                         Break(deserBlock))));

                //Callee to_size_t("to_size_t", builder);

                //TExpr arraySize = to_size_t(natExprInfo.dependentExprs[0]);

                //DFC_DEBUG("Array size : "<<arraySize->toString()<<" type "<<arraySize->getExprType()->toString());

                TExpr body = Let(statusVar, resultVal,
                            Block(
                                  Block(
                                    k_free(valueVar), assign(valueVar, KIARA::IR::PrimLiteral::getNullPtr(world))),
                                  deserBlock,
                                  If(notEqual(statusVar, successVal),
                                      Block(k_free(valueVar), assign(valueVar, KIARA::IR::PrimLiteral::getNullPtr(world)))),
                                  statusVar));
                func->setBody(body);

                genCtx.addGlobalFunction(func);

                DFC_DEBUG("func : "<<func->toString());
                funcDef = func;
            }

            if (natArgExpr->getExprType() != RefType::get(world.c_type<size_t>()))
            {
                Type::Ptr sizeType = natExprInfo.dependentExprs[0]->getExprType();

                KIARA::IR::Prototype::Arg args[] = {
                    KIARA::IR::Prototype::Arg("$msg", inObject->getExprType()),
                    KIARA::IR::Prototype::Arg("$value", natArgExpr->getExprType()),
                    KIARA::IR::Prototype::Arg("$size", sizeType)
                };
                KIARA::IR::Prototype::Ptr proto = KIARA::Compiler::createMangledFuncProto(
                    config.makeReadArrayTypeName(idlArrayType),
                    natType->getWorld().c_type<int>(),
                    args);
                proto->setAttribute("always_inline", "true");

                KIARA::IR::FunctionDefinition::Ptr funcDef = builder.getFunctionFromScope(proto, genCtx.topScope);
                if (!funcDef)
                {
                    TFunction func = Function(proto);
                    KIARA::Compiler::IRBuilder::ScopeGuard g(
                            new KIARA::Compiler::Scope(builder.getWorld(), "func", genCtx.topScope),
                            builder);
                    builder.initFunctionScope(func);

                    TLiteral successVal = Literal<KIARA_Result>(KIARA_SUCCESS, builder);
                    TLiteral zeroVal = Literal<size_t>(0, builder);
                    TLiteral oneVal = Literal<size_t>(1, builder);

                    TVar statusVar = Var("$status", builder.getWorld().type_c_int(), builder);
                    TLiteral resultVal = successVal;
                    TVar msgVar = Arg(func, 0);
                    TVar valueVar = Arg(func, 1);
                    TVar sizeVar = Arg(func, 2);


                    TVar tmpSizeVar = Var("$arraysize", world.c_type<size_t>(), builder);
                    std::string castFuncName;
                    builder.createCastCode(
                        tmpSizeVar->getExprType(),
                        dyn_cast<RefType>(sizeVar->getExprType())->getElementType(),
                        castFuncName,
                        genCtx.expressions,
                        builder.getScope()->getTopScope());
                    Callee convFunc(castFuncName, builder);
                    builder.createAssignCode(sizeType, TypeUtils::getDereferencedType(sizeType), genCtx.expressions, builder.getScope()->getTopScope());
                    TExpr expr = Let(tmpSizeVar, Literal<size_t>(0, builder),
                        Block(assign(statusVar, readArrayType(inObject, valueVar, tmpSizeVar)),
                            assign(sizeVar, convFunc(tmpSizeVar))));

                    TExpr body = Let(statusVar, resultVal,
                                Block(expr, statusVar));
                    func->setBody(body);

                    genCtx.addGlobalFunction(func);

                    DFC_DEBUG("func : "<<func->toString());
                    funcDef = func;
                }
            }

            return readArrayType(inObject, natArgExpr, natExprInfo.dependentExprs[0]);
        }
    }

    return 0;
}

} // namespace KIARA
