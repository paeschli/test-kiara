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
 * kiara_dds_macros.h
 *
 *  Created on: 15.01.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_DDS_MACROS_H_INCLUDED
#define KIARA_DDS_MACROS_H_INCLUDED

#include <KIARA/kiara_macros.h>

/* Workaround for missing RTI_WIN32 define in ndds_cpp.h */
#if defined(_WIN32) && !defined(RTI_WIN32)
#define RTI_WIN32
#endif

#ifndef NDDS_STANDALONE_TYPE
    #ifdef __cplusplus
        #ifndef ndds_cpp_h
            #include "ndds/ndds_cpp.h"
        #endif
    #else
        #ifndef ndds_c_h
            #include "ndds/ndds_c.h"
        #endif
    #endif
#else
    #include "ndds_standalone_type.h"
#endif

/* Primitive Types (based on ndds/cdr/cdr_type.h) */
#define KIARA_CDR_CHAR KIARA_CHAR
#define KIARA_CDR_WCHAR KIARA_UINT32_T
#define KIARA_CDR_OCTET KIARA_UINT8_T
#define KIARA_CDR_SHORT KIARA_INT16_T
#define KIARA_CDR_UNSIGNEDSHORT KIARA_UINT16_T
#define KIARA_CDR_LONG KIARA_INT32_T
#define KIARA_CDR_UNSIGNEDLONG KIARA_UINT32_T
#define KIARA_CDR_LONGLONG KIARA_INT64_T
#define KIARA_CDR_UNSIGNEDLONGLONG KIARA_UINT64_T
#define KIARA_CDR_FLOAT KIARA_FLOAT
#define KIARA_CDR_DOUBLE KIARA_DOUBLE

#if RTI_CDR_SIZEOF_LONG_DOUBLE == 16
#define KIARA_CDR_LONGDOUBLE KIARA_LONGDOUBLE
#else

#define KIARA_CDR_LONGDOUBLE RTICdrLongDouble

KIARA_DECL_FIXED_ARRAY(KIARA_char_array_16, KIARA_CHAR, 16)
KIARA_DECL_STRUCT(KIARA_CDR_LONGDOUBLE,
        KIARA_STRUCT_MEMBER(KIARA_char_array_16, bytes))

#endif



#define KIARA_CDR_BOOLEAN KIARA_UINT8_T
#define KIARA_CDR_ENUM KIARA_UINT32_T

#define KIARA_DDS_CHAR KIARA_CDR_CHAR
#define KIARA_DDS_WCHAR KIARA_CDR_WCHAR
#define KIARA_DDS_OCTET KIARA_CDR_OCTET
#define KIARA_DDS_SHORT KIARA_CDR_SHORT
#define KIARA_DDS_UNSIGNEDSHORT KIARA_CDR_UNSIGNEDSHORT
#define KIARA_DDS_LONG KIARA_CDR_LONG
#define KIARA_DDS_UNSIGNEDLONG KIARA_CDR_UNSIGNEDLONG
#define KIARA_DDS_LONGLONG KIARA_CDR_LONGLONG
#define KIARA_DDS_UNSIGNEDLONGLONG KIARA_CDR_UNSIGNEDLONGLONG
#define KIARA_DDS_FLOAT KIARA_CDR_FLOAT
#define KIARA_DDS_DOUBLE KIARA_CDR_DOUBLE
#define KIARA_DDS_LONGDOUBLE KIARA_CDR_LONGDOUBLE
#define KIARA_DDS_BOOLEAN KIARA_CDR_BOOLEAN
#define KIARA_DDS_ENUM KIARA_CDR_ENUM

KIARA_DECL_DERIVED_BUILTIN(DDS_Char, KIARA_DDS_CHAR)
KIARA_DECL_DERIVED_BUILTIN(DDS_Wchar, KIARA_DDS_WCHAR)
KIARA_DECL_DERIVED_BUILTIN(DDS_Octet, KIARA_DDS_OCTET)
KIARA_DECL_DERIVED_BUILTIN(DDS_Short, KIARA_DDS_SHORT)
KIARA_DECL_DERIVED_BUILTIN(DDS_UnsignedShort, KIARA_DDS_UNSIGNEDSHORT)
KIARA_DECL_DERIVED_BUILTIN(DDS_Long, KIARA_DDS_LONG)
KIARA_DECL_DERIVED_BUILTIN(DDS_UnsignedLong, KIARA_DDS_UNSIGNEDLONG)
KIARA_DECL_DERIVED_BUILTIN(DDS_LongLong, KIARA_DDS_LONGLONG)
KIARA_DECL_DERIVED_BUILTIN(DDS_UnsignedLongLong, KIARA_DDS_UNSIGNEDLONGLONG)
KIARA_DECL_DERIVED_BUILTIN(DDS_Float, KIARA_DDS_FLOAT)
KIARA_DECL_DERIVED_BUILTIN(DDS_Double, KIARA_DDS_DOUBLE)
KIARA_DECL_DERIVED_BUILTIN(DDS_LongDouble, KIARA_DDS_LONGDOUBLE)
KIARA_DECL_DERIVED_BUILTIN(DDS_Boolean, KIARA_DDS_BOOLEAN)
KIARA_DECL_DERIVED_BUILTIN(DDS_Enum, KIARA_DDS_ENUM)


