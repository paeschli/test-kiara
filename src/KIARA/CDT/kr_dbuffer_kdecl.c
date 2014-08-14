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
 * kr_dbuffer_kdecl.c
 *
 *  Created on: Sep 14, 2013
 *      Author: rubinste
 */
#define KIARA_LIB
#include "kr_dbuffer_kdecl.h"

KIARA_UserType * kr_dbuffer_allocate(void)
{
    return (KIARA_UserType *)kr_dbuffer_new();
}

void kr_dbuffer_deallocate(KIARA_UserType *value)
{
    kr_dbuffer_delete((kr_dbuffer_t*)value);
}
