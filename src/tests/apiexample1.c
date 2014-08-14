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
 * apiexample1.c
 *
 *  Created on: 19.10.2012
 *      Author: Dmitri Rubinstein
 */
#include <KIARA/kiara.h>
#include <KIARA/kiara_macros.h>
#include <stdio.h>

typedef struct {
   char   flavor;
   char   color;
   int    charge;
   double mass;
   double x, y, z;
   double px, py, pz;
} Quark;

/*
 * Declare structure with KIARA_DECL_STRUCT macro:
 *
 * KIARA_DECL_STRUCT(structName, members)
 *
 * Structure structName will be declared by the macro.
 * For initialization KIARA_REGISTER_TYPE or KIARA_GET_TYPE macros can be used.
 * Both require KIARA context and type name.
 * KIARA_GET_TYPE returns a handle to the registered type.
 *
 * To members arguments are passed arbitrary number
 * of KIARA_STRUCT_MEMBER macros not separated by commas:
 *
 * KIARA_STRUCT_MEMBER(type, memberName)
 *
 * where type can be one of
 *
 * KIARA_CHAR
 * KIARA_INT
 * KIARA_INT32_T
 * KIARA_INT64_T
 * KIARA_INT_OF_SIZE(size)
 * KIARA_FLOAT
 * KIARA_DOUBLE
 * KIARA_CHAR_PTR
 * KIARA_VOID_PTR
 * name of the type
 */

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
    float x;
    float y;
} Vec2f;

KIARA_DECL_STRUCT(Vec2f,
  KIARA_STRUCT_MEMBER(KIARA_FLOAT, x)
  KIARA_STRUCT_MEMBER(KIARA_FLOAT, y)
)

KIARA_DECL_ANNOTATED(Vec2fA, Vec2f, NONE, NONE)

typedef struct Linef {
    Vec2f a;
    Vec2f b;
} Linef;

KIARA_DECL_STRUCT(Linef,
  KIARA_STRUCT_MEMBER(Vec2f, a)
  KIARA_STRUCT_MEMBER(Vec2f, b)
)

/* IntList and IntList* */

typedef struct IntList {
    struct IntList *next;
    int data;
} IntList;

/* Forward declaration of a named type */
KIARA_FORWARD_DECL(IntList)
/* Pointer declaration */
KIARA_DECL_PTR(IntListPtr, IntList)

KIARA_DECL_STRUCT(IntList,
  KIARA_STRUCT_MEMBER(IntListPtr, next)
  KIARA_STRUCT_MEMBER(KIARA_INT, data)
)

/* Func1 and Vec2f* */

KIARA_DECL_PTR(Vec2fPtr, Vec2f)
KIARA_DECL_PTR(FloatPtr, KIARA_FLOAT)

/*
 * Functions declaration:
 *
 * KIARA_DECL_FUNC(funcName,
 *                 returnType,
 *                 arguments)
 *
 * where to arguments are passed arbitrary number of
 *
 * KIARA_FUNC_ARG(type, argumentName)
 *
 * where type and returnType can be one of
 *
 * KIARA_CHAR
 * KIARA_INT
 * KIARA_INT32_T
 * KIARA_INT64_T
 * KIARA_FLOAT
 * KIARA_DOUBLE
 * KIARA_CHAR_PTR
 * name of the type
 *
 */

KIARA_DECL_FUNC(Func1,
  KIARA_FUNC_ARG(FloatPtr, result)
  KIARA_FUNC_ARG(KIARA_INT, a)
  KIARA_FUNC_ARG(KIARA_FLOAT, b)
  KIARA_FUNC_ARG(Vec2fPtr, c)
)

/* float mat[4][4] and Matrix44 */

typedef struct Matrix44 {
    float mat[4][4];
} Matrix44;

/* Fixed size 2D array declaration */
KIARA_DECL_FIXED_ARRAY_2D(mat44, KIARA_FLOAT, 4, 4)

KIARA_DECL_STRUCT(Matrix44,
    KIARA_STRUCT_MEMBER(mat44, mat)
)

/* IntArray and FloatArray4 */

typedef int IntArray[];
typedef float FloatArray4[4];

/* Unbounded array declaration */
KIARA_DECL_ARRAY(IntArray, KIARA_INT)
/* Fixed size 1D array declaration */
KIARA_DECL_FIXED_ARRAY(FloatArray4, KIARA_FLOAT, 4)

