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
 * SourceLocation.hpp
 *
 *  Created on: 23.03.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMPILER_SOURCELOCATION_HPP_INCLUDED
#define KIARA_COMPILER_SOURCELOCATION_HPP_INCLUDED

#include <boost/lexical_cast.hpp>

namespace KIARA {

namespace Compiler {

struct SourceLocation
{
    int line;
    int col;

    SourceLocation() : line(1), col(1) { }
    SourceLocation(int line, int pos) : line(line), col(pos) { }
};

inline std::ostream &operator<<(std::ostream &out, const SourceLocation &loc)
{
    return out << loc.line << ":" << loc.col;
}


// Stream manipulator
class source_location
{
public:
    source_location(const SourceLocation *loc) : loc_(loc) { }
    source_location(const SourceLocation &loc) : loc_(&loc) { }
    std::ostream &operator()(std::ostream &out) const
    {
        if (loc_)
            out << *loc_;
        return out;
    }
private:
    const SourceLocation *loc_;
};

inline std::ostream &operator<<(std::ostream &out, source_location loc)
{
    return loc(out);
}

} // namespace Compiler

} // namespace KIARA

#endif /* KIARA_COMPILER_SOURCELOCATION_HPP_INCLUDED */
