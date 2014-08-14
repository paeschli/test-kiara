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
 * JITConfiguration.hpp
 *
 *  Created on: Nov 8, 2013
 *      Author: Simon Moll
 */

#ifndef KIARA_DB_JITCONFIGURATION_HPP_INCLUDED
#define KIARA_DB_JITCONFIGURATION_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <string>

namespace KIARA
{

class KIARA_API JITConfiguration
{
public:
	JITConfiguration();

	bool useLegacyJIT;

    void clear();

};

} // namespace KIARA

#endif /* KIARA_DB_JITCONFIGURATION_HPP_INCLUDED */