/* IDL
 *
 * namespace * test
 *
 * struct TestData {
 *   i32 i;
 *   float f;
 * }
 *
 * service Test {
 *   void setData(TestData data)
 * }
 *
 * Path                                 Type
 *
 * "test.TestData.i"                    i32
 * "test.TestData.f"                    float
 * "test.Test.setData.Request.data"     TestData
 * "test.Test.setData.Response"         void
 *
 */

typedef struct Data {
    int i;
    float f;
} Data;

KIARA_DECL_STRUCT(Data,
  KIARA_STRUCT_MEMBER(KIARA_INT, i)
  KIARA_STRUCT_MEMBER(KIARA_FLOAT, f)
)

/*
 * Path       Type
 * "Data.i"   int
 * "Data.f"   float
 */

KIARA_DECL_PTR(DataPtr, Data)
KIARA_DECL_FUNC(Test_SetData,
  KIARA_FUNC_ARG(DataPtr, data)
)

/*
 * Path                         Type
 * "Test_SetData.Request.data"  Data
 * "Test_SetData.Response"      void
 */

#define DUMP_TYPE_INFO(T)                                               \
    {                                                                   \
        KIARA_Type *ty = kiaraGetTypeFromContext(ctx, KIARA_TYPE(T));   \
        kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(T));                      \
        kiaraDbgDumpType(ty);                                           \
    }


int main(int argc, char **argv)
{
    KIARA_Context *ctx;
    KIARA_Connection *conn;

    kiaraInit(&argc, argv);

    /* Output all static types */

    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(Quark));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(Vec2f));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(Vec2fPtr));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(Linef));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(IntList));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(Func1));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(mat44));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(Matrix44));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(IntArray));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(FloatArray4));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(Data));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DataPtr));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(Test_SetData));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(Vec2fA));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(KIARA_char_ptr));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(KIARA_raw_char_ptr));

    /* Create context and connection */

    ctx = kiaraNewContext();

    DUMP_TYPE_INFO(Quark);
    DUMP_TYPE_INFO(Vec2f);
    DUMP_TYPE_INFO(Vec2fPtr);
    DUMP_TYPE_INFO(Linef);
    DUMP_TYPE_INFO(IntList);
    DUMP_TYPE_INFO(Func1);
    DUMP_TYPE_INFO(mat44);
    DUMP_TYPE_INFO(Matrix44);
    DUMP_TYPE_INFO(IntArray);
    DUMP_TYPE_INFO(FloatArray4);
    DUMP_TYPE_INFO(Data);
    DUMP_TYPE_INFO(DataPtr);
    DUMP_TYPE_INFO(Test_SetData);
    DUMP_TYPE_INFO(Vec2fA);
    DUMP_TYPE_INFO(KIARA_char_ptr);
    DUMP_TYPE_INFO(KIARA_raw_char_ptr);

    conn = kiaraOpenConnection(ctx, "http://testdata.service.net/");

    if (!conn)
    {
        printf("Could not open connection : %s\n", kiaraGetContextError(ctx));
        exit(1);
    }

    KIARA_REGISTER_TYPE(ctx, Test_SetData);

    {
        Data data = {1, 2};

        /**
         * For generating client stub we call
         *
         * KIARA_GENERATE_CLIENT_FUNC(KIARA_Connection *connection, idlMethodName, typeName, const char * mapping)
         *
         * where
         *
         * connection     : KIARA_Connection created with kiaraOpenConnection function
         * idlMethodName  : name of the IDL service method which will be called
         * staticTypeDecl : static type declaration created with macros
         * mapping        : string with syntax :
         *                      "abstract_type_path : native_type_path;"
         *                      "abstract_type_path : native_type_path;"
         *                      ...
         *                      "abstract_type_path : native_type_path"
         *
         *
         */
        KIARA_FUNC_OBJ(Test_SetData) func = KIARA_GENERATE_CLIENT_FUNC(conn, "test.Test.setData", Test_SetData, "test.Test.setData : Test_SetData");
        KIARA_CALL(func, &data);
    }

    kiaraCloseConnection(conn);

    kiaraFreeContext(ctx);

    kiaraFinalize();

    return 0;
}
