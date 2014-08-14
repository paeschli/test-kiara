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
 * Preprocessor.hpp
 *
 *  Created on: 19.02.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMPILER_PREPROCESSOR_HPP_INCLUDED
#define KIARA_COMPILER_PREPROCESSOR_HPP_INCLUDED

#include "Config.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <map>

namespace KIARA
{

// Simple text preprocessor
// supports variables : ${NAME}
// and function calls $(FUNCNAME arg1 arg2 ...)

class KIARA_COMPILER_API Preprocessor
{
public:

    typedef std::map<std::string, std::string> VariableMap;
    //typedef boost::function<bool (const std::vector<std::string> &args, std::vector<std::string> &result)> UserFunction;

    Preprocessor(VariableMap &vars);

    std::string getVar(const std::string &name) const;
    void setVar(const std::string &name, const std::string &value);

    /// substitute all vars in form $varname in str and store to result.
    /// if returns false, result contains error message
    bool substVars(const std::string &str, std::string &result);

    /// substitute all vars in form $varname in str.
    std::string substVars(const std::string &str)
    {
        std::string result;
        substVars(str, result);
        return result;
    }

private:
    struct Function {
    };
    VariableMap &vars_;

    bool hasFunction(const std::string &funcName) const;
    bool passByName(const std::string &funcName) const;

    bool execCall(const std::vector<std::string> &args, std::vector<std::string> &result);
    static std::istream &parseConst(std::istream &is, std::string &dest);
    bool parseVar(std::istream &is, std::string &varName);
    bool parseCall(std::istream &is, std::vector<std::string> &result, bool evaluate = true);
    bool parseCall(std::istream &is, std::ostream &out, bool evaluate = true);
};

} // namespace KIARA

#endif /* KIARA_COMPILER_PREPROCESSOR_HPP_INCLUDED */
