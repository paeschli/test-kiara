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
#include <KIARA/kiara.h>
#include <KIARA/kiara_macros.h>
#include <KIARA/CDT/kr_dstring.h>
#include <stdio.h>

int mu_tests_run;

#define MU_CHECK_MSG(message, test) do { if (!(test)) return message; } while (0)
#define MU_CHECK(test) MU_CHECK_MSG("Test (" #test ") failed at line " KIARA_STRINGIZE(__LINE__), test)
#define MU_RUN_TEST(test) do { const char *message = test(); mu_tests_run++; \
                               if (message) return message; } while (0)

#define MU_TEST(name) static const char * name()

#define CHECK_KIARA_ERROR(ctx)                  \
    do {                                        \
        if (kiaraGetContextErrorCode(ctx))      \
        {                                       \
            return kiaraGetContextError(ctx);   \
        }                                       \
    } while (0)

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
    float x;
    float y;
} Vec2f;

KIARA_DECL_STRUCT(Vec2f,
  KIARA_STRUCT_MEMBER(KIARA_FLOAT, x)
  KIARA_STRUCT_MEMBER(KIARA_FLOAT, y)
)

typedef struct Linef {
    Vec2f a;
    Vec2f b;
} Linef;

KIARA_DECL_STRUCT(Linef,
  KIARA_STRUCT_MEMBER(Vec2f, a)
  KIARA_STRUCT_MEMBER(Vec2f, b)
)

typedef struct Matrix44 {
    float mat[4][4];
} Matrix44;

#define KIARA_ARRAY_2D(elementType, numRows, numCols) \
        KIARA_FIXED_ARRAY(KIARA_FIXED_ARRAY(KIARA_FLOAT, numCols), numRows)

KIARA_DECL_FIXED_ARRAY_2D(mat44, KIARA_FLOAT, 4, 4)

KIARA_DECL_STRUCT(Matrix44,
    KIARA_STRUCT_MEMBER(mat44, mat)
)

KIARA_DECL_ENCRYPTED(EncMatrix44, Matrix44)

typedef struct MatrixEncInfo {
    int x, y;
    EncMatrix44 encMatrix;
} MatrixEncPos;

KIARA_DECL_STRUCT(MatrixEncPos,
    KIARA_STRUCT_MEMBER(int, x)
    KIARA_STRUCT_MEMBER(int, y)
    KIARA_STRUCT_MEMBER(EncMatrix44, encMatrix)
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

int dstring_SetCString(KIARA_UserType *ustr, const char *cstr)
{
    int result = kr_dstring_assign_str((kr_dstring_t*)ustr, cstr);
    return result ? 0 : 1;
}

int dstring_GetCString(KIARA_UserType *ustr, const char **cstr)
{
    *cstr = kr_dstring_str((kr_dstring_t*)ustr);
    return 0;
}

KIARA_DECL_OPAQUE_TYPE(kr_dstring_t,
       KIARA_USER_API(SetCString, dstring_SetCString)
       KIARA_USER_API(GetCString, dstring_GetCString))
KIARA_DECL_PTR(DStringPtr, kr_dstring_t)
/* KIARA_DECL_CONST_PTR(ConstDStringPtr, kr_dstring_t) */

KIARA_DECL_ENCRYPTED(EncDString, kr_dstring_t)

KIARA_DECL_FUNC(SendEncString,
  KIARA_FUNC_ARG(EncDString, a)
)

MU_TEST(test_context)
{
    KIARA_Context *ctx = kiaraNewContext();
	KIARA_Type *c_char, *c_short, *c_int, *c_float, *c_size_t, *tVec2f, *i8, *i16, *i32;
    KIARA_Type * tLinef, * tQuark, * tTest;
    KIARA_Type * tMatrix44, *tMatrixEncPos, *tEncDString, *tSendEncString;
    MU_CHECK(ctx != NULL);

    c_char = kiaraType_c_char(ctx);
    MU_CHECK(c_char != NULL);
    c_short = kiaraType_c_short(ctx);
    MU_CHECK(c_short != NULL);
    c_int = kiaraType_c_int(ctx);
    MU_CHECK(c_int != NULL);
    c_float = kiaraType_c_float(ctx);
    MU_CHECK(c_float != NULL);
    c_size_t = kiaraType_c_size_t(ctx);
    MU_CHECK(c_size_t != NULL);

    i8 = kiaraType_i8(ctx);
    i16 = kiaraType_i16(ctx);
    i32 = kiaraType_i32(ctx);

    MU_CHECK(i8 != c_char);
    MU_CHECK(i16 != c_short);
    MU_CHECK(i32 != c_int);

    kiaraDbgDumpType(c_char);
    kiaraDbgDumpType(c_short);
    kiaraDbgDumpType(c_int);
    kiaraDbgDumpType(c_float);
    kiaraDbgDumpType(c_size_t);
    kiaraDbgDumpType(i8);
    kiaraDbgDumpType(i16);
    kiaraDbgDumpType(i32);

    tVec2f = KIARA_GET_TYPE(ctx, Vec2f);

    CHECK_KIARA_ERROR(ctx);
    kiaraDbgDumpType(tVec2f);

    tLinef = KIARA_GET_TYPE(ctx, Linef);

    CHECK_KIARA_ERROR(ctx);
    kiaraDbgDumpType(tLinef);

    tQuark = KIARA_GET_TYPE(ctx, Quark);

    CHECK_KIARA_ERROR(ctx);
    kiaraDbgDumpType(tQuark);

    tTest = KIARA_GET_TYPE(ctx, Test);

    kiaraDbgDumpType(tTest);

    /* Matrix44 */

    kiaraDbgDumpDeclTypeGetter(_KIARA_DTYPE_GETTER(Matrix44));

    tMatrix44 = KIARA_GET_TYPE(ctx, Matrix44);

    kiaraDbgDumpType(tMatrix44);

    /* EncMatrix44 */

    kiaraDbgDumpDeclTypeGetter(_KIARA_DTYPE_GETTER(EncMatrix44));

    /* EncDString */

    kiaraDbgDumpDeclTypeGetter(_KIARA_DTYPE_GETTER(EncDString));

    tEncDString = KIARA_GET_TYPE(ctx, EncDString);
    MU_CHECK(tEncDString != NULL);

    kiaraDbgDumpType(tEncDString);

    /* SendEncString */

    kiaraDbgDumpDeclTypeGetter(_KIARA_DTYPE_GETTER(SendEncString));

    tSendEncString = KIARA_GET_TYPE(ctx, SendEncString);
    MU_CHECK(tSendEncString != NULL);

    kiaraDbgDumpType(tSendEncString);

    /* MatrixEncPos */

    kiaraDbgDumpDeclTypeGetter(_KIARA_DTYPE_GETTER(MatrixEncPos));

    tMatrixEncPos = KIARA_GET_TYPE(ctx, MatrixEncPos);

    kiaraDbgDumpType(tMatrixEncPos);

    kiaraFreeContext(ctx);
    return NULL;
}

MU_TEST(all_tests)
{
    kiaraInit(NULL, NULL);

    MU_RUN_TEST(test_context);

    kiaraFinalize();
    return NULL;
}

int main (int argc, char **argv)
{
    const char *result = all_tests();
    if (result)
    {
        printf("Test Error: %s\n", result);
    }
    else
    {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", mu_tests_run);

    return result != NULL;
}
