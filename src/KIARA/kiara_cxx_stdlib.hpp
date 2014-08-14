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
 * kiara_cxx_stdlib.hpp
 *
 *  Created on: 18.06.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_CXX_STDLIB_HPP_INCLUDED
#define KIARA_CXX_STDLIB_HPP_INCLUDED

#include <KIARA/kiara.h>
#include <KIARA/kiara_cxx_macros.hpp>

#include <string>

// std::string

namespace KIARA
{

static int std_string_SetCString(KIARA_UserType *ustr, const char *cstr)
{
    if (cstr)
        ((std::string*)ustr)->assign(cstr);
    else
        ((std::string*)ustr)->clear();
    return 0;
}

static int std_string_GetCString(KIARA_UserType *ustr, const char **cstr)
{
    *cstr = ((std::string*)ustr)->c_str();
    return 0;
}

static KIARA_UserType * std_string_Allocate(void)
{
    return (KIARA_UserType *)new std::string;
}

static void std_string_Deallocate(KIARA_UserType *value)
{
    delete (std::string*)value;
}

} // namespace KIARA

KIARA_CXX_DECL_OPAQUE_TYPE(std::string,
       KIARA_CXX_USER_API(SetCString, KIARA::std_string_SetCString)
       KIARA_CXX_USER_API(GetCString, KIARA::std_string_GetCString)
       KIARA_CXX_USER_API(AllocateType, KIARA::std_string_Allocate)
       KIARA_CXX_USER_API(DeallocateType, KIARA::std_string_Deallocate))

#endif /* KIARA_CXX_STDLIB_HPP_INCLUDED */
