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
 * preprocessor.cpp
 *
 *  Created on: Nov 20, 2013
 *      Author: Dmitri Rubinstein
 */

#include "ASTParser.hpp"
#include "CodeGenerator.hpp"

#include <cstring>
#include <ctime>
#include <boost/algorithm/string.hpp>
#include <llvm/Support/raw_ostream.h>

void showUsage()
{
    llvm::outs()<<"Run preprocessor\n"
        "Usage:\n"
        "kiara-preprocessor [options] files\n"
        "\n"
        "  -h    | -help | --help         : print this and exit\n"
        "  -msvc                          : assume MSVC compiler when processing sources (you it if you use MS C/C++ standard libraries)\n"
        "  -lang language                 : setup source code language\n"
        "  -D name                        : define 'name' as macro\n"
        "  -U name                        : undefine 'name' as macro\n"
        "  -I dir                         : add the directory dir to the list of directories to be searched for"
        " header files.\n"
        "  -isystem dir                   : search dir for header files, after all directories specified by -I"
        " but before the standard system directories.\n"
        "  -o file                        : output generated code to the file.\n";
}

#define ARG(str) (!strcmp(argv[i], str))
#define ARG_STARTS_WITH(str,len) (!strncmp(argv[i], str, len))
#define ARG_CONTAINS(str) strstr(argv[i], str)
#define APP_ERROR(msg) { llvm::errs()<<msg<<"\n"; exit(1); }

std::string genRandomStr(const size_t len)
{
    static const char range[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static const size_t rangelen = sizeof(range)-1;

    std::string dest;
    dest.resize(len);

    for (size_t i = 0; i < len; ++i)
    {
        dest[i] = range[static_cast<size_t>(std::rand() * (1.0 / (RAND_MAX + 1.0)) * rangelen)];
    }

    return dest;
}

int main(int argc, char **argv)
{
    const char *outputFile = 0;
    bool printUsedIncludes = true;
    std::vector<const char *> files;
    typedef std::vector<const char *>::iterator Iter;

    srand(time(NULL)); // Init random number generator

    KIARA::ASTParser::Options options;

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
            else if (ARG("-msvc"))
            {
                options.msvcCompiler = true;
            }
            else if (ARG("-no-used-includes"))
            {
                printUsedIncludes = false;
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
            else if (ARG("-lang"))
            {
                if (++i < argc)
                {
                    std::string lang = argv[i];
                    options.cxxLang = boost::iequals(lang, "c++") ||
                        boost::iequals(lang, "cpp") ||
                        boost::iequals(lang, "cxx");
                }
                else
                {
                    APP_ERROR("-lang option require argument");
                }
            }
            else if (ARG("-D"))
            {
                if (++i < argc)
                {
                    options.defs.push_back(argv[i]);
                }
                else
                {
                    APP_ERROR("-D option require argument");
                }
            }
            else if (ARG("-U"))
            {
                if (++i < argc)
                {
                    options.undefs.push_back(argv[i]);
                }
                else
                {
                    APP_ERROR("-U option require argument");
                }
            }
            else if (ARG("-I"))
            {
                if (++i < argc)
                {
                    options.includeDirs.push_back(argv[i]);
                }
                else
                {
                    APP_ERROR("-I option require argument");
                }
            }
            else if (ARG("-isystem"))
            {
                if (++i < argc)
                {
                    options.sysIncludeDirs.push_back(argv[i]);
                }
                else
                {
                    APP_ERROR("-isystem option require argument");
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

    std::string guard = "KIARA_PP_" + genRandomStr(20) + "_H";

    llvm::raw_ostream *out = 0;

    if (outputFile)
    {
        std::string errorInfo;
        out = new llvm::raw_fd_ostream(outputFile, errorInfo);
        if (!errorInfo.empty())
        {
            llvm::errs() << "Error: " << errorInfo << "\n";
            exit(1);
        }
    }
    else
        out = &llvm::outs();

    *out << "#ifndef " << guard << "\n"
         << "#define " << guard << "\n\n"
         << "#include <KIARA/kiara.h>\n";
    if (options.cxxLang)
        *out << "#include <KIARA/kiara_cxx_macros.hpp>\n";
    else
        *out << "#include <KIARA/kiara_macros.h>\n";

    *out << "\n/* This file was generated by kiara-preprocessor tool */\n\n";

    KIARA::ASTParser::HeaderFileList headers;

    for (Iter it = files.begin(), end = files.end(); it != end; ++it)
    {
        KIARA::ASTParser astParser(options, llvm::errs());

        astParser.parseAST(*it);

        KIARA::PP::CodeGenerator codeGenerator(astParser.getASTContext());

        if (printUsedIncludes)
        {
            astParser.getIncludedFiles(headers);
            for (KIARA::ASTParser::HeaderFileList::const_iterator it = headers.begin(), end = headers.end();
                it != end; ++it)
            {
                if (it->first == KIARA::ASTParser::USER_HEADER)
                    *out << "#include \"" << it->second << "\"\n";
            }
            *out << "\n";
        }

        codeGenerator.scan();

        codeGenerator.generate(*out);

        headers.clear();
    }

    *out << "\n#endif\n";
    if (outputFile)
        delete out;
}
