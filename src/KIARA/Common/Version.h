/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2013, 2014  German Research Center for Artificial Intelligence (DFKI)
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
 * Version.h
 *
 *  Created on: 24.05.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMMON_VERSION_H_INCLUDED
#define KIARA_COMMON_VERSION_H_INCLUDED

#define KIARA_VERSION_MAJOR 0
#define KIARA_VERSION_MINOR 12
#define KIARA_VERSION_PATCH 0

#define KIARA_VERSION (KIARA_VERSION_MAJOR * 10000    \
                         + KIARA_VERSION_MINOR * 100  \
                         + KIARA_VERSION_PATCH)

#endif /* KIARA_COMMON_VERSION_H_INCLUDED */
