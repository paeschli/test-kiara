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
 * SubstituteBuiltins.hpp
 *
 *  Created on: 22.03.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMPILER_SUBSTITUTEBUILTINS_HPP_INCLUDED
#define KIARA_COMPILER_SUBSTITUTEBUILTINS_HPP_INCLUDED

#include "Config.hpp"
#include "IRTransformer.hpp"
#include "IRReplacer.hpp"

namespace KIARA
{

namespace Compiler
{

class SubstituteBuiltins : public IR::IRReplacer<SubstituteBuiltins>
{
public:
    typedef IR::IRReplacer<SubstituteBuiltins> InheritedType;
    using InheritedType::visit;
    using InheritedType::apply;

    KIARA_COMPILER_API SubstituteBuiltins();
    KIARA_COMPILER_API ~SubstituteBuiltins();

    KIARA_COMPILER_API void visit(IR::CallExpr &expr);
    KIARA_COMPILER_API void visit(IR::Intrinsic &func);
};

} // namespace Compiler

} // namespace KIARA


#endif /* KIARA_COMPILER_SUBSTITUTEBUILTINS_HPP_INCLUDED */
