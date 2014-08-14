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
 * IRGen.cpp
 *
 *  Created on: 11.07.2013
 *      Author: Dmitri Rubinstein
 */

#define KIARA_LIB
#include "KIARA/IRGen/IRGen.hpp"
#include "KIARA/Impl/Core.hpp"
#include "KIARA/DB/Attributes.hpp"
#include "KIARA/Compiler/Mangler.hpp"
#include "KIARA/Compiler/IRUtils.hpp"
#include "KIARA/Compiler/IR.hpp"
#include <KIARA/Compiler/PrettyPrinter.hpp>

// #define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

namespace KIARA
{

bool IRGen::isEncryptedIDLType(const TypeInfo &typeInfo)
{
    World &world = typeInfo.type->getWorld();
    if (!typeInfo.typeData.hasAttributeValue<AnnotationListAttr>())
        return false;

    const AnnotationList *alist = typeInfo.typeData.getAttributeValuePtr<AnnotationListAttr>();

    return getFirstAnnotationOfType(*alist, world.getEncryptedAnnotation());
}

Type::Ptr IRGen::getEncryptedNativeType(const Type::Ptr &type, const IRGenContext &genCtx)
{
    Type::Ptr elemType;
    if (PtrType::Ptr pty = dyn_cast<PtrType>(type))
        elemType = pty->getElementType();
    else if (RefType::Ptr rty = dyn_cast<RefType>(type))
        elemType = rty->getElementType();
    else
        return 0;
    StructType::Ptr sty = dyn_cast<StructType>(elemType);
    if (!sty)
        return 0;
    Type::Ptr encryptedType = sty->getAttributeValue<KIARA::NativeEncryptedTypeAttr>();
    if (encryptedType == 0)
        return 0;
    /* FIXME : do we need following tests ? */
    if (sty->getNumElements() != 1)
        return 0;
    KIARA::Type::Ptr dbufferType = genCtx.builder.lookupType("kr_dbuffer_t");
    if (sty->getElementAt(0) != dbufferType)
        return 0;
    return encryptedType;
}

bool IRGen::isNativeAndIDLTypeCompatible(const KIARA::Type::Ptr &natType, const TypeInfo &idlTypeInfo)
{
    const Type::Ptr &idlType = idlTypeInfo.type;
    World &world = natType->getWorld();


    // When natType is a pointer or reference we store its element type
    // to natElemType
    KIARA::Type::Ptr natElemType;
    //bool isReference = false;
    if (KIARA::PtrType::Ptr pty = KIARA::dyn_cast<KIARA::PtrType>(natType))
    {
        natElemType = pty->getElementType();
    }
    else if (KIARA::RefType::Ptr rty = KIARA::dyn_cast<KIARA::RefType>(natType))
    {
        //isReference = true;
        natElemType = rty->getElementType();
    }

    KIARA::Type::Ptr realNatType = natType;
    if (natElemType)
        realNatType = natElemType;

    // custom string
    if (idlType == world.type_string() &&
        natType != world.type_c_char_ptr() &&
        (realNatType->hasAttributeValue<KIARA::GetCStringAPIAttr>() ||
            realNatType->hasAttributeValue<KIARA::SetCStringAPIAttr>()))
        return true;

#define _TYPEMAP(abstract, native)                          \
    if (idlType == world.KIARA_JOIN(type_,abstract)() &&    \
        natType == world.KIARA_JOIN(type_c_,native)())      \
    {                                                       \
        return true;                                        \
    } else
#define _TYPEMAP_END()                                      \
    {                                                       \
        return false;                                       \
    }

    _TYPEMAP(i8, int8_t)
    _TYPEMAP(u8, uint8_t)
    _TYPEMAP(i16, int16_t)
    _TYPEMAP(u16, uint16_t)
    _TYPEMAP(i32, int32_t)
    _TYPEMAP(u32, uint32_t)
    _TYPEMAP(i64, int64_t)
    _TYPEMAP(u64, uint64_t)
    _TYPEMAP(float, float)
    _TYPEMAP(double, double)
    _TYPEMAP(string, char_ptr)
    // FIXME support for structs, arrays, etc. should go here
    _TYPEMAP_END()

#undef _TYPEMAP
#undef _TYPEMAP_END

    return false;
}

KIARA::IR::IRExpr::Ptr IRGen::createSerializer(
        IRGenContext &genCtx,
        const NativeExprInfo &natExprInfo, // argument to the serialization function
        const TypeInfo &idlTypeInfo,
        const KIARA::IR::IRExpr::Ptr &outMessage)
{
    using namespace KIARA::Compiler;

    //const Type::Ptr &idlType = idlTypeInfo.type;
    //const KIARA::Type::Ptr &natType = natArgExpr->getExprType();
    //KIARA_Base *baseCtx = genCtx.baseCtx;
    //KIARA::Compiler::IRBuilder &builder = genCtx.builder;

    DFC_DEBUG("createSerializer: natArgExpr "<<KIARA::IR::PrettyPrinter::toString(natExprInfo.argExpr)<<" TYPE "
              <<KIARA::IR::PrettyPrinter::toString(natExprInfo.argExpr->getExprType()));

    if (isEncryptedIDLType(idlTypeInfo))
    {
        DFC_DEBUG("createSerializer: Encrypted type: "<<*idlTypeInfo.type);
        TExpr result = createBinarySerializerToMessage(genCtx, natExprInfo, idlTypeInfo, outMessage, true);
        DFC_DEBUG("createSerializer: Encrypted expr: "<<(result ? result->toString() : "NO"));
        return result;
    }

    return createSerializer(genCtx, _defaultSerializerConfig, natExprInfo, idlTypeInfo, outMessage);
}

KIARA::IR::IRExpr::Ptr IRGen::createSerializer(
        IRGenContext &genCtx,
        const std::string &natArgName,
        const TypeInfo &idlTypeInfo,
        const KIARA::IR::IRExpr::Ptr &outMessage)
{
    using namespace KIARA::Compiler;

    //KIARA_Base *baseCtx = genCtx.baseCtx;
    KIARA::Compiler::IRBuilder &builder = genCtx.builder;
    TExpr natArgExpr = builder.lookupExpr(natArgName);
    if (!natArgExpr)
    {
        IRGEN_ERROR(genCtx, KIARA_INVALID_ARGUMENT,
                "Could not lookup symbol : '"<<natArgName<<"'");
    }

    return createSerializer(genCtx, natArgExpr, idlTypeInfo, outMessage);
}

KIARA::IR::IRExpr::Ptr IRGen::createSerializer(
    IRGenContext &genCtx,
    const IRGen::SerializerConfig &config,
    const NativeExprInfo &natExprInfo, // argument to the serialization function
    const TypeInfo &idlTypeInfo,
    const KIARA::IR::IRExpr::Ptr &outObject)
{
    using namespace KIARA::Compiler;

    KIARA::IR::IRExpr::Ptr natArgExpr = natExprInfo.argExpr;
    //const Type::Ptr &idlType = idlTypeInfo.type;
    const KIARA::Type::Ptr &natType = natArgExpr->getExprType();
    //KIARA_Base *baseCtx = genCtx.baseCtx;
    KIARA::Compiler::IRBuilder &builder = genCtx.builder;

    // map IDL type to corresponding native type
    //KIARA::World &world = idlType->getWorld();
    KIARA::Type::Ptr natIdlType;

    // When natType is a pointer or reference we store its element type
    // to natElemType
    KIARA::Type::Ptr natElemType;
    bool isReference = false;
    if (KIARA::PtrType::Ptr pty = KIARA::dyn_cast<KIARA::PtrType>(natType))
    {
        natElemType = pty->getElementType();
    }
    else if (KIARA::RefType::Ptr rty = KIARA::dyn_cast<KIARA::RefType>(natType))
    {
        isReference = true;
        natElemType = rty->getElementType();

        KIARA::Compiler::IRBuilder::ScopeGuard g(genCtx.topScope, builder);
        builder.createAddressOfCode(natType, genCtx.expressions);
    }

    KIARA::IR::IRExpr::Ptr result = createStringSerializer(
        genCtx, config, natExprInfo, natElemType, isReference, idlTypeInfo, outObject);
    if (result || IS_IRGEN_ERROR(genCtx))
        return result;

    result = createStructSerializer(
        genCtx, config, natExprInfo, natElemType, isReference, idlTypeInfo, outObject);
    if (result || IS_IRGEN_ERROR(genCtx))
        return result;

    result = createArraySerializer(
        genCtx, config, natExprInfo, natElemType, isReference, idlTypeInfo, outObject);
    if (result || IS_IRGEN_ERROR(genCtx))
        return result;

    result = createPrimitiveSerializer(
        genCtx, config, natExprInfo, natElemType, isReference, idlTypeInfo, outObject);
    return result;
}

//#define DFC_DO_DEBUG
//#include <DFC/Utils/Debug.hpp>

// convert abstract IDL type to native type and call deserializer
KIARA::IR::IRExpr::Ptr IRGen::createDeserializer(
        IRGenContext &genCtx,
        const NativeExprInfo &natExprInfo, // argument to the serialization function
        const TypeInfo &idlTypeInfo,
        const KIARA::IR::IRExpr::Ptr &inMessage)
{
    using namespace KIARA::Compiler;

    //const Type::Ptr &idlType = idlTypeInfo.type;
    //const KIARA::Type::Ptr &natType = natArgExpr->getExprType();
    //KIARA_Base *baseCtx = genCtx.baseCtx;
    //KIARA::Compiler::IRBuilder &builder = genCtx.builder;

    if (isEncryptedIDLType(idlTypeInfo))
    {
        DFC_DEBUG("createDeserializer: Encrypted type: "<<*idlTypeInfo.type);
        TExpr result = createBinaryDeserializerFromMessage(genCtx, natExprInfo, idlTypeInfo, inMessage, true);
        DFC_DEBUG("createDeserializer: Decrypted expr: "<<(result ? result->toString() : "NO"));
        return result;
    }

    return createDeserializer(genCtx, _defaultDeserializerConfig, natExprInfo, idlTypeInfo, inMessage);
}

KIARA::IR::IRExpr::Ptr IRGen::createDeserializer(
        IRGenContext &genCtx,
        const std::string &natArgName,
        const TypeInfo &idlTypeInfo,
        const KIARA::IR::IRExpr::Ptr &inMessage)
{
    using namespace KIARA::Compiler;
    //KIARA_Base *baseCtx = genCtx.baseCtx;
    KIARA::Compiler::IRBuilder &builder = genCtx.builder;
    TExpr natArgExpr = builder.lookupExpr(natArgName);
    if (!natArgExpr)
    {
        IRGEN_ERROR(genCtx, KIARA_INVALID_ARGUMENT,
                "Could not lookup symbol : '"<<natArgName<<"'");
    }
    return createDeserializer(genCtx, natArgExpr, idlTypeInfo, inMessage);
}

//#undef DFC_DO_DEBUG
//#include <DFC/Utils/Debug.hpp>

KIARA::IR::IRExpr::Ptr IRGen::createDeserializer(
    IRGenContext &genCtx,
    const DeserializerConfig &config,
    const NativeExprInfo &natExprInfo, // argument to the serialization function
    const TypeInfo &idlTypeInfo,
    const KIARA::IR::IRExpr::Ptr &inObject)
{
    using namespace KIARA::Compiler;

    KIARA::IR::IRExpr::Ptr natArgExpr = natExprInfo.argExpr;

    //const Type::Ptr &idlType = idlTypeInfo.type;
    const KIARA::Type::Ptr &natType = natArgExpr->getExprType();
    //KIARA_Base *baseCtx = genCtx.baseCtx;
    KIARA::Compiler::IRBuilder &builder = genCtx.builder;
    //World &world = builder.getWorld();

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

    // map IDL type to corresponding native type
    KIARA::Type::Ptr natIdlType;

    KIARA::IR::IRExpr::Ptr result = createStringDeserializer(
        genCtx, config, natExprInfo, natElemType, isReference, idlTypeInfo, inObject);
    if (result || IS_IRGEN_ERROR(genCtx))
        return result;

    result = createStructDeserializer(
            genCtx, config, natExprInfo, natElemType, isReference, idlTypeInfo, inObject);
        if (result || IS_IRGEN_ERROR(genCtx))
            return result;

    result = createArrayDeserializer(
            genCtx, config, natExprInfo, natElemType, isReference, idlTypeInfo, inObject);
        if (result || IS_IRGEN_ERROR(genCtx))
            return result;

    // deserialize primitive data types
    result = createPrimitiveDeserializer(
        genCtx, config, natExprInfo, natElemType, isReference, idlTypeInfo, inObject);

    return result;
}

} // namespace KIARA
