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
 * apidemo.cpp
 *
 *  Created on: 06.09.2012
 *      Author: Dmitri Rubinstein
 */
#include <KIARA/kiara.h>
#include <KIARA/kiara_macros.h>
#include <boost/typeof/typeof.hpp>

struct Vec2f
{
    float x;
    float y;
};

KIARA_DECL_STRUCT(Vec2f,
    KIARA_STRUCT_MEMBER(KIARA_FLOAT, x)
    KIARA_STRUCT_MEMBER(KIARA_FLOAT, y)
)

struct Linef
{
    Vec2f a;
    Vec2f b;
};

KIARA_DECL_STRUCT(Linef,
    KIARA_STRUCT_MEMBER(Vec2f, a)
    KIARA_STRUCT_MEMBER(Vec2f, b)
)

struct Quark
{
   char   flavor;
   char   color;
   int    charge;
   double mass;
   double x, y, z;
   double px, py, pz;
};

KIARA_DECL_STRUCT(Quark,
  KIARA_STRUCT_MEMBER(KIARA_CHAR, flavor)
  KIARA_STRUCT_MEMBER(KIARA_CHAR, color)
  KIARA_STRUCT_MEMBER(KIARA_INT, charge)
  KIARA_STRUCT_MEMBER(KIARA_DOUBLE, mass)
  KIARA_STRUCT_MEMBER(KIARA_DOUBLE, x)
  KIARA_STRUCT_MEMBER(KIARA_DOUBLE, y)
  KIARA_STRUCT_MEMBER(KIARA_DOUBLE, z)
  KIARA_STRUCT_MEMBER(KIARA_DOUBLE, px)
  KIARA_STRUCT_MEMBER(KIARA_DOUBLE, py)
  KIARA_STRUCT_MEMBER(KIARA_DOUBLE, pz)
)

struct Test
{
    char * a;
    float b;
    double c;
};

KIARA_DECL_STRUCT(Test,
    KIARA_STRUCT_MEMBER(KIARA_CHAR_PTR, a)
    KIARA_STRUCT_MEMBER(KIARA_FLOAT, b)
    KIARA_STRUCT_MEMBER(KIARA_DOUBLE, c)
)

KIARA_DECL_PTR(IntPtr, KIARA_INT)

KIARA_DECL_PTR(Vec2fPtr, Vec2f)

KIARA_DECL_FUNC(Func,
    KIARA_FUNC_RESULT(IntPtr, result)
    KIARA_FUNC_ARG(KIARA_FLOAT, a)
    KIARA_FUNC_ARG(KIARA_FLOAT, b)
    KIARA_FUNC_ARG(Vec2fPtr, c)
)

KIARA_DECL_PTR(FloatPtr, KIARA_FLOAT)

KIARA_DECL_FUNC(Func1,
    KIARA_FUNC_RESULT(FloatPtr, result)
    KIARA_FUNC_ARG(KIARA_INT, a)
    KIARA_FUNC_ARG(KIARA_FLOAT, b)
    KIARA_FUNC_ARG(Vec2fPtr, c)
)

int main(int argc, char **argv)
{
    /* Initialize KIARA library */
    kiaraInit(&argc, argv);

    /* Create KIARA context.
     * Each thread require a separate context.
     */
    KIARA_Context *ctx = kiaraNewContext();

    /*
     * Declare struct with native C-type members.
     */

    KIARA_Type * tVec2f = KIARA_GET_TYPE(ctx, Vec2f);


    /*
     * Declare struct that uses already declared type (Vec2f).
     */

    KIARA_Type * tLinef = KIARA_GET_TYPE(ctx, Linef);

    /*
     * Declare a larger structure.
     */

    KIARA_Type * tQuark = KIARA_GET_TYPE(ctx, Quark);

    KIARA_Type * tTest = KIARA_GET_TYPE(ctx, Test);

    /*
     * Declare function type that accepts two floats and Vec2f
     * pointer and returns int.
     */

    KIARA_Type * tFunc = KIARA_GET_TYPE(ctx, Func);

    KIARA_Type *tFunc1 = KIARA_GET_TYPE(ctx, Func1);

    /* Dump debug informations */
    kiaraDbgDumpType(tVec2f);
    kiaraDbgDumpType(tLinef);
    kiaraDbgDumpType(tQuark);
    kiaraDbgDumpType(tTest);
    kiaraDbgDumpType(tFunc);
    kiaraDbgDumpType(tFunc1);

    /* Finalize context and library */

    kiaraFreeContext(ctx);

    kiaraFinalize();

    return 0;
}
