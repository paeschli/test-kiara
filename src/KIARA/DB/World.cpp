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
 * World.cpp
 *
 *  Created on: 02.08.2012
 *      Author: Dmitri Rubinstein
 */
#define KIARA_LIB
#include "World.hpp"
#include "Type.hpp"
#include "Attributes.hpp"
#include <KIARA/Common/stddef.h>
#include <KIARA/Common/TypeTraits.hpp>
#include <cstddef>

namespace KIARA
{

World::World()
    : CycleCollector()
    , objects_()
    , namespace_()
{
    namespace_ = Namespace::create(*this, "kiara");

    type_ = getOrCreate<TypeType>(new TypeType(*this));
    void_ = getOrCreate<VoidType>(new VoidType(*this));
    unresolved_symbol_ = getOrCreate<UnresolvedSymbolType>(new UnresolvedSymbolType(*this));
    any_ = getOrCreate<AnyType>(new AnyType(*this));
    i8_ = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_i8));
    u8_ = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_u8));
    i16_ = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_i16));
    u16_ = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_u16));
    i32_ = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_i32));
    u32_ = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_u32));
    i64_ = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_i64));
    u64_ = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_u64));
    float_ = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_float));
    double_ = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_double));
    boolean_ = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_boolean));
    string_ = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_string));

    c_int8_t_   = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_c_int8_t));
    c_uint8_t_  = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_c_uint8_t));
    c_int16_t_  = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_c_int16_t));
    c_uint16_t_ = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_c_uint16_t));
    c_int32_t_  = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_c_int32_t));
    c_uint32_t_ = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_c_uint32_t));
    c_int64_t_  = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_c_int64_t));
    c_uint64_t_ = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_c_uint64_t));
    c_float_    = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_c_float));
    c_double_   = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_c_double));
    c_longdouble_ = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_c_longdouble));
    c_bool_ = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_c_bool));
    c_nullptr_ = getOrCreate<PrimType>(new PrimType(*this, PRIMTYPE_c_nullptr));

    c_char_ = TypedefType::create("char", c_ntype<normalize_type<char>::type>());
    c_wchar_t_ = 0; // FIXME
    c_schar_ = TypedefType::create("schar", c_ntype<normalize_type<signed char>::type>());
    c_uchar_ = TypedefType::create("uchar", c_ntype<normalize_type<unsigned char>::type>());
    c_short_ = TypedefType::create("short", c_ntype<normalize_type<short>::type>());
    c_ushort_ = TypedefType::create("ushort", c_ntype<normalize_type<unsigned short>::type>());
    c_int_ = TypedefType::create("int", c_ntype<normalize_type<int>::type>());
    c_uint_ = TypedefType::create("uint", c_ntype<normalize_type<unsigned int>::type>());
    c_long_ = TypedefType::create("long", c_ntype<normalize_type<long>::type>());
    c_ulong_ = TypedefType::create("ulong", c_ntype<normalize_type<unsigned long>::type>());
    c_longlong_ = TypedefType::create("longlong", c_ntype<normalize_type<long long>::type>());
    c_ulonglong_ = TypedefType::create("ulonglong", c_ntype<normalize_type<unsigned long long>::type>());
    c_size_t_ = TypedefType::create("size_t", c_ntype<normalize_type<size_t>::type>());
    c_ssize_t_ = TypedefType::create("ssize_t", c_ntype<normalize_type<ssize_t>::type>());

    c_void_ptr_ = type_c_ptr(void_);
    c_raw_char_ptr_ = type_c_ptr(c_char_);

    // Construct CStringType

    c_string_ptr_ = TypedefType::create("c_string_ptr", c_raw_char_ptr_);
    c_string_ptr_->setAttributeValue<TypeSemanticsAttr>(TypeSemantics(TypeSemantics::AM_MALLOC_FREE, TypeSemantics::VI_CSTRING_PTR));
    c_char_ptr_ = c_string_ptr_;

    // builtin annotations

    {
        Type::Ptr elements[] = { type_string() };

        encryptedAnnotation_ = StructType::create(*this, "Encrypted", sizeof(elements)/sizeof(elements[0]));
        encryptedAnnotation_->setElementNameAt(0, "keyName");
        encryptedAnnotation_->setElements(elements);
        encryptedAnnotation_->setAttributeValue<AnnotationTypeAttr>(true);
    }

    // bind system types
#define BUILTIN_TYPE(name)              \
    if (KIARA_JOIN(type_, name)())      \
        namespace_->bindType(KIARA_STRINGIZE(name), KIARA_JOIN(type_, name)());
#include <KIARA/DB/Type.def>
}

World::~World()
{
    objects_.clear();
    namespace_.reset();
#define BUILTIN_TYPE(name) \
    KIARA_JOIN(name, _).reset();
#include <KIARA/DB/Type.def>
}

const Namespace::Ptr & World::getWorldNamespace()
{
    return namespace_;
}

Object::Ptr World::findObject(const Object::Ptr &val)
{
    ObjectSet::iterator i = objects_.find(val);
    if (i != objects_.end())
        return *i;
    objects_.insert(val);
    return val;
}

PtrType::Ptr World::type_c_ptr(const Type::Ptr &elementType)
{
    return PtrType::get(elementType);
}

RefType::Ptr World::type_c_ref(const Type::Ptr &elementType)
{
    return RefType::get(elementType);
}

void World::dump()
{
}

} // namespace KIARA
