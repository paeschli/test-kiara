/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2012  German Research Center for Artificial Intelligence (DFKI)
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
 * kiara_defs.hpp
 *
 *  Created on: 11.12.2012
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_DEFS_HPP_INCLUDED
#define KIARA_DEFS_HPP_INCLUDED

#include "user_types.hpp"
#include <KIARA/kiara.h>
#include <KIARA/kiara_cxx_macros.hpp>

KIARA_CXX_DECL_STRUCT(User::gps_position,
    KIARA_CXX_STRUCT_MEMBER(degrees)
    KIARA_CXX_STRUCT_MEMBER(minutes)
    KIARA_CXX_STRUCT_MEMBER(seconds))

KIARA_CXX_DECL_STRUCT(User::gps_direction,
    KIARA_CXX_STRUCT_MEMBER(pos)
    KIARA_CXX_STRUCT_MEMBER(distance))

KIARA_CXX_DECL_STRUCT(User::Quark,
    KIARA_CXX_STRUCT_MEMBER(flavor)
    KIARA_CXX_STRUCT_MEMBER(color)
    KIARA_CXX_STRUCT_MEMBER(charge)
    KIARA_CXX_STRUCT_MEMBER(mass)
    KIARA_CXX_STRUCT_MEMBER(x)
    KIARA_CXX_STRUCT_MEMBER(y)
    KIARA_CXX_STRUCT_MEMBER(z)
    KIARA_CXX_STRUCT_MEMBER(px)
    KIARA_CXX_STRUCT_MEMBER(py)
    KIARA_CXX_STRUCT_MEMBER(pz))

KIARA_CXX_DECL_STRUCT(User::Vec2f,
    KIARA_CXX_STRUCT_MEMBER(x)
    KIARA_CXX_STRUCT_MEMBER(y))

KIARA_CXX_DECL_STRUCT(User::Linef,
    KIARA_CXX_STRUCT_MEMBER(a)
    KIARA_CXX_STRUCT_MEMBER(b))

KIARA_CXX_DECL_STRUCT(User::IntList,
    KIARA_CXX_STRUCT_MEMBER(next)
    KIARA_CXX_STRUCT_MEMBER(data))

KIARA_CXX_DECL_STRUCT(User::Matrix44,
    KIARA_CXX_STRUCT_MEMBER(mat))

KIARA_CXX_DECL_STRUCT(User::Data,
    KIARA_CXX_STRUCT_MEMBER(i)
    KIARA_CXX_STRUCT_MEMBER(f))

KIARA_CXX_DECL_STRUCT(User::MatrixPos,
    KIARA_CXX_STRUCT_MEMBER(row)
    KIARA_CXX_STRUCT_MEMBER(column))

KIARA_CXX_DECL_STRUCT(User::Matrix1k,
    KIARA_CXX_STRUCT_MEMBER(mat)
    KIARA_CXX_STRUCT_MEMBER(pos))

KIARA_DECL_FUNC_OBJ(Test_SetMatrix)

namespace User {
    typedef float mat44[4][4];
    typedef float * FloatPtr;
}

/* Following macros are optional and used to specify readable names for types */
KIARA_CXX_DECL_TYPENAME(User::mat44)
KIARA_CXX_DECL_TYPENAME(User::IntArray)
KIARA_CXX_DECL_TYPENAME(User::FloatArray4)
KIARA_CXX_DECL_TYPENAME(User::FloatPtr)

/* Decl func Test_SetMatrix */

KIARA_CXX_DECL_FUNC(Test_SetMatrix,
        KIARA_CXX_FUNC_ARG(User::Matrix1k*, matrix))

KIARA_CXX_DECL_FUNC(Func1,
  KIARA_CXX_FUNC_ARG(float *, result)
  KIARA_CXX_FUNC_ARG(int, a)
  KIARA_CXX_FUNC_ARG(float, b)
  KIARA_CXX_FUNC_ARG(User::Vec2f *, c)
)

#endif /* KIARA_DEFS_HPP_INCLUDED */
