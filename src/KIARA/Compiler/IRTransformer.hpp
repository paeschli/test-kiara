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
 * IRTransformer.hpp
 *
 *  Created on: 20.03.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMPILER_IRTRANSFORMER_HPP_INCLUDED
#define KIARA_COMPILER_IRTRANSFORMER_HPP_INCLUDED

#include "Config.hpp"
#include "IR.hpp"

namespace KIARA
{

namespace IR
{

template <typename SubClass>
class IRTransformer : public ExprVisitor<SubClass, Object::Ptr>
{
public:

    typedef ExprVisitor<SubClass, Object::Ptr> InheritedType;
    using InheritedType::visit;
    using InheritedType::apply;

    void clearResultCache()
    {
        resultCache_.clear();
    }

    Object::Ptr apply(KIARA::Object &expr)
    {
        ResultCache::iterator it = resultCache_.find(&expr);
        if (it != resultCache_.end())
        {
            //std::cerr<<"XFM: don't process: "<<&expr<<" "<<expr.getTypeName()<<": "<<expr<<std::endl;
            return it->second;
        }

        // processing
        //std::cerr<<"XFM: start process: "<<&expr<<" "<<expr.getTypeName()<<": "<<expr<<" {"<<std::endl;
        Object::Ptr result = InheritedType::apply(expr);
        //std::cerr<<"XFM: end process "<<&expr<<" "<<expr.getTypeName()<<"}"<<std::endl;

        resultCache_[&expr] = result;
        return result;
    }

    // Apply transform to the expression expr of type ExprType,
    // and store transfomed expression to the new exprVar variable.
    #define _APPLY_TRANSFORM(ExprType, exprVar, expr)                   \
        ExprType::Ptr exprVar = expr;                                   \
        if (exprVar)                                                    \
        {                                                               \
            ExprType::Ptr newExpr = dyn_cast<ExprType>(apply(exprVar)); \
            if (!newExpr)                                               \
                return 0;                                               \
            changed |= exprVar != newExpr;                              \
            exprVar = newExpr;                                          \
        }

    Object::Ptr visit(Type &type)
    {
        // leaf
        return &type;
    }

    Object::Ptr visit(PrimLiteral &expr)
    {
        // leaf
        return &expr;
    }

    Object::Ptr visit(IR::TypeExpr &expr)
    {
        // leaf
        return &expr;
    }

    Object::Ptr visit(IR::DefExpr &expr)
    {
        // leaf
        return &expr;
    }

    Object::Ptr visit(IR::MemRef &expr)
    {
        bool changed = false;

        _APPLY_TRANSFORM(IRExpr, value, expr.getValue());

        if (changed)
            return value->getReference();
        return &expr;
    }

    Object::Ptr visit(IR::CallExpr &expr)
    {
        bool changed = false;
        // Don't process callee otherwise we will have infinite loop with recursive functions
    #if 0
        FunctionDefinition::Ptr callee = dyn_cast<FunctionDefinition>(apply(expr.getCallee()));
        if (!callee)
            return 0;
        changed |= callee != expr.getCallee();
    #else
        IR::IRExpr::Ptr callee = expr.getCallee();
    #endif
        std::vector<IRExpr::Ptr> args(expr.getNumArgs());
        for (size_t i = 0; i < expr.getNumArgs(); ++i)
        {
            const IRExpr::Ptr oldArg = expr.getArg(i);
            IRExpr::Ptr &arg = args[i];
            arg = dyn_cast<IRExpr>(apply(oldArg));
            if (!arg)
                return 0;
            changed |= arg != oldArg;
        }
        if (changed)
            return new IR::CallExpr(callee, args);
        return &expr;
    }

    Object::Ptr visit(IR::SymbolExpr &expr)
    {
        // leaf
        return &expr;
    }

    Object::Ptr visit(IR::IfExpr &expr)
    {
        bool changed = false;

        _APPLY_TRANSFORM(IRExpr, cond, expr.getCond());
        _APPLY_TRANSFORM(IRExpr, then, expr.getThen());
        _APPLY_TRANSFORM(IRExpr, _else, expr.getElse());

        if (changed)
            return new IR::IfExpr(cond, then, _else, expr.getWorld());
        return &expr;
    }

    Object::Ptr visit(IR::LoopExpr &expr)
    {
        bool changed = false;

        _APPLY_TRANSFORM(IRExpr, body, expr.getBody());

        if (changed)
            return new IR::LoopExpr(body);
        return &expr;
    }

