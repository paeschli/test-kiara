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
* apidemo_c.c
*
*  Created on: 04.09.2012
*      Author: Dmitri Rubinstein
*/
#include <KIARA/kiara.h>
#include <KIARA/kiara_macros.h>
#include <string.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

/*
* In this KIARA API demo application is demonstrated how to
* describe native application datatypes.
*/

typedef struct {
    float x;
    float y;
} Vec2f;

KIARA_DECL_STRUCT(Vec2f,
  KIARA_STRUCT_MEMBER(KIARA_FLOAT, x)
  KIARA_STRUCT_MEMBER(KIARA_FLOAT, y)
)

typedef struct {
    Vec2f a;
    Vec2f b;
} Linef;

KIARA_DECL_STRUCT(Linef,
  KIARA_STRUCT_MEMBER(Vec2f, a)
  KIARA_STRUCT_MEMBER(Vec2f, b)
)

typedef struct {
    char   flavor;
    char   color;
    int    charge;
    double mass;
    double x, y, z;
    double px, py, pz;
} Quark;

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

typedef struct {
    int a;
    float b;
    double c;
} Test;

KIARA_DECL_STRUCT(Test,
  KIARA_STRUCT_MEMBER(KIARA_INT, a)
  KIARA_STRUCT_MEMBER(KIARA_FLOAT, b)
  KIARA_STRUCT_MEMBER(KIARA_DOUBLE, c)
)

typedef int (*Func)(float a, float b, Vec2f *c);

/*
* Functions can also be declared with macros:
*
* KIARA_DECL_FUNC(funcName,
*                 arguments)
*
* where to arguments are passed arbitrary number of
*
* KIARA_FUNC_ARG(typeHandle, argumentName)
*
* and single
*
* KIARA_FUNC_RESULT(typeHandle, argumentName)
*
*/

KIARA_DECL_PTR(FloatPtr, KIARA_FLOAT)
KIARA_DECL_PTR(Vec2fPtr, Vec2f)

KIARA_DECL_FUNC(Func1,
    KIARA_FUNC_RESULT(FloatPtr, result)
    KIARA_FUNC_ARG(KIARA_INT, a)
    KIARA_FUNC_ARG(KIARA_FLOAT, b)
    KIARA_FUNC_ARG(Vec2fPtr, c)
)

int main(int argc, char **argv)
{
    KIARA_Context * ctx = NULL;
    KIARA_Type * tVec2f = NULL;
    KIARA_Type * tLinef = NULL;
    KIARA_Type * tQuark = NULL;
    KIARA_Type * tFunc  = NULL;

    /* Initialize KIARA library */
    kiaraInit(&argc, argv);

    /* Create KIARA context.
    * Each thread require a separate context.
    * Using a single context in multiple threads requires a mutex-based locking.
    * Most of API functions require context.
    * Handles to KIARA objects created with different contexts can't be mixed.
    */
    ctx = kiaraNewContext();

    /* For each C datatype KIARA provides a function which returns a corresponding
    * type handle:
    *
    * int    kiaraType_c_int
    * float  kiaraType_c_float
    * double kiaraType_c_double
    * ...
    *
    * Derived types like structs can be constructed by specifying
    * types, names and offsets of the elements:
    */
    {
        KIARA_StructDecl declVec2f[] = {
            {kiaraType_c_float(ctx), "x", offsetof(Vec2f, x)},
            {kiaraType_c_float(ctx), "y", offsetof(Vec2f, y)}
        };

        tVec2f = kiaraDeclareStructType(ctx, "Vec2f", 2, declVec2f);
    }

    /*
    * Handles to already declared types can be used to construct
    * new types:
    *
    * Declare struct that uses already declared type (Vec2f).
    */

    {
        KIARA_StructDecl declLinef[] = {
            {tVec2f, "x", offsetof(Vec2f, x)},
            {tVec2f, "y", offsetof(Vec2f, y)}
        };

        tLinef = kiaraDeclareStructType(ctx, "Linef", 2, declLinef);
        (void)tLinef; /* Prevent warnings */
    }

    /*
    * Declare a larger structure.
    */

    {
        KIARA_StructDecl declQuark[] = {
            {kiaraType_c_char(ctx), "flavor", offsetof(Quark, flavor)},
            {kiaraType_c_char(ctx), "color", offsetof(Quark, color)},
            {kiaraType_c_int(ctx), "charge", offsetof(Quark, charge)},
            {kiaraType_c_double(ctx), "mass", offsetof(Quark, mass)},
            {kiaraType_c_double(ctx), "x", offsetof(Quark, x)},
            {kiaraType_c_double(ctx), "y", offsetof(Quark, y)},
            {kiaraType_c_double(ctx), "z", offsetof(Quark, z)},
            {kiaraType_c_double(ctx), "px", offsetof(Quark, px)},
            {kiaraType_c_double(ctx), "py", offsetof(Quark, py)},
            {kiaraType_c_double(ctx), "pz", offsetof(Quark, pz)}
        };

        tQuark = kiaraDeclareStructType(ctx, "Quark",
            sizeof(declQuark)/sizeof(declQuark[0]), declQuark);
        (void)tQuark; /* Prevent warnings */
    }

    /*
    * Convenience macros can be used to reduce typing.
    * Internal implementation of macros require boost preprocessor library.
    *
    * Declare structure with KIARA_DECL_STRUCT macro:
    *
    * KIARA_DECL_STRUCT(typeVariable, context, structName, members)
    *
    * typeVariable will be declared by the macro as a type handle and
    * becomes a result of a declaration.
    *
    * To members arguments are passed arbitrary number
    * of KIARA_STRUCT_MEMBER macros not separated by commas:
    *
    * KIARA_STRUCT_MEMBER(typeHandle, memberName)
    *
    */

    KIARA_REGISTER_TYPE(ctx, Test);

    /*
    * Declaration of functions is similar to structs, but no
    * offsets are used:
    *
    * Declare function type that accepts two floats and Vec2f
    * pointer and returns an int.
    */
    {
        KIARA_FuncDecl declFunc[] = {
            {kiaraType_c_float(ctx), "a"},
            {kiaraType_c_float(ctx), "b"},
            {kiaraType_c_ptr(tVec2f), "c"}
        };

        tFunc = kiaraDeclareFuncType(
            ctx, "Func",
            kiaraType_c_int(ctx),
            sizeof(declFunc)/sizeof(declFunc[0]),
            declFunc);
        (void)tFunc; /* Prevent warnings */
    }

    KIARA_REGISTER_TYPE(ctx, Func1);

    /* Finalize context and library */

    kiaraFreeContext(ctx);

    kiaraFinalize();

    return 0;
}
