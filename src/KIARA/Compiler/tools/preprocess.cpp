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
 * preprocess.cpp
 *
 *  Created on: 12.03.2013
 *      Author: Dmitri Rubinstein
 */
// KIARA
#include <KIARA/Core/LibraryInit.hpp>
#include <DFC/Utils/StrUtils.hpp>
#include <KIARA/LLVM/Utils.hpp>
#include <cstdlib>
#include <cstdio>
#include <KIARA/Compiler/Preprocessor.hpp>

void showUsage()
{
    std::cout<<"Run preprocessor\n"
        "Usage:\n"
        "kiara-macro-preprocessor [options] files\n"
        "\n"
        "  -h    | -help | --help         : prints this and exit\n"
        "  -o filename                    : output LLVM module to file"
        <<std::endl;
}

#define ARG(str) (!strcmp(argv[i], str))
#define ARG_STARTS_WITH(str,len) (!strncmp(argv[i], str, len))
#define ARG_CONTAINS(str) strstr(argv[i], str)
#define APP_ERROR(msg) { std::cerr<<msg<<std::endl; exit(1); }

bool readFile(const char *filename, std::string &contents)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp)
        return false;

    fseek(fp, 0, SEEK_END);
    contents.resize(std::ftell(fp));
    rewind(fp);
    fread(&contents[0], 1, contents.size(), fp);
    fclose(fp);
    return true;
}

int main(int argc, char **argv)
{
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

    std::string contents;
    for (std::vector<const char *>::const_iterator it = files.begin(), end =files.end();
            it != end; ++it)
    {
        if (!readFile(*it, contents))
        {
            std::cerr<<"Could not read file "<<*it<<std::endl;
            continue;
        }

        // preprocess assembly
        KIARA::Preprocessor::VariableMap ppvars;
        ppvars["name"] = "NAME";
        ppvars["mangledName"] = "MANGLEDNAME";

        KIARA::Preprocessor pp(ppvars);
        std::cout<<pp.substVars(contents)<<std::endl;
    }

    return 0;
}
