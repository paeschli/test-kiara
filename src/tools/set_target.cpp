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
 * kiara_set_target.cpp
 *
 *  Created on: 08.12.2011
 *      Author: Dmitri Rubinstein
 */

#include <iostream>

#include <KIARA/Core/Exception.hpp>
#include <KIARA/LLVM/Utils.hpp>

// LLVM-specific headers
#include "llvm/Config/llvm-config.h"
#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
#include <llvm/IR/Module.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/Host.h>
#else
#include <llvm/Module.h>
#include <llvm/Target/TargetData.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/Host.h>
#endif

using namespace std;
using namespace KIARA;

// help

void showUsage()
{
    cout<<"Set target information for llvm modules"<<endl
        <<"Usage:"<<endl
        <<"kiara-set-target [options] bc-filename\n"
        <<"\n"
        <<"  -h    | -help | --help         : prints this and exit\n"
        <<"  -t    | --target-triple  name  : set target triple,\n"
        <<"                                   use 'native' for"
          " host target triple\n"
        <<"  -l    | --data-layout   name   : set data layout,\n"
        <<"                                   use 'native' for"
          " host data layout\n"
        <<"  -o filename                    : output to this file\n"
        <<endl;
}

// main

#define ARG(str) (!strcmp(argv[i], str))
#define ARG_STARTS_WITH(str,len) (!strncmp(argv[i], str, len))
#define ARG_CONTAINS(str) strstr(argv[i], str)
#define APP_ERROR(msg) { std::cerr<<msg<<std::endl; exit(1); }

class LibraryInit
{
public:

    LibraryInit()
    {
        llvmInitialize();
    }

    ~LibraryInit()
    {
        llvmShutdown();
    }
};

int main(int argc, char *argv[])
{
    const char *outputFile = 0;
    std::string targetTriple = "native";
    std::string dataLayoutName = "native";
    vector<const char *> files;
    //typedef vector<const char *>::iterator Iter;

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
            else if (ARG("-t") || ARG("--target-triple"))
            {
                if (++i < argc)
                {
                    targetTriple = argv[i];
                }
                else
                {
                    APP_ERROR("-t option require argument");
                }
            }
            else if (ARG("-l") || ARG("--data-layout"))
            {
                if (++i < argc)
                {
                    dataLayoutName = argv[i];
                }
                else
                {
                    APP_ERROR("-l option require argument");
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

    LibraryInit init;
    if (files.size() == 0)
    {
        cerr<<"No file specified\n"<<endl;
        showUsage();
        exit(1);
    }

    if (files.size() > 1)
    {
        cerr<<"Only a single file is allowed\n"<<endl;
        showUsage();
        exit(1);
    }

    llvm::Module *module = 0;

    const char *infile = files[0];

    try
    {
        std::string errorMsg;
        module = llvmLoadModuleFromFile(infile, errorMsg);

        if (!module)
        {
            cerr<<"Could not load module from file : "<<infile<<endl;
            cerr<<errorMsg<<endl;
            exit(1);
        }

        if (targetTriple == "native" || dataLayoutName == "native")
        {
            std::string Error;
            std::string hostTriple = llvm::sys::getDefaultTargetTriple();

            const llvm::Target *hostTarget =
                    llvm::TargetRegistry::lookupTarget(hostTriple, Error);
            if (!hostTarget)
            {
                std::cerr<<"Error: "<<Error<<std::endl;
                return 1;
            }

            llvm::TargetOptions options;
            llvm::TargetMachine *tm =
                    hostTarget->createTargetMachine(hostTriple, "", "", options);
            if (!tm)
            {
                std::cerr<<"Error: Could not create target machine"<<std::endl;
                return 1;
            }

#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
            const llvm::DataLayout* dataLayout = tm->getDataLayout();
            if (!dataLayout)
            {
                std::cerr<<"Error: Could not get data layout"<<std::endl;
                return 1;
            }
#else
            const llvm::TargetData* targetData = tm->getTargetData();
            if (!targetData)
            {
                std::cerr<<"Error: Could not get target data"<<std::endl;
                return 1;
            }
#endif

            if (targetTriple == "native")
                targetTriple = hostTriple;
#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
            if (dataLayoutName == "native")
                dataLayoutName = dataLayout->getStringRepresentation();
#else
            if (dataLayoutName == "native")
                dataLayoutName = targetData->getStringRepresentation();
#endif
        }

        module->setTargetTriple(targetTriple);
        cerr<<"Set target to \""<<targetTriple<<"\""<<endl;
        module->setDataLayout(dataLayoutName);
        cerr<<"Set data layout to \""<<dataLayoutName<<"\""<<endl;
    }
    catch (std::exception &e)
    {
        cerr<<"Error: "<<e.what()<<endl;
        return 1;
    }
    std::string errorMsg;
    if (outputFile)
    {
        llvmWriteModuleToFile(module, outputFile, errorMsg);
    }
    else
    {
        llvmWriteModuleToStdout(module, errorMsg);
    }

    return 0;
}
