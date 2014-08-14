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
 * IRGen.hpp
 *
 *  Created on: 11.07.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_IRGEN_IRGEN_HPP_INCLUDED
#define KIARA_IRGEN_IRGEN_HPP_INCLUDED

#include "KIARA/Common/Config.hpp"
#include "KIARA/Compiler/IRBuilder.hpp"
#include "KIARA/Compiler/Mangler.hpp"
#include "KIARA/DB/MemberSemantics.hpp"
#include "KIARA/DB/Attributes.hpp"
#include <boost/noncopyable.hpp>

namespace KIARA
{

namespace Impl
{
class Base;
}

struct FunctionLinkInfo
{
    void *funcPtr;
    std::string funcName;

    FunctionLinkInfo(void *funcPtr = 0)
        : funcPtr(funcPtr)
        , funcName()
    { }

    FunctionLinkInfo(void *funcPtr, const std::string &funcName)
        : funcPtr(funcPtr)
        , funcName(funcName)
    { }
};

typedef std::map<std::string, FunctionLinkInfo> FunctionLinkMap;

struct IRGenContext
{
    KIARA::Impl::Base *baseCtx;
    KIARA::Compiler::Scope::Ptr topScope;
    KIARA::Compiler::IRBuilder builder;
    std::vector<KIARA::IR::IRExpr::Ptr> expressions;
    FunctionLinkMap functionLinkMap;

    IRGenContext(KIARA::Impl::Base *baseCtx, const KIARA::Compiler::Scope::Ptr &topScope)
        : baseCtx(baseCtx)
        , topScope(topScope)
        , builder(topScope)
        , functionLinkMap()
    { }

    KIARA::IR::ExternFunction::Ptr addExternFunction(const KIARA::IR::Prototype::Ptr &proto, const FunctionLinkInfo &funcInfo)
    {
        KIARA::IR::ExternFunction::Ptr funcDef = new KIARA::IR::ExternFunction(proto);
        builder.addFunctionToScope(funcDef, topScope);
        expressions.push_back(funcDef);
        functionLinkMap[proto->getMangledName()] = funcInfo;
        return funcDef;
    }

    KIARA::IR::ExternFunction::Ptr addExternFunction(const KIARA::IR::Prototype::Ptr &proto, void *func)
    {
        return addExternFunction(proto, FunctionLinkInfo(func));
    }

    void addGlobalFunction(const KIARA::IR::Function::Ptr &func)
    {
        builder.addFunctionToScope(func, topScope);
        expressions.push_back(func);
    }

    void * getLinkedFunction(const std::string &name) const
    {
        FunctionLinkMap::const_iterator it = functionLinkMap.find(name);
        if (it != functionLinkMap.end())
            return it->second.funcPtr;
        return 0;
    }
};

#define IRGEN_ERROR_RET(irGenCtx, errorCode, message, retValue) \
do {                                                            \
    std::ostringstream msgs;                                    \
    msgs << message;                                            \
    (irGenCtx).baseCtx->setError(errorCode, msgs.str());        \
    return (retValue);                                          \
} while(false)

#define IRGEN_ERROR(irGenCtx, errorCode, message)               \
    IRGEN_ERROR_RET(irGenCtx, errorCode, message, 0)

#define IS_IRGEN_ERROR(irGenCtx)                                \
    (irGenCtx).baseCtx->isError()

class IRGen : private boost::noncopyable
{
public:

    enum ArgKind
    {
        ARG_INPUT,
        ARG_OUTPUT,
        ARG_SPECIAL
    };

    /** Advanced information about IDL or native type */
    struct TypeInfo
    {
        KIARA::Type::Ptr type;
        ElementData typeData;

        TypeInfo() : type(), typeData() { }
        TypeInfo(const Type::Ptr &type) : type(type), typeData() { }
        TypeInfo(const Type::Ptr &type, const ElementData &typeData) : type(type), typeData(typeData) { }
        TypeInfo(const TypeInfo &typeInfo) : type(typeInfo.type), typeData(typeInfo.typeData) { }

    };

    /** Information about native expressions that should be serialized/deserialized */
    struct NativeExprInfo
    {
        KIARA::IR::IRExpr::Ptr argExpr; /* primary serialization/deserialization argument expression */
        KIARA::MemberSemantics memberSemantics;
        std::vector<KIARA::IR::IRExpr::Ptr> dependentExprs; /* secondary dependent arguments for serialization/deserialization */

