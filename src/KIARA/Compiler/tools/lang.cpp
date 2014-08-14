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
 * lang.cpp
 *
 *  Created on: 28.02.2013
 *      Author: Dmitri Rubinstein
 */


// KIARA
#include <KIARA/kiara.h>
#include <KIARA/Core/LibraryInit.hpp>
#include <DFC/Utils/StrUtils.hpp>
#include <KIARA/LLVM/Utils.hpp>

// Lang
#include <KIARA/Compiler/Lexer.hpp>
#include <KIARA/Compiler/LangParser.hpp>
#include <KIARA/Compiler/LLVM/Evaluator.hpp>

// stdlib
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <map>
#include <vector>
#include <fstream>

namespace KIARA
{


} // namespace KIARA

//===----------------------------------------------------------------------===//
// "Library" functions that can be "extern'd" from user code.
//===----------------------------------------------------------------------===//

/// putchard - putchar that takes a double and returns 0.
extern "C" double putchard(double X)
{
    putchar((char) X);
    return 0;
}

/// printd - printf that takes a double prints it as "%f\n", returning 0.
extern "C" double printd(double X)
{
    printf("%f\n", X);
    return 0;
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//

void showUsage()
{
    std::cout<<"Run language compiler\n"
        "Usage:\n"
        "kiara-lang [options] files\n"
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

    KIARA::LibraryInit init;

    KIARA::World world;

    KIARA::Compiler::Lexer lexer("=<>!+-*&|/%^", "=<>&|");
    lexer.setCXXComment(false);
    std::string fileName("stdin");
    std::ifstream fin;
    if (files.size())
    {
        fileName = files[0];
        fin.open(files[0]);
        if (!fin)
        {
            std::cerr<<"Error: Could not open file: "<<files[0]<<std::endl;
            return 1;
        }
        lexer.reset(fin);
    }

    // Initialize LLVM and JIT
    KIARA::llvmInitialize();
    KIARA::llvmInitializeJIT();

#if 1 // set to 0 in order to output tokens
    KIARA::Compiler::LangParser parser(world);
    parser.initParser(lexer, fileName);

    KIARA::Compiler::Evaluator::addSymbol("printd", (void*) &printd);
    KIARA::Compiler::Evaluator::addSymbol("putchard", (void*) &putchard);

    // check exception only on Windows for better error reporting
#ifdef _WIN32
    try
#endif
    {
        KIARA::Compiler::Evaluator evaluator(parser);

        // Run the main "interpreter loop" now.
        evaluator.mainLoop();
        if (outputFile)
            evaluator.writeModule(outputFile);
    }
#ifdef _WIN32
    catch (const std::exception &ex)
    {
        std::cerr<<"!!! Exception: "<<ex.what()<<std::endl;
        throw;
    }
#endif

#else
    KIARA::Compiler::Token token;
    while (lexer.next(token) != KIARA::Compiler::TOK_EOF)
    {
        std::cout<<token<<" at "<<token.loc.line<<":"<<token.loc.col<<std::endl;
    }
#endif

    kiaraFinalize();

    return 0;
}
