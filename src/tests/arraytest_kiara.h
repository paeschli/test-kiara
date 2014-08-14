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
 * arraytest_kiara.h
 *
 *  Created on: Nov 29, 2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_ARRAYTEST_KIARA_H_INCLUDED
#define KIARA_ARRAYTEST_KIARA_H_INCLUDED

#include "arraytest_types.h"
#include <KIARA/kiara_macros.h>

KIARA_DECL_PTR(IntPtr, KIARA_INT)

KIARA_DECL_PTR(Int8Ptr, KIARA_INT8_T)
KIARA_DECL_PTR(UInt8Ptr, KIARA_UINT8_T)

KIARA_DECL_PTR(Int16Ptr, KIARA_INT16_T)
KIARA_DECL_PTR(UInt16Ptr, KIARA_UINT16_T)

KIARA_DECL_PTR(Int32Ptr, KIARA_INT32_T)
KIARA_DECL_PTR(UInt32Ptr, KIARA_UINT32_T)

KIARA_DECL_PTR(Int64Ptr, KIARA_INT64_T)
KIARA_DECL_PTR(UInt64Ptr, KIARA_UINT64_T)

KIARA_DECL_PTR(FloatPtr, KIARA_FLOAT)
KIARA_DECL_PTR(DoublePtr, KIARA_DOUBLE)

/** Describe Data */

KIARA_DECL_STRUCT_WITH_API(Data,
  KIARA_STRUCT_ARRAY_MEMBER(IntPtr, array_boolean, KIARA_INT, size_boolean)
  KIARA_STRUCT_ARRAY_MEMBER(Int8Ptr, array_i8, KIARA_INT, size_i8)
  KIARA_STRUCT_ARRAY_MEMBER(UInt8Ptr, array_u8, KIARA_INT, size_u8)
  KIARA_STRUCT_ARRAY_MEMBER(Int16Ptr, array_i16, KIARA_INT, size_i16)
  KIARA_STRUCT_ARRAY_MEMBER(UInt16Ptr, array_u16, KIARA_INT, size_u16)
  KIARA_STRUCT_ARRAY_MEMBER(Int32Ptr, array_i32, KIARA_INT, size_i32)
  KIARA_STRUCT_ARRAY_MEMBER(UInt32Ptr, array_u32, KIARA_INT, size_u32)
  KIARA_STRUCT_ARRAY_MEMBER(Int64Ptr, array_i64, KIARA_INT, size_i64)
  KIARA_STRUCT_ARRAY_MEMBER(UInt64Ptr, array_u64, KIARA_INT, size_u64)
  KIARA_STRUCT_ARRAY_MEMBER(FloatPtr, array_float, KIARA_INT, size_float)
  KIARA_STRUCT_ARRAY_MEMBER(DoublePtr, array_double, KIARA_INT, size_double)
  ,
  KIARA_USER_API(AllocateType, Data_Allocate)
  KIARA_USER_API(DeallocateType, Data_Deallocate))

KIARA_DECL_PTR(DataPtr, Data)
KIARA_DECL_CONST_PTR(ConstDataPtr, Data)

KIARA_DECL_FUNC(SendData,
  KIARA_FUNC_RESULT(DataPtr, result)
  KIARA_FUNC_ARG(ConstDataPtr, a))

KIARA_DECL_SERVICE(OnSendData,
    KIARA_SERVICE_RESULT(DataPtr, result)
    KIARA_SERVICE_ARG(DataPtr, a))

#endif /* KIARA_ARRAYTEST_KIARA_H_INCLUDED */
