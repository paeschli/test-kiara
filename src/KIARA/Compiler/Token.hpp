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
 * Token.hpp
 *
 *  Created on: 01.03.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMPILER_TOKEN_HPP_INCLUDED
#define KIARA_COMPILER_TOKEN_HPP_INCLUDED

#include <KIARA/Common/stdint.h>
#include <DFC/Utils/StrUtils.hpp>
#include "SourceLocation.hpp"
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/assert.hpp>
#include <iostream>

namespace KIARA {

namespace Compiler {

enum TokenType
{
    TOK_EMPTY       = -1,
    TOK_ERROR       = -2,
    TOK_EOF         = -3,
    TOK_NAME        = -4,
    TOK_OPERATOR    = -5,
    TOK_ICONST      = -6,
    TOK_FCONST      = -7,
    TOK_SCONST      = -8
};

union TokenValue
{
    int64_t iconst;
    double fconst;
};

struct Token
{
    TokenType type;
    TokenValue value;
    std::string str;
    SourceLocation loc;

    Token()
        : type(TOK_EMPTY)
        , str()
        , loc(0, 0)
    {
        value.iconst = 0;
    }

    Token(TokenType _type, const SourceLocation &_loc)
        : type(_type)
        , str()
        , loc(_loc)
    {
        value.iconst = 0;
    }

    Token(TokenType _type, const std::string &_str, const SourceLocation &_loc)
        : type(_type)
        , str(_str)
        , loc(_loc)
    {
        value.iconst = 0;
    }

    Token(const Token &other)
        : type(other.type)
        , value(other.value)
        , str(other.str)
        , loc(other.loc)
    {
    }

    double getFloatValue() const
    {
        BOOST_ASSERT(type == TOK_FCONST);
        return value.fconst;
    }

    int64_t getIntValue() const
    {
        BOOST_ASSERT(type == TOK_ICONST);
        return value.iconst;
    }

    const char * getError() const
    {
        if (type == TOK_ERROR)
            return str.c_str();
        return "";
    }

    const std::string & getStringValue() const
    {
        BOOST_ASSERT(isSymbol() || type == TOK_SCONST);
        return str;
    }

    bool isEof() const
    {
        return type == TOK_EOF;
    }

    bool isConstant() const
    {
        return type == TOK_ICONST || type == TOK_FCONST || type == TOK_SCONST;
    }

    bool isNumericConstant() const
    {
        return type == TOK_ICONST || type == TOK_FCONST;
    }

    bool isIntegerConstant() const
    {
        return type == TOK_ICONST;
    }

    bool isFloatConstant() const
    {
        return type == TOK_FCONST;
    }

    bool isStringConstant() const
    {
        return type == TOK_SCONST;
    }

    bool isError() const { return type == TOK_ERROR; }

    typedef TokenType Token::*UnspecifiedBoolType;

    operator UnspecifiedBoolType () const
    {
        return isError() ? 0 : &Token::type;
    }

    bool isName() const
    {
        return type == TOK_NAME;
    }

    bool isOperator() const
    {
        return type == TOK_OPERATOR;
    }

    bool isSymbol() const
    {
        return type == TOK_NAME || type == TOK_OPERATOR;
    }

    bool isSymbol(const std::string &sym) const
    {
        return (type == TOK_NAME || type == TOK_OPERATOR) && str == sym;
    }

    static bool isSymbol(TokenType type)
    {
        return type == TOK_NAME || type == TOK_OPERATOR;
    }

    const SourceLocation & getLocation() const { return loc; }

    static const char * getTypeDescr(TokenType type)
    {
        switch (type)
        {
            case TOK_EMPTY:     return "empty token";
            case TOK_ERROR:     return "error";
            case TOK_EOF:       return "eof";
            case TOK_NAME:      return "name";
            case TOK_OPERATOR:  return "operator";
            case TOK_ICONST:    return "int constant";
            case TOK_FCONST:    return "float constant";
            case TOK_SCONST:    return "string constant";
            default:            break;
        }
        return "UNKNOWN";
    }

    std::string getDescr() const
    {
        switch (type)
        {
            case TOK_EMPTY:     return "empty token";
            case TOK_ERROR:     return "error "+str;
            case TOK_EOF:       return "eof";
            case TOK_NAME:      return "name '"+str+"'";
            case TOK_OPERATOR:  return "operator '"+str+"'";
            case TOK_ICONST:
                return "int constant "+boost::lexical_cast<std::string>(value.iconst);
            case TOK_FCONST:
                return "float constant "+boost::lexical_cast<std::string>(value.fconst);
            case TOK_SCONST:
                return "string constant "+DFC::StrUtils::quoted(str);
            default:
                return "unknown token ("+
                    boost::lexical_cast<std::string>(type)+
                    " '"+std::string(1, static_cast<char>(type))+"')";
        }
    }

    void set(TokenType _type, const SourceLocation &_loc)
    {
        type = _type;
        loc = _loc;
        value.iconst = 0;
        str.clear();
    }

    void set(TokenType _type, const std::string &_str, const SourceLocation &_loc)
    {
        type = _type;
        loc = _loc;
        value.iconst = 0;
        str = _str;
    }

    void setStringConst(const std::string &_str, const SourceLocation &_loc)
    {
        type = TOK_SCONST;
        loc = _loc;
        value.iconst = 0;
        str = _str;
    }

    void setFloatConst(double _value, const SourceLocation &_loc)
    {
        type = TOK_FCONST;
        loc = _loc;
        value.fconst = _value;
        str.clear();
    }

    void setIntConst(int64_t _value, const SourceLocation &_loc)
    {
        type = TOK_ICONST;
        loc = _loc;
        value.iconst = _value;
        str.clear();
    }

    void clear()
    {
        type = TOK_EMPTY;
        value.iconst = 0;
        str.clear();
        loc.line = 0;
        loc.col = 0;
    }
};

inline std::ostream & operator<<(std::ostream &out, const Token &token)
{
    out<<token.getDescr();
    return out;
}

} // namespace Compiler

} // namespace KIARA

#endif /* KIARA_COMPILER_TOKEN_HPP_INCLUDED */