    Object::Ptr visit(IR::ForExpr &expr)
    {
        bool changed = false;

        _APPLY_TRANSFORM(DefExpr, var, expr.getVar());
        _APPLY_TRANSFORM(IRExpr, start, expr.getStart());
        _APPLY_TRANSFORM(IRExpr, end, expr.getEnd());
        _APPLY_TRANSFORM(IRExpr, step, expr.getStep());
        _APPLY_TRANSFORM(IRExpr, body, expr.getBody());

        if (changed)
            return new IR::ForExpr(var, start, end, step, body);
        return &expr;
    }

    Object::Ptr visit(IR::LetExpr &expr)
    {
        bool changed = false;

        _APPLY_TRANSFORM(DefExpr, var, expr.getVar());
        _APPLY_TRANSFORM(IRExpr, initValue, expr.getInitValue());
        _APPLY_TRANSFORM(IRExpr, body, expr.getBody());
        if (changed)
            return new IR::LetExpr(var, initValue, body);
        return &expr;
    }

    Object::Ptr visit(IR::BlockExpr &expr)
    {
        bool changed = false;
        IR::BlockExpr::ExprList exprList(expr.getExprList());
        for (size_t i = 0; i < exprList.size(); ++i)
        {
            const IRExpr::Ptr oldExpr = expr.getExprList()[i];
            IRExpr::Ptr &newExpr = exprList[i];
            newExpr = dyn_cast<IRExpr>(apply(oldExpr));
            if (!newExpr)
                return 0;
            changed |= newExpr != oldExpr;
        }
        if (changed)
        {
            IR::BlockExpr::Ptr block = new IR::BlockExpr(exprList, expr.getName());
            blockCache_[&expr] = block;
            return block;
        }
        return &expr;
    }

    Object::Ptr visit(IR::BreakExpr &expr)
    {
        bool changed = false;

        _APPLY_TRANSFORM(IRExpr, value, expr.getValue());
        // cannot apply to block, it causes infinite loop
        // _APPLY_TRANSFORM(IRExpr, block, expr.getBlock());
        IRExpr::Ptr block = expr.getBlock();

        BlockCache::iterator it = blockCache_.find(block);
        if (it != blockCache_.end())
        {
            block = it->second;
            changed = true;
        }

        if (changed)
            return new IR::BreakExpr(block, value);
        return &expr;
    }

    Object::Ptr visit(IR::Prototype &proto)
    {
        // TODO should we process types as well ?
        return &proto;
    }

    Object::Ptr visit(IR::Function &func)
    {
        bool changed = false;

        _APPLY_TRANSFORM(Prototype, proto, func.getProto());
        _APPLY_TRANSFORM(IRExpr, body, func.getBody());

        if (changed)
            return new IR::Function(proto, body);
        return &func;
    }

    Object::Ptr visit(IR::ExternFunction &func)
    {
        bool changed = false;

        _APPLY_TRANSFORM(Prototype, proto, func.getProto());

        if (changed)
            return new IR::ExternFunction(proto);
        return &func;
    }

    Object::Ptr visit(IR::Intrinsic &func)
    {
        bool changed = false;

        _APPLY_TRANSFORM(Prototype, proto, func.getProto());

        if (changed)
            return new IR::Intrinsic(proto, func.getBody(), func.getWorld());
        return &func;
    }

    Object::Ptr visit(IR::FunctionDeclaration &decl)
    {
        bool changed = false;

        _APPLY_TRANSFORM(Prototype, proto, decl.getProto());

        if (changed)
            return new IR::FunctionDeclaration(proto, decl.isExtern());
        return &decl;
    }

    Object::Ptr visit(IR::TypeDefinition &decl)
    {
        bool changed = false;

        _APPLY_TRANSFORM(Type, type, decl.getDefinedType());

        if (changed)
            return new IR::TypeDefinition(decl.getDefinedTypeName(), type);
        return &decl;
    }

private:
    typedef std::map<Object*, Object::Ptr> ResultCache;
    ResultCache resultCache_;
    typedef std::map<Object::Ptr, IR::BlockExpr::Ptr> BlockCache;
    BlockCache blockCache_;
};

} // namespace IR

} // namespace KIARA

#endif /* KIARA_COMPILER_IRTRANSFORMER_HPP_INCLUDED */
