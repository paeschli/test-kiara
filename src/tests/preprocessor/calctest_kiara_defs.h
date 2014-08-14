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
#ifndef calctest_kiara_defs_h_included
#define calctest_kiara_defs_h_included

/* This file should be automatically generated from calctest_defs.h */

#include <KIARA/kiara.h>
#include <KIARA/kiara_macros.h>

KIARA_DECL_PTR(IntPtr, KIARA_INT)
KIARA_DECL_PTR(FloatPtr, KIARA_FLOAT)
KIARA_DECL_PTR(CharPtrPtr, KIARA_CHAR_PTR)

KIARA_DECL_OPAQUE_TYPE(kr_dstring_t,
       KIARA_USER_API(SetCString, dstring_SetCString)
       KIARA_USER_API(GetCString, dstring_GetCString))
KIARA_DECL_PTR(DStringPtr, kr_dstring_t)
KIARA_DECL_CONST_PTR(ConstDStringPtr, kr_dstring_t)

KIARA_DECL_FUNC(Calc_Add,
  KIARA_FUNC_RESULT(IntPtr, result)
  KIARA_FUNC_ARG(KIARA_INT, a)
  KIARA_FUNC_ARG(KIARA_INT, b)
)

KIARA_DECL_FUNC(Calc_Add_Float,
  KIARA_FUNC_RESULT(FloatPtr, result)
  KIARA_FUNC_ARG(KIARA_FLOAT, a)
  KIARA_FUNC_ARG(KIARA_FLOAT, b)
)

KIARA_DECL_FUNC(Calc_String_To_Int32,
  KIARA_FUNC_RESULT(IntPtr, result)
  KIARA_FUNC_ARG(KIARA_const_char_ptr, s)
)

KIARA_DECL_FUNC(Calc_Int32_To_String,
  KIARA_FUNC_RESULT(CharPtrPtr, result)
  KIARA_FUNC_ARG(KIARA_INT, i)
)

KIARA_DECL_FUNC(Calc_DString_To_Int32,
  KIARA_FUNC_RESULT(IntPtr, result)
  KIARA_FUNC_ARG(ConstDStringPtr, s)
)

KIARA_DECL_FUNC(Calc_Int32_To_DString,
  KIARA_FUNC_RESULT(DStringPtr, result)
  KIARA_FUNC_ARG(KIARA_INT, i)
)

KIARA_DECL_STRUCT(Vec3f,
  KIARA_STRUCT_MEMBER(KIARA_FLOAT, x)
  KIARA_STRUCT_MEMBER(KIARA_FLOAT, y)
  KIARA_STRUCT_MEMBER(KIARA_FLOAT, z)
)

KIARA_DECL_STRUCT(Quatf,
  KIARA_STRUCT_MEMBER(KIARA_FLOAT, r)
  KIARA_STRUCT_MEMBER(Vec3f, v)
)

KIARA_DECL_STRUCT(Location,
  KIARA_STRUCT_MEMBER(Vec3f, position)
  KIARA_STRUCT_MEMBER(Quatf, rotation)
)

#endif
