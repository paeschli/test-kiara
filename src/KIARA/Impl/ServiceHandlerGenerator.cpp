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
 * kiara_servicehandler_generator_impl.cpp
 *
 *  Created on: 11.07.2013
 *      Author: Dmitri Rubinstein
 */
#define KIARA_LIB

#include <KIARA/Common/Config.hpp>
#include <KIARA/DB/Type.hpp>
#include <KIARA/DB/Attributes.hpp>
#include <KIARA/DB/TypeUtils.hpp>
#include <KIARA/Impl/Core.hpp>
#include <KIARA/Core/Exception.hpp>
#include <KIARA/IDL/IDLParserContext.hpp>
#include <KIARA/Compiler/PrettyPrinter.hpp>
#include <KIARA/Compiler/IRUtils.hpp>
#include <boost/assert.hpp>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstring>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "KIARA/Impl/Network.hpp"
#include "KIARA/Runtime/RuntimeEnvironment.hpp"
#include "KIARA/IRGen/IRGen.hpp"

// #define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

namespace KIARA
{
namespace Impl
{

#define REG_ERROR(irGenCtx, errorCode, message) \
    IRGEN_ERROR_RET(irGenCtx, errorCode, message, errorCode)

KIARA_Result Service::registerServiceFunc(
        const char *idlMethodName,
        KIARA_GetDeclType declTypeGetter,
        const char *mapping,
        KIARA_ServiceFunc serviceFunc)
{
    assert(declTypeGetter != 0);

    if (isError())
        return getErrorCode();

    KIARA::Type::Ptr ty = getContext()->getTypeFromDeclTypeGetter(declTypeGetter, getError());

    if (isError())
        return getErrorCode();

    KIARA::FunctionType::Ptr fty = KIARA::dyn_cast<KIARA::FunctionType>(ty);

    if (!fty)
    {
        setError(KIARA_INVALID_TYPE,
                "not a function type passed to kiaraRegisterServiceFunc");
        return KIARA_INVALID_TYPE;
    }

    KIARA_ServiceFunc serviceFuncPtr = serviceFunc ? serviceFunc : fty->getAttributeValue<KIARA::ServiceFuncAttr>();

    if (!serviceFuncPtr)
    {
        setError(KIARA_INVALID_OPERATION,
                "function type has no registered service function");
        return KIARA_INVALID_OPERATION;
    }

    std::string serviceMethodName = idlMethodName;
    std::vector<std::string> namePath;
    boost::algorithm::split(namePath, serviceMethodName,
            boost::algorithm::is_any_of("."));
    if (namePath.size() != 2)
    {
        setError(KIARA_UNSUPPORTED_FEATURE,
                serviceMethodName+" is invalid, only method names in format service_name '.' method_name are supported. ");
        return getErrorCode();
    }

    KIARA::ServiceType::Ptr serviceType = KIARA::dyn_cast<KIARA::ServiceType>(getContext()->getModule()->lookupType(namePath[0]));
    if (!serviceType)
    {
        setError(KIARA_INVALID_ARGUMENT,
                std::string("no such service: '")+namePath[0]+"'");
        return getErrorCode();
    }

    size_t serviceMethodIndex = serviceType->getElementIndexByName(namePath[1]);
    if (serviceMethodIndex == KIARA::StructType::npos)
    {
        setError(KIARA_INVALID_ARGUMENT,
                std::string("no service method '")+serviceMethodName+"' found");
        return getErrorCode();
    }

    KIARA::FunctionType::Ptr serviceMethodType =
            KIARA::dyn_cast<KIARA::FunctionType>(serviceType->getElementAt(serviceMethodIndex));
    if (!serviceMethodType)
    {
        setError(KIARA_INVALID_ARGUMENT,
                std::string("service element '")+serviceMethodName+"' is not a function");
        return getErrorCode();
    }

    DFC_DEBUG("METHOD TYPE: "<<serviceMethodType->toReprString());

    serviceFuncList_.push_back(ServiceFuncRecord(
        idlMethodName,
        fty,
        serviceMethodName,
        serviceMethodType,
        serviceFuncPtr));

    return KIARA_SUCCESS;
}


KIARA_Result ServiceHandler::compileServiceFunc(
    const ServiceFuncRecord &serviceFunc)
{
    const std::string &idlMethodName = serviceFunc.idlMethodName;
    const KIARA::FunctionType::Ptr &fty = serviceFunc.funcType;                         // native function type
    const std::string &serviceMethodName = serviceFunc.serviceMethodName;               // IDL
    const KIARA::FunctionType::Ptr &serviceMethodType = serviceFunc.serviceMethodType;  // IDL
    KIARA_ServiceFunc serviceFuncPtr = serviceFunc.serviceFuncPtr;


    SyncServiceFuncObjMap::iterator it = syncServiceFuncObjMap_.find(serviceFuncPtr);
    if (it != syncServiceFuncObjMap_.end())
    {
        // FIXME function called twice with the same type
        //       should we check mappings as well ?
        if (getContext()->unwrapType(it->second->base.funcType) == fty)
            return KIARA_SUCCESS;
    }

    // create mapping of argument names to IDL service method arguments

    //KIARA::StructType::Ptr serviceArgsType = serviceMethodType->getArgsType();
    const size_t serviceNumArgs = serviceMethodType->getNumParams();

    size_t numServiceInArgs = 0;
    //size_t numServiceOutArgs = 0;

    typedef std::map<std::string, KIARA::IRGen::TypeInfo> NameToTypeInfoMap;
    typedef std::map<std::string, KIARA::IRGen::ArgInfo> NameToIndexAndTypeMap;
    NameToIndexAndTypeMap idlMap;

    idlMap["$result"] = KIARA::IRGen::ArgInfo(
        KIARA::IRGen::ARG_OUTPUT, -1,
        KIARA::IRGen::TypeInfo(serviceMethodType->getReturnType(),
                               serviceMethodType->getReturnElementData()));
    //idlMap["$args"] = KIARA::IRGen::ArgInfo(KIARA::IRGen::ARG_SPECIAL, -1, serviceArgsType);
    for (size_t i = 0; i < serviceNumArgs; ++i)
    {
        DFC_DEBUG("SERVICE IDL ARGUMENT "<<i<<" name '"<<serviceMethodType->getParamName(i)<<"' type "
                <<KIARA::IR::PrettyPrinter::toString(serviceMethodType->getParamType(i)));
        idlMap[serviceMethodType->getParamName(i)] =
                KIARA::IRGen::ArgInfo(
                    KIARA::IRGen::ARG_INPUT, i,
                    KIARA::IRGen::TypeInfo(serviceMethodType->getParamType(i),
                                           serviceMethodType->getParamElementDataAt(i)));
        ++numServiceInArgs;
    }

    // create mapping of argument names to generated function arguments

    NameToTypeInfoMap nativeMap;
    size_t numNativeInArgs = 0;
    for (size_t i = 0; i < fty->getNumParams(); ++i)
    {
        DFC_DEBUG("NATIVE ARGUMENT "<<i<<" name '"<<fty->getParamName(i)<<"' "
                <<KIARA::IR::PrettyPrinter::toString(fty->getParamType(i)));
        nativeMap[fty->getParamName(i)] =
            KIARA::IRGen::TypeInfo(fty->getParamType(i),
                                   fty->getParamElementDataAt(i));

        // FIXME: this is pretty stupid check if the argument is a real input argument or not.
        //        It depends on having '$' at the beginning of special argument
        //        names like $result or $exception.
        if (!boost::algorithm::starts_with(fty->getParamName(i), "$"))
            ++numNativeInArgs;
    }

    DFC_DEBUG("INFO: Number of native arguments: "<<numNativeInArgs);
    DFC_DEBUG("INFO: Number of IDL arguments: "<<numServiceInArgs);

    if (numNativeInArgs != numServiceInArgs)
    {
        // FIXME this should cause an error in strict mode
        DFC_DEBUG("WARNING: Number of native arguments is not equal to number of IDL arguments :"
                " native="<<numNativeInArgs<<" IDL="<<numServiceInArgs);
    }

    DFC_DEBUG(*fty);

    using namespace KIARA::Compiler;

    DFC_DEBUG("INFO: Creating service handler for function: "<<*fty);

    // KIARA service handlers have always following signature
    // int Func(KIARA_Connection * conn, KIARA_Message *msgOut, KIARA_Message *msgIn);

    KIARA::IRGenContext genCtx(this, getRuntimeEnvironment().getTopScope());

    KIARA::Compiler::IRBuilder &builder = genCtx.builder;

    std::vector<KIARA::IR::Prototype::Arg> args;

    args.push_back(
            KIARA::IR::Prototype::Arg(
                    "$closure",
                    getContext()->getServiceFuncObjPtrType()));
    args.push_back(
            KIARA::IR::Prototype::Arg(
                    "$msgOut",
                    getContext()->getMessagePtrType()));
    args.push_back(
            KIARA::IR::Prototype::Arg(
                    "$msgIn",
                    getContext()->getMessagePtrType()));

    KIARA::IR::Prototype::Ptr proto = KIARA::Compiler::createCFuncProto(
        fty->getFullTypeName(),
        getWorld().type_c_int(),
        args,
        getWorld());
    proto->setAttribute("always_inline", "true");
    KIARA::IR::Function::Ptr func = new KIARA::IR::Function(proto, 0);

    {
        KIARA::Compiler::IRBuilder::ScopeGuard g("func", builder);
        builder.initFunctionScope(func);

        TLiteral successVal = Literal<KIARA_Result>(KIARA_SUCCESS, builder);
        TLiteral exceptionVal = Literal<KIARA_Result>(KIARA_EXCEPTION, builder);

        Callee getServiceConnection("getServiceConnection", builder);
        Callee assign("=", builder);
        Callee equal("==", builder);
        Callee notEqual("!=", builder);
        Callee derefPtr("*", builder);
        Callee addressOf("&", builder);

        TVar statusVar = Var("$status", getWorld().type_c_int(), builder);
        TLiteral resultVal = successVal;
        TVar connVar = Var("$connection", getContext()->getConnectionPtrType(), builder);
        TCall connVal = getServiceConnection(Arg(func, 0));
        TVar msgOut = Arg(func, 1);
        TVar msgIn = Arg(func, 2);

        TBlock innerBlock = NamedBlock("innerBlock", getWorld());
        TExpr varDecls;

        DFC_DEBUG("idlMap size = "<<idlMap.size());

        // create allocators and deallocators
        for (NameToIndexAndTypeMap::iterator it = idlMap.begin(), end = idlMap.end(); it != end; ++it)
        {
            const std::string &argName = it->first;
            const KIARA::IRGen::ArgInfo &argInfo = it->second;

            DFC_DEBUG("idlMap item "<<argName<<" "<<argInfo.index<<" type "
                    <<KIARA::IR::PrettyPrinter::toString(argInfo.getType()));

            // we handle only inputs and outputs here
            if (argInfo.kind != KIARA::IRGen::ARG_INPUT &&
                argInfo.kind != KIARA::IRGen::ARG_OUTPUT)
                continue;
            // void type require no mapping
            if (argInfo.getType() == KIARA::VoidType::get(getWorld()))
                continue;
            NameToTypeInfoMap::iterator natIt = nativeMap.find(argName);
            if (natIt == nativeMap.end())
            {
                REG_ERROR(genCtx, KIARA_INVALID_OPERATION,
                        "no mapping for argument '"<<argName
                        <<"' of IDL service method '"<<serviceMethodName<<"'");
            }

            // argument name : it->first
            // IDL type      : it->second.getType()
            // native type   : natIt->second.type

            const KIARA::IRGen::TypeInfo &destNatTypeInfo = natIt->second;

            TVar var = Var(argName, destNatTypeInfo.type, builder);
            TExpr initValue;
            TExpr freeValue;
            TExpr constrValue;
            TExpr destrValue;
            TExpr body = varDecls ? varDecls : innerBlock;
            if (KIARA::PtrType::Ptr pty = KIARA::dyn_cast<KIARA::PtrType>(destNatTypeInfo.type))
            {
                builder.createDereferenceCode(pty, genCtx.expressions, builder.getScope()->getTopScope());

                // perform dereference of the type info (FIXME this is really awkward, is there a better way ?)
                KIARA::IRGen::TypeInfo typeInfo(pty->getElementType());

                initValue = KIARA::IRGen::createAllocator(genCtx, typeInfo, &destNatTypeInfo);
                if (isError())
                    return getErrorCode();

                freeValue = KIARA::IRGen::createDeallocator(genCtx, var, destNatTypeInfo);
                if (isError())
                    return getErrorCode();

                {
                    TExpr derefVar = derefPtr(var);

                    constrValue = KIARA::IRGen::createConstructor(genCtx,
                                                                  KIARA::IRGen::NativeExprInfo(derefVar),
                                                                  KIARA::IRGen::TypeInfo(derefVar->getExprType()));
                    if (isError())
                        return getErrorCode();
                }

                {
                    TExpr derefVar = derefPtr(var);

                    destrValue = KIARA::IRGen::createDestructor(genCtx,
                                                                KIARA::IRGen::NativeExprInfo(derefVar),
                                                                KIARA::IRGen::TypeInfo(derefVar->getExprType()));
                    if (isError())
                        return getErrorCode();
                }

                if (constrValue)
                {
                    body = Block(constrValue, body);
                }

                if (destrValue)
                {
                    body = Block(body, destrValue);
                }

                if (freeValue)
                {
                    // we deallocating, so body need to have a call to a deallocator
                    body = Block(body, freeValue);
                }
            }
            else if (KIARA::RefType::Ptr rty = KIARA::dyn_cast<KIARA::RefType>(destNatTypeInfo.type))
            {
                KIARA::PtrType::Ptr pty = KIARA::PtrType::get(rty->getElementType());
                builder.createDereferenceCode(pty, genCtx.expressions, builder.getScope()->getTopScope());
                builder.createAddressOfCode(rty, genCtx.expressions, builder.getScope()->getTopScope());

                // perform dereference of the type info (FIXME this is really awkward, is there a better way ?)
                KIARA::IRGen::TypeInfo typeInfo(rty->getElementType());

                initValue = KIARA::IRGen::createAllocator(genCtx, typeInfo, &destNatTypeInfo);

                if (isError())
                    return getErrorCode();
                initValue = derefPtr(initValue);

                TExpr varPtr = addressOf(var);

                freeValue = KIARA::IRGen::createDeallocator(genCtx, varPtr, KIARA::IRGen::TypeInfo(varPtr->getExprType()));
                if (isError())
                    return getErrorCode();

                constrValue = KIARA::IRGen::createConstructor(genCtx, KIARA::IRGen::NativeExprInfo(var), destNatTypeInfo);
                if (isError())
                    return getErrorCode();

                destrValue = KIARA::IRGen::createDestructor(genCtx, KIARA::IRGen::NativeExprInfo(var), destNatTypeInfo);
                if (isError())
                    return getErrorCode();

                if (constrValue)
                {
                    body = Block(constrValue, body);
                }

                if (destrValue)
                {
                    body = Block(body, destrValue);
                }

                if (freeValue)
                {
                    // we deallocating, so body need to have a call to a deallocator
                    body = Block(body, freeValue);
                }
            }

            varDecls = Let(var, initValue, body);
        }

        // allocate exception
        KIARA::Type::Ptr exceptionNativeType;

        {
            NameToTypeInfoMap::iterator it = nativeMap.find("$exception");

            if (it != nativeMap.end())
            {
                const std::string &excName = it->first;
                const KIARA::IRGen::TypeInfo &destNatTypeInfo = it->second;
                exceptionNativeType = destNatTypeInfo.type;

                TVar var = builder.createVariableInScope(excName, exceptionNativeType);
                TExpr initValue;
                TExpr freeValue;
                TExpr constrValue;
                TExpr destrValue;
                TExpr body = varDecls ? varDecls : innerBlock;
                if (KIARA::PtrType::Ptr pty = KIARA::dyn_cast<KIARA::PtrType>(exceptionNativeType))
                {
                    builder.createDereferenceCode(pty, genCtx.expressions, builder.getScope()->getTopScope());

                    // perform dereference of the type info (FIXME this is really awkward, is there a better way ?)
                    KIARA::IRGen::TypeInfo typeInfo(pty->getElementType());

                    initValue = KIARA::IRGen::createAllocator(genCtx, typeInfo, &destNatTypeInfo);
                    if (isError())
                        return getErrorCode();

                    freeValue = KIARA::IRGen::createDeallocator(genCtx, var, destNatTypeInfo);
                    if (isError())
                        return getErrorCode();

                    {

                        TExpr derefVar = derefPtr(var);

                        constrValue = KIARA::IRGen::createConstructor(genCtx,
                                                                      KIARA::IRGen::NativeExprInfo(derefVar),
                                                                      KIARA::IRGen::TypeInfo(derefVar->getExprType()));
                        if (isError())
                            return getErrorCode();
                    }

                    {
                        TExpr derefVar = derefPtr(var);

                        destrValue = KIARA::IRGen::createDestructor(genCtx,
                                                                    KIARA::IRGen::NativeExprInfo(derefVar),
                                                                    KIARA::IRGen::TypeInfo(derefVar->getExprType()));
                        if (isError())
                            return getErrorCode();
                    }

                    if (constrValue)
                    {
                        body = Block(constrValue, body);
                    }

                    if (destrValue)
                    {
                        body = Block(body, destrValue);
                    }

                    if (freeValue)
                    {
                        // we deallocating, so body need to have a call to a deallocator
                        body = Block(body, freeValue);
                    }
                }
                else if (KIARA::RefType::Ptr rty = KIARA::dyn_cast<KIARA::RefType>(exceptionNativeType))
                {
                    KIARA::PtrType::Ptr pty = KIARA::PtrType::get(rty->getElementType());
                    builder.createDereferenceCode(pty, genCtx.expressions, builder.getScope()->getTopScope());
                    builder.createAddressOfCode(rty, genCtx.expressions, builder.getScope()->getTopScope());

                    // perform dereference of the type info (FIXME this is really awkward, is there a better way ?)
                    KIARA::IRGen::TypeInfo typeInfo(rty->getElementType());

                    initValue = KIARA::IRGen::createAllocator(genCtx, typeInfo, &destNatTypeInfo);
                    if (isError())
                        return getErrorCode();
                    initValue = builder.createCall("*", initValue);

                    TExpr varPtr = builder.createCall("&", var);

                    freeValue = KIARA::IRGen::createDeallocator(genCtx, varPtr, KIARA::IRGen::TypeInfo(varPtr->getExprType()));
                    if (isError())
                        return getErrorCode();

                    constrValue = KIARA::IRGen::createConstructor(genCtx, KIARA::IRGen::NativeExprInfo(var), destNatTypeInfo);
                    if (isError())
                        return getErrorCode();

                    destrValue = KIARA::IRGen::createDestructor(genCtx, KIARA::IRGen::NativeExprInfo(var), destNatTypeInfo);
                    if (isError())
                        return getErrorCode();

                    if (constrValue)
                    {
                        body = Block(constrValue, body);
                    }

                    if (destrValue)
                    {
                        body = Block(body, destrValue);
                    }

                    if (freeValue)
                    {
                        // we deallocating, so body need to have a call to a deallocator
                        body = Block(body, freeValue);
                    }
                }

                varDecls = Let(var, initValue, body);
            }
        }

        TBlock deserBlock = NamedBlock("deserBlock", getWorld());
        TBlock callBlock = NamedBlock("callBlock", getWorld());
        TBlock serBlock = NamedBlock("serBlock", getWorld());

        deserBlock->setExprListSize(numServiceInArgs);

        // create deserializers
        for (NameToIndexAndTypeMap::iterator it = idlMap.begin(), end = idlMap.end(); it != end; ++it)
        {
            DFC_DEBUG("idlMap item "<<it->first<<" "<<it->second.index<<" type "
                    <<KIARA::IR::PrettyPrinter::toString(it->second.getType()));

            // we handle only inputs here
            if (it->second.kind != KIARA::IRGen::ARG_INPUT)
                continue;
            NameToTypeInfoMap::iterator natIt = nativeMap.find(it->first);
            if (natIt == nativeMap.end())
            {
                REG_ERROR(genCtx, KIARA_INVALID_OPERATION,
                        "no mapping for argument '"<<it->first
                        <<"' of IDL service method '"<<serviceMethodName<<"'");
            }

            // argument name : it->first
            // IDL type      : it->second.getType()
            // native type   : natIt->second
            TExpr arg = builder.lookupExpr(it->first);
            // FIXME following is a temporary hack, meaningful type conversion for deserialization
            //       is required
            if (getWorld().type_c_char_ptr() == arg->getExprType() ||
                    (!KIARA::dyn_cast<KIARA::PtrType>(arg->getExprType()) &&
                    !KIARA::dyn_cast<KIARA::RefType>(arg->getExprType())))
            {
                builder.createAddressOfCode(KIARA::RefType::get(arg->getExprType()), genCtx.expressions, genCtx.topScope);
                arg = addressOf(arg);
            }
            TExpr expr = KIARA::IRGen::createDeserializer(
                genCtx, arg, it->second.getTypeInfo(), msgIn);
            if (!expr)
            {
                return getErrorCode();
            }

            expr = Block(
                assign(statusVar, expr),
                If(notEqual(statusVar, successVal),
                    Break(callBlock)));

            if (deserBlock->getExprListSize() <= it->second.index)
                deserBlock->setExprListSize(it->second.index+1);
            deserBlock->setExprAt(it->second.index, expr);
        }
        deserBlock->update();

        // get types
        KIARA::Type::Ptr resultIDLType = serviceMethodType->getReturnType();
        const KIARA::ElementData &resultElementData = serviceMethodType->getReturnElementData();
        KIARA::Type::Ptr resultNativeType;

        {
            NameToTypeInfoMap::iterator it = nativeMap.find("$result");
            if (it != nativeMap.end())
                resultNativeType = it->second.type;
        }

        // generate exception handling
        TExpr excExpr;
        if (exceptionNativeType)
        {
            TBlock excBlock = NamedBlock("excBlock", getWorld());
            excExpr = If(equal(statusVar, exceptionVal),
                            excBlock);

            TExpr exceptionExpr = builder.lookupExpr("$exception");
            BOOST_ASSERT(exceptionExpr->getExprType() == exceptionNativeType);
            TExpr errorSerializer = KIARA::IRGen::createGenericErrorSerializer(
                                                        genCtx,
                                                        exceptionExpr,
                                                        msgOut);
            if (!errorSerializer)
                return getErrorCode();

            excBlock->addExpr(assign(statusVar, errorSerializer));
            excBlock->addExpr(assign(statusVar, exceptionVal));
            excBlock->addExpr(Break(callBlock));
        }

        // generate serialization of outputs
        if (resultIDLType != KIARA::VoidType::get(getWorld()))
        {
            if (resultNativeType)
            {
                // create derefence code only for composite types (references and pointers) with
                // primitive element types

                KIARA::PtrType::Ptr pty = KIARA::dyn_cast<KIARA::PtrType>(resultNativeType);
                KIARA::RefType::Ptr rty = KIARA::dyn_cast<KIARA::RefType>(resultNativeType);
                if ((pty) ||
                    (rty && !KIARA::TypeUtils::isOpaqueType(rty->getElementType())))
                {
                    builder.createDereferenceCode(resultNativeType, genCtx.expressions);
                }
                TExpr resultVar = builder.lookupExpr("$result");
                if (KIARA::dyn_cast<KIARA::PtrType>(resultNativeType))
                    resultVar = derefPtr(resultVar);

                DFC_DEBUG("resultVar : "<<resultVar->toString()<<" type : "<<KIARA::IR::IRUtils::getTypeName(resultVar->getExprType()));

                TExpr expr = KIARA::IRGen::createSerializer(
                        genCtx, resultVar, KIARA::IRGen::TypeInfo(resultIDLType, resultElementData), msgOut);
                if (!expr)
                    return getErrorCode();

                expr = Block(
                        assign(statusVar, expr),
                        If(notEqual(statusVar, successVal),
                            Break(serBlock)));

                serBlock->addExpr(expr);
            }
            else
                REG_ERROR(genCtx, KIARA_INVALID_OPERATION, "Missing native '$result' function argument");
        }
        else if (!resultIDLType)
        {
            REG_ERROR(genCtx, KIARA_INVALID_OPERATION, "Missing IDL service method result");
        }

        // create function call
        TExpr callExpr;
        {

            // Following name must be unique !
            std::string serviceFuncName = fty->getTypeName() + "_handler";
            KIARA::IR::FunctionDefinition::Ptr serviceDef = builder.lookupFunction(serviceFuncName);
            if (!serviceDef)
            {
                std::vector<KIARA::IR::Prototype::Arg> protoArgs;
                protoArgs.push_back(KIARA::IR::Prototype::Arg(
                                    "$connection",
                                    getContext()->getConnectionPtrType()));
                for (size_t i = 0; i < fty->getNumParams(); ++i)
                {
                    protoArgs.push_back(KIARA::IR::Prototype::Arg(fty->getParamName(i), fty->getParamType(i)));
                }
                KIARA::IR::Prototype::Ptr proto = KIARA::Compiler::createCFuncProto(
                    serviceFuncName,
                    fty->getReturnType(),
                    protoArgs,
                    builder.getWorld());

                DFC_DEBUG("PROTO:\n"); //???DEBUG
                DFC_DEBUG(proto->toString());

                serviceDef = new KIARA::IR::ExternFunction(proto);
                builder.addFunctionToScope(serviceDef);
                genCtx.expressions.push_back(serviceDef);
                genCtx.functionLinkMap[serviceFuncName] = reinterpret_cast<void*>(serviceFuncPtr);
            }

            std::vector<TExpr> args;
            args.reserve(fty->getNumParams()+1);
            args.push_back(connVar);
            for (size_t i = 0; i < fty->getNumParams(); ++i)
            {
                DFC_DEBUG("NATIVE ARGUMENT "<<i<<" name '"<<fty->getParamName(i)<<"' type "
                        <<KIARA::IR::PrettyPrinter::toString(fty->getParamType(i)));
                TExpr arg = builder.lookupExpr(fty->getParamName(i));
                BOOST_ASSERT(arg != 0);
                args.push_back(arg);
            }
            callExpr = builder.createCall("=",
                    statusVar, builder.createCall(serviceFuncName, args));
        }


        callBlock->addExpr(deserBlock);
        callBlock->addExpr(callExpr);
        if (excExpr)
            callBlock->addExpr(excExpr);
        callBlock->addExpr(If(notEqual(statusVar, successVal),
                            Break(callBlock)));
        callBlock->addExpr(serBlock);
        innerBlock->addExpr(callBlock);

        TExpr body = Let(statusVar, resultVal,
            Let(connVar, connVal, Block(varDecls, statusVar)));
        func->setBody(body);
    }

    DFC_DEBUG("DUMP SCOPE: "<<getRuntimeEnvironment().getTopScope()->toReprString());

    DFC_DEBUG("FUNC : "<<func->toString());

    if (getRuntimeEnvironment().isCompilationSupported())
    {
        void *funcPtr = getRuntimeEnvironment().compileFunction(func, genCtx, "KIARA_SERVICE");
        assert(funcPtr != 0);

        KIARA_SyncServiceHandler serviceHandler = (KIARA_SyncServiceHandler)funcPtr;

        KIARA_ServiceFuncObj *serviceFuncObj = createServiceFuncObj();
        serviceFuncObj->base.syncHandler = serviceHandler;
        serviceFuncObj->base.funcType = getContext()->wrapType(fty);

        installServiceFunc(idlMethodName, serviceFuncPtr, serviceFuncObj);

        return KIARA_SUCCESS;
    }
    else
    {
        setError(KIARA_UNSUPPORTED_FEATURE, "Registration with interpreter not implemented yet");
        return getErrorCode();
    }
}

} // namespace Impl
} // namespace KIARA
