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
 * calctest_cpp.cpp
 *
 *  Created on: 26.04.2013
 *      Author: Dmitri Rubinstein
 */
#include <KIARA/kiara.h>
#include <KIARA/kiara_cxx_macros.hpp>
#include <iostream>

int std_string_SetCString(KIARA_UserType *ustr, const char *cstr)
{
    if (cstr)
        ((std::string*)ustr)->assign(cstr);
    else
        ((std::string*)ustr)->clear();
    return 0;
}

int std_string_GetCString(KIARA_UserType *ustr, const char **cstr)
{
    *cstr = ((std::string*)ustr)->c_str();
    return 0;
}

KIARA_CXX_DECL_OPAQUE_TYPE(std::string,
       KIARA_CXX_USER_API(SetCString, std_string_SetCString)
       KIARA_CXX_USER_API(GetCString, std_string_GetCString))

KIARA_CXX_DECL_FUNC(Calc_Add,
  KIARA_CXX_FUNC_RESULT(int &, result)
  KIARA_CXX_FUNC_ARG(int, a)
  KIARA_CXX_FUNC_ARG(int, b)
  )

KIARA_CXX_DECL_FUNC(Calc_Add_Float,
    KIARA_CXX_FUNC_RESULT(float &, result)
    KIARA_CXX_FUNC_ARG(float, a)
    KIARA_CXX_FUNC_ARG(float, b)
)

KIARA_CXX_DECL_FUNC(Calc_String_To_Int32,
    KIARA_CXX_FUNC_RESULT(int &, result)
    KIARA_CXX_FUNC_ARG(const char *, s)
)

KIARA_CXX_DECL_FUNC(Calc_Int32_To_String,
    KIARA_CXX_FUNC_RESULT(char **, result)
    KIARA_CXX_FUNC_ARG(int, i)
)

KIARA_CXX_DECL_FUNC(Calc_StdString_To_Int32,
    KIARA_CXX_FUNC_RESULT(int &, result)
    KIARA_CXX_FUNC_ARG(const std::string &, s)
)

KIARA_CXX_DECL_FUNC(Calc_Int32_To_StdString,
    KIARA_CXX_FUNC_RESULT(std::string &, result)
    KIARA_CXX_FUNC_ARG(int, i)
)

int main(int argc, char **argv)
{
    /* Initialize KIARA */
    kiaraInit(&argc, argv);

    KIARA_Context *ctx = kiaraNewContext();

    /* Open connection to the service */
    KIARA_Connection *conn = kiaraOpenConnection(ctx, "http://localhost:8080/service");

    if (!conn)
    {
        std::cerr<<"Error: Could not open connection : "<<kiaraGetContextError(ctx)<<std::endl;
        exit(1);
    }

    {
        int result;
        // IDL -> native
        Calc_Add add = KIARA_GENERATE_CLIENT_FUNC(conn, "calc.add", Calc_Add, "");
        if (add) // FIXME this should be always true
        {
            int errorCode = add(result, 21, 32);
            if (errorCode != KIARA_SUCCESS)
            {
                std::cerr<<"Error: call failed: "<<kiaraGetConnectionError(conn)<<std::endl;
            }
            else
            {
                std::cout<<"calc.add: result = "<<result<<std::endl;
            }
        }
        else
        {
            std::cerr<<"Error: code generation failed: "<<kiaraGetConnectionError(conn)<<std::endl;
        }
    }

    {
        float result;
        // IDL -> native
        Calc_Add_Float addf = KIARA_GENERATE_CLIENT_FUNC(conn, "calc.addf", Calc_Add_Float, "");
        if (addf) // FIXME this should be always true
        {
            int errorCode = addf(result, 21.0, 32.0);
            if (errorCode != KIARA_SUCCESS)
            {
                std::cerr<<"Error: call failed: "<<kiaraGetConnectionError(conn)<<std::endl;
            }
            else
            {
                std::cout<<"calc.addf: result = "<<result<<std::endl;
            }
        }
        else
        {
            std::cerr<<"Error: code generation failed: "<<kiaraGetConnectionError(conn)<<std::endl;
        }
    }

    {
        int result;
        // IDL -> native
        Calc_String_To_Int32 strToInt = KIARA_GENERATE_CLIENT_FUNC(conn, "calc.stringToInt32", Calc_String_To_Int32, "");
        if (strToInt) // FIXME this should be always true
        {
            int errorCode = strToInt(result, "125");
            if (errorCode != KIARA_SUCCESS)
            {
                std::cerr<<"Error: call failed: "<<kiaraGetConnectionError(conn)<<std::endl;
            }
            else
            {
                std::cout<<"calc.stringToInt32: result = "<<result<<std::endl;
            }
        }
        else
        {
            std::cerr<<"Error: code generation failed: "<<kiaraGetConnectionError(conn)<<std::endl;
        }
    }

    {
        char *result = 0; /* must be initialized to NULL or malloc-allocated memory ! */
        /* IDL -> native */
        Calc_Int32_To_String intToStr = KIARA_GENERATE_CLIENT_FUNC(conn, "calc.int32ToString", Calc_Int32_To_String, "");
        if (intToStr) /* FIXME this should be always true */
        {
            int errorCode = intToStr(&result, 42);
            if (errorCode != KIARA_SUCCESS)
            {
                std::cerr<<"Error: call failed: "<<kiaraGetConnectionError(conn)<<std::endl;
            }
            else
            {
                std::cout<<"calc.int32ToString: result = "<<result<<std::endl;
            }
        }
        else
        {
            std::cerr<<"Error: code generation failed: "<<kiaraGetConnectionError(conn)<<std::endl;
        }
        free(result); /* deallocation is needed */
    }

    {
        int result;
        // IDL -> native
        Calc_StdString_To_Int32 strToInt = KIARA_GENERATE_CLIENT_FUNC(conn, "calc.stringToInt32", Calc_StdString_To_Int32, "");
        if (strToInt) // FIXME this should be always true
        {
            int errorCode = strToInt(result, "521");
            if (errorCode != KIARA_SUCCESS)
            {
                std::cerr<<"Error: call failed: "<<kiaraGetConnectionError(conn)<<std::endl;
            }
            else
            {
                std::cout<<"calc.stringToInt32: result = "<<result<<std::endl;
            }
        }
        else
        {
            std::cerr<<"Error: code generation failed: "<<kiaraGetConnectionError(conn)<<std::endl;
        }
    }

    {
        std::string result;
        /* IDL -> native */
        Calc_Int32_To_StdString intToStr = KIARA_GENERATE_CLIENT_FUNC(conn, "calc.int32ToString", Calc_Int32_To_StdString, "");
        if (intToStr) /* FIXME this should be always true */
        {
            int errorCode = intToStr(result, 142);
            if (errorCode != KIARA_SUCCESS)
            {
                std::cerr<<"Error: call failed: "<<kiaraGetConnectionError(conn)<<std::endl;
            }
            else
            {
                std::cout<<"calc.int32ToString: result = "<<result<<std::endl;
            }
        }
        else
        {
            std::cerr<<"Error: code generation failed: "<<kiaraGetConnectionError(conn)<<std::endl;
        }
    }

    kiaraCloseConnection(conn);

    kiaraFreeContext(ctx);

    kiaraFinalize();

    return 0;
}
