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
 * PathFinder.hpp
 *
 *  Created on: 22.04.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_UTILS_PATHFINDER_HPP_INCLUDED
#define KIARA_UTILS_PATHFINDER_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <string>
#include <vector>

namespace KIARA
{

class KIARA_API PathFinder
{
public:

    struct FromEnvVarTag { };

    PathFinder(bool searchInCWD = false);

    PathFinder(const char *path, bool searchInCWD = false);

    PathFinder(const char *envVarName, FromEnvVarTag, bool searchInCWD = false);

    bool isSearchInCWD() const { return searchInCWD_; }

    void setSearchInCWD(bool value) { searchInCWD_ = value; }

    void clearSearchPaths()
    {
        searchPaths_.clear();
    }

    void appendSearchPath(const std::string &path)
    {
        searchPaths_.push_back(path);
    }

    void setSearchPathsFromEnvVar(const char *envVarName);

    /* Path list is a string separated by platform-specific path separator character
     * ':' on *nix
     * ';' on Windows
     */
    void setSearchPathsFromPathList(const char *pathList);

    std::string findPath(const std::string &fileName, std::string *errorMsg = 0) const;

private:
    bool searchInCWD_;
    std::vector<std::string> searchPaths_;
};

} // namespace KIARA

#endif /* KIARA_UTILS_PATHFINDER_HPP_INCLUDED */
