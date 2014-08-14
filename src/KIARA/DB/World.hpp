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
 * World.hpp
 *
 *  Created on: 01.08.2012
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_DB_WORLD_HPP_INCLUDED
#define KIARA_DB_WORLD_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <KIARA/Common/Types.hpp>
#include <KIARA/Utils/Error.hpp>
#include <KIARA/Core/CycleCollector.hpp>
#include <KIARA/DB/Type.hpp>
#include <KIARA/DB/DerivedTypes.hpp>
#include <string>

namespace KIARA
{

class KIARA_API World : public CycleCollector
{
public:

    World();
    virtual ~World();

    const Namespace::Ptr & getWorldNamespace();

    // Abstract built-in types
    const TypeType::Ptr & type_type() const { return type_; }
    const VoidType::Ptr & type_void() const { return void_; }
    const UnresolvedSymbolType::Ptr & type_unresolved_symbol() const { return unresolved_symbol_; }
    const AnyType::Ptr & type_any() const { return any_; }
    const PrimType::Ptr & type_i8() const { return i8_; }
    const PrimType::Ptr & type_u8() const { return u8_; }
    const PrimType::Ptr & type_i16() const { return i16_; }
    const PrimType::Ptr & type_u16() const { return u16_; }
    const PrimType::Ptr & type_i32() const { return i32_; }
    const PrimType::Ptr & type_u32() const { return u32_; }
    const PrimType::Ptr & type_i64() const { return i64_; }
    const PrimType::Ptr & type_u64() const { return u64_; }
    const PrimType::Ptr & type_float() const { return float_; }
    const PrimType::Ptr & type_double() const { return double_; }
    const PrimType::Ptr & type_boolean() const { return boolean_; }
    const PrimType::Ptr & type_string() const { return string_; }

    // Native built-in types

    const PrimType::Ptr & type_c_int8_t() const { return c_int8_t_; }
    const PrimType::Ptr & type_c_uint8_t() const { return c_uint8_t_; }
    const PrimType::Ptr & type_c_int16_t() const { return c_int16_t_; }
    const PrimType::Ptr & type_c_uint16_t() const { return c_uint16_t_; }
    const PrimType::Ptr & type_c_int32_t() const { return c_int32_t_; }
    const PrimType::Ptr & type_c_uint32_t() const { return c_uint32_t_; }
    const PrimType::Ptr & type_c_int64_t() const { return c_int64_t_; }
    const PrimType::Ptr & type_c_uint64_t() const { return c_uint64_t_; }

    const Type::Ptr & type_c_char() const { return c_char_; }
    const Type::Ptr & type_c_wchar_t() const { return c_wchar_t_; }
    const Type::Ptr & type_c_schar() const { return c_schar_; }
    const Type::Ptr & type_c_uchar() const { return c_uchar_; }
    const Type::Ptr & type_c_short() const { return c_short_; }
    const Type::Ptr & type_c_ushort() const { return c_ushort_; }
    const Type::Ptr & type_c_int() const { return c_int_; }
    const Type::Ptr & type_c_uint() const { return c_uint_; }
    const Type::Ptr & type_c_long() const { return c_long_; }
    const Type::Ptr & type_c_ulong() const { return c_ulong_; }
    const Type::Ptr & type_c_longlong() const { return c_longlong_; }
    const Type::Ptr & type_c_ulonglong() const { return c_ulonglong_; }
    const Type::Ptr & type_c_size_t() const { return c_size_t_; }
    const Type::Ptr & type_c_ssize_t() const { return c_ssize_t_; }
    const PrimType::Ptr & type_c_float() const { return c_float_; }
    const PrimType::Ptr & type_c_double() const { return c_double_; }
    const PrimType::Ptr & type_c_longdouble() const { return c_longdouble_; }
    const PrimType::Ptr & type_c_bool() const { return c_bool_; }
    const PrimType::Ptr & type_c_nullptr() const { return c_nullptr_; }

    PtrType::Ptr type_c_ptr(const Type::Ptr &elementType);

    RefType::Ptr type_c_ref(const Type::Ptr &elementType);

    const Type::Ptr & type_c_char_ptr() { return c_char_ptr_; }

    const PtrType::Ptr & type_c_raw_char_ptr() { return c_raw_char_ptr_; }

    const PtrType::Ptr & type_c_void_ptr() { return c_void_ptr_; }

    const Type::Ptr & type_c_string_ptr() { return c_string_ptr_; }

    StructType::Ptr getEncryptedAnnotation() const { return encryptedAnnotation_; }

    template <typename T>
    inline const PrimType::Ptr & type() { KIARA_UNREACHABLE("Not a primitive type"); }

    template <typename T>
    inline Type::Ptr c_type() { KIARA_UNREACHABLE("Not a primitive C type"); }

    template <typename T>
    inline const PrimType::Ptr & c_ntype() { KIARA_UNREACHABLE("Not a primitive C type"); }

    template <class T>
    typename T::Ptr getOrCreate(const typename T::Ptr &val)
    {
        return dyn_cast<T>(findObject(val));
    }

    void dump();

protected:

    Object::Ptr findObject(const Object::Ptr &val);

private:
    std::map<std::string, StructType::Ptr> uniqueStructTypes_;
    ObjectSet objects_;
    Namespace::Ptr namespace_;

    TypeType::Ptr type_;
    VoidType::Ptr void_;
    UnresolvedSymbolType::Ptr unresolved_symbol_;
    AnyType::Ptr any_;
    PrimType::Ptr i8_;
    PrimType::Ptr u8_;
    PrimType::Ptr i16_;
    PrimType::Ptr u16_;
    PrimType::Ptr i32_;
    PrimType::Ptr u32_;
    PrimType::Ptr i64_;
    PrimType::Ptr u64_;
    PrimType::Ptr float_;
    PrimType::Ptr double_;
    PrimType::Ptr boolean_;
    PrimType::Ptr string_;

