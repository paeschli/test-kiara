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
 * PrettyPrinter.cpp
 *
 *  Created on: 25.06.2013
 *      Author: Dmitri Rubinstein
 */
#define KIARA_COMPILER_LIB
#include "PrettyPrinter.hpp"
#include "IRUtils.hpp"
#include <DFC/Utils/StrUtils.hpp>

namespace KIARA
{

namespace IR
{

PrettyPrinter::PrettyPrinter()
    : isMemRef_(false), out_(&std::cout), isb_(std::cout)
{ }

PrettyPrinter::PrettyPrinter(std::ostream &out)
    : isMemRef_(false), out_(&out), isb_(out)
{ }

void PrettyPrinter::visit(IR::MemRef &expr)
{
    if (expr.getValue())
    {
        isMemRef_ = true;
        if (expr.getValue())
            apply(expr.getValue());
        isMemRef_ = false;
    }
    else
        (*out_)<<"<NULL>";
}

void PrettyPrinter::visit(Type &type)
{
    (*out_)<<IR::IRUtils::getTypeName(&type);
}

void PrettyPrinter::visit(IR::PrimLiteral &expr)
{
    switch (expr.getPrimTypeKind())
    {
        case PRIMTYPE_c_int8_t:
        case PRIMTYPE_i8:
            (*out_)<<static_cast<int>(expr.get<int8_t>()); break;
        case PRIMTYPE_c_uint8_t:
        case PRIMTYPE_u8:
            (*out_)<<static_cast<int>(expr.get<uint8_t>()); break;
        case PRIMTYPE_c_int16_t:
        case PRIMTYPE_i16:
            (*out_)<<expr.get<int16_t>(); break;
        case PRIMTYPE_c_uint16_t:
        case PRIMTYPE_u16:
            (*out_)<<expr.get<uint16_t>(); break;
        case PRIMTYPE_c_int32_t:
        case PRIMTYPE_i32:
            (*out_)<<expr.get<int32_t>(); break;
        case PRIMTYPE_c_uint32_t:
        case PRIMTYPE_u32:
            (*out_)<<expr.get<uint32_t>(); break;
        case PRIMTYPE_c_int64_t:
        case PRIMTYPE_i64:
            (*out_)<<expr.get<int64_t>(); break;
        case PRIMTYPE_c_uint64_t:
        case PRIMTYPE_u64:
            (*out_)<<expr.get<uint64_t>(); break;
        case PRIMTYPE_c_float:
        case PRIMTYPE_float:
            (*out_)<<expr.get<float>(); break;
        case PRIMTYPE_c_double:
        case PRIMTYPE_double:
        case PRIMTYPE_c_longdouble:
            (*out_)<<expr.get<double>(); break;
        case PRIMTYPE_boolean:
        case PRIMTYPE_c_bool:
            (*out_)<<(expr.get<bool>() ? "true" : "false"); break;
        case PRIMTYPE_c_nullptr:
            (*out_)<<"nullptr"; break;
        case PRIMTYPE_string:
            (*out_)<<DFC::StrUtils::quoted(expr.get<const char *>()); break;
        default:
            KIARA_UNREACHABLE("Unknown type ID");
            break;
    }
}

void PrettyPrinter::visit(IR::TypeExpr &expr)
{
    (*out_)<<IR::IRUtils::getTypeName(expr.getTypeValue());
}

void PrettyPrinter::visit(IR::DefExpr &expr)
{
    (*out_)<<expr.getName();
}

void PrettyPrinter::visit(IR::CallExpr &expr)
{
    if (IR::FunctionDefinition::Ptr fdef = expr.getCalledFunction())
    {
        const IR::Prototype::Ptr &proto = fdef->getProto();
        if (proto->isUnaryOp())
        {
            BOOST_ASSERT(expr.getNumArgs() == 1);

            (*out_)<<"("<<proto->getOperatorName();
            apply(expr.getArg(0));
            (*out_)<<")";

            return;
        }

        if (proto->isBinaryOp())
        {
            BOOST_ASSERT(expr.getNumArgs() == 2);

            (*out_)<<"(";

            apply(expr.getArg(0));

            (*out_)<<" "<<proto->getOperatorName()<<" ";

            apply(expr.getArg(1));

            (*out_)<<")";

            return;
        }

        (*out_)<<(expr.getCallee() ? proto->getName() : "<NULL>")<<"(";
    }
    else
    {
        if (expr.getCallee())
            apply(expr.getCallee());
        else
            (*out_)<<"<NULL>";
        (*out_)<<"(";
    }

    typedef std::vector<IR::IRExpr::Ptr> IRExprList;
    IRExprList::const_iterator it = expr.getArgs().begin();

    if (expr.getNumArgs() > 0)
    {
        apply(expr.getArg(0));
        ++it;
    }

    for (IRExprList::const_iterator end = expr.getArgs().end(); it != end; ++it)
    {
        (*out_)<<", ";
        apply(*it);
    }

    (*out_)<<")";
}

void PrettyPrinter::visit(IR::SymbolExpr &expr)
{
    (*out_)<<expr.getName();
}

void PrettyPrinter::visit(IR::IfExpr &expr)
{
    (*out_)<<"if ";
    if (expr.getCond())
        apply(expr.getCond());
    else
        (*out_)<<"<NULL>";
    (*out_)<<" then";

    bool isBlock = isa<IR::BlockExpr>(expr.getThen());

    if (!isBlock)
        isb_.incr_indentation(1);
    (*out_)<<"\n";
    apply(expr.getThen());
    if (!isBlock)
        isb_.incr_indentation(-1);

    //(*out_)<<"\n";

    if (expr.getElse())
    {
        (*out_)<<"\nelse";

        bool isBlock = isa<IR::BlockExpr>(expr.getElse());

        if (!isBlock)
            isb_.incr_indentation(1);
        (*out_)<<"\n";
        apply(expr.getElse());
        if (!isBlock)
            isb_.incr_indentation(-1);
        //(*out_)<<"\n";
    }
}

void PrettyPrinter::visit(IR::LoopExpr &expr)
{
    (*out_)<<"(loop";
    printBody(expr.getBody());
    (*out_)<<")";
}

void PrettyPrinter::visit(IR::ForExpr &expr)
{
    (*out_)<<"(for "<<expr.getVarName();

    if (expr.getVarType())
        (*out_)<<":"<<IR::IRUtils::getTypeName(expr.getVarType());

    (*out_)<<" = ";
    apply(expr.getStart());
    (*out_)<<", ";
    apply(expr.getEnd());

    if (expr.getStep())
    {
        (*out_)<<", ";
        apply(expr.getStep());
    }
    (*out_)<<" in";
    printBody(expr.getBody());
    (*out_)<<")";
}

void PrettyPrinter::visit(IR::LetExpr &expr)
{
    (*out_)<<"var ";
    printObject(expr.getVar());

    if (expr.getVar())
        (*out_)<<":"<<IR::IRUtils::getTypeName(expr.getVar()->getExprType());
    else
        (*out_)<<":<NULL>";

    if (expr.getInitValue())
    {
        (*out_)<<" = ";
        printObject(expr.getInitValue());
    }

    (*out_)<<" in";
    printBody(expr.getBody());
    //(*out_)<<";";
}

void PrettyPrinter::visit(IR::BlockExpr &expr)
{
    if (!expr.getName().empty())
        (*out_)<<expr.getName()<<": ";

    (*out_)<<"{\n";
    {
        isb_.incr_indentation(1);
        for (IR::BlockExpr::ExprList::const_iterator it = expr.getExprList().begin(),
                end = expr.getExprList().end(); it != end; ++it)
        {
            apply(*it);
            if (!isa<IR::BlockExpr>(*it))
                (*out_)<<";";
            (*out_)<<"\n";
        }
        isb_.incr_indentation(-1);
    }
    (*out_)<<"}";
}

void PrettyPrinter::visit(IR::BreakExpr &expr)
{
    (*out_)<<"break(";
    if (expr.getBlock())
    {
        if (IR::BlockExpr::Ptr b = dyn_cast<IR::BlockExpr>(expr.getBlock()))
        {
            (*out_)<<b->getName();
        }
        else
        {
            (*out_)<<expr.getBlock()->getTypeName();
        }
    }
    if (expr.getValue())
    {
        if (expr.getBlock())
            (*out_)<<",";
        apply(expr.getValue());
    }
    (*out_)<<")";
}

void PrettyPrinter::visit(IR::Prototype &proto)
{
    proto.StringAttributeHolder::print(*out_);
    if (proto.isUnaryOp() && !proto.getName().empty())
        (*out_)<<"unary "<<proto.getName()<<" ";
    else if (proto.isBinaryOp() && !proto.getName().empty())
        (*out_)<<"binary "<<proto.getName()<<" "<<proto.getBinaryPrecedence()<<" ";
    else
        (*out_)<<proto.getName();

    (*out_)<<"(";
    typedef std::vector<IR::Prototype::Arg>::const_iterator Iter;
    for (Iter it = proto.getArgs().begin(), end = proto.getArgs().end();
            it != end; ++it)
    {
        (*out_)<<it->first;
        if (it->second)
        {
            if (!it->first.empty())
                (*out_)<<":";
            (*out_)<<IR::IRUtils::getTypeName(it->second);
        }
        if (it+1 != end)
        {
            (*out_)<<", ";
        }
    }
    (*out_)<<")";
    if (proto.getReturnType())
    {
        (*out_)<<":";
        (*out_)<<IR::IRUtils::getTypeName(proto.getReturnType());
    }
}

void PrettyPrinter::visit(IR::Function &func)
{
    (*out_)<<"def ";
    apply(func.getProto());

    printBody(func.getBody());

    if (!isa<IR::BlockExpr>(func.getBody()))
        (*out_)<<";";
}

void PrettyPrinter::visit(IR::ExternFunction &func)
{
    if (isMemRef_)
    {
        (*out_)<<func.getName();
    }
    else
    {
        (*out_)<<"extern ";
        if (func.getProto())
            apply(func.getProto());
        (*out_)<<";";
    }
}

void PrettyPrinter::visit(IR::Intrinsic &func)
{
    if (isMemRef_)
    {
        (*out_)<<func.getName();
    }
    else
    {
        (*out_)<<"def intrinsic ";
        if (func.getProto())
            apply(func.getProto());
        (*out_)<<"\n";
        isb_.incr_indentation(1);
        (*out_)<<DFC::StrUtils::quoted(func.getBody());
        (*out_)<<";";
        isb_.incr_indentation(-1);
    }
}

void PrettyPrinter::visit(IR::FunctionDeclaration &decl)
{
    if (isMemRef_)
    {
        (*out_)<<decl.getName();
    }
    else
    {
        if (decl.isExtern())
            (*out_)<<"extern ";

        (*out_)<<"def ";
        apply(decl.getProto());
    }
}

void PrettyPrinter::visit(IR::TypeDefinition &tdef)
{
    // FIXME output typedef ?
    if (tdef.getDefinedType())
        (*out_)<<*tdef.getDefinedType();
    else
        (*out_)<<"<NULL>";
}

void PrettyPrinter::printObject(const Object::Ptr &obj)
{
    if (obj)
        apply(obj);
    else
        (*out_)<<"<NULL>";
}

void PrettyPrinter::printBody(const Object::Ptr &obj)
{
    bool indent;
    if (isa<IR::BlockExpr>(obj))
    {
        (*out_)<<" ";
        indent = false;
    }
    else
    {
        (*out_)<<"\n";
        indent = true;
    }
    if (obj)
    {
        if (indent)
            isb_.incr_indentation(1);
        apply(obj);
        if (indent)
            isb_.incr_indentation(-1);
    }
}

} // namespace Compiler

} // namespace KIARA