        NativeExprInfo() : argExpr(), memberSemantics(), dependentExprs() { }
        NativeExprInfo(const KIARA::IR::IRExpr::Ptr &natArgExpr) : argExpr(natArgExpr), memberSemantics(), dependentExprs() { }

        void clear()
        {
            argExpr = 0;
            memberSemantics.setToDefaults();
            dependentExprs.clear();
        }
    };

    /** Information about native types */
    struct NativeTypeInfo
    {
        KIARA::Type::Ptr type;
        KIARA::MemberSemantics memberSemantics;

        NativeTypeInfo() : type(), memberSemantics() { }
        NativeTypeInfo(const KIARA::Type::Ptr &type) : type(type), memberSemantics() { }

        void clear()
        {
            type = 0;
            memberSemantics.setToDefaults();
        }
    };

    /** Information about function arguments that should be serialized/deserialized */
    struct ArgInfo
    {
        ArgKind kind;
        size_t index;
        TypeInfo typeInfo;

        ArgInfo() : kind(ARG_INPUT), index(-1), typeInfo() { }

        //ArgInfo(ArgKind kind, size_t index, const KIARA::Type::Ptr &type)
        //    : kind(kind)
        //    , index(index)
        //    , typeInfo(type)
        //{ }

        ArgInfo(ArgKind kind, size_t index, const TypeInfo &typeInfo)
            : kind(kind)
            , index(index)
            , typeInfo(typeInfo)
        { }

        const TypeInfo & getTypeInfo() const { return typeInfo; }

        const Type::Ptr & getType() const { return typeInfo.type; }
    };

    static bool isEncryptedIDLType(const TypeInfo &typeInfo);

    /* Returns original native type from the encrypted type */
    static Type::Ptr getEncryptedNativeType(const Type::Ptr &type, const IRGenContext &genCtx);

    static bool isNativeAndIDLTypeCompatible(const KIARA::Type::Ptr &natType, const TypeInfo &idlTypeInfo);

    // Serializers

    static KIARA::IR::IRExpr::Ptr createSerializer(
            IRGenContext &genCtx,
            const NativeExprInfo &natExprInfo, // argument to the serialization function
            const TypeInfo &idlTypeInfo,
            const KIARA::IR::IRExpr::Ptr &outMessage);

    static KIARA::IR::IRExpr::Ptr createSerializer(
            IRGenContext &genCtx,
            const std::string &natArgName,
            const TypeInfo &idlTypeInfo,
            const KIARA::IR::IRExpr::Ptr &outMessage);

    static KIARA::IR::IRExpr::Ptr createBinarySerializerToMessage(
            IRGenContext &genCtx,
            const NativeExprInfo &natExprInfo, // argument to the serialization function
            const TypeInfo &idlTypeInfo,
            const KIARA::IR::IRExpr::Ptr &outMessage,
            bool encryptDataIfPossible = false);

    struct SerializerConfig
    {
        std::string serializerNamePrefix;
        std::string writeStructBeginName;
        std::string writeStructEndName;
        std::string writeFieldBeginName;
        std::string writeFieldEndName;
        std::string writeUserTypeName;
        std::string writeArrayBeginName;
        std::string writeArrayEndName;
        std::string writeArrayTypeName;

        SerializerConfig() { }

        SerializerConfig(const char *serializerNamePrefix,
                         const char *writeStructBeginName,
                         const char *writeStructEndName,
                         const char *writeFieldBeginName,
                         const char *writeFieldEndName,
                         const char *writeUserTypeName,
                         const char *writeArrayBeginName,
                         const char *writeArrayEndName,
                         const char *writeArrayTypeName)
            : serializerNamePrefix(serializerNamePrefix)
            , writeStructBeginName(writeStructBeginName)
            , writeStructEndName(writeStructEndName)
            , writeFieldBeginName(writeFieldBeginName)
            , writeFieldEndName(writeFieldEndName)
            , writeUserTypeName(writeUserTypeName)
            , writeArrayBeginName(writeArrayBeginName)
            , writeArrayEndName(writeArrayEndName)
            , writeArrayTypeName(writeArrayTypeName)
        { }