    PrimType::Ptr c_int8_t_;
    PrimType::Ptr c_uint8_t_;
    PrimType::Ptr c_int16_t_;
    PrimType::Ptr c_uint16_t_;
    PrimType::Ptr c_int32_t_;
    PrimType::Ptr c_uint32_t_;
    PrimType::Ptr c_int64_t_;
    PrimType::Ptr c_uint64_t_;
    PrimType::Ptr c_float_;
    PrimType::Ptr c_double_;
    PrimType::Ptr c_longdouble_;
    PrimType::Ptr c_bool_;
    PrimType::Ptr c_nullptr_;

    Type::Ptr c_char_;
    Type::Ptr c_wchar_t_;
    Type::Ptr c_schar_;
    Type::Ptr c_uchar_;
    Type::Ptr c_short_;
    Type::Ptr c_ushort_;
    Type::Ptr c_int_;
    Type::Ptr c_uint_;
    Type::Ptr c_long_;
    Type::Ptr c_ulong_;
    Type::Ptr c_longlong_;
    Type::Ptr c_ulonglong_;
    Type::Ptr c_size_t_;
    Type::Ptr c_ssize_t_;

    Type::Ptr c_string_ptr_;

    PtrType::Ptr c_void_ptr_;
    PtrType::Ptr c_raw_char_ptr_;
    Type::Ptr c_char_ptr_;

    StructType::Ptr encryptedAnnotation_;
};

template <> inline const PrimType::Ptr & World::type<int8_t>() { return i8_; }
template <> inline const PrimType::Ptr & World::type<uint8_t>() { return u8_; }
template <> inline const PrimType::Ptr & World::type<int16_t>() { return i16_; }
template <> inline const PrimType::Ptr & World::type<uint16_t>() { return u16_; }
template <> inline const PrimType::Ptr & World::type<int32_t>() { return i32_; }
template <> inline const PrimType::Ptr & World::type<uint32_t>() { return u32_; }
template <> inline const PrimType::Ptr & World::type<int64_t>() { return i64_; }
template <> inline const PrimType::Ptr & World::type<uint64_t>() { return u64_; }
template <> inline const PrimType::Ptr & World::type<float>() { return float_; }
template <> inline const PrimType::Ptr & World::type<double>() { return double_; }
template <> inline const PrimType::Ptr & World::type<bool>() { return boolean_; }
template <> inline const PrimType::Ptr & World::type<const char *>() { return string_; }
template <> inline const PrimType::Ptr & World::type<char *>() { return string_; }
template <> inline const PrimType::Ptr & World::type<std::string>() { return string_; }
template <> inline const PrimType::Ptr & World::type<const std::string>() { return string_; }

template <> inline const PrimType::Ptr & World::c_ntype<int8_t>() { return c_int8_t_; }
template <> inline const PrimType::Ptr & World::c_ntype<uint8_t>() { return c_uint8_t_; }
template <> inline const PrimType::Ptr & World::c_ntype<int16_t>() { return c_int16_t_; }
template <> inline const PrimType::Ptr & World::c_ntype<uint16_t>() { return c_uint16_t_; }
template <> inline const PrimType::Ptr & World::c_ntype<int32_t>() { return c_int32_t_; }
template <> inline const PrimType::Ptr & World::c_ntype<uint32_t>() { return c_uint32_t_; }
template <> inline const PrimType::Ptr & World::c_ntype<int64_t>() { return c_int64_t_; }
template <> inline const PrimType::Ptr & World::c_ntype<uint64_t>() { return c_uint64_t_; }

template <> inline Type::Ptr World::c_type<char>() { return c_char_; }
//template <> inline const Type::Ptr World::c_type<wchar>() { return c_wchar_; }
template <> inline Type::Ptr World::c_type<signed char>() { return c_schar_; }
template <> inline Type::Ptr World::c_type<unsigned char>() { return c_uchar_; }
template <> inline Type::Ptr World::c_type<short>() { return c_short_; }
template <> inline Type::Ptr World::c_type<unsigned short>() { return c_ushort_; }
template <> inline Type::Ptr World::c_type<int>() { return c_int_; }
template <> inline Type::Ptr World::c_type<unsigned int>() { return c_uint_; }
template <> inline Type::Ptr World::c_type<long>() { return c_long_; }
template <> inline Type::Ptr World::c_type<unsigned long>() { return c_ulong_; }
template <> inline Type::Ptr World::c_type<long long>() { return c_longlong_; }
template <> inline Type::Ptr World::c_type<unsigned long long>() { return c_ulonglong_; }
template <> inline Type::Ptr World::c_type<float>() { return c_float_; }
template <> inline Type::Ptr World::c_type<double>() { return c_double_; }
template <> inline Type::Ptr World::c_type<long double>() { return c_longdouble_; }
template <> inline Type::Ptr World::c_type<bool>() { return c_bool_; }
template <> inline Type::Ptr World::c_type<void *>() { return c_void_ptr_; }
template <> inline Type::Ptr World::c_type<char *>() { return c_char_ptr_; }
template <> inline Type::Ptr World::c_type<const void *>() { return c_void_ptr_; }
template <> inline Type::Ptr World::c_type<const char *>() { return c_char_ptr_; }

} // namespace KIARA

#endif /* KIARA_IDL_WORLD_HPP_INCLUDED */
