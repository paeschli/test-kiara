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
 * kiara_module.h
 *
 *  Created on: Dec 30, 2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMPONENTS_MODULE_H_INCLUDED
#define KIARA_COMPONENTS_MODULE_H_INCLUDED

#include "kiara_directives.h"

KIARA_INFO_BEGIN
KIARA_EXPORT_TYPE(KIARA_Context)
KIARA_EXPORT_TYPE(KIARA_Connection)
KIARA_EXPORT_TYPE(KIARA_Message)
KIARA_EXPORT_TYPE(KIARA_FuncObj)
KIARA_EXPORT_TYPE(KIARA_ServiceFuncObj)
KIARA_EXPORT_TYPE(KIARA_UserType)
KIARA_EXPORT_TYPE(kr_dbuffer_t)
KIARA_EXPORT_TYPE(KIARA_BinaryStream)
KIARA_INFO_END

#endif /* KIARA_COMPONENTS_MODULE_H_INCLUDED */
