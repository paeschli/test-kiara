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
 * base64.h
 *
 *  Created on: Aug 22, 2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_BASE64_H_INCLUDED
#define KIARA_BASE64_H_INCLUDED

#include <KIARA/Common/Config.h>
#include <KIARA/CDT/kr_dbuffer.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Returns 1 on success and 0 otherwise. If linesize is 0 no newline characters will be added */
KIARA_API int kr_base64_encode(const void *data, size_t dataSize, kr_dbuffer_t *dest, int linesize);

/* Returns 1 on success and 0 otherwise */
KIARA_API int kr_base64_decode(const char *str, size_t len, kr_dbuffer_t *dest);

#ifdef __cplusplus
}
#endif

#endif /* KIARA_BASE64_H_INCLUDED */
