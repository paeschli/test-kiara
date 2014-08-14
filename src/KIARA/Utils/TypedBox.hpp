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
 * TypedBox.hpp
 *
 *  Created on: 06.03.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_UTILS_TYPEDBOX_HPP_INCLUDED
#define KIARA_UTILS_TYPEDBOX_HPP_INCLUDED

#include <KIARA/Common/Types.hpp>
#include <KIARA/Utils/Box.hpp>
#include <boost/functional/hash.hpp>
#include <cstdlib>
#include <cstring>

namespace KIARA
{

enum BoxType
{
    BT_EMPTY,
    BT_INT8_T,
    BT_UINT8_T,
    BT_INT16_T,
    BT_UINT16_T,
    BT_INT32_T,
    BT_UINT32_T,
    BT_INT64_T,
    BT_UINT64_T,
    BT_FLOAT,
    BT_DOUBLE,
    BT_BOOL,
    BT_STRING,
    BT_VOIDPTR,
    BT_LAST = BT_VOIDPTR
};

#ifdef _MSC_VER
#ifndef strdup
#define strdup(x) _strdup(x)
#endif
#endif

struct TypedBox
{
public:

    TypedBox() : value_(), type_(BT_EMPTY) {  }
    TypedBox(int8_t  v) : value_(v), type_(BT_INT8_T) { }
    TypedBox(uint8_t  v) : value_(v), type_(BT_UINT8_T) { }
    TypedBox(int16_t  v) : value_(v), type_(BT_INT16_T) { }
    TypedBox(uint16_t  v) : value_(v), type_(BT_UINT16_T) { }
    TypedBox(int32_t  v) : value_(v), type_(BT_INT32_T) { }
    TypedBox(uint32_t  v) : value_(v), type_(BT_UINT32_T) { }
    TypedBox(int64_t  v) : value_(v), type_(BT_INT64_T) { }
    TypedBox(uint64_t  v) : value_(v), type_(BT_UINT64_T) { }
    TypedBox(float   v) : value_(v), type_(BT_FLOAT) { }
    TypedBox(double  v) : value_(v), type_(BT_DOUBLE) { }

    TypedBox(bool v) : value_(v), type_(BT_BOOL) { }
    TypedBox(void* v) : value_(v), type_(BT_VOIDPTR) { }
    TypedBox(const_char_ptr v) : value_(strdup(v)), type_(BT_STRING) { }
    TypedBox(const std::string &v) : value_(strdup(v.c_str())), type_(BT_STRING) { }

    TypedBox(const TypedBox &v)
        : value_(v.type_ == BT_STRING ? strdup(v.value_.get_string()) : v.value_)
        ,  type_(v.type_)
    {
    }

    ~TypedBox()
    {
        clear();
    }

    void clear()
    {
        if (type_ == BT_STRING)
            free(const_cast<char*>(value_.get_string()));
        type_ = BT_EMPTY;
    }

    bool operator==(const TypedBox& other) const
    {
        if (other.type_ != type_)
            return false;
        if (type_ == BT_STRING)
        {
            const_char_ptr a = other.value_.get_string();
            const_char_ptr b = value_.get_string();
            if (a == b)
                return true;
            if (a == 0 || b == 0)
                return false;
            return !strcmp(a, b);
        }
        return other.value_ == value_;
    }

    bool operator!=(const TypedBox& other) const
    {
        return !(*this == other);
    }

    template <typename T>
    T get() const { return value_.get<T>(); }

    int8_t   get_i8() const { return value_.get_i8(); }
    uint8_t  get_u8() const { return value_.get_u8(); }
    int16_t  get_i16() const { return value_.get_i16(); }
    uint16_t get_u16() const { return value_.get_u16(); }
    int32_t  get_i32() const { return value_.get_i32(); }
    uint32_t get_u32() const { return value_.get_u32(); }
    int64_t  get_i64() const { return value_.get_i64(); }
    uint64_t get_u64() const { return value_.get_u64(); }
    float  get_float() const { return value_.get_float(); }
    double get_double() const { return value_.get_double(); }

    bool get_bool() const { return value_.get_bool(); }
    void * get_ptr() const { return value_.get_ptr(); }
    const_char_ptr get_string() const { return value_.get_string(); }

    BoxType getType() const { return type_; }
    const Box &getValue() const { return value_; }

    size_t hash() const
    {
        size_t seed = 0;
        boost::hash_combine(seed, type_);
        switch (type_)
        {
            case BT_EMPTY:
                break;
            case BT_INT8_T:
                boost::hash_combine(seed, value_.i8);
                break;
            case BT_UINT8_T:
                boost::hash_combine(seed, value_.u8);
                break;
            case BT_INT16_T:
                boost::hash_combine(seed, value_.i16);
                break;
            case BT_UINT16_T:
                boost::hash_combine(seed, value_.u16);
                break;
            case BT_INT32_T:
                boost::hash_combine(seed, value_.i32);
                break;
            case BT_UINT32_T:
                boost::hash_combine(seed, value_.u32);
                break;
            case BT_INT64_T:
                boost::hash_combine(seed, value_.i64);
                break;
            case BT_UINT64_T:
                boost::hash_combine(seed, value_.u64);
                break;
            case BT_FLOAT:
                boost::hash_combine(seed, value_.f);
                break;
            case BT_DOUBLE:
                boost::hash_combine(seed, value_.d);
                break;
            case BT_BOOL:
                boost::hash_combine(seed, value_.b);
                break;
            case BT_STRING:
                seed ^= (value_.s ? boost::hash_range(value_.s, value_.s+strlen(value_.s)) : 0) +
                    0x9e3779b9 + (seed << 6) + (seed >> 2);
                break;
            default: // BT_VOIDPTR and rest
                boost::hash_combine(seed, value_.p);
                break;
        }
        return seed;
    }

private:
    Box value_;
    BoxType type_;
};

inline std::size_t hash_value(const TypedBox & b)
{
    return b.hash();
}

} // namespace KIARA

#endif /* KIARA_UTILS_TYPEDBOX_HPP_INCLUDED */
