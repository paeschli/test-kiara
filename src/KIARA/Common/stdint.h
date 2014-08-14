/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2012, 2013  German Research Center for Artificial Intelligence (DFKI)
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
 * stdint.h
 *
 *  Created on: 27.07.2011
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMMON_STDINT_H_INCLUDED
#define KIARA_COMMON_STDINT_H_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER < 1600) && !defined(__clang__)
#include <KIARA/Common/MS/stdint.h>
#else
#include <stdint.h>
#endif

#endif /* KIARA_COMMON_STDINT_H_INCLUDED */
