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
 * stddef.h
 *
 *  Created on: 18.10.2012
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMMON_STDDEF_H_INCLUDED
#define KIARA_COMMON_STDDEF_H_INCLUDED

#include <stddef.h>

#if defined(_MSC_VER) && !defined(__clang__)

#  ifndef _SSIZE_T_DEFINED
#    ifdef  _WIN64
typedef __int64    ssize_t;
#    else
typedef _W64 int   ssize_t;
#    endif

#    define _SSIZE_T_DEFINED
#  endif

#elif defined(__GNUC__) && defined(__STRICT_ANSI__)

/* Include unistd.h for ssize_t */
#include <unistd.h>

#endif

#endif /* KIARA_COMMON_STDDEF_H_INCLUDED */
