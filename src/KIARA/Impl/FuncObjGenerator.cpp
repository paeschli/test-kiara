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
 * kiara_funcobj_generator_impl.cpp
 *
 *  Created on: 18.04.2013
 *      Author: Dmitri Rubinstein
 */
#define KIARA_LIB

#include <KIARA/Common/Config.hpp>
#include <KIARA/DB/Attributes.hpp>
#include <KIARA/Impl/Core.hpp>
#include <KIARA/Impl/Network.hpp>
#include <KIARA/Impl/Interpreter.hpp>
#include <KIARA/Core/Exception.hpp>
#include <KIARA/IDL/IDLParserContext.hpp>
#include <KIARA/Compiler/PrettyPrinter.hpp>
#include <boost/assert.hpp>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "KIARA/Runtime/RuntimeEnvironment.hpp"
#include "KIARA/IRGen/IRGen.hpp"

// #define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

namespace KIARA
{
namespace Impl
{

KIARA_FuncObj * Connection::generateClientFuncObj(const char *idlMethodName, KIARA_GetDeclType declTypeGetter, const char *mapping)
{
    assert(declTypeGetter != 0);

    if (isError())
        return 0;

    KIARA::Type::Ptr ty = getContext()->getTypeFromDeclTypeGetter(declTypeGetter, getError());

    if (isError())
        return 0;

    KIARA::FunctionType::Ptr fty = KIARA::dyn_cast<KIARA::FunctionType>(ty);

    if (!fty)
    {
        setError(KIARA_INVALID_TYPE,
                "not a function type passed to kiaraGenerateClientFuncObj");
        return 0;
    }
    if (!fty->getAttributeValue<KIARA::WrapperFuncAttr>())
    {
        setError(KIARA_INVALID_OPERATION,
                "function type has no registered wrapper function");
        return 0;
    }

    FuncObjMap::iterator it = funcObjects_.find(fty);
    if (it != funcObjects_.end())
        return it->second;

    std::string serviceMethodName = idlMethodName;
    std::vector<std::string> namePath;
    boost::algorithm::split(namePath, serviceMethodName,
            boost::algorithm::is_any_of("."));
    if (namePath.size() != 2)
    {
        setError(KIARA_UNSUPPORTED_FEATURE,
                serviceMethodName+" is invalid, only method names in format service_name '.' method_name are supported. ");
        return 0;
    }

    KIARA::ServiceType::Ptr serviceType = KIARA::dyn_cast<KIARA::ServiceType>(getContext()->getModule()->lookupType(namePath[0]));
    if (!serviceType)
    {
        setError(KIARA_INVALID_ARGUMENT,
                std::string("no such service: '")+namePath[0]+"'");
        return 0;
    }

    size_t serviceMethodIndex = serviceType->getElementIndexByName(namePath[1]);
    if (serviceMethodIndex == KIARA::StructType::npos)
    {
        setError(KIARA_INVALID_ARGUMENT,
                std::string("no service method '")+serviceMethodName+"' found");
        return 0;
    }

    KIARA::FunctionType::Ptr serviceMethodType =
            KIARA::dyn_cast<KIARA::FunctionType>(serviceType->getElementAt(serviceMethodIndex));
    if (!serviceMethodType)
    {
        setError(KIARA_INVALID_ARGUMENT,
                std::string("service element '")+serviceMethodName+"' is not a function");
        return 0;
    }

    DFC_DEBUG("METHOD TYPE: "<<serviceMethodType->toReprString());

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

    DFC_DEBUG("INFO: Creating wrapper for function: "<<*fty);

    // KIARA FuncObj functions have always following signature
    // int Func(KIARA_FuncObj * closure, Arg1 type1, Arg2 type2, ...);
    // Note: the number of arguments and their type is fixed,
    //       no varargs are used.

    KIARA::IRGenContext genCtx(this, getRuntimeEnvironment().getTopScope());

    KIARA::Compiler::IRBuilder &builder = genCtx.builder;

    std::vector<KIARA::IR::Prototype::Arg> args;

    args.push_back(
            KIARA::IR::Prototype::Arg(
                    "$closure",
                    getContext()->getFuncObjPtrType()));

    for (size_t i = 0; i < fty->getNumParams(); ++i)
    {
        args.push_back(
                KIARA::IR::Prototype::Arg(
                        fty->getParamName(i),
                        fty->getParamType(i)));
    }

    KIARA::IR::Prototype::Ptr proto = KIARA::Compiler::createCFuncProto(
            fty->getFullTypeName(),
            getWorld().type_c_int(),
            args,
            getWorld());
    proto->setAttribute("always_inline", "true");
    TFunction func = Function(proto);

    {
        KIARA::Compiler::IRBuilder::ScopeGuard g("func", builder);
        builder.initFunctionScope(func);

        TLiteral successVal = Literal<KIARA_Result>(KIARA_SUCCESS, builder);
        TLiteral exceptionVal = Literal<KIARA_Result>(KIARA_EXCEPTION, builder);

        Callee getConnection("getConnection", builder);
        Callee createRequestMessage("createRequestMessage", builder);
        Callee assign("=", builder);
        Callee notEqual("!=", builder);
        Callee equal("==", builder);
        Callee sendMessageSync("sendMessageSync", builder);
        Callee freeMessage("freeMessage", builder);

        TVar statusVar = Var("$status", getWorld().type_c_int(), builder);
        TLiteral resultVal = successVal;
        TVar connVar = Var("$connection", getContext()->getConnectionPtrType(), builder);
        TCall connVal = getConnection(Arg(func, 0));
        TVar msgVar = Var("$msg", getContext()->getMessagePtrType(), builder);

        TCall msgVal = createRequestMessage(connVar,
                Literal<std::string>(serviceMethodName, builder),
                Literal<size_t>(serviceMethodName.length(), builder));

        TBlock msgBlock = NamedBlock("msgBlock", getWorld());
        TBlock serBlock = NamedBlock("serBlock", getWorld());
        TBlock deserBlock = NamedBlock("deserBlock", getWorld());

        serBlock->setExprListSize(numServiceInArgs);

        // generate serialization of inputs

        DFC_DEBUG("idlMap size = "<<idlMap.size());

        for (NameToIndexAndTypeMap::iterator it = idlMap.begin(), end = idlMap.end(); it != end; ++it)
        {
            DFC_DEBUG("idlMap item "<<it->first<<" "<<it->second.index<<" type "
                    <<KIARA::IR::PrettyPrinter::toString(it->second.getType()));

            // We handle only inputs here
            if (it->second.kind != KIARA::IRGen::ARG_INPUT)
                continue;
            NameToTypeInfoMap::iterator natIt = nativeMap.find(it->first);
            if (natIt == nativeMap.end())
            {
                IRGEN_ERROR(genCtx, KIARA_INVALID_OPERATION,
                        "no mapping for argument '"<<it->first
                        <<"' of IDL service method '"<<serviceMethodName<<"'");
            }

            // argument name : it->first
            // IDL type      : it->second.second
            // native type   : natIt->second
            // argument type must be natIt->second
            TExpr expr = KIARA::IRGen::createSerializer(
                genCtx, it->first, it->second.getTypeInfo(), msgVar);
            if (!expr)
                return 0;

            expr = Block(
                    assign(statusVar, expr),
                    If(notEqual(statusVar, successVal),
                        Break(serBlock)));

            serBlock->setExprAt(it->second.index, expr);
        }


        TBlock ioBlock = NamedBlock("ioBlock", getWorld());
        // send message
        ioBlock->addExpr(
            assign(statusVar, sendMessageSync(connVar, msgVar, msgVar)));

        // get types
        KIARA::Type::Ptr resultIDLType = serviceMethodType->getReturnType();
        const KIARA::ElementData &resultElementData = serviceMethodType->getReturnElementData();
        KIARA::Type::Ptr resultNativeType;
        KIARA::Type::Ptr exceptionNativeType;

        {
            NameToTypeInfoMap::iterator it = nativeMap.find("$result");
            if (it != nativeMap.end())
                resultNativeType = it->second.type;
            it = nativeMap.find("$exception");
            if (it != nativeMap.end())
                exceptionNativeType = it->second.type;
        }

        // exception handling
        if (exceptionNativeType)
        {
            TExpr exceptionExpr = builder.lookupExpr("$exception");
            BOOST_ASSERT(exceptionExpr->getExprType() == exceptionNativeType);

            TBlock excBlock = NamedBlock("excBlock",
                assign(statusVar,
                    KIARA::IRGen::createGenericErrorDeserializer(
                        genCtx,
                        exceptionExpr,
                        msgVar)),
                If(notEqual(statusVar, successVal),
                    Break(serBlock)),
                assign(statusVar, exceptionVal));

            ioBlock->addExpr(If(equal(statusVar, exceptionVal), excBlock));
        }

        ioBlock->addExpr(If(notEqual(statusVar, successVal),
                            Break(serBlock)));

        serBlock->addExpr(ioBlock);

        // generate deserialization of outputs

        if (resultIDLType != KIARA::VoidType::get(getWorld()))
        {
            if (resultNativeType)
            {
                // expression "$result" must be of type resultNativeType
                TExpr expr = KIARA::IRGen::createDeserializer(
                    genCtx, "$result", KIARA::IRGen::TypeInfo(resultIDLType, resultElementData),
                    msgVar);
                if (!expr)
                    return 0;

                expr = Block(assign(statusVar, expr),
                             If(notEqual(statusVar, successVal),
                                Break(serBlock)));

                serBlock->addExpr(expr);
            }
            else
                IRGEN_ERROR(genCtx, KIARA_INVALID_OPERATION, "Missing native '$result' function argument");
        }
        else if (!resultIDLType)
        {
            IRGEN_ERROR(genCtx, KIARA_INVALID_OPERATION, "Missing IDL service method result");
        }

        msgBlock->addExpr(serBlock);

        // msgBlock->addExpr(print(Literal<std::string>("OK\n")));

        msgBlock->addExpr(freeMessage(msgVar));
        msgBlock->addExpr(statusVar);

        TExpr body = Let(statusVar, resultVal,
            Let(connVar, connVal, Let(msgVar, msgVal, msgBlock)));
        func->setBody(body);
    }

    DFC_DEBUG("DUMP SCOPE: "<<getRuntimeEnvironment().getTopScope()->toReprString());

    DFC_DEBUG("FUNC : "<<func->toString());

    KIARA_FuncObj *fobj = createFuncObj();
    if (getRuntimeEnvironment().isCompilationSupported())
    {
        void *funcPtr = getRuntimeEnvironment().compileFunction(func, genCtx, "KIARA_FUNC");
        assert(funcPtr != 0);

        fobj->func = (KIARA_Func)funcPtr;
        fobj->base.funcType = getContext()->wrapType(fty);
    }
    else
    {
        // compilation is not supported
        fobj->base.vafunc = KIARA_Interpreter; // interpreter
        fobj->base.funcType = getContext()->wrapType(fty);
        fobj->func = fty->getAttributeValue<KIARA::WrapperFuncAttr>();
    }

    funcObjects_[fty] = fobj;
    return fobj;
}

} // namespace Impl
} // namespace KIARA
