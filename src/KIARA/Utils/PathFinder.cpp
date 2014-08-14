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
 * PathFinder.cpp
 *
 *  Created on: 22.04.2013
 *      Author: Dmitri Rubinstein
 */
#define KIARA_LIB
#include "PathFinder.hpp"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>

namespace KIARA
{

// PathFinder

PathFinder::PathFinder(bool searchInCWD)
    : searchInCWD_(searchInCWD)
{
}

PathFinder::PathFinder(const char *path, bool searchInCWD)
    : searchInCWD_(searchInCWD)
{
    if (path)
        searchPaths_.push_back(path);
}

PathFinder::PathFinder(const char *envVarName, PathFinder::FromEnvVarTag, bool searchInCWD)
    : searchInCWD_(searchInCWD)
{
    setSearchPathsFromEnvVar(envVarName);
}

void PathFinder::setSearchPathsFromPathList(const char *pathList)
{
    searchPaths_.clear();
    if (pathList)
    {
        // split directories
#ifdef _WIN32
        const char *const searchPathSep = ";";
#else
        const char *const searchPathSep = ":";
#endif
        boost::algorithm::split(searchPaths_, pathList,
                                boost::algorithm::is_any_of(
                                    searchPathSep));
    }
    else
    {
        // search in current working directory by default
        searchPaths_.push_back(".");
    }
}

void PathFinder::setSearchPathsFromEnvVar(const char *envVarName)
{
    setSearchPathsFromPathList(getenv(envVarName));
}

std::string PathFinder::findPath(
    const std::string &fileName,
    std::string *errorMsg) const
{
    boost::filesystem::path path(fileName);

    if (searchInCWD_ && boost::filesystem::exists(path))
        return fileName;

    // don't search in searchPaths list if fileName is absolute
#if BOOST_FILESYSTEM_VERSION == 3
    if (!path.is_absolute())
#else
    if (!path.is_complete())
#endif
    {
        typedef std::vector<std::string>::const_iterator SVIter;
        for (SVIter it = searchPaths_.begin(), end = searchPaths_.end();
             it != end; ++it)
        {
            path = *it;
            path /= fileName;
            if (boost::filesystem::exists(path))
                return path.string();
        }
    }

    if (errorMsg)
        *errorMsg = std::string("Could not find ")+fileName;

    return "";
}

} // namespace KIARA
