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
 * runtimetest.cpp
 *
 *  Created on: Dec 30, 2013
 *      Author: Dmitri Rubinstein
 */

// KIARA
#define HAVE_LLVM
#include <KIARA/Core/LibraryInit.hpp>
#include <DFC/Utils/StrUtils.hpp>
#include <KIARA/LLVM/Utils.hpp>
#include <KIARA/DB/Module.hpp>
#include <KIARA/Impl/Core.hpp>
#include <KIARA/Runtime/RuntimeContext.hpp>
#include <KIARA/Runtime/RuntimeEnvironment.hpp>

// stdlib
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <map>
#include <vector>
#include <fstream>

#define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

void showUsage()
{
    std::cout<<"Runtime test\n"
        "Usage:\n"
        "kiara_runtimetest [options] files\n"
        "\n"
        "  -h    | -help | --help         : prints this and exit\n"
        "  -o filename                    : output LLVM module to file"
        <<std::endl;
}

#define ARG(str) (!strcmp(argv[i], str))
#define ARG_STARTS_WITH(str,len) (!strncmp(argv[i], str, len))
#define ARG_CONTAINS(str) strstr(argv[i], str)
#define APP_ERROR(msg) { std::cerr<<msg<<std::endl; exit(1); }

int main(int argc, char **argv)
{
    kiaraInit(&argc, argv);

    const char *outputFile = 0;
    std::vector<const char *> files;
    typedef std::vector<const char *>::iterator Iter;

    // parse command line
    int i;
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (ARG("-help") || ARG("--help") || ARG("-h"))
            {
                showUsage();
                exit(0);
            }
            else if (ARG("-o"))
            {
                if (++i < argc)
                {
                    outputFile = argv[i];
                }
                else
                {
                    APP_ERROR("-o option require argument");
                }
            }
            else if (ARG("--"))
            {
                i++;
                break;
            }
            else
            {
                APP_ERROR("Unknown option "<<argv[i]);
            }
        }
        else
        {
            // no '-' prefix, assume this is a file name
            files.push_back(argv[i]);
        }
    }

    // read rest files
    for (; i < argc; i++)
    {
        files.push_back(argv[i]);
    }

    {
        KIARA::World world;
        KIARA::LLVMRuntimeContext *ctx = reinterpret_cast<KIARA::LLVMRuntimeContext*>(KIARA::RuntimeContext::create(world));
        KIARA::LLVMRuntimeEnvironment *env1 = reinterpret_cast<KIARA::LLVMRuntimeEnvironment*>(ctx->createEnvironment());
        KIARA::LLVMRuntimeEnvironment *env2 = reinterpret_cast<KIARA::LLVMRuntimeEnvironment*>(ctx->createEnvironment());

        ctx->setSearchPaths(::getenv("KIARA_MODULE_PATH"));
        env1->startInitialization();
        env1->loadComponent("tbp");
        env1->writeModule("M1.bc");

        env2->startInitialization();
        env2->loadComponent("jsonrpc");
        env2->writeModule("M2.bc");

        delete env1;
        delete env2;
        delete ctx;
    }

    kiaraFinalize();

    return 0;
}
