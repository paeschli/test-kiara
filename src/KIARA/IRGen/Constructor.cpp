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
 * Constructor.cpp
 *
 *  Created on: Dec 21, 2013
 *      Author: Dmitri Rubinstein
 */

#define KIARA_LIB
#include "KIARA/IRGen/IRGen.hpp"
#include "KIARA/DB/Attributes.hpp"
#include "KIARA/DB/TypeUtils.hpp"
#include "KIARA/Compiler/PrettyPrinter.hpp"
#include "KIARA/Impl/Core.hpp"

// #define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

namespace KIARA
{

KIARA::IR::IRExpr::Ptr IRGen::createConstructor(
    IRGenContext &genCtx,
    const NativeExprInfo &natExprInfo,
    const TypeInfo &natTypeInfo)
{
    using namespace KIARA::Compiler;

    KIARA::IR::IRExpr::Ptr natArgExpr = natExprInfo.argExpr;
    KIARA::Compiler::IRBuilder &builder = genCtx.builder;
    KIARA::World &world = builder.getWorld();

    KIARA::RefType::Ptr argTy = KIARA::dyn_cast<KIARA::RefType>(natArgExpr->getExprType());
    if (!argTy)
        IRGEN_ERROR(genCtx, KIARA_INVALID_ARGUMENT,
            "Type of native argument expression is not a reference");

    assert(argTy == natTypeInfo.type);

    const KIARA::Type::Ptr &natElemType = argTy->getElementType();

    DFC_DEBUG("createConstructor: natArgExpr "<<KIARA::IR::PrettyPrinter::toString(natExprInfo.argExpr)<<" TYPE "
              <<KIARA::IR::PrettyPrinter::toString(natExprInfo.argExpr->getExprType()));

    if (KIARA::StructType::Ptr natStructType = KIARA::dyn_cast<KIARA::StructType>(natElemType))
    {
        if (natStructType->getNumElements() > 0 && !natStructType->isOpaque())
        {
            DFC_DEBUG("CONSTRUCT NATIVE TYPE: "<<natStructType->toString());

            builder.createStructCode(natStructType, genCtx.expressions, genCtx.topScope);

            KIARA::IR::Prototype::Arg args[] = {
                KIARA::IR::Prototype::Arg("$value", argTy)
            };
            KIARA::IR::Prototype::Ptr proto = KIARA::Compiler::createMangledFuncProto(
                    "constructType",
                    world.c_type<KIARA_Result>(),
                    args);
            proto->setAttribute("always_inline", "true");

            Callee assign("=", builder);
            Callee notEqual("!=", builder);
            Callee getField(".", builder);
            Callee constructType("constructType", builder);

            KIARA::IR::FunctionDefinition::Ptr funcDef = builder.getFunctionFromScope(proto, genCtx.topScope);

            if (!funcDef)
            {
                TFunction func = Function(proto);
                KIARA::Compiler::IRBuilder::ScopeGuard g(
                        new KIARA::Compiler::Scope(builder.getWorld(), "func", genCtx.topScope),
                        builder);
                builder.initFunctionScope(func);

                TLiteral successVal = Literal<KIARA_Result>(KIARA_SUCCESS, builder);

                TVar statusVar = Var("$status", builder.getWorld().type_c_int(), builder);
                TLiteral resultVal = successVal;
                TVar valueVar = Arg(func, 0);

                TBlock constrBlock = NamedBlock("constrBlock", builder.getWorld());

                // iterate over all members in IDL and create serialization
                // for corresponding members in native struct

                // call begin struct
                TExpr expr = Block(
                        assign(statusVar, successVal));

                constrBlock->addExpr(expr);

                NativeExprInfo natMemberInfo;

                for (size_t i = 0, numElems = natStructType->getNumElements();
                        i < numElems; ++i)
                {
                    KIARA::Type::Ptr elemType = natStructType->getElementAt(i);
                    const KIARA::ElementData &elemData = natStructType->getElementDataAt(i);
                    const std::string &elemName = elemData.getName();

                    DFC_DEBUG("Processing "<<i<<"-th element '"<<elemName<<"'");

                    DFC_DEBUG("construct element "<<elemName<<" of type "<<elemType->toString());

                    // if member has a main member annotation, we do not serialize it
                    if (elemData.hasAttributeValue<MainMemberAttr>())
                    {
                        DFC_DEBUG("Element with name '"<<elemName<<"' does have a main member annotation: "
                            <<elemData.getAttributeValue<MainMemberAttr>());
                        continue;
                    }

                    natMemberInfo.clear();

                    // check if a member contains a list of dependent members
                    if (const std::vector<std::string> *dependentMembers = elemData.getAttributeValuePtr<DependentMembersAttr>())
                    {
                        for (std::vector<std::string>::const_iterator it = dependentMembers->begin(),
                            end = dependentMembers->end(); it != end; ++it)
                        {
                            size_t natElemIndex = natStructType->getElementIndexByName(*it);

                            if (natElemIndex == KIARA::StructType::npos)
                                IRGEN_ERROR(genCtx, KIARA_INVALID_ARGUMENT,
                                        "Could not create constructor for native struct type '"
                                        <<natStructType->getTypeName()
                                        <<"', no dependent element '"<<*it
                                        <<"' found in the native struct type '"
                                        <<natStructType->getTypeName()<<"'");

                            TExpr elemAccess = getField(valueVar, Symbol(*it, builder));
                            natMemberInfo.dependentExprs.push_back(elemAccess);
                        }
                    }

                    // construct field

                    TExpr elemAccess = getField(valueVar, Symbol(elemName, builder));
                    DFC_DEBUG("access element code: "<<elemAccess->toString());

                    natMemberInfo.argExpr = elemAccess;


                    TypeInfo typeInfo(natMemberInfo.argExpr->getExprType(), elemData);

                    DFC_DEBUG("TRY TO CREATE MEMBER CONSTRUCTOR FOR MEMBER "<<elemName<<" TYPE "<<*typeInfo.type<<" TSEM "<<TypeUtils::getOrCreateTypeSemantics(typeInfo.type));

                    TExpr constrMember = createConstructor(genCtx, natMemberInfo, typeInfo);
                    if (IS_IRGEN_ERROR(genCtx))
                        return 0;
                    if (!constrMember)
                        continue;

                    DFC_DEBUG("member constructor "<<constrMember->toString()<<" of type "<<constrMember->getExprType()->toString());

                    expr = Block(
                            assign(statusVar, constrMember),
                            If(notEqual(statusVar, successVal),
                                Break(constrBlock)));

                    constrBlock->addExpr(expr);
                }

                // call end struct

                TExpr body = Let(statusVar, resultVal,
                    Block(constrBlock, statusVar));
                func->setBody(body);

                genCtx.addGlobalFunction(func);

                DFC_DEBUG("func : "<<func->toString());

                funcDef = func;
            }

            return constructType(natArgExpr);
        }
    }

    if (TypeUtils::isRefToCArrayPtrType(natTypeInfo.type))
    {
        KIARA::PtrType::Ptr natArrayType = KIARA::dyn_cast<KIARA::PtrType>(TypeUtils::removeTypedefs(natElemType));
        assert(natArrayType != 0);

        {
            DFC_DEBUG("CONSTRUCT NATIVE TYPE (REF TO ARRAY PTR): "<<natArrayType->toString());

            // we should have size expression
            if (natExprInfo.dependentExprs.size() < 1)
            {
                IRGEN_ERROR(genCtx, KIARA_INVALID_ARGUMENT,
                    "array representation as a pointer require dependent expression with the array size: "
                    <<natArrayType->toString());
            }

            Callee assign("=", builder);

            Type::Ptr sizeRefType = natExprInfo.dependentExprs[0]->getExprType();
            Type::Ptr sizeType = TypeUtils::getDereferencedType(sizeRefType);
            builder.createAssignCode(sizeType, sizeType, genCtx.expressions, builder.getScope()->getTopScope());
            builder.createAssignCode(natArgExpr->getExprType(), TypeUtils::getDereferencedType(natArgExpr->getExprType()), genCtx.expressions, builder.getScope()->getTopScope());

            TLiteral successVal = Literal<KIARA_Result>(KIARA_SUCCESS, builder);
            TLiteral zeroVal = IR::PrimLiteral::getZero(sizeType);

            return Block(
                assign(natExprInfo.dependentExprs[0], zeroVal),
                assign(natArgExpr, IR::PrimLiteral::getNullPtr(world)),
                successVal);
        }
    }

    return 0;
}

KIARA::IR::IRExpr::Ptr IRGen::createDestructor(
    IRGenContext &genCtx,
    const NativeExprInfo &natExprInfo,
    const TypeInfo &natTypeInfo)
{
    using namespace KIARA::Compiler;

    KIARA::IR::IRExpr::Ptr natArgExpr = natExprInfo.argExpr;
    KIARA::Compiler::IRBuilder &builder = genCtx.builder;
    KIARA::World &world = builder.getWorld();

    KIARA::RefType::Ptr argTy = KIARA::dyn_cast<KIARA::RefType>(TypeUtils::removeTypedefs(natArgExpr->getExprType()));
    if (!argTy)
        IRGEN_ERROR(genCtx, KIARA_INVALID_ARGUMENT,
            "Type of native argument expression is not a reference");

    assert(argTy == natTypeInfo.type);

    const KIARA::Type::Ptr &natElemType = argTy->getElementType();

    DFC_DEBUG("createDestructor: natArgExpr "<<KIARA::IR::PrettyPrinter::toString(natExprInfo.argExpr)<<" TYPE "
              <<KIARA::IR::PrettyPrinter::toString(natExprInfo.argExpr->getExprType()));

    if (KIARA::StructType::Ptr natStructType = KIARA::dyn_cast<KIARA::StructType>(TypeUtils::removeTypedefs(natElemType)))
    {
        if (natStructType->getNumElements() > 0 && !natStructType->isOpaque())
        {
            DFC_DEBUG("DESTRUCT NATIVE TYPE: "<<natStructType->toString());

            builder.createStructCode(natStructType, genCtx.expressions, genCtx.topScope);

            KIARA::IR::Prototype::Arg args[] = {
                KIARA::IR::Prototype::Arg("$value", argTy)
            };
            KIARA::IR::Prototype::Ptr proto = KIARA::Compiler::createMangledFuncProto(
                    "destructType",
                    world.c_type<KIARA_Result>(),
                    args);
            proto->setAttribute("always_inline", "true");

            Callee assign("=", builder);
            Callee notEqual("!=", builder);
            Callee getField(".", builder);
            Callee constructType("destructType", builder);

            KIARA::IR::FunctionDefinition::Ptr funcDef = builder.getFunctionFromScope(proto, genCtx.topScope);

            if (!funcDef)
            {
                TFunction func = Function(proto);
                KIARA::Compiler::IRBuilder::ScopeGuard g(
                        new KIARA::Compiler::Scope(builder.getWorld(), "func", genCtx.topScope),
                        builder);
                builder.initFunctionScope(func);

                TLiteral successVal = Literal<KIARA_Result>(KIARA_SUCCESS, builder);

                TVar statusVar = Var("$status", builder.getWorld().type_c_int(), builder);
                TLiteral resultVal = successVal;
                TVar valueVar = Arg(func, 0);

                TBlock constrBlock = NamedBlock("destrBlock", builder.getWorld());

                // iterate over all members in IDL and create serialization
                // for corresponding members in native struct

                // call begin struct
                TExpr expr = Block(
                        assign(statusVar, successVal));

                constrBlock->addExpr(expr);

                NativeExprInfo natMemberInfo;

                for (size_t i = 0, numElems = natStructType->getNumElements();
                        i < numElems; ++i)
                {
                    KIARA::Type::Ptr elemType = natStructType->getElementAt(i);
                    const KIARA::ElementData &elemData = natStructType->getElementDataAt(i);
                    const std::string &elemName = elemData.getName();

                    DFC_DEBUG("Processing "<<i<<"-th element '"<<elemName);

                    DFC_DEBUG("destruct element "<<elemName<<" of type "<<elemType->toString());


                    // if member has a main member annotation, we do not serialize it
                    if (elemData.hasAttributeValue<MainMemberAttr>())
                    {
                        DFC_DEBUG("Element with name '"<<elemName<<"' does have a main member annotation: "
                            <<elemData.getAttributeValue<MainMemberAttr>());
                        continue;
                    }

                    natMemberInfo.clear();

                    // check if a member contains a list of dependent members
                    if (const std::vector<std::string> *dependentMembers = elemData.getAttributeValuePtr<DependentMembersAttr>())
                    {
                        for (std::vector<std::string>::const_iterator it = dependentMembers->begin(),
                            end = dependentMembers->end(); it != end; ++it)
                        {
                            size_t natElemIndex = natStructType->getElementIndexByName(*it);

                            if (natElemIndex == KIARA::StructType::npos)
                                IRGEN_ERROR(genCtx, KIARA_INVALID_ARGUMENT,
                                        "Could not create destructor for native struct type '"
                                        <<natStructType->getTypeName()
                                        <<"', no dependent element '"<<*it
                                        <<"' found in the native struct type '"
                                        <<natStructType->getTypeName()<<"'");

                            TExpr elemAccess = getField(valueVar, Symbol(*it, builder));
                            natMemberInfo.dependentExprs.push_back(elemAccess);
                        }
                    }

                    // serialize field

                    TExpr elemAccess = getField(valueVar, Symbol(elemName, builder));
                    DFC_DEBUG("access element code: "<<elemAccess->toString());

                    natMemberInfo.argExpr = elemAccess;

                    TypeInfo typeInfo(natMemberInfo.argExpr->getExprType(), elemData);

                    DFC_DEBUG("TRY TO CREATE MEMBER DESTRUCTOR FOR MEMBER "<<elemName<<" TYPE "<<*typeInfo.type<<" TSEM "<<TypeUtils::getOrCreateTypeSemantics(typeInfo.type));

                    TExpr destrMember = createDestructor(genCtx, natMemberInfo, typeInfo);
                    if (IS_IRGEN_ERROR(genCtx))
                        return 0;
                    if (!destrMember)
                        continue;

                    DFC_DEBUG("member constructor "<<destrMember->toString()<<" of type "<<destrMember->getExprType()->toString());

                    expr = Block(
                            assign(statusVar, destrMember),
                            If(notEqual(statusVar, successVal),
                                Break(constrBlock)));

                    constrBlock->addExpr(expr);
                }

                // call end struct

                TExpr body = Let(statusVar, resultVal,
                    Block(constrBlock, statusVar));
                func->setBody(body);

                genCtx.addGlobalFunction(func);

                DFC_DEBUG("func : "<<func->toString());

                funcDef = func;
            }

            return constructType(natArgExpr);
        }
    }

    if (TypeUtils::isRefToCArrayPtrType(natTypeInfo.type))
    {
        KIARA::PtrType::Ptr natArrayType = KIARA::dyn_cast<KIARA::PtrType>(TypeUtils::removeTypedefs(natElemType));

        assert(natArrayType != 0);

        {
            DFC_DEBUG("DESTRUCT NATIVE TYPE (REF TO ARRAY PTR): "<<natArrayType->toString());

            // we should have size expression
            if (natExprInfo.dependentExprs.size() < 1)
            {
                IRGEN_ERROR(genCtx, KIARA_INVALID_ARGUMENT,
                           "array representation as a pointer require dependent expression with the array size: "
                            <<natArrayType->toString());
            }

            Callee assign("=", builder);
            Callee k_free("free", builder);

            Type::Ptr sizeRefType = natExprInfo.dependentExprs[0]->getExprType();
            Type::Ptr sizeType = TypeUtils::getDereferencedType(sizeRefType);
            builder.createAssignCode(sizeType, sizeType, genCtx.expressions, builder.getScope()->getTopScope());
            builder.createAssignCode(natArgExpr->getExprType(), TypeUtils::getDereferencedType(natArgExpr->getExprType()), genCtx.expressions, builder.getScope()->getTopScope());
            builder.createDereferenceCode(natArgExpr->getExprType(), genCtx.expressions);

            TLiteral successVal = Literal<KIARA_Result>(KIARA_SUCCESS, builder);
            TLiteral zeroVal = IR::PrimLiteral::getZero(sizeType);

            return Block(
                assign(natExprInfo.dependentExprs[0], zeroVal),
                k_free(natArgExpr),
                assign(natArgExpr, IR::PrimLiteral::getNullPtr(world)),
                successVal);
        }
    }

    return 0;
}

} // namespace KIARA

