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
 * BinarySerializer.cpp
 *
 *  Created on: Oct 31, 2013
 *      Author: rubinste
 */

#define KIARA_LIB
#include "KIARA/IRGen/IRGen.hpp"
#include "KIARA/DB/Attributes.hpp"
#include "KIARA/Compiler/IRUtils.hpp"
#include "KIARA/Impl/Core.hpp"

// #define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

namespace KIARA
{

static std::string makeWriteTypeToMessageAsBinaryName(const KIARA::Type::Ptr &type)
{
    return "writeTypeToMessageAsBinary_" + type->getFullTypeName();
}

static std::string makeReadTypeFromMessageAsBinaryName(const KIARA::Type::Ptr &type)
{
    return "readTypeFromMessageAsBinary_" + type->getFullTypeName();
}

KIARA::IR::IRExpr::Ptr IRGen::createBinarySerializerToMessage(
        IRGenContext &genCtx,
        const NativeExprInfo &natExprInfo, // argument to the serialization function
        const TypeInfo &idlTypeInfo,
        const KIARA::IR::IRExpr::Ptr &outMessage,
        bool encryptDataIfPossible)
{
    using namespace KIARA::Compiler;

    KIARA::IR::IRExpr::Ptr natArgExpr = natExprInfo.argExpr;

    const Type::Ptr &idlType = idlTypeInfo.type;
    const KIARA::Type::Ptr &natType = natArgExpr->getExprType();
    //KIARA_Base *baseCtx = genCtx.baseCtx;
    KIARA::Compiler::IRBuilder &builder = genCtx.builder;

    Callee addressOf("&", builder);
    Callee to_KIARA_UserType_ptr("to_KIARA_UserType_ptr", builder);
    Callee derefPtr("*", builder);
    Callee assign("=", builder);
    Callee notEqual("!=", builder);
    Callee getField(".", builder);
    Callee deref("__deref__", builder);

    Callee createOutputStream("createOutputStream", builder);
    Callee freeStream("freeStream", builder);
    Callee encryptStream("encryptStream", builder);
    Callee writeMessage_binary_stream("writeMessage_binary_stream", builder);
    Callee writeTypeToMessageAsBinary(makeWriteTypeToMessageAsBinaryName(idlType), builder);

    Callee copyBufferToStream("copyBufferToStream", builder);
    Callee to_kr_dbuffer_t_ptr("to_kr_dbuffer_t_ptr", builder);

    TExpr connVar = builder.lookupExpr("$connection");

    ////

    KIARA::IR::Prototype::Arg args[] = {
        KIARA::IR::Prototype::Arg("$connection", connVar->getExprType()),
        KIARA::IR::Prototype::Arg("$msg", outMessage->getExprType()),
        KIARA::IR::Prototype::Arg("$value", natType)
    };
    KIARA::IR::Prototype::Ptr proto = KIARA::Compiler::createMangledFuncProto(
            writeTypeToMessageAsBinary.name,
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

        TVar statusVar = Var("$status", builder.getWorld().type_c_int(), builder);
        TLiteral resultVal = successVal;
        TVar connVar = Arg(func, 0);
        TVar msgVar = Arg(func, 1);
        TVar valueVar = Arg(func, 2);

        Type::Ptr encryptedNativeType;

        if ((encryptedNativeType = getEncryptedNativeType(valueVar->getExprType(), genCtx)))
        {
            DFC_DEBUG("createBinarySerializerToMessage: Encrypted native type: "<<KIARA::IR::PrettyPrinter::toString(encryptedNativeType));

            if (!isNativeAndIDLTypeCompatible(encryptedNativeType, idlTypeInfo))
            {
                IRGEN_ERROR(genCtx, KIARA_INVALID_OPERATION,
                        "type mismatch in binary serializer from message : provided encrypted native type: "
                        <<KIARA::IR::IRUtils::getTypeName(encryptedNativeType)
                        <<" is incompatible with IDL type: "
                        <<KIARA::IR::IRUtils::getTypeName(idlType));
            }
        }

        TBlock serBlock = NamedBlock("serBlock", builder.getWorld());

        TVar outStreamVar = Var("$out", genCtx.baseCtx->getContext()->getBinaryStreamPtrType(), builder);

        TCall outStreamVal = createOutputStream(connVar);

        TBlock outBlock = NamedBlock("outBlock", serBlock);

        if (encryptedNativeType)
        {
            TExpr dbufferPtr = valueVar;
            if (isa<KIARA::RefType>(valueVar->getExprType()))
            {
                builder.createAddressOfCode(valueVar->getExprType(), genCtx.expressions, builder.getScope()->getTopScope());
                dbufferPtr = addressOf(dbufferPtr);
            }
            serBlock->addExpr(
                copyBufferToStream(outStreamVar, to_kr_dbuffer_t_ptr(dbufferPtr)));
        }
        else
        {
            TExpr result = createSerializer(genCtx, _binarySerializerConfig, NativeExprInfo(valueVar), idlTypeInfo, outStreamVar);
            if (!result)
                return 0;

            serBlock->addExpr(
                assign(statusVar, result),
                If(notEqual(statusVar, successVal),
                    Break(serBlock)));

            if (encryptDataIfPossible)
            {
                TExpr result = encryptStream(connVar, outStreamVar, Literal<std::string>("default", builder));
                serBlock->addExpr(
                    assign(statusVar, result),
                    If(notEqual(statusVar, successVal),
                        Break(serBlock)));
            }
        }

        serBlock->addExpr(
            assign(statusVar,
                writeMessage_binary_stream(msgVar, outStreamVar)),
            If(notEqual(statusVar, successVal),
                Break(serBlock)));

        outBlock->addExpr(
            freeStream(outStreamVar),
            statusVar);

        TExpr body = Let(statusVar, resultVal,
            Let(outStreamVar, outStreamVal, outBlock));
        func->setBody(body);

        genCtx.addGlobalFunction(func);

        DFC_DEBUG("func : "<<func->toString());
        funcDef = func;
    }

    return writeTypeToMessageAsBinary(connVar, outMessage, natArgExpr);
}

//#define DFC_DO_DEBUG
//#include <DFC/Utils/Debug.hpp>

KIARA::IR::IRExpr::Ptr IRGen::createBinaryDeserializerFromMessage(
        IRGenContext &genCtx,
        const NativeExprInfo &natExprInfo, // argument to the serialization function
        const TypeInfo &idlTypeInfo,
        const KIARA::IR::IRExpr::Ptr &inMessage,
        bool decryptDataIfPossible)
{
    using namespace KIARA::Compiler;

    KIARA::IR::IRExpr::Ptr natArgExpr = natExprInfo.argExpr;

    const Type::Ptr &idlType = idlTypeInfo.type;
    const KIARA::Type::Ptr &natType = natArgExpr->getExprType();
    //KIARA_Base *baseCtx = genCtx.baseCtx;
    KIARA::Compiler::IRBuilder &builder = genCtx.builder;

    Callee addressOf("&", builder);
    Callee to_KIARA_UserType_ptr("to_KIARA_UserType_ptr", builder);
    Callee derefPtr("*", builder);
    Callee assign("=", builder);
    Callee notEqual("!=", builder);
    Callee getField(".", builder);
    Callee deref("__deref__", builder);

    Callee readMessage_binary_stream("readMessage_binary_stream", builder);
    Callee decryptStream("decryptStream", builder);
    Callee freeStream("freeStream", builder);
    Callee readTypeFromMessageAsBinary(makeReadTypeFromMessageAsBinaryName(idlType), builder);
    Callee createInputStream("createInputStream", builder);

    Callee copyStreamToBuffer("copyStreamToBuffer", builder);
    Callee to_kr_dbuffer_t_ptr("to_kr_dbuffer_t_ptr", builder);

    TExpr connVar = builder.lookupExpr("$connection");
    BOOST_ASSERT(connVar != 0);

    ////

    KIARA::IR::Prototype::Arg args[] = {
        KIARA::IR::Prototype::Arg("$connection", connVar->getExprType()),
        KIARA::IR::Prototype::Arg("$msg", inMessage->getExprType()),
        KIARA::IR::Prototype::Arg("$value", natType)
    };
    KIARA::IR::Prototype::Ptr proto = KIARA::Compiler::createMangledFuncProto(
            readTypeFromMessageAsBinary.name,
            natType->getWorld().c_type<int>(),
            args,
            natType->getWorld());
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

        TVar statusVar = Var("$status", builder.getWorld().type_c_int(), builder);
        TLiteral resultVal = successVal;
        TVar connVar = Arg(func, 0);
        TVar msgVar = Arg(func, 1);
        TVar valueVar = Arg(func, 2);

        Type::Ptr encryptedNativeType;

        if ((encryptedNativeType = getEncryptedNativeType(valueVar->getExprType(), genCtx)))
        {
            DFC_DEBUG("createBinaryDeserializerFromMessage: Encrypted native type: "<<KIARA::IR::PrettyPrinter::toString(encryptedNativeType));

            if (!isNativeAndIDLTypeCompatible(encryptedNativeType, idlTypeInfo))
            {
                IRGEN_ERROR(genCtx, KIARA_INVALID_OPERATION,
                        "type mismatch in binary deserializer from message : provided encrypted native type: "
                        <<KIARA::IR::IRUtils::getTypeName(encryptedNativeType)
                        <<" is incompatible with IDL type: "
                        <<KIARA::IR::IRUtils::getTypeName(idlType));
            }
        }

        TBlock deserBlock = NamedBlock("deserBlock", builder.getWorld());

        TVar inStreamVar = Var("$in", genCtx.baseCtx->getContext()->getBinaryStreamPtrType(), builder);

        TCall inStreamVal = createInputStream(connVar);

        TBlock inBlock = NamedBlock("inBlock", deserBlock);

        deserBlock->addExpr(
            assign(statusVar, readMessage_binary_stream(msgVar, inStreamVar)),
            If(notEqual(statusVar, successVal),
                Break(deserBlock)));

        if (encryptedNativeType)
        {
            TExpr dbufferPtr = valueVar;
            if (isa<KIARA::RefType>(valueVar->getExprType()))
            {
                builder.createAddressOfCode(valueVar->getExprType(), genCtx.expressions, builder.getScope()->getTopScope());
                dbufferPtr = addressOf(dbufferPtr);
            }

            deserBlock->addExpr(
                copyStreamToBuffer(to_kr_dbuffer_t_ptr(dbufferPtr), inStreamVar));
        }
        else
        {
            if (decryptDataIfPossible)
            {
                TExpr result = decryptStream(connVar, inStreamVar, Literal<std::string>("testkey", builder));
                deserBlock->addExpr(
                    assign(statusVar, result),
                    If(notEqual(statusVar, successVal),
                        Break(deserBlock)));
            }

            TExpr result = createDeserializer(genCtx, _binaryDeserializerConfig, NativeExprInfo(valueVar), idlTypeInfo, inStreamVar);
            if (!result)
                return 0;

            deserBlock->addExpr(
                assign(statusVar, result),
                If(notEqual(statusVar, successVal),
                    Break(deserBlock)));
        }

        //inBlock->addExpr(
        //        builder.createCall("print", Literal<std::string>("OK\n", builder))
        //        );

        inBlock->addExpr(
                freeStream(inStreamVar),
                statusVar);

        TExpr body = Let(statusVar, resultVal,
            Let(inStreamVar, inStreamVal, inBlock));
        func->setBody(body);

        genCtx.addGlobalFunction(func);

        DFC_DEBUG("func : "<<func->toString());
        funcDef = func;
    }

    return readTypeFromMessageAsBinary(connVar, inMessage, natArgExpr);
}

} // namespace KIARA