        std::string makeWriteArrayTypeName(const KIARA::ArrayType::Ptr &idlArrayType) const
        {
            return writeArrayTypeName + "_" + KIARA::Compiler::Mangler::getMangledName(idlArrayType->getElementType());
        }

        std::string makeWriteUserTypeName(const KIARA::StructType::Ptr &idlStructType) const
        {
            return writeUserTypeName + "_" + idlStructType->getFullTypeName();
        }

    };

    static KIARA::IR::IRExpr::Ptr createSerializer(
            IRGenContext &genCtx,
            const SerializerConfig &config,
            const NativeExprInfo &natExprInfo, // argument to the serialization function
            const TypeInfo &idlTypeInfo,
            const KIARA::IR::IRExpr::Ptr &outObject);

    // Deserializers

    struct DeserializerConfig
    {
        std::string deserializerNamePrefix;
        std::string readStructBeginName;
        std::string readStructEndName;
        std::string readFieldBeginName;
        std::string readFieldEndName;
        std::string readUserTypeName;
        std::string readArrayBeginName;
        std::string readArrayEndName;
        std::string readArrayTypeName;

        DeserializerConfig() { }

        DeserializerConfig(const char *deserializerNamePrefix,
                           const char *readStructBeginName,
                           const char *readStructEndName,
                           const char *readFieldBeginName,
                           const char *readFieldEndName,
                           const char *readUserTypeName,
                           const char *readArrayBeginName,
                           const char *readArrayEndName,
                           const char *readArrayTypeName)
            : deserializerNamePrefix(deserializerNamePrefix)
            , readStructBeginName(readStructBeginName)
            , readStructEndName(readStructEndName)
            , readFieldBeginName(readFieldBeginName)
            , readFieldEndName(readFieldEndName)
            , readUserTypeName(readUserTypeName)
            , readArrayBeginName(readArrayBeginName)
            , readArrayEndName(readArrayEndName)
            , readArrayTypeName(readArrayTypeName)
        { }

        std::string makeReadArrayTypeName(const KIARA::ArrayType::Ptr &idlArrayType) const
        {
            return readArrayTypeName + "_" + KIARA::Compiler::Mangler::getMangledName(idlArrayType->getElementType());
        }

        std::string makeReadUserTypeName(const KIARA::StructType::Ptr &idlStructType) const
        {
            return readUserTypeName + "_" + idlStructType->getFullTypeName();
        }
    };

    static KIARA::IR::IRExpr::Ptr createDeserializer(
            IRGenContext &genCtx,
            const DeserializerConfig &config,
            const NativeExprInfo &natExprInfo, // argument to the serialization function
            const TypeInfo &idlTypeInfo,
            const KIARA::IR::IRExpr::Ptr &inObject);

    static KIARA::IR::IRExpr::Ptr createDeserializer(
            IRGenContext &genCtx,
            const NativeExprInfo &natExprInfo, // argument to the serialization function
            const TypeInfo &idlTypeInfo,
            const KIARA::IR::IRExpr::Ptr &inMessage);

    static KIARA::IR::IRExpr::Ptr createDeserializer(
            IRGenContext &genCtx,
            const std::string &natArgName,
            const TypeInfo &idlTypeInfo,
            const KIARA::IR::IRExpr::Ptr &inMessage);

    static KIARA::IR::IRExpr::Ptr createBinaryDeserializerFromMessage(
            IRGenContext &genCtx,
            const NativeExprInfo &natExprInfo, // argument to the serialization function
            const TypeInfo &idlTypeInfo,
            const KIARA::IR::IRExpr::Ptr &inMessage,
            bool decryptDataIfPossible = false);

    // Error serialization/deserialization

    static KIARA::IR::IRExpr::Ptr createGenericErrorSerializer(
            IRGenContext &genCtx,
            const NativeExprInfo &natExprInfo, // argument to the serialization function
            const KIARA::IR::IRExpr::Ptr &outMessage);  // message

    static KIARA::IR::IRExpr::Ptr createGenericErrorDeserializer(
            IRGenContext &genCtx,
            const NativeExprInfo &natExprInfo, // argument to the serialization function
            const KIARA::IR::IRExpr::Ptr &inMessage);  // message

    // Allocation/deallocation

    static KIARA::IR::IRExpr::Ptr createAllocator(
            IRGenContext &genCtx,
            const TypeInfo &natTypeInfo,          // natTypeInfo     - type info of the allocated type : T
            const TypeInfo *destNatTypeInfo = 0); // destNatTypeInfo - type info of the storage type   : T*

