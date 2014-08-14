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
 * Lexer.hpp
 *
 *  Created on: 28.02.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMPILER_LEXER_HPP_INCLUDED
#define KIARA_COMPILER_LEXER_HPP_INCLUDED

#include "Config.hpp"
#include "Token.hpp"
#include <string>
#include <iostream>

namespace KIARA {

namespace Compiler {

class KIARA_COMPILER_API Lexer
{
public:

    Lexer(const std::string &prefix = "<>+-&", const std::string &suffix = "=>&:");
    Lexer(std::istream &in, const std::string &prefix = "<>+-&", const std::string &suffix = "=>&:");
    ~Lexer();

    void reset();
    void reset(std::istream &in);

    // returns type of the next parsed token
    TokenType next(Token &token);

    const std::string & getPrefix() const { return prefix_; }
    const std::string & getSuffix() const { return suffix_; }

    void setCXXComment(bool cxxComment)
    {
        cxxComment_ = cxxComment;
    }

    bool getCXXComment() const
    {
        return cxxComment_;
    }

private:
    std::istream *in_;
    std::string prefix_;
    std::string suffix_;
    bool cxxComment_;
    SourceLocation loc_;
};

} // namespace Compiler

} // namespace KIARA

#endif /* KIARA_COMPILER_LEXER_HPP_INCLUDED */
