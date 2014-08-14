/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2012  German Research Center for Artificial Intelligence (DFKI)
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
#define KIARA_LIB
#include "IDLParserContext.hpp"
#include <boost/assert.hpp>

namespace KIARA
{

IDLParserContext::IDLParserContext(const Module::Ptr &module, const std::string &fileName)
    : scanner(0)
    , parser(0)
    , is(&std::cin)
    , fileName(fileName)
    , lineNum(0)
    , token()
    , scannerError()
    , parsingFailed(false)
    , parserErrors()
    , module_(module)
{
    BOOST_ASSERT(module.get() != 0);

    initScanner();
    initParser();
}

IDLParserContext::IDLParserContext(const Module::Ptr &module,
                                   std::istream &is,
                                   const std::string &fileName)
    : scanner(0)
    , parser(0)
    , is(&is)
    , fileName(fileName)
    , lineNum(0)
    , token()
    , scannerError()
    , parsingFailed(false)
    , parserErrors()
    , module_(module)
{
    BOOST_ASSERT(module.get() != 0);

    initScanner();
    initParser();
}

IDLParserContext::~IDLParserContext()
{
    destroyScanner();
    destroyParser();
}

} // namespace KIARA
