/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2012, 2013  German Research Center for Artificial Intelligence (DFKI)
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
 * Box.hpp
 *
 *  Created on: 06.08.2012
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_UTILS_BOX_HPP_INCLUDED
#define KIARA_UTILS_BOX_HPP_INCLUDED

#include <KIARA/Common/Types.hpp>
#include <KIARA/Utils/Error.hpp>
#include <cstring>
#include <string>

namespace KIARA
{

typedef const char* const_char_ptr;

union Box
{
    int8_t     i8;
    uint8_t    u8;
    int16_t   i16;
    uint16_t  u16;
    int32_t   i32;
    uint32_t  u32;
    int64_t   i64;
    uint64_t  u64;
    float     f;
    double    d;

    bool      b;
    void*     p;
    const_char_ptr s;

    Box()      { reset(); }
    Box(int8_t  v) { reset(); i8 = v; }
    Box(uint8_t  v) { reset(); u8 = v; }
    Box(int16_t  v) { reset(); i16 = v; }
    Box(uint16_t  v) { reset(); u16 = v; }
    Box(int32_t  v) { reset(); i32 = v; }
    Box(uint32_t  v) { reset(); u32 = v; }
    Box(int64_t  v) { reset(); i64 = v; }
    Box(uint64_t  v) { reset(); u64 = v; }
    Box(float   v) { reset(); f = v; }
    Box(double  v) { reset(); d = v; }

    Box(bool v)  { reset(); b = v; }
    Box(void* v) { reset(); p = v; }
    Box(const_char_ptr v) { reset(); s = v; }

    bool operator == (const Box& other) const
    {
        return memcmp(this, &other, sizeof(Box)) == 0;
    }

    template <typename T>
    inline T get() const { KIARA_UNREACHABLE("Unknown type"); }

    template <typename T>
    inline void set(T value) { KIARA_UNREACHABLE("Unknown type"); }

    int8_t   get_i8() const { return i8; }
    uint8_t  get_u8() const { return u8; }
    int16_t  get_i16() const { return i16; }
    uint16_t get_u16() const { return u16; }
    int32_t  get_i32() const { return i32; }
    uint32_t get_u32() const { return u32; }
    int64_t  get_i64() const { return i64; }
    uint64_t get_u64() const { return u64; }
    float  get_float() const { return f; }
    double get_double() const { return d; }

    bool get_bool() const { return b; }
    void * get_ptr() const { return p; }
    const_char_ptr get_string() const { return s; }

    void set_i8(int8_t v) { i8 = v; }
    void set_u8(uint8_t v) { u8 = v; }
    void set_i16(int16_t v) { i16 = v; }
    void set_u16(uint16_t v) { u16 = v; }
    void set_i32(int32_t v) { i32 = v; }
    void set_u32(uint32_t v) { u32 = v; }
    void set_i64(int64_t v) { i64 = v; }
    void set_u64(uint64_t v) { u64 = v; }
    void set_float(float v) { f = v; }
    void set_double(double v) { d = v; }

    void set_bool(bool v) { b = v; }
    void set_ptr(void *v) { p = v; }
    void set_string(const_char_ptr v) { s = v; }

private:
    void reset() { memset(this, 0, sizeof(Box)); }
};

template <> inline int8_t Box::get<int8_t>() const { return i8; }
template <> inline uint8_t Box::get<uint8_t>() const { return u8; }
template <> inline int16_t Box::get<int16_t>() const { return i16; }
template <> inline uint16_t Box::get<uint16_t>() const { return u16; }
template <> inline int32_t Box::get<int32_t>() const { return i32; }
template <> inline uint32_t Box::get<uint32_t>() const { return u32; }
template <> inline int64_t Box::get<int64_t>() const { return i64; }
template <> inline uint64_t Box::get<uint64_t>() const { return u64; }
template <> inline float Box::get<float>() const { return f; }
template <> inline double Box::get<double>() const { return d; }
template <> inline bool Box::get<bool>() const { return b; }
template <> inline void * Box::get<void*>() const { return p; }
template <> inline const_char_ptr Box::get<const_char_ptr>() const { return s; }
template <> inline std::string Box::get<std::string>() const
{
    return s ? s : "";
}

template <> inline void Box::set<int8_t>(int8_t v) { i8 = v; }
template <> inline void Box::set<uint8_t>(uint8_t v) { u8 = v; }
template <> inline void Box::set<int16_t>(int16_t v) { i16 = v; }
template <> inline void Box::set<uint16_t >(uint16_t v) { u16 = v; }
template <> inline void Box::set<int32_t>(int32_t v) { i32 = v; }
template <> inline void Box::set<uint32_t >(uint32_t v) { u32 = v; }
template <> inline void Box::set<int64_t>(int64_t v) { i64 = v; }
template <> inline void Box::set<uint64_t>(uint64_t v) { u64 = v; }
template <> inline void Box::set<float>(float v) { f = v; }
template <> inline void Box::set<double>(double v) { d = v; }
template <> inline void Box::set<bool>(bool v) { b = v; }
template <> inline void Box::set<void*>(void *v) { p = v; }
template <> inline void Box::set<const_char_ptr>(const_char_ptr v) { s = v; }

} // namespace KIARA

#endif /* KIARA_BOX_HPP_INCLUDED */
