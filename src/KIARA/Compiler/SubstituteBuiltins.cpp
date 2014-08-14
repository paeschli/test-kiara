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
 * SubstituteBuiltins.cpp
 *
 *  Created on: 22.03.2013
 *      Author: Dmitri Rubinstein
 */
#define KIARA_COMPILER_LIB
#include "SubstituteBuiltins.hpp"
#include "IRUtils.hpp"
#include "KIARA/Common/TypeTraits.hpp"
#include "KIARA/Core/Exception.hpp"
#include "KIARA/DB/Attributes.hpp"

namespace KIARA
{

namespace Compiler
{

SubstituteBuiltins::SubstituteBuiltins()
{

}

SubstituteBuiltins::~SubstituteBuiltins()
{

}

void SubstituteBuiltins::visit(IR::CallExpr &expr)
{
    if (IR::Intrinsic::Ptr callee = dyn_cast<IR::Intrinsic>(expr.getCallee()))
    {
        if (IR::Prototype::Ptr proto = callee->getProto())
        {
            if (proto->hasAttribute("builtin"))
            {
                const std::string &builtinName = callee->getBody();
                if (builtinName == "sizeof" && callee->getNumArgs() == 1)
                {
                    // sizeof intrinsic
                    IR::IRExpr::Ptr arg = expr.getArg(0);
                    Type::Ptr sizeofType;
                    if (IR::TypeExpr::Ptr tyArg = dyn_cast<IR::TypeExpr>(arg))
                        sizeofType = tyArg->getTypeValue();
                    else
                        sizeofType = arg->getExprType();
                    sizeofType = sizeofType->getCanonicalType();
                    if (PrimType::Ptr primType = dyn_cast<PrimType>(sizeofType))
                    {
                        replaceExpr(&expr, new IR::PrimLiteral(
                                normalize_value(static_cast<size_t>(primType->getByteSize())),
                                sizeofType->getWorld()));
                        return;
                    }
                    if (dyn_cast<PtrType>(sizeofType))
                    {
                        replaceExpr(&expr, new IR::PrimLiteral(
                                normalize_value(sizeof(void*)),
                                sizeofType->getWorld()));
                        return;
                    }
                    if (sizeofType->hasAttributeValue<NativeSizeAttr>())
                    {
                        replaceExpr(&expr, new IR::PrimLiteral(
                            normalize_value(static_cast<size_t>(
                                sizeofType->getAttributeValue<NativeSizeAttr>())),
                                sizeofType->getWorld()));
                        return;
                    }
                    DFC_THROW_EXCEPTION(KIARA::Exception, "Cannot compute sizeof of type: "<<IR::IRUtils::getTypeName(sizeofType));
                }
                else
                {
                    DFC_THROW_EXCEPTION(KIARA::Exception, "Unsupported builtin intrinsic: "<<*callee);
                }
            }
        }
    }
    InheritedType::visit(expr);
}

void SubstituteBuiltins::visit(IR::Intrinsic &func)
{
    // delete sizeof ?
    InheritedType::visit(func);
}

} // namespace Compiler

} // namespace KIARA
