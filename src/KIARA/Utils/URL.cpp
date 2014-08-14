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
 * URL.cpp
 *
 *  Created on: 2010-02-16 17:01:07 +0100
 *   Author(s): Dmitri Rubinstein
 */

#define KIARA_LIB
#include <KIARA/Common/Config.hpp>

#include "URL.hpp"
#include <DFC/Base/Utils/FileSystem.hpp>

#include <map>
#include <cctype>
#include <uriparser/Uri.h>

// #define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

namespace KIARA
{

using namespace std;

URL::URL()
{
}

URL::URL(const std::string &url, bool normalize)
{
    UriParserStateA state;
    UriUriA parsedUri;
    state.uri = &parsedUri;
    if (uriParseUriA(&state, url.c_str()) == URI_SUCCESS)
    {
        if (normalize)
        {
            if (uriNormalizeSyntaxA(&parsedUri) != URI_SUCCESS)
            {
                uriFreeUriMembersA(&parsedUri);
                return;
            }
        }

        scheme.assign(parsedUri.scheme.first, parsedUri.scheme.afterLast);

        if (parsedUri.userInfo.first)
            userInfo.assign(parsedUri.userInfo.first, parsedUri.userInfo.afterLast);

        if (parsedUri.hostText.first)
            host.assign(parsedUri.hostText.first, parsedUri.hostText.afterLast);

        if (parsedUri.portText.first)
            port.assign(parsedUri.portText.first, parsedUri.portText.afterLast);

        if (parsedUri.pathHead)
        {
            if (parsedUri.absolutePath)
                path = "/";

            UriPathSegmentA *p = parsedUri.pathHead;
            while (p)
            {
                if (p->text.first)
                {
                    path.append(p->text.first, p->text.afterLast);
                    if (p->next && p->next->text.first)
                        path += "/";
                }
                p = p->next;
            }
        }

        if (parsedUri.query.first)
            query.assign(parsedUri.query.first, parsedUri.query.afterLast);

        if (parsedUri.fragment.first)
            query.assign(parsedUri.fragment.first, parsedUri.fragment.afterLast);
    }
    uriFreeUriMembersA(&parsedUri);
}

std::string URL::toString() const
{
    std::string url;
    if (!scheme.empty())
        url = scheme + ":";
    if (!host.empty())
    {
        url += "//";
        if (!userInfo.empty())
        {
            url += userInfo + "@";
        }
        url += host;
        if (!port.empty())
        {
            url += ":" + port;
        }
    }
    if (!path.empty())
    {
        if (path[0] != '/')
            url += '/';
        url += path;
    }
    if (!fragment.empty())
        url += "#" + fragment;
    if (!query.empty())
        url += "?" + query;
    return url;
}

void URL::clear()
{
    scheme.clear();
    userInfo.clear();
    host.clear();
    port.clear();
    path.clear();
    query.clear();
    fragment.clear();
}

size_t URL::getAbsolutePos(const string &url)
{
    size_t colon = url.find(':');
    if (colon == 0 || colon == string::npos)
    {
        return string::npos;
    }
    // Check all chars between 0 and colon
    for (size_t pos = 0; pos != colon; pos++)
    {
        char ch = url[pos];
        if (!(isascii(ch) &&
              (isalnum(ch) || ch == '+' || ch == '-' || ch == '.')))
            return string::npos;
    }
    return colon+1;
}

bool URL::getScheme(const std::string &url, std::string &scheme)
{
    size_t afterScheme = getAbsolutePos(url);
    if (afterScheme == std::string::npos)
        return false;
    scheme.assign(url.begin(), url.begin()+afterScheme-1);
    return true;
}

bool URL::parseURL(const std::string &url,
                   std::string &scheme,
                   std::string &userInfo,
                   std::string &hostName,
                   std::string &port)
{
    bool result = false;
    UriParserStateA state;
    UriUriA parsedUri;
    state.uri = &parsedUri;
    if (uriParseUriA(&state, url.c_str()) == URI_SUCCESS)
    {
        scheme.assign(parsedUri.scheme.first, parsedUri.scheme.afterLast);

        if (parsedUri.userInfo.first)
            userInfo.assign(parsedUri.userInfo.first, parsedUri.userInfo.afterLast);
        else
            userInfo.clear();

        if (parsedUri.hostText.first)
            hostName.assign(parsedUri.hostText.first, parsedUri.hostText.afterLast);
        else
            hostName.clear();

        if (parsedUri.portText.first)
            port.assign(parsedUri.portText.first, parsedUri.portText.afterLast);
        else
            port.clear();

        result = true;
    }
    uriFreeUriMembersA(&parsedUri);

    return result;
}


bool URL::isURL(const std::string &url)
{
    if (url.size() == 0)
        return false;
    size_t pathPos = getAbsolutePos(url);
    if (pathPos == std::string::npos)
        pathPos = 0;

    // Following test is only meaningful for encoding URL for transfer
    // so we accept everything currently. Also see RFC 1738.
#if 0
    const size_t len = url.size();
    for (size_t i = pathPos; i < len; i++)
    {
        const char ch = url[i];
        if (!::isprint(ch) || ch == '{' || ch == '}' || ch == '|' ||
            ch == '\\' || ch == '^' || ch == '~' || ch == '[' ||
            ch == ']' || ch == '`')
            return false;
    }
#endif
    return true;
}

bool URL::localFilePathToURL(const string &path, string &url)
{
    if (path.size() == 0)
        return false;
    if (DFC::FileSystem::isAbsolutePath(path))
        url = string("file://") + DFC::FileSystem::convertNativeToUnixPath(path);
    else
        url = DFC::FileSystem::convertNativeToUnixPath(path);
    return true;
}

bool URL::toLocalFilePath(const string &url, string &path)
{
    if (url.size() == 0)
        return false;
#if defined(DFC_WINDOWS)
    if (DFC::FileSystem::isAbsoluteWinPath(url) || DFC::FileSystem::isWinPath(url))
    {
        path = url;
        return true;
    }
#endif
    if (!isAbsolute(url))
    {
        path = DFC::FileSystem::convertUnixToNativePath(url);
        return true;
    }
    if (url.compare(0, 5, "file:") == 0)
    {
        string urlPath(url, 5);
        // check for forms :
        // file://localhost/path -> /path
        // file:///path          -> /path
        // file://xxxx/path      -> //xxxx/path (this not really correct)
        if (urlPath.compare(0, 12, "//localhost/") == 0)
        {
            // '/' after localhost is included
            path.assign(urlPath, 11, string::npos);
        }
        else if (urlPath.compare(0, 3, "///") == 0)
        {
            path.assign(urlPath, 2, string::npos);
        }
        else
            path.assign(urlPath);
        path = DFC::FileSystem::convertUnixToNativePath(path);
        return true;
    }
    return false; // if protocol is not "file:"
}

// Note: this code is probably not really conform with URI/URL spec.
const std::string URL::resolve(
    const std::string &baseUrl,
    const std::string &url)
{
    if (isAbsolute(url))
        return url; // url is absolute URL
    size_t slash;
    if (url.size() && url[0] == '/')
    {
        // url is absolute path
        DFC_DEBUG(url<<" is absolute path");
        slash = baseUrl.find('/'); // it's a first one,

        DFC_DEBUG("slash == "<<slash);
        DFC_DEBUG("getAbsPos(url) == "<<getAbsolutePos(baseUrl));

        if (slash == string::npos ||
            slash == 0 ||
            getAbsolutePos(baseUrl) != slash)
            return url;

        // baseUrl looks like "protocol:/.*"
        DFC_DEBUG(baseUrl<<" looks like 'protocol:/.*'");
        if (slash+1 < baseUrl.size() && baseUrl[slash+1] == '/')
        {
            slash++;
            // baseUrl looks like "protocol://.*"
            DFC_DEBUG(baseUrl<<" looks like 'protocol://.*'");
            size_t nslash = (slash+1 < baseUrl.size() ?
                             baseUrl.find('/', slash+1) : string::npos);
            if (nslash == string::npos)
            {
                // baseUrl looks like "protocol://[^/]*"
                DFC_DEBUG(baseUrl<<" looks like 'protocol://[^/]*'");
                // we assume that this means "protocol://[user@]host"
                return baseUrl + url;
            }
            // baseUrl looks like "protocol://[^/]*/"
            DFC_DEBUG(baseUrl<<" looks like 'protocol://[^/]*/'");
            if (slash+1 == nslash) {
                // baseUrl looks like "protocol:///.*"
                DFC_DEBUG(baseUrl<<" looks like 'protocol:///.*'");
                return baseUrl.substr(0, slash-1).append(url); // "protocol:url"
            }
            // baseUrl looks like "protocol://[^/]+/.*"
            DFC_DEBUG(baseUrl<<" looks like 'protocol://[^/]+/.*'");
            return baseUrl.substr(0, nslash).append(url);
        }
        // baseUrl looks like "protocol:/[^/]*"
        DFC_DEBUG(baseUrl<<" looks like 'protocol:/[^/]*'");
        return baseUrl.substr(0, slash).append(url);
    }
    // url is relative path
    slash = baseUrl.rfind('/');
    if (slash == string::npos) return url;
    return baseUrl.substr(0, slash+1).append(url);
}


// check for host and port equality for
// https://host:port/xxx and https://host:port/yyy
bool URL::httpsEqHostAndPort(const std::string &url1, const std::string &url2)
{
    if ((url1.compare(0, 8, "https://") == 0) &&
        (url2.compare(0, 8, "https://") == 0))
    {
        string urlAdr1(url1, 8);
        string urlAdr2(url2, 8);

        size_t slashPos1 = urlAdr1.find('/');
        if (slashPos1 == 0 || slashPos1 == string::npos)
        {
            return false;
        }

        size_t slashPos2 = urlAdr2.find('/');
        if (slashPos2 == 0 || slashPos2 == string::npos || (!(slashPos1 == slashPos2)))
        {
            return false;
        }
        for (size_t i = 0; i < slashPos1; i++)
        {
            const char ch1 = urlAdr1[i];
            const char ch2 = urlAdr2[i];
            if (!(ch1 == ch2))
               return false;
        }
        return true;
    }
    else
        return false;
}

} // namespace KIARA