    static KIARA::IR::IRExpr::Ptr createDeallocator(
            IRGenContext &genCtx,
            const KIARA::IR::IRExpr::Ptr &value,
            const TypeInfo &natTypeInfo);

    // Construction / Destruction

    static KIARA::IR::IRExpr::Ptr createConstructor(
            IRGenContext &genCtx,
            const NativeExprInfo &natExprInfo,
            const TypeInfo &natTypeInfo);

    static KIARA::IR::IRExpr::Ptr createDestructor(
            IRGenContext &genCtx,
            const NativeExprInfo &natExprInfo,
            const TypeInfo &natTypeInfo);

protected:

    static KIARA::IR::IRExpr::Ptr createStructSerializer(
        IRGenContext &genCtx,
        const IRGen::SerializerConfig &config,
        const NativeExprInfo &natExprInfo,
        const KIARA::Type::Ptr &natElemType,
        bool isReference,
        const TypeInfo &idlTypeInfo,
        const KIARA::IR::IRExpr::Ptr &outObject);

    static KIARA::IR::IRExpr::Ptr createStructDeserializer(
        IRGenContext &genCtx,
        const IRGen::DeserializerConfig &config,
        const NativeExprInfo &natExprInfo,
        const KIARA::Type::Ptr &natElemType,
        bool isReference,
        const TypeInfo &idlTypeInfo,
        const KIARA::IR::IRExpr::Ptr &inObject);

    static KIARA::IR::IRExpr::Ptr createArraySerializer(
        IRGenContext &genCtx,
        const IRGen::SerializerConfig &config,
        const NativeExprInfo &natExprInfo,
        const KIARA::Type::Ptr &natElemType,
        bool isReference,
        const TypeInfo &idlTypeInfo,
        const KIARA::IR::IRExpr::Ptr &outObject);

    static KIARA::IR::IRExpr::Ptr createArrayDeserializer(
        IRGenContext &genCtx,
        const IRGen::DeserializerConfig &config,
        const NativeExprInfo &natExprInfo,
        const KIARA::Type::Ptr &natElemType,
        bool isReference,
        const TypeInfo &idlTypeInfo,
        const KIARA::IR::IRExpr::Ptr &inObject);

    static KIARA::IR::IRExpr::Ptr createStringSerializer(
        IRGenContext &genCtx,
        const IRGen::SerializerConfig &config,
        const NativeExprInfo &natExprInfo,
        const KIARA::Type::Ptr &natElemType,
        bool isReference,
        const TypeInfo &idlTypeInfo,
        const KIARA::IR::IRExpr::Ptr &outObject);

    static KIARA::IR::IRExpr::Ptr createStringDeserializer(
        IRGenContext &genCtx,
        const IRGen::DeserializerConfig &config,
        const NativeExprInfo &natExprInfo,
        const KIARA::Type::Ptr &natElemType,
        bool isReference,
        const TypeInfo &idlTypeInfo,
        const KIARA::IR::IRExpr::Ptr &inObject);

    static KIARA::IR::IRExpr::Ptr createPrimitiveSerializer(
        IRGenContext &genCtx,
        const IRGen::SerializerConfig &config,
        const NativeExprInfo &natExprInfo,
        const KIARA::Type::Ptr &natElemType,
        bool isReference,
        const TypeInfo &idlTypeInfo,
        const KIARA::IR::IRExpr::Ptr &outObject);

    static KIARA::IR::IRExpr::Ptr createPrimitiveDeserializer(
        IRGenContext &genCtx,
        const IRGen::DeserializerConfig &config,
        const NativeExprInfo &natExprInfo,
        const KIARA::Type::Ptr &natElemType,
        bool isReference,
        const TypeInfo &idlTypeInfo,
        const KIARA::IR::IRExpr::Ptr &inObject);


private:
    static IRGen::SerializerConfig _defaultSerializerConfig;

    static IRGen::DeserializerConfig _defaultDeserializerConfig;

    static IRGen::SerializerConfig _binarySerializerConfig;

    static IRGen::DeserializerConfig _binaryDeserializerConfig;
};

} // namespace KIARA

#endif /* KIARA_IRGEN_HPP_INCLUDED */
