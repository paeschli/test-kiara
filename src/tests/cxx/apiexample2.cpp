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
 * apiexample2.cpp
 *
 *  Created on: 10.12.2012
 *      Author: Dmitri Rubinstein
 */

#include "user_types.hpp"
#include "kiara_defs.hpp"

#include <cstdio>
#include <iostream>
#include <boost/scoped_ptr.hpp>
using namespace User;

int main(int argc, char **argv)
{
    boost::scoped_ptr<Matrix1k> matrix(new Matrix1k);

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

    /* Output all static types */
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(char));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(unsigned int));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(gps_position));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(gps_direction));

    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(Quark));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(Vec2f));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(Vec2f*));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(Linef));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(IntList));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(IntList*));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(Func1));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(float[4][4]));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(Matrix44));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(IntArray));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(FloatArray4));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(Data));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(Data*));
    kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(Test_SetMatrix));

    KIARA_Context *ctx = kiaraNewContext();

    /* Open connection to the service */
    KIARA_Connection *conn = kiaraOpenConnection(ctx, "http://testdata.service.net/");

    if (!conn)
    {
        printf("Could not open connection : %s\n", kiaraGetContextError(ctx));
        exit(1);
    }


    /**
     * For generating client closure we call
     *
     * KIARA_GENERATE_CLIENT_FUNC(KIARA_Connection *connection, idlMethodName, typeName, const char * mapping)
     *
     * where
     *
     * connection     : KIARA_Connection created with kiaraOpenConnection function
     * idlMethodName  : name of the IDL service method which will be called
     * typeName       : name of a type described with KIARA macro declaration
     * mapping        : string with syntax :
     *                      "abstract_type_path : native_type_path;"
     *                      "abstract_type_path : native_type_path;"
     *                      ...
     *                      "abstract_type_path : native_type_path"
     *
     *
     */

    Test_SetMatrix func = KIARA_GENERATE_CLIENT_FUNC(conn, "test.Test.setMatrix", Test_SetMatrix, "test.Test.setMatrix : Test_SetMatrix");
    func(matrix.get());

    kiaraCloseConnection(conn);

    kiaraFreeContext(ctx);

    kiaraFinalize();

    return 0;
}
