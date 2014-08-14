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
 * apiexample_dds_cpp.cpp
 *
 *  Created on: 16.01.2013
 *      Author: Dmitri Rubinstein
 */
#include <KIARA/kiara.h>
#include <KIARA/kiara_dds_macros.h>
#include <stdio.h>

typedef struct Sequences {
    DDS_LongSeq ls;
    DDS_FloatSeq fs;
} Sequences;

KIARA_DECL_STRUCT(Sequences,
  KIARA_STRUCT_MEMBER(DDS_LongSeq, ls)
  KIARA_STRUCT_MEMBER(DDS_FloatSeq, fs)
)

typedef struct Vec2f {
    DDS_Float x;
    DDS_Float y;
} Vec2f;

KIARA_DECL_STRUCT(Vec2f,
  KIARA_STRUCT_MEMBER(KIARA_DDS_FLOAT, x)
  KIARA_STRUCT_MEMBER(KIARA_DDS_FLOAT, y)
)

typedef struct Linef {
    Vec2f a;
    Vec2f b;
} Linef;

KIARA_DECL_STRUCT(Linef,
  KIARA_STRUCT_MEMBER(Vec2f, a)
  KIARA_STRUCT_MEMBER(Vec2f, b)
)

typedef struct MatrixPos {
    DDS_Long row;
    DDS_Long column;
} MatrixPos;

KIARA_DECL_STRUCT(MatrixPos,
  KIARA_STRUCT_MEMBER(KIARA_DDS_LONG, row)
  KIARA_STRUCT_MEMBER(KIARA_DDS_LONG, column)
)

/* Linef mat1k[1000][1000] and Matrix1k */

typedef struct Matrix1k {
    Linef mat[1000][1000];
    MatrixPos pos;
} Matrix1k;

/* Fixed size 2D array declaration */
KIARA_DECL_FIXED_ARRAY_2D(mat1k, Linef, 1000, 1000)

KIARA_DECL_STRUCT(Matrix1k,
    KIARA_STRUCT_MEMBER(mat1k, mat)
    KIARA_STRUCT_MEMBER(MatrixPos, pos)
)

/* IDL
 *
 * namespace * test
 *
 * struct Vec2f {
 *   float x;
 *   float y;
 * }
 *
 * struct Linef {
 *   Linef a;
 *   Linef b;
 * }
 *
 * struct MatrixPos {
 *   i32 row;
 *   i32 column;
 * }
 *
 * typedef array<array<Linef, 1000>, 1000> mat1k
 *
 * struct Matrix1k {
 *   mat1k mat;
 *   MatrixPos pos;
 * }
 *
 * service Test {
 *   void setMatrix(Matrix1k matrix)
 * }
 *
 * Path                                  Type
 *
 * "test.Matrix1k.mat"                   mat1k = array<array<Linef, 1000>, 1000>
 * "test.Matrix1k.pos"                   MatrixPos
 * "test.Test.setMatrix.Request.matrix"  Matrix1k
 * "test.Test.setMatrix.Response"        void
 *
 */

KIARA_DECL_PTR(Matrix1kPtr, Matrix1k)

KIARA_DECL_FUNC(Test_SetMatrix,
  KIARA_FUNC_ARG(Matrix1kPtr, matrix)
)

/*
 * Path                             Type
 * "Test_SetMatrix.Request.matrix"  Matrix1k
 * "Test_SetMatrix.Response"        void
 */

int main(int argc, char **argv)
{
    KIARA_Context *ctx;
    KIARA_Connection *conn;

    KIARA_FUNC_OBJ(Test_SetMatrix) func;

    Matrix1k *matrix = (Matrix1k *)malloc(sizeof(Matrix1k));

    int i, j;

    /* Fill matrix */
    for (i = 0; i < 1000; ++i)
    {
        for (j = 0; j < 1000; ++j)
        {
            Linef *l = &matrix->mat[i][j];
            l->a.x = l->b.x = i;
            l->a.y = l->b.y = j;
        }
    }
    matrix->pos.column = 0;
    matrix->pos.row = 0;

    /* Initialize KIARA */
    kiaraInit(&argc, argv);

    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_Char));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_Wchar));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_Octet));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_Short));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_UnsignedShort));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_Long));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_UnsignedLong));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_LongLong));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_UnsignedLongLong));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_Float));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_Double));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_LongDouble));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_Boolean));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_Enum));

    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_CharSeq));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_WcharSeq));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_OctetSeq));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_ShortSeq));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_UnsignedShortSeq));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_LongSeq));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_UnsignedLongSeq));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_LongLongSeq));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_UnsignedLongLongSeq));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_FloatSeq));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_DoubleSeq));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_LongDoubleSeq));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(DDS_BooleanSeq));

    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(Sequences));

    ctx = kiaraNewContext();

    /* Open connection to the service */
    conn = kiaraOpenConnection(ctx, "http://testdata.service.net/");

    if (!conn)
    {
        printf("Could not open connection : %s\n", kiaraGetContextError(ctx));
        exit(1);
    }

    /**
     * For generating client closure we call
     *
     * KIARA_GENERATE_CLIENT_FUNC(KIARA_Connection *connection, typeName, const char * mapping)
     *
     * where
     *
     * connection     : KIARA_Connection created with kiaraOpenConnection function
     * typeName       : name of a type described with KIARA macro declaration
     * mapping        : string with syntax :
     *                      "abstract_type_path : native_type_path;"
     *                      "abstract_type_path : native_type_path;"
     *                      ...
     *                      "abstract_type_path : native_type_path"
     *
     *
     */

    func = KIARA_GENERATE_CLIENT_FUNC(conn, "test.Test.setMatrix", Test_SetMatrix, "test.Test.setMatrix : Test_SetMatrix");
    KIARA_CALL(func, matrix);

    kiaraCloseConnection(conn);

    kiaraFreeContext(ctx);

    kiaraFinalize();

    return 0;
}
