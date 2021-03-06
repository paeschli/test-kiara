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
 * ServerConfiguration.hpp
 *
 *  Created on: 24.04.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_UTILS_SERVERCONFIGURATION_HPP_INCLUDED
#define KIARA_UTILS_SERVERCONFIGURATION_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <string>
#include <vector>

namespace KIARA
{

class ProtocolInfo
{
public:
    std::string name;

    void clear()
    {
        name.clear();
    }
};

class TransportInfo
{
public:
    std::string name;
    std::string url;

    void clear()
    {
        name.clear();
        url.clear();
    }

};

class ServerInfo
{
public:
    std::vector<std::string> services;
    ProtocolInfo protocol;
    TransportInfo transport;

    void clear()
    {
        services.clear();
        protocol.clear();
    }
};

class KIARA_API ServerConfiguration
{
public:
    std::string info;
    std::string idlURL;
    std::string idlContents;
    std::vector<ServerInfo> servers;

    void clear();

    bool fromJSON(const std::string &json, std::string *errorMsg = 0);

    std::string toJSON() const;
};

} // namespace KIARA

#endif /* KIARA_SERVERCONFIGURATION_HPP_INCLUDED */
