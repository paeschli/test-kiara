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
 * LibraryConfiguration.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: Dmitri Rubinstein
 */
#define KIARA_LIB
#include "LibraryConfiguration.hpp"
#include <KIARA/DB/ValueIO.hpp>
#include <DFC/Base/Utils/FileSystem.hpp>
#include <boost/filesystem.hpp>
#include <boost/assert.hpp>
#include <boost/algorithm/string.hpp>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>

namespace KIARA
{

static void removeArguments(int firstArg, int num, int &argc, char **argv)
{
    BOOST_ASSERT(firstArg >= 0 && firstArg + num <= argc && argc >= 0 && argv != 0);

    for (int i = firstArg + num; i < argc; i++)
        argv[i - num] = argv[i];
    argc -= num;
}

void LibraryConfiguration::clear()
{
    configFiles.clear();
    moduleSearchPath.clear();
    config.setNull();
}

static std::string makeNativePath(const std::string &path)
{
    boost::filesystem::path p(path);
    return DFC::FileSystem::expandUser(p.string());
}

void LibraryConfiguration::setToDefaults()
{
    clear();

    configFiles.push_back(makeNativePath("~/.kiara/main.cfg"));

    if (char *val = getenv("KIARA_MODULE_PATH"))
    {
        moduleSearchPath = val;
    }
}

SecurityConfiguration LibraryConfiguration::getSecurityConfiguration() const
{
    // read following entries
    // security.certFile
    // security.keyFile
    // security.caCertFile
    SecurityConfiguration sc;

    if (config.isDict())
    {
        const DictValue &dict = config.getDict();
        DictValue::const_iterator it = dict.find("security");
        if (it != dict.end())
        {
            const Value &security = it->second;
            if (security.isDict())
            {
                const DictValue &securityDict = security.getDict();
                it = securityDict.find("certFile");
                if (it != securityDict.end() && it->second.isString())
                {
                    sc.certFile = makeNativePath(it->second.getString());
                }
                it = securityDict.find("keyFile");
                if (it != securityDict.end() && it->second.isString())
                {
                    sc.keyFile = makeNativePath(it->second.getString());
                }
                it = securityDict.find("caCertFile");
                if (it != securityDict.end() && it->second.isString())
                {
                    sc.caCertFile = makeNativePath(it->second.getString());
                }

                // FIXME: This is totally unsafe, password should be not stored
                //        in plain text.
                it = securityDict.find("encryptionPassword");
                if (it != securityDict.end() && it->second.isString())
                {
                    sc.encryptionPassword = it->second.getString();
                }
                else
                {
                    sc.encryptionPassword = "password";
                }
            }
        }
    }

    return sc;
}

JITConfiguration LibraryConfiguration::getJITConfiguration() const
{
    // read following entries
    // security.certFile
    // security.keyFile
    // security.caCertFile
    JITConfiguration jc;

    if (config.isDict())
    {
        const DictValue &dict = config.getDict();
        DictValue::const_iterator it = dict.find("jit");
        if (it != dict.end())
        {
            const Value &jit = it->second;
            if (jit.isDict())
            {
                const DictValue &jitDict = jit.getDict();
                it = jitDict.find("useLegacyJIT");

                if (it != jitDict.end() && it->second.isBool())
                {
                    jc.useLegacyJIT = it->second.getBool();
                }
            }
        }
    }

    // Environment has precedence over configuration files
    char *jitEngine = ::getenv("KIARA_JIT_ENGINE");
    if (jitEngine)
    {
        if (boost::algorithm::iequals(jitEngine, "JIT"))
            jc.useLegacyJIT = true;
        else if (boost::algorithm::iequals(jitEngine, "MCJIT"))
            jc.useLegacyJIT = false;
    }

    return jc;
}

#undef CLERROR
#undef ARG
#undef ARG_STARTS_WITH
#undef ARG_CONTAINS

#define CLERROR(msg) std::cerr<<"Error : "<<msg<<std::endl; return
#define ARG() (argv[i])
#define ARG_IS(str) (!strcmp(argv[i], str))
#define ARG_STARTS_WITH(str,len) (!strncmp(argv[i], str, len))
#define ARG_CONTAINS(str) strstr(argv[i], str)

void LibraryConfiguration::parseCommandLine(int *argc, char **argv)
{
    if (!argc)
        return;

    bool configFilesCleared = false;

    for (int i = 1; i < *argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (ARG_IS("-kiara-help"))
            {
                removeArguments(i--, 1, *argc, argv);
                std::cerr<<"KIARA supports following command line arguments:"<<std::endl;
                printSupportedArguments(std::cerr);
            }
            else if (ARG_IS("-kiara-cfg") || ARG_IS("-kiara-config"))
            {
                if (i+1 < *argc)
                {
                    // clear list of config files when they are specified at the command line
                    if (!configFilesCleared)
                    {
                        configFiles.clear();
                        configFilesCleared = true;
                    }
                    configFiles.push_back(makeNativePath(std::string(argv[i+1])));
                    removeArguments(i--, 2, *argc, argv);
                }
                else
                {
                    CLERROR(ARG()<<" option require argument");
                }
            }
            else if (ARG_IS("-kiara-module-path"))
            {
                if (i+1 < *argc)
                {
                    moduleSearchPath = makeNativePath(std::string(argv[i+1]));
                    removeArguments(i--, 2, *argc, argv);
                }
                else
                {
                    CLERROR(ARG()<<" option require argument");
                }
            }
            else if (ARG_IS("--"))
            {
                break;
            }
        }
    }
}

void LibraryConfiguration::printSupportedArguments(std::ostream &out)
{
    out << "-kiara-cfg | -kiara-config           : specify configuration file\n"
           "-kiara-module-path                   : module search path (by default value of environment variable\n"
           "                                       KIARA_MODULE_PATH will be used)\n";
    out << std::flush;
}

static bool readFileToString(const char *filename, std::string &contents)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp)
        return false;

    fseek(fp, 0, SEEK_END);
    contents.resize(std::ftell(fp));
    rewind(fp);
    if (fread(&contents[0], contents.size(), 1, fp) != 1)
        return false;
    fclose(fp);
    return true;
}

bool LibraryConfiguration::load(std::string *errorMsg)
{
    // FIXME currently we support only a single file
    if (configFiles.empty())
        return true;
    std::string configFilePath = configFiles[0];
    if (!boost::filesystem::exists(configFilePath))
        return true;
    return loadFromFile(configFilePath, errorMsg);
}

bool LibraryConfiguration::loadFromFile(const std::string &fileName, std::string *errorMsg)
{
    std::string configContents;
    if (!readFileToString(fileName.c_str(), configContents))
    {
        if (errorMsg)
            *errorMsg = "Could not read file '"+fileName+"'";
        return false;
    }

    if (!ValueIO::fromJSON(configContents, config, errorMsg))
        return false;

    return true;
}

} // namespace KIARA
