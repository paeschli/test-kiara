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
 * kiara_dds_cxx_macros.hpp
 *
 *  Created on: 16.01.2013
 *      Author: Dmitri Rubinstein
 */
#ifndef KIARA_DDS_CXX_MACROS_HPP_INCLUDED
#define KIARA_DDS_CXX_MACROS_HPP_INCLUDED

#include <KIARA/kiara_cxx_macros.hpp>

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

#if RTI_CDR_SIZEOF_LONG_DOUBLE != 16

KIARA_CXX_DECL_STRUCT(DDS_LongDouble,
        KIARA_CXX_STRUCT_MEMBER(bytes))

#endif

#define KIARA_CXX_DECL_DDS_SEQUENCE(type_name)              \
    KIARA_CXX_DECL_STRUCT(type_name,                        \
      KIARA_CXX_STRUCT_MEMBER(_owned)                       \
      KIARA_CXX_STRUCT_MEMBER(_contiguous_buffer)           \
      KIARA_CXX_STRUCT_MEMBER(_discontiguous_buffer)        \
      KIARA_CXX_STRUCT_MEMBER(_maximum)                     \
      KIARA_CXX_STRUCT_MEMBER(_length)                      \
      KIARA_CXX_STRUCT_MEMBER(_sequence_init)               \
      KIARA_CXX_STRUCT_MEMBER(_read_token1)                 \
      KIARA_CXX_STRUCT_MEMBER(_read_token2)                 \
      KIARA_CXX_STRUCT_MEMBER(_elementPointersAllocation)   \
    )


KIARA_CXX_DECL_DDS_SEQUENCE(DDS_CharSeq)
KIARA_CXX_DECL_DDS_SEQUENCE(DDS_WcharSeq)
KIARA_CXX_DECL_DDS_SEQUENCE(DDS_OctetSeq)
KIARA_CXX_DECL_DDS_SEQUENCE(DDS_ShortSeq)
KIARA_CXX_DECL_DDS_SEQUENCE(DDS_UnsignedShortSeq)
KIARA_CXX_DECL_DDS_SEQUENCE(DDS_LongSeq)
KIARA_CXX_DECL_DDS_SEQUENCE(DDS_UnsignedLongSeq)
KIARA_CXX_DECL_DDS_SEQUENCE(DDS_LongLongSeq)
KIARA_CXX_DECL_DDS_SEQUENCE(DDS_UnsignedLongLongSeq)
KIARA_CXX_DECL_DDS_SEQUENCE(DDS_FloatSeq)
KIARA_CXX_DECL_DDS_SEQUENCE(DDS_DoubleSeq)
KIARA_CXX_DECL_DDS_SEQUENCE(DDS_LongDoubleSeq)
KIARA_CXX_DECL_DDS_SEQUENCE(DDS_BooleanSeq)
KIARA_CXX_DECL_DDS_SEQUENCE(DDS_StringSeq)
KIARA_CXX_DECL_DDS_SEQUENCE(DDS_WstringSeq)

#endif /* KIARA_DDS_CXX_MACROS_HPP_INCLUDED */
