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
 * PrimitiveSerializer.cpp
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

KIARA::IR::IRExpr::Ptr IRGen::createPrimitiveSerializer(
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

    Callee deref("__deref__", builder);

    // serialize primitive types
    KIARA::Type::Ptr natArgType = natType;
    // we support references along with values
    if (natElemType && isReference)
    {
        // create dereference code before emitting __deref__ call
        KIARA::Compiler::IRBuilder::ScopeGuard g(genCtx.topScope, builder);
        builder.createDereferenceCode(natType, genCtx.expressions);

        natArgExpr = deref(natArgExpr);
        natArgType = natArgExpr->getExprType();
    }

    // suffix of the serialization function
    const char *suffix = 0;
    KIARA::Type::Ptr natIdlType;

#define _TYPEMAP(abstract, native)                          \
    if (canonicallyEqual(idlType, world.KIARA_JOIN(type_,abstract)()))      \
    {                                                       \
        suffix = KIARA_STRINGIZE(abstract);                 \
        natIdlType = world.KIARA_JOIN(type_c_,native)();    \
    } else
#define _TYPEMAP_END()                                                                      \
    {                                                                                       \
        IRGEN_ERROR(genCtx, KIARA_UNSUPPORTED_FEATURE,                                         \
                   "unsupported abstract type in serializer: "<<idlType->getTypeName());    \
        return 0;                                                                           \
    }

    _TYPEMAP(boolean, int)
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

    if (!canonicallyEqual(natIdlType, natArgType))
    {
        IRGEN_ERROR(genCtx, KIARA_INVALID_OPERATION,
                "type mismatch in serializer: provided native type: "
                <<KIARA::IR::IRUtils::getTypeName(natType)
                <<", service method require native type: "
                <<KIARA::IR::IRUtils::getTypeName(natIdlType)
                <<", or compatible for abstract type: "
                <<KIARA::IR::IRUtils::getTypeName(idlType));
    }


    DFC_DEBUG("Primitive: provided native type: "
              <<KIARA::IR::IRUtils::getTypeName(natType)
              <<", service method require native type: "
              <<KIARA::IR::IRUtils::getTypeName(natIdlType)
              <<", or compatible for abstract type: "
              <<KIARA::IR::IRUtils::getTypeName(idlType));

    Callee serFunc(config.serializerNamePrefix + suffix, builder);
    return serFunc(outObject, natArgExpr);
}

KIARA::IR::IRExpr::Ptr IRGen::createPrimitiveDeserializer(
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

    Callee addressOf("&", builder);

    // suffix of the deserialization function
    const char *suffix = 0;
    KIARA::Type::Ptr natIdlType;

#define _TYPEMAP(abstract, native)                          \
    if (canonicallyEqual(idlType, world.KIARA_JOIN(type_,abstract)()))      \
    {                                                       \
        suffix = KIARA_STRINGIZE(abstract);                 \
        natIdlType = world.KIARA_JOIN(type_c_,native)();    \
    } else
#define _TYPEMAP_END()                                                                      \
    {                                                                                       \
        IRGEN_ERROR(genCtx, KIARA_UNSUPPORTED_FEATURE,                                         \
                   "unsupported abstract type in deserializer: "<<idlType->getTypeName());  \
        return 0;                                                                           \
    }

    _TYPEMAP(boolean, int)
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

    if (!canonicallyEqual(natIdlType, natElemType))
    {
        IRGEN_ERROR(genCtx, KIARA_INVALID_OPERATION,
                "type mismatch in deserializer: provided native type: "
                <<KIARA::IR::IRUtils::getTypeName(natType)
                <<", service method require native type: "
                <<KIARA::IR::IRUtils::getTypeName(KIARA::PtrType::get(natIdlType))
                <<", or compatible for abstract type: "
                <<KIARA::IR::IRUtils::getTypeName(idlType));
    }

    if (isReference)
    {
        natArgExpr = addressOf(natArgExpr);
    }

    // FIXME add handling of pointers/references

    Callee deserFunc(config.deserializerNamePrefix + suffix, builder);
    return deserFunc(inObject, natArgExpr);
}


} // namespace KIARA
