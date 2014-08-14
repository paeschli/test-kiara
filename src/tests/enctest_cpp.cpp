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
 * enctest_cpp.cpp
 *
 *  Created on: Aug 15, 2013
 *      Author: rubinste
 */
#include <KIARA/kiara.h>
#include <KIARA/kiara_cxx_macros.hpp>
#include <KIARA/CDT/kr_dstring.h>
#include <iostream>

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

KIARA_CXX_DECL_OPAQUE_TYPE(kr_dstring_t,
       KIARA_CXX_USER_API(SetCString, dstring_SetCString)
       KIARA_CXX_USER_API(GetCString, dstring_GetCString))

KIARA_CXX_DECL_ENCRYPTED(EncDString, kr_dstring_t)

KIARA_CXX_DECL_FUNC(SendEncString,
  KIARA_CXX_FUNC_ARG(EncDString, a)
)

int main(int argc, char **argv)
{
    const char *url = NULL;

    /* This code is required for testing tool when compiled with MS CRT library and valgrind */
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    /* Initialize KIARA */
    kiaraInit(&argc, argv);

    KIARA_Context *ctx = kiaraNewContext();

    if (argc > 1)
    {
        url = argv[1];
    }
    else
    {
        url = "http://localhost:8080/service";
    }

    /* Open connection to the service */
    printf("Opening connection to %s...\n", url);
    KIARA_Connection *conn = kiaraOpenConnection(ctx, url);

    if (!conn)
    {
        std::cerr<<"Error: Could not open connection : "<<kiaraGetContextError(ctx)<<std::endl;
        exit(1);
    }

    {
        KIARA_Type *ty = kiaraGetTypeFromDecl(conn, KIARA_TYPE(SendEncString));
        kiaraDbgDumpDeclTypeGetter(KIARA_TYPE(SendEncString));
        kiaraDbgDumpType(ty);
    }

    {
        EncDString encDString = new_EncDString();
        /* IDL -> native */
        SendEncString sendEncString = KIARA_GENERATE_CLIENT_FUNC(conn, "enc.sendEncString", SendEncString, "");
        if (sendEncString) /* FIXME this should be always true */
        {
            int errorCode = sendEncString(encDString);
            if (errorCode != KIARA_SUCCESS)
            {
                std::cerr<<"Error: call failed: "<<kiaraGetConnectionError(conn)<<std::endl;
            }
            else
            {
                std::cout<<"enc.sendEncString successfully called"<<std::endl;
            }
        }
        else
        {
            std::cerr<<"Error: code generation failed: "<<kiaraGetConnectionError(conn)<<std::endl;
        }
        delete_EncDString(encDString);
    }

    kiaraCloseConnection(conn);

    kiaraFreeContext(ctx);

    kiaraFinalize();

    return 0;
}
