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
 * IRUtils.hpp
 *
 *  Created on: 19.02.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMPILER_IRUTILS_HPP_INCLUDED
#define KIARA_COMPILER_IRUTILS_HPP_INCLUDED

#include "Config.hpp"
#include "IR.hpp"
#include "Scope.hpp"
#include <stack>
#include <map>

namespace KIARA
{

namespace IR
{

class KIARA_COMPILER_API IRUtils
{
public:

    static std::string getObjectName(const Object::Ptr &object);

    static std::string getTypeName(const Type::Ptr &type);

    static std::string getArgumentTypeNames(ArrayRef<IR::IRExpr::Ptr> args);

    // make from expression type, variable type
    static Type::Ptr makeVariableType(const Expr::Ptr &argExpr);

    static DefExpr::Ptr createVariable(const std::string &name, const Type::Ptr &valueType);

    static bool addObjectToScope(
            const std::string &name,
            const Object::Ptr &object,
            const Compiler::Scope::Ptr &scope,
            std::string *errorMsg = 0);

    static bool addFunctionToScope(
            const IR::FunctionDefinition::Ptr &func,
            const Compiler::Scope::Ptr &scope,
            std::string *errorMsg = 0);

    static void addDefaultTypesToScope(const Compiler::Scope::Ptr &scope);

private:
    IRUtils();
    IRUtils(const IRUtils &);
    IRUtils & operator=(const IRUtils &);
};

} // namespace IR

} // namespace KIARA

#endif /* KIARA_LANG_ASTUTILS_HPP_INCLUDED */
