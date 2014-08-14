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
 * URL.hpp
 *
 *  Created on: 2010-02-16 17:01:07 +0100
 *   Author(s): Dmitri Rubinstein
 */

#ifndef KIARA_UTILS_URL_HPP_INCLUDED
#define KIARA_UTILS_URL_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <string>
#include <vector>
#include <fstream>

// URL support

namespace KIARA
{

class KIARA_API URL
{
public:

    std::string scheme;
    std::string userInfo;
    std::string host;
    std::string port;
    std::string path;
    std::string query;
    std::string fragment;

    URL();

    URL(const std::string &url, bool normalize = true);

    std::string toString() const;

    void clear();

    bool isValid() const
    {
        return (!scheme.empty() ||
                !userInfo.empty() ||
                !host.empty() ||
                !port.empty() ||
                !path.empty() ||
                !query.empty() ||
                !fragment.empty());
    }

    // Checks string for an url scheme, that conforms to RFC 1738
    // Returns position of url's contents after 'scheme:' or
    // std::string::npos if it is not a correct url
    static size_t getAbsolutePos(const std::string &url);

    // Returns true if string argument passed is an URL and sets scheme string
    static bool getScheme(const std::string &url, std::string &scheme);

    static bool parseURL(const std::string &url,
                         std::string &scheme,
                         std::string &userInfo,
                         std::string &hostName,
                         std::string &port);

    // the same as previous, but returns true if url is an absolute one
    static bool isAbsolute(const std::string &url)
    {
        return getAbsolutePos(url) != std::string::npos;
    }

    static bool isURL(const std::string &url);

    // tries to convert path to url.
    // returns false if it is impossible, and copies path to url
    static bool localFilePathToURL(const std::string &path, std::string &url);

    static const std::string localFilePathToURL(const std::string &path)
    {
        std::string url;
        localFilePathToURL(path, url);
        return url;
    }

    // tries to convert url to local file path.
    // returns false if it is impossible, and copies url to file
    // only works if the url describes a local file
    // (returns false for remote files)
    static bool toLocalFilePath(const std::string &url, std::string &path);

    static const std::string toLocalFilePath(const std::string &url)
    {
        std::string path;
        toLocalFilePath(url, path);
        return path;
    }

    static const std::string resolve(
        const std::string &baseUrl,
        const std::string &url);

    static bool httpsEqHostAndPort(const std::string &url1, const std::string &url2);
};

} // namespace KIARA

#endif
