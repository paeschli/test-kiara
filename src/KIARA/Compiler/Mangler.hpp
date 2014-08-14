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
 * Mangler.hpp
 *
 *  Created on: 05.03.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMPILER_MANGLER_HPP_INCLUDED
#define KIARA_COMPILER_MANGLER_HPP_INCLUDED

#include "Config.hpp"
#include <KIARA/DB/Type.hpp>
#include <KIARA/Utils/ArrayRef.hpp>
#include "IR.hpp"

namespace KIARA
{

namespace Compiler
{

class KIARA_COMPILER_API Mangler
{
public:

    enum MangleMode
    {
        FUNC_PARAMETER_TYPE,
        ELEMENT_TYPE
    };

    static std::string getMangledName(const Type::Ptr &type, MangleMode mode = ELEMENT_TYPE);

    static std::string getMangledFuncPrefix(const std::string &name, int numArgs);

    static std::string getMangledFuncName(const std::string &name,
                                          ArrayRef<Type::Ptr> argTypes);

    static std::string getMangledFuncName(const std::string &name,
                                          ArrayRef<IR::IRExpr::Ptr> args);

    static std::string getMangledFuncName(const std::string &name,
                                          const Type::Ptr &arg)
    {
        Type::Ptr args[] = {arg};
        return getMangledFuncName(name, args);
    }

    static std::string getMangledFuncName(const std::string &name,
                                          const Type::Ptr &arg1,
                                          const Type::Ptr &arg2)
    {
        Type::Ptr args[] = {arg1, arg2};
        return getMangledFuncName(name, args);
    }

    static std::string getMangledFuncName(const std::string &name,
                                          ArrayRef<std::pair<std::string, Type::Ptr> > argTypes);

private:
    Mangler(const Mangler &);
    Mangler & operator=(const Mangler &);
};

} // namespace Compiler

} // namespace KIARA

#endif /* KIARA_COMPILER_MANGLER_HPP_INCLUDED */