#define _KIARA_PTRNAME(type_name) KIARA_JOIN(KIARA_, KIARA_JOIN(type_name, _ptr))
#define _KIARA_PTRPTRNAME(type_name) KIARA_JOIN(KIARA_, KIARA_JOIN(type_name, _ptr_ptr))

#define KIARA_DECL_DDS_SEQUENCE2(type_name, member_ptr_type, member_ptr_ptr_type)   \
    KIARA_DECL_STRUCT(type_name,                                                    \
      KIARA_STRUCT_MEMBER(KIARA_DDS_BOOLEAN, _owned)                                \
      KIARA_STRUCT_MEMBER(member_ptr_type, _contiguous_buffer)                      \
      KIARA_STRUCT_MEMBER(member_ptr_ptr_type, _discontiguous_buffer)               \
      KIARA_STRUCT_MEMBER(DDS_UnsignedLong, _maximum)                               \
      KIARA_STRUCT_MEMBER(DDS_UnsignedLong, _length)                                \
      KIARA_STRUCT_MEMBER(DDS_Long, _sequence_init)                                 \
      KIARA_STRUCT_MEMBER(KIARA_VOID_PTR, _read_token1)                             \
      KIARA_STRUCT_MEMBER(KIARA_VOID_PTR, _read_token2)                             \
      KIARA_STRUCT_MEMBER(DDS_Boolean, _elementPointersAllocation)                  \
    )

#define KIARA_DECL_DDS_SEQUENCE(type_name, member_type)                             \
    typedef struct type_name type_name;                                             \
    typedef member_type * _KIARA_PTRNAME(member_type);                              \
    KIARA_DECL_PTR(_KIARA_PTRNAME(member_type), member_type)                        \
    typedef member_type ** _KIARA_PTRPTRNAME(member_type);                          \
    KIARA_DECL_PTR(_KIARA_PTRPTRNAME(member_type), _KIARA_PTRNAME(member_type))     \
    KIARA_DECL_DDS_SEQUENCE2(type_name, _KIARA_PTRNAME(member_type),                \
            _KIARA_PTRPTRNAME(member_type))

KIARA_DECL_DDS_SEQUENCE(DDS_CharSeq, DDS_Char)
KIARA_DECL_DDS_SEQUENCE(DDS_WcharSeq, DDS_Wchar)
KIARA_DECL_DDS_SEQUENCE(DDS_OctetSeq, DDS_Octet)
KIARA_DECL_DDS_SEQUENCE(DDS_ShortSeq, DDS_Short)
KIARA_DECL_DDS_SEQUENCE(DDS_UnsignedShortSeq, DDS_UnsignedShort)
KIARA_DECL_DDS_SEQUENCE(DDS_LongSeq, DDS_Long)
KIARA_DECL_DDS_SEQUENCE(DDS_UnsignedLongSeq, DDS_UnsignedLong)
KIARA_DECL_DDS_SEQUENCE(DDS_LongLongSeq, DDS_LongLong)
KIARA_DECL_DDS_SEQUENCE(DDS_UnsignedLongLongSeq, DDS_UnsignedLongLong)
KIARA_DECL_DDS_SEQUENCE(DDS_FloatSeq, DDS_Float)
KIARA_DECL_DDS_SEQUENCE(DDS_DoubleSeq, DDS_Double)
KIARA_DECL_DDS_SEQUENCE(DDS_LongDoubleSeq, DDS_LongDouble)
KIARA_DECL_DDS_SEQUENCE(DDS_BooleanSeq, DDS_Boolean)

/* DDS_StringSeq */

KIARA_DECL_PTR(KIARA_char_ptr_ptr, KIARA_CHAR_PTR)
typedef char** KIARA_char_ptr_ptr;
KIARA_DECL_PTR(KIARA_char_ptr_ptr_ptr, KIARA_char_ptr_ptr)
typedef char*** KIARA_char_ptr_ptr_ptr;

typedef struct DDS_StringSeq DDS_StringSeq;
KIARA_DECL_DDS_SEQUENCE2(DDS_StringSeq, KIARA_CHAR_PTR, KIARA_char_ptr_ptr)

/* DDS_WstringSeq */

KIARA_DECL_PTR(KIARA_DDS_Wchar_ptr_ptr_ptr, KIARA_DDS_Wchar_ptr_ptr)
typedef DDS_Wchar*** KIARA_DDS_Wchar_ptr_ptr_ptr;

typedef struct DDS_WstringSeq DDS_WstringSeq;
KIARA_DECL_DDS_SEQUENCE2(DDS_WstringSeq, KIARA_DDS_Wchar_ptr, KIARA_DDS_Wchar_ptr_ptr)

#endif /* KIARA_DDS_MACROS_H_INCLUDED */
