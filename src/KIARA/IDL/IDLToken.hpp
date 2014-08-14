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
#ifndef KIARA_IDL_IDLTOKEN_HPP_INCLUDED
#define KIARA_IDL_IDLTOKEN_HPP_INCLUDED

#include <KIARA/Common/stdint.h>
/*
#ifdef _WIN32
#include "win/inttypes.h"
#else
#include <inttypes.h>
#endif
*/
#include <string>
#include <iostream>

namespace KIARA
{

union IDLTokenValue
{
    int64_t        iconst;
    double         dconst;
};

struct IDLToken
{
    int id;
    IDLTokenValue val;
    std::string str;
    int lineNum;

    IDLToken() : id(0), str(), lineNum(0)
    {
        val.iconst = 0;
    }

    IDLToken(const IDLToken &other)
        : id(other.id)
        , val(other.val)
        , str(other.str)
        , lineNum(other.lineNum)
    {
    }

    void clear()
    {
        id = 0;
        val.iconst = 0;
        str.clear();
        lineNum = 0;
    }
};

std::ostream & operator<<(std::ostream &out, const IDLToken &token);

} // namespace KIARA

#endif
