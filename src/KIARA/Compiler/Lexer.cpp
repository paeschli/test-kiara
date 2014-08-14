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
 * Lexer.cpp
 *
 *  Created on: 28.02.2013
 *      Author: Dmitri Rubinstein
 */
#define KIARA_COMPILER_LIB
#include "Lexer.hpp"
#include <boost/lexical_cast.hpp>
#include <cerrno>
#include <cstdlib>

#if defined(_MSC_VER)
#define strtoll(nptr,  endptr, base) _strtoi64(nptr, endptr, base)
#endif

namespace KIARA {

namespace Compiler {

Lexer::Lexer(const std::string &prefix, const std::string &suffix)
    : in_(&std::cin)
    , prefix_(prefix)
    , suffix_(suffix)
    , cxxComment_(true)
    , loc_(1, 1)
{
}

Lexer::Lexer(std::istream &in, const std::string &prefix, const std::string &suffix)
    : in_(&in)
    , prefix_(prefix)
    , suffix_(suffix)
    , cxxComment_(true)
    , loc_(1, 1)
{
}

Lexer::~Lexer()
{
}

void Lexer::reset()
{
    reset(std::cin);
}

void Lexer::reset(std::istream &in)
{
    in_ = &in;
    loc_.line = 1;
    loc_.col = 1;
}

// Based on JavaScript code by Douglas Crockford
// http://javascript.crockford.com/tdop/tokens.js

// returnts type of the next parsed token
TokenType Lexer::next(Token &token)
{
    std::string str; // The string value.

    // Begin tokenization. If the source string is empty, return nothing.
    if (!in_ || !*in_)
    {
        token.set(TOK_EOF, loc_);
        return token.type;
    }

#define IO_GETCHAR() (in_->get())
#define IO_EOF() (in_->eof())
#define IO_ERROR() (!in_->good())
    // IO_UNGETCHAR does not work with newlines
#define IO_UNGETCHAR()                      \
        { in_->clear(); in_->unget(); }

#define RETURN_EOF()                        \
        {                                   \
            token.set(TOK_EOF, loc_);       \
            return token.type;              \
        }

#define RETURN_ERROR(msg)                   \
        {                                   \
            token.set(TOK_ERROR, msg, loc_);\
            return token.type;              \
        }

    SourceLocation startLoc;
    int c;
    while (true)
    {
        c = IO_GETCHAR();

        if (IO_EOF())
            RETURN_EOF();

        if (IO_ERROR())
            RETURN_ERROR("Could not read char");

        // newline
        if (c == '\n')
        {
            ++loc_.line;
            loc_.col = 1;
            continue;
        }

        // ignore whitespace.

        if (c <= ' ')
        {
            ++loc_.col;
            continue;
        }

        // name.
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
        {
            startLoc = loc_;
            ++loc_.col;
            str.assign(1, c);
            for (;;)
            {
                c = IO_GETCHAR();
                if (IO_EOF())
                    break;
                if (IO_ERROR())
                    RETURN_ERROR("Could not read name");

                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                        (c >= '0' && c <= '9') || c == '_')
                {
                    str += c;
                    ++loc_.col;
                }
                else
                {
                    IO_UNGETCHAR();
                    break;
                }
            }
            token.set(TOK_NAME, str, startLoc);
            return token.type;
        }

        // number.

        // A number cannot start with a decimal point. It must start with a digit,
        // possibly '0'.
        if (c >= '0' && c <= '9')
        {
            bool fpNum = false;

            startLoc = loc_;
            ++loc_.col;
            str.assign(1, c);

            // Look for more digits.

            for (;;)
            {
                c = IO_GETCHAR();
                if (IO_EOF())
                    break;
                if (IO_ERROR())
                    RETURN_ERROR("Could not read number");

                if ((c < '0' || c > '9') &&
                        // hex prefix
                        !((c == 'x' || c == 'X') &&
                        str.length() == 1 &&
                        str[0] == '0'))
                {
                    break;
                }
                ++loc_.col;
                str += c;
            }

            // Look for a decimal fraction part.

            if (c == '.')
            {
                fpNum = true;
                ++loc_.col;
                str += c;
                for (;;)
                {
                    c = IO_GETCHAR();
                    if (IO_EOF())
                        break;
                    if (IO_ERROR())
                        RETURN_ERROR("Could not read decimal fraction part");

                    if (c < '0' || c > '9')
                    {
                        break;
                    }

                    ++loc_.col;
                    str += c;
                }
            }

            // Look for an exponent part.

            if (c == 'e' || c == 'E')
            {
                fpNum = true;
                ++loc_.col;
                str += c;
                c = IO_GETCHAR();
                if (IO_EOF())
                    break;
                if (IO_ERROR())
                    RETURN_ERROR("Could not read exponent part");

                if (c == '-' || c == '+')
                {
                    ++loc_.col;
                    str += c;
                    c = IO_GETCHAR();
                    if (IO_EOF() || IO_ERROR())
                        RETURN_ERROR("Could not read exponent part");
                }
                if (c < '0' || c > '9')
                {
                    RETURN_ERROR("Bad exponent: "+str);
                }
                do
                {
                    ++loc_.col;
                    str += c;
                    c = IO_GETCHAR();
                    if (IO_EOF())
                        break;
                    if (IO_ERROR())
                        RETURN_ERROR("Could not read exponent part");
                } while (c >= '0' && c <= '9');
            }

            // Make sure the next character is not a letter.

            if (c >= 'a' && c <= 'z')
            {
                ++loc_.col;
                str += c;
                RETURN_ERROR("Bad number: "+str);
            }
            else
            {
                IO_UNGETCHAR();
            }

            // Convert the string value to a number. If it is finite, then it is a good
            // token.

            if (fpNum)
            {
                errno = 0;
                double value = strtod(str.c_str(), 0);
                if (errno != 0)
                    RETURN_ERROR("Bad number: "+str);
                token.setFloatConst(value, startLoc);
                return token.type;
            }

            if (str.length() > 1 && str[0] == '0' &&
                    (str[1] == 'x' || str[1] == 'X'))
            {
                // hex
                errno = 0;
                int64_t value = strtoll(str.c_str()+2, NULL, 16);
                if (errno != 0)
                    RETURN_ERROR("Bad number: "+str);
                token.setIntConst(value, startLoc);
                return token.type;
            }

            if (str.length() > 0 && str[0] == '0')
            {
                // octal
                errno = 0;
                int64_t value = strtoll(str.c_str()+1, NULL, 8);
                if (errno != 0)
                    RETURN_ERROR("Bad number: "+str);
                token.setIntConst(value, startLoc);
                return token.type;
            }

            // decimal
            errno = 0;
            int64_t value = strtoll(str.c_str(), NULL, 10);
            if (errno != 0)
                RETURN_ERROR("Bad number: "+str);
            token.setIntConst(value, startLoc);
            return token.type;
        }

        // string
        if (c == '\'' || c == '"')
        {
            startLoc = loc_;
            str.clear();
            int q = c; // The quote character.
            ++loc_.col;
            for (;;)
            {
                c = IO_GETCHAR();
                if (IO_EOF())
                    RETURN_ERROR("Unterminated string");
                if (IO_ERROR())
                    RETURN_ERROR("Could not read string");

                if (c < ' ')
                {
                    RETURN_ERROR((c == '\n' || c == '\r' ?
                            "Unterminated string" : "Control character in string"));
                }

                // Look for the closing quote.

                if (c == q)
                    break;

                // Look for escapement.

                if (c == '\\')
                {
                    ++loc_.col;
                    c = IO_GETCHAR();
                    if (IO_EOF())
                        RETURN_ERROR("Unterminated string");
                    if (IO_ERROR())
                        RETURN_ERROR("Could not read string");

                    switch (c)
                    {
                        case 'a': c = '\a'; break;
                        case 'b': c = '\b'; break;
                        case 'f': c = '\f'; break;
                        case 'n': c = '\n'; break;
                        case 'r': c = '\r'; break;
                        case 't': c = '\t'; break;
                        case 'v': c = '\v'; break;
                        case '?': c = '\?'; break;
                        case '\'': c = '\''; break;
                        case '\"': c = '\"'; break;
                        case '\\': c = '\\'; break;
                        // TODO add \\x H H
                        // TODO add "\\" O O O
                    }
                }
                str += c;
                ++loc_.col;
            }
            ++loc_.col; // for ending quote
            token.setStringConst(str, startLoc);
            return token.type;
        }

        // comment.

        if (cxxComment_ && c == '/')
        {
            c = IO_GETCHAR();
            if (!IO_EOF() && IO_ERROR())
                RETURN_ERROR("I/O error while reading");
            if (!IO_EOF() && (c == '/' || c == '*'))
            {
                loc_.col+=2;

                if (c == '/')
                {
                    for (;;)
                    {
                        c = IO_GETCHAR();
                        if (!IO_EOF() && IO_ERROR())
                            RETURN_ERROR("Could not read comment");
                        if (IO_EOF())
                            break;
                        if (c == '\n' || c == '\r')
                        {
                            ++loc_.line;
                            loc_.col = 1;
                            break;
                        }
                        ++loc_.col;
                    }
                }
                else
                {
                    // /* ... */ comment
                    c = '\0';
                    for (;;)
                    {
                        char c0 = c;
                        c = IO_GETCHAR();
                        if (!IO_EOF() && IO_ERROR())
                            RETURN_ERROR("Could not read comment");
                        if (IO_EOF())
                            RETURN_ERROR("Unterminated comment");
                        ++loc_.col;
                        if (c0 == '*' && c == '/')
                            break;
                    }
                }
                continue;
            }
            else
            {
                // undo
                c = '/';
                IO_UNGETCHAR();
            }
        }

        if (!cxxComment_ && c == '#')
        {
            ++loc_.col;
            for (;;)
            {
                c = IO_GETCHAR();
                if (!IO_EOF() && IO_ERROR())
                    RETURN_ERROR("Could not read comment");
                if (IO_EOF())
                    break;
                if (c == '\n' || c == '\r')
                {
                    ++loc_.line;
                    loc_.col = 1;
                    break;
                }
                ++loc_.col;
            }
            continue;
        }

        // combining

        if (prefix_.find(c) != std::string::npos)
        {
            startLoc = loc_;
            str.assign(1, c);
            ++loc_.col;
            while (true)
            {
                c = IO_GETCHAR();
                if (IO_EOF())
                    break;
                if (IO_ERROR())
                    RETURN_ERROR("Could not read operator");

                if (suffix_.find(c) == std::string::npos)
                {
                    IO_UNGETCHAR();
                    break;
                }
                str += c;
                ++loc_.col;
            }
            token.set(TOK_OPERATOR, str, startLoc);
            return token.type;
        }

        // single-character operator
        {
            startLoc = loc_;
            ++loc_.col;
            token.set(TOK_OPERATOR, std::string(1, c), startLoc);
            return token.type;
        }
    }
    token.set(TOK_ERROR, "Unknown char:"+std::string(1, c), loc_);
    return token.type;
}

} // namespace Compiler

} // namespace KIARA
