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
 * Preprocessor.cpp
 *
 *  Created on: 19.02.2013
 *      Author: Dmitri Rubinstein
 */

#define KIARA_COMPILER_LIB
#include "Preprocessor.hpp"
#include <DFC/Utils/StrUtils.hpp>
#include <sstream>
#include <iterator>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <algorithm>

namespace KIARA
{

Preprocessor::Preprocessor(VariableMap &vars)
    : vars_(vars)
{
}

std::string Preprocessor::getVar(const std::string &name) const
{
    std::map<std::string, std::string>::const_iterator it = vars_.find(name);
    if (it != vars_.end())
    {
        return it->second;
    }
    return "";
}


void Preprocessor::setVar(const std::string &name, const std::string &value)
{
    vars_[name] = value;
}

static inline bool isSpecialChar(char c)
{
    return (c == '\\' || c == '/' || c == '(' || c == ')' ||
            c == '{' || c == '}' || c == '[' || c == ']');
}

bool Preprocessor::hasFunction(const std::string &funcName) const
{
    return (funcName == "if" || funcName == "null" ||
        funcName == "concat" || funcName == "join" ||
        funcName == "set" || funcName == "length" || funcName == "print" ||
        funcName == "println" || funcName == "error" ||
        funcName == "errorln" || funcName == "equal" ||
        funcName == "if" || funcName == "true" ||
        funcName == "last" || funcName == "first" ||
        funcName == "null" || funcName == "quote" ||
        funcName == "getsysname");
}

bool Preprocessor::passByName(const std::string &funcName) const
{
    if (funcName == "if" || funcName == "null")
        return true;
    return false;
}

bool Preprocessor::execCall(
        const std::vector<std::string> &args,
        std::vector<std::string> &result)
{
    typedef std::vector<std::string>::const_iterator Iter;

    if (args.size() == 0)
    {
        result.assign(1, "<no arguments>");
        return false;
    }

    if (args[0] == "concat")
    {
        std::string resultStr;
        for (Iter it = args.begin()+1, et = args.end(); it != et; ++it)
        {
            resultStr+=*it;
        }
        result.assign(1, resultStr);
        return true;
    }
    else if (args[0] == "join")
    {
        if (args.size() < 2)
        {
            result.assign(1, "<join require 2 arguments : $(join separator ...)>");
            return false;
        }
        std::string resultStr;
        Iter it = args.begin()+1;
        Iter end = args.end();
        if (it != end)
            resultStr+=*(it++);
        for (; it != end; ++it)
        {
            resultStr+=args[1]+*it;
        }
        result.assign(1, resultStr);
        return true;
    }
    else if (args[0] == "set")
    {
        if (args.size() != 3)
        {
            result.assign(1, "<set require 2 arguments>");
            return false;
        }
        setVar(args[1], args[2]);
        return true;
    }
    else if (args[0] == "length")
    {
        if (args.size() != 2)
        {
            result.assign(1, "<length require 1 argument>");
            return false;
        }
        result.assign(1, boost::lexical_cast<std::string>(args[1].length()));
        return true;
    }
    else if (args[0] == "print" || args[0] == "println")
    {
        std::ostream_iterator<std::string> out_it(std::cout, " ");
        std::copy(args.begin()+1, args.end(), out_it);
        if (args[0] == "println")
        {
            std::cout<<std::endl;
        }
        return true;
    }
    else if (args[0] == "error" || args[0] == "errorln")
    {
        std::ostream_iterator<std::string> out_it(std::cerr, " ");
        std::copy(args.begin()+1, args.end(), out_it);
        if (args[0] == "errorln")
        {
            std::cerr<<std::endl;
        }
        return true;
    }
    else if (args[0] == "equal")
    {
        if (args.size() != 3)
        {
            result.assign(1, "<equal require 3 arguments: $(equal a b)>");
            return false;
        }
        if (args[1] == args[2])
            result.assign(1, "true");
        else
            result.assign(1, "");
        return true;
    }
    else if (args[0] == "if")
    {
        if (args.size() != 3 && args.size() != 4)
        {
            result.assign(1, "<if require 3 or 4 arguments: $(if conditionExpr thenExpr [elseExpr]), got "+
                    boost::algorithm::join(args, " ")+">");
            return false;
        }
        std::string cond;
        if (!substVars(args[1], cond))
        {
            result.assign(1, cond);
            return false;
        }
        if (cond == "true")
            result.assign(1, substVars(args[2]));
        else
        {
            if (args.size() == 4)
                result.assign(1, substVars(args[3]));
        }
        return true;
    }
    else if (args[0] == "last")
    {
        if (args.size() > 1)
            result.assign(1, args.back());
        return true;
    }
    else if (args[0] == "first")
    {
        if (args.size() > 1)
            result.assign(1, args[1]);
        return true;
    }
    else if (args[0] == "null")
    {
        return true;
    }
    else if (args[0] == "quote")
    {
        if (args.size() > 1)
            result.assign(1, DFC::StrUtils::quoted(args.back()));
        return true;
    }
    else if (args[0] == "getsysname")
    {
#if defined(DFC_WINDOWS)
		result.assign(1, "windows");
#elif defined(DFC_MACOSX)
		result.assign(1, "macosx");
#else
		result.assign(1, "posix");
#endif
        return true;
    }

    result.assign(1, std::string("<no such function '")+args[0]+"'>");
    return false;
}

/** Read string surrounded by single or double quotes or word
 *  that does not start with quotes and have no whitespaces and
 *  special characters.
 */
std::istream &Preprocessor::parseConst(std::istream &is, std::string &dest)
{
    if (!DFC::StrUtils::eatWhiteSpace(is))
        return is;
    char c;
    if (!is.get(c))
        return is;
    if (c == '\'' || c == '"')
    {
        const char quotingChar = c;
        dest.clear();
        while (is.get(c))
        {
            if (c == quotingChar)
                return is;

            switch (c)
            {
                case '\\':
                    if (is.get(c))
                    {
                        switch (c)
                        {
                            case 'n': c = '\n'; break;
                            case 'r': c = '\r'; break;
                            case 'a': c = '\a'; break;
                            case 't': c = '\t'; break;
                            case 'b': c = '\b'; break;
                        }
                    }
                default:
                    dest+=c;
                    break;
            }
        }
    }
    else
    {
        dest+=c;
        while (is.get(c))
        {
            if (isSpecialChar(c) || isspace(c))
            {
                is.putback(c);
                break;
            }
            dest+=c;
        }
    }
    return is;
}

bool Preprocessor::parseVar(std::istream &is, std::string &varName)
{
    char c;
    if (!is.get(c))
        return false;
    if (c != '{' && (isSpecialChar(c) || isspace(c)))
    {
        is.putback(c);
        return false;
    }

    varName.clear();
    bool stopAtBrace = c == '{';
    if (!stopAtBrace)
        varName+=c;

    while (is.get(c))
    {
        if (stopAtBrace && c == '}')
            return true;

        if (!stopAtBrace && (isSpecialChar(c) || isspace(c)))
        {
            is.putback(c);
            return true;
        }
        varName+=c;
    }
    return !stopAtBrace;
}

static inline std::string quoted(const std::string &str)
{
    if (boost::algorithm::starts_with(str, "${") ||
        boost::algorithm::starts_with(str, "$("))
        return str;
    else
        return DFC::StrUtils::quoted(str);
}

bool Preprocessor::parseCall(std::istream &is, std::vector<std::string> &result, bool evaluate)
{
    char c;
    if (!is.get(c))
        return false;
    if (c != '(')
    {
        is.putback(c);
        return false;
    }

    std::vector<std::string> args;
    std::vector<std::string> tmp;
    result.clear();
    bool evaluateArgs = evaluate;

    while (is.get(c))
    {
        if (c == ')')
        {
            if (evaluate)
            {
                if (!execCall(args, result))
                    return false;
            }
            else
            {
                std::transform(args.begin(), args.end(), args.begin(), quoted);
                result.assign(1, "$("+boost::algorithm::join(args, " ")+")");
            }
            return true;
        }
        else if (c == '$')
        {
            std::string varName;
            bool ok = parseVar(is, varName);
            if (ok)
            {
                if (evaluateArgs)
                    args.push_back(getVar(varName));
                else
                    args.push_back("${"+varName+"}");
                continue;
            }
            tmp.clear();
            ok = parseCall(is, tmp, evaluateArgs);
            if (ok)
            {
                std::copy(tmp.begin(), tmp.end(), std::inserter(args, args.end()));
                continue;
            }
            else
            {
                result.assign(1, "<Error: could not parse $("+boost::algorithm::join(args, " ")+" ...) : "+boost::algorithm::join(tmp, " ")+">");
                return false;
            }
        }
        else if (!isspace(c))
        {
            is.putback(c);
            std::string value;
            if (parseConst(is, value))
            {
                if (args.size() == 0 && !hasFunction(value))
                {
                    result.assign(1, "<Error: unknown function '"+value+"'>");
                    return false;
                }
                if (args.size() == 0 && evaluate)
                    evaluateArgs = !passByName(value);
                args.push_back(value);
                continue;
            }
        }
    }
    return false;
}

bool Preprocessor::parseCall(std::istream &is, std::ostream &out, bool evaluate)
{
    std::vector<std::string> result;
    bool ok = parseCall(is, result, evaluate);
    std::vector<std::string>::iterator it = result.begin();
    std::vector<std::string>::iterator end = result.end();
    if (it != end)
        out<<*(it++);
    for (;it != end; ++it)
        out<<" "<<*it;
    return ok;
}

bool Preprocessor::substVars(const std::string &str, std::string &resultStr)
{
    std::ostringstream result;
    std::istringstream is(str);
    char c;
    bool ok = true;

    while (is.get(c))
    {
        if (c == '$')
        {
            std::string varName;
            ok = parseVar(is, varName);
            if (ok)
            {
                result << getVar(varName);
                continue;
            }
            ok = parseCall(is, result);
            if (!ok)
                break;
        }
        else
        {
            result.put(c);
        }
    }
    resultStr = result.str();
    return ok;
}

} // namespace KIARA
