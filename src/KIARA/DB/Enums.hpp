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
 * Enums.hpp
 *
 *  Created on: 02.08.2012
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_DB_ENUMS_HPP_INCLUDED
#define KIARA_DB_ENUMS_HPP_INCLUDED

namespace KIARA
{

enum NodeKind
{
    FIRST_NODE_KIND = 0,
    FIRST_PRIMTYPE_NODE = FIRST_NODE_KIND,
    NODE_PRIMTYPE_i8 = FIRST_PRIMTYPE_NODE,
    NODE_PRIMTYPE_u8,
    NODE_PRIMTYPE_i16,
    NODE_PRIMTYPE_u16,
    NODE_PRIMTYPE_i32,
    NODE_PRIMTYPE_u32,
    NODE_PRIMTYPE_i64,
    NODE_PRIMTYPE_u64,
    NODE_PRIMTYPE_float,
    NODE_PRIMTYPE_double,
    NODE_PRIMTYPE_boolean,
    NODE_PRIMTYPE_string,
    NODE_PRIMTYPE_c_int8_t,
    NODE_PRIMTYPE_c_uint8_t,
    NODE_PRIMTYPE_c_int16_t,
    NODE_PRIMTYPE_c_uint16_t,
    NODE_PRIMTYPE_c_int32_t,
    NODE_PRIMTYPE_c_uint32_t,
    NODE_PRIMTYPE_c_int64_t,
    NODE_PRIMTYPE_c_uint64_t,
    NODE_PRIMTYPE_c_float,
    NODE_PRIMTYPE_c_double,
    NODE_PRIMTYPE_c_longdouble,
    NODE_PRIMTYPE_c_bool,
    NODE_PRIMTYPE_c_nullptr,
    LAST_PRIMTYPE_NODE = NODE_PRIMTYPE_c_nullptr,
    FIRST_C_PRIMTYPE_NODE = NODE_PRIMTYPE_c_int8_t,
    NUM_PRIMTYPES = LAST_PRIMTYPE_NODE - FIRST_PRIMTYPE_NODE,
    NODE_VOIDTYPE = LAST_PRIMTYPE_NODE + 1,
    NODE_PRIMVALUETYPE,
    NODE_UNRESOLVEDSYMBOLTYPE,
    NODE_SYMBOLTYPE,
    NODE_ANYTYPE,
    NODE_TYPETYPE,
    NODE_TYPEDEFTYPE,
    NODE_ENUMTYPE,
    NODE_PTRTYPE,
    NODE_REFTYPE,
    NODE_ARRAYTYPE,
    NODE_FIXEDARRAYTYPE,
    NODE_STRUCTTYPE,
    NODE_FUNCTYPE,
    NODE_SERVICETYPE,
    LAST_NODE_KIND = NODE_SERVICETYPE
};

enum PrimTypeKind
{
    PRIMTYPE_i8 = NODE_PRIMTYPE_i8,
    PRIMTYPE_u8 = NODE_PRIMTYPE_u8,
    PRIMTYPE_i16 = NODE_PRIMTYPE_i16,
    PRIMTYPE_u16 = NODE_PRIMTYPE_u16,
    PRIMTYPE_i32 = NODE_PRIMTYPE_i32,
    PRIMTYPE_u32 = NODE_PRIMTYPE_u32,
    PRIMTYPE_i64 = NODE_PRIMTYPE_i64,
    PRIMTYPE_u64 = NODE_PRIMTYPE_u64,
    PRIMTYPE_float = NODE_PRIMTYPE_float,
    PRIMTYPE_double = NODE_PRIMTYPE_double,
    PRIMTYPE_boolean = NODE_PRIMTYPE_boolean,
    PRIMTYPE_string = NODE_PRIMTYPE_string,
    PRIMTYPE_c_int8_t   = NODE_PRIMTYPE_c_int8_t,
    PRIMTYPE_c_uint8_t  = NODE_PRIMTYPE_c_uint8_t,
    PRIMTYPE_c_int16_t  = NODE_PRIMTYPE_c_int16_t,
    PRIMTYPE_c_uint16_t = NODE_PRIMTYPE_c_uint16_t,
    PRIMTYPE_c_int32_t  = NODE_PRIMTYPE_c_int32_t,
    PRIMTYPE_c_uint32_t = NODE_PRIMTYPE_c_uint32_t,
    PRIMTYPE_c_int64_t  = NODE_PRIMTYPE_c_int64_t,
    PRIMTYPE_c_uint64_t = NODE_PRIMTYPE_c_uint64_t,
    PRIMTYPE_c_float    = NODE_PRIMTYPE_c_float,
    PRIMTYPE_c_double   = NODE_PRIMTYPE_c_double,
    PRIMTYPE_c_longdouble = NODE_PRIMTYPE_c_longdouble,
    PRIMTYPE_c_bool = NODE_PRIMTYPE_c_bool,
    PRIMTYPE_c_nullptr = NODE_PRIMTYPE_c_nullptr,
    FIRST_C_PRIMTYPE = FIRST_C_PRIMTYPE_NODE
};

} // namespace KIARA

#endif /* KIARA_ENUMS_HPP_INCLUDED */
