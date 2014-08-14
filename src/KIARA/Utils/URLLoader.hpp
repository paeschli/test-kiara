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
 * URLLoader.hpp
 *
 *  Created on: 23.04.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_UTILS_URLLOADER_HPP_INCLUDED
#define KIARA_UTILS_URLLOADER_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <string>
#include <curl/curl.h>
#include <boost/noncopyable.hpp>
#include <KIARA/DB/SecurityConfiguration.hpp>
#include <KIARA/CDT/kr_dbuffer.h>

namespace KIARA
{

class KIARA_API URLLoader : private boost::noncopyable
{
public:

    struct Connection;

    static Connection * createConnection();

    static void deleteConnection(Connection *connection);

    static bool sendData(Connection *connection, const std::string &url, const std::string &contentType,
                         const void *data, size_t dataSize, kr_dbuffer_t * dest,
                         const SecurityConfiguration *securityConfiguration = 0, std::string *errorMsg = 0);

    static bool sendData(Connection *connection, const std::string &url, const std::string &contentType,
                         const void *data, size_t dataSize, kr_dbuffer_t * dest,
                         const SecurityConfiguration &securityConfiguration, std::string *errorMsg = 0)
    {
        return sendData(connection, url, contentType, data, dataSize, dest, &securityConfiguration, errorMsg);
    }

    /**
     *  Returns true when URL contents are loaded into dest and false otherwise.
     *  When load failed and errorMsg is not NULL, error message will be
     *  assigned to errorMsg.
     */
    static bool load(const std::string &url, std::string &dest, std::string *errorMsg = 0);
    static bool load(Connection *connection, const std::string &url, std::string &dest, const SecurityConfiguration &securityConfiguration, std::string *errorMsg = 0)
    {
        return load(connection, url, dest, &securityConfiguration, errorMsg);
    }

    static bool load(Connection *connection, const std::string &url, std::string &dest, const SecurityConfiguration *securityConfiguration = 0, std::string *errorMsg = 0);

};

} // namespace KIARA

#endif /* KIARA_UTILS_URLLOADER_HPP_INCLUDED */
