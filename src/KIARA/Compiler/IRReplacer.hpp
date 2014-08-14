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

#ifndef KIARA_COMPILER_IRREPLACER_HPP_INCLUDED
#define KIARA_COMPILER_IRREPLACER_HPP_INCLUDED

#include "Config.hpp"
#include "IR.hpp"
#include <KIARA/Utils/VarGuard.hpp>
#include <iostream>

namespace KIARA
{

namespace IR
{

template <typename SubClass>
class IRReplacer : public ExprVisitor<SubClass, void>
{
public:

    typedef ExprVisitor<SubClass, void> InheritedType;
    using InheritedType::visit;
    using InheritedType::apply;

    void reset()
    {
        clearReplaceCache();
    }

    void clearReplaceCache()
    {
        replaceCache_.clear();
    }

    void apply(KIARA::Object &expr)
    {
        ReplaceCache::iterator it = replaceCache_.find(&expr);
        if (it != replaceCache_.end())
        {
            //std::cerr<<"XFM: don't process: "<<&expr<<" "<<expr.getTypeName()<<": "<<expr<<std::endl;
            parent_->replaceExpr(&expr, it->second);
            return;
        }

        // processing
        //std::cerr<<"XFM: start process: "<<&expr<<" "<<expr.getTypeName()<<": "<<expr<<" {"<<std::endl;
        InheritedType::apply(expr);
        //std::cerr<<"XFM: end process "<<&expr<<" "<<expr.getTypeName()<<"}"<<std::endl;
    }

    void safeApply(const KIARA::Object::Ptr &expr)
    {
        if (expr)
            apply(expr);
    }

    void visit(Type &type)
    {
        // leaf
    }

    void visit(PrimLiteral &expr)
    {
        // leaf
    }

    void visit(IR::TypeExpr &expr)
    {
        // leaf
    }

    void visit(IR::DefExpr &expr)
    {
        // leaf
    }

    void visit(IR::MemRef &expr)
    {
        VarGuard<Expr::Ptr> g(parent_, &expr);
        safeApply(expr.getValue());
    }

    void visit(IR::CallExpr &expr)
    {
        VarGuard<Expr::Ptr> g(parent_, &expr);

        for (size_t i = 0; i < expr.getNumArgs(); ++i)
        {
            safeApply(expr.getArg(i));
        }
    }

    void visit(IR::SymbolExpr &expr)
    {
        // leaf
    }

    void visit(IR::IfExpr &expr)
    {
        VarGuard<Expr::Ptr> g(parent_, &expr);

        safeApply(expr.getCond());
        safeApply(expr.getThen());
        safeApply(expr.getElse());
    }

    void visit(IR::LoopExpr &expr)
    {
        VarGuard<Expr::Ptr> g(parent_, &expr);

        safeApply(expr.getBody());
    }

    void visit(IR::ForExpr &expr)
    {
        VarGuard<Expr::Ptr> g(parent_, &expr);

        safeApply(expr.getVar());

        safeApply(expr.getStart());
        safeApply(expr.getEnd());
        safeApply(expr.getStep());
        safeApply(expr.getBody());
    }

    void visit(IR::LetExpr &expr)
    {
        VarGuard<Expr::Ptr> g(parent_, &expr);

        safeApply(expr.getVar());
        safeApply(expr.getInitValue());
        safeApply(expr.getBody());
    }

    void visit(IR::BlockExpr &expr)
    {
        VarGuard<Expr::Ptr> g(parent_, &expr);

        IR::BlockExpr::ExprList &exprList = expr.getExprList();
        for (size_t i = 0; i < exprList.size(); ++i)
        {
            safeApply(exprList[i]);
        }
    }

    void visit(IR::BreakExpr &expr)
    {
        VarGuard<Expr::Ptr> g(parent_, &expr);

        safeApply(expr.getValue());
        // safeApply(expr.getBlock()); // FIXME this will cause infinite loop
    }

    void visit(IR::Prototype &proto)
    {
        // TODO should we process types as well ?
    }

    void visit(IR::Function &func)
    {
        VarGuard<Expr::Ptr> g(parent_, &func);

        safeApply(func.getProto());
        safeApply(func.getBody());
    }

    void visit(IR::ExternFunction &func)
    {
        VarGuard<Expr::Ptr> g(parent_, &func);

        safeApply(func.getProto());
    }

    void visit(IR::Intrinsic &func)
    {
        VarGuard<Expr::Ptr> g(parent_, &func);

        safeApply(func.getProto());
    }

    void visit(IR::FunctionDeclaration &decl)
    {
        VarGuard<Expr::Ptr> g(parent_, &decl);

        safeApply(decl.getProto());
    }

    void visit(IR::TypeDefinition &decl)
    {
        // VarGuard<Expr::Ptr> g(parent_, &decl);

        safeApply(decl.getDefinedType());
    }

protected:

    const Expr::Ptr & getParent() const { return parent_; }

    void replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr)
    {
        if (parent_)
            parent_->replaceExpr(oldExpr, newExpr);
        replaceCache_[oldExpr] = newExpr;
    }

private:
    typedef std::map<Object::Ptr, Object::Ptr> ReplaceCache;
    ReplaceCache replaceCache_;
    Expr::Ptr parent_;
};

} // namespace IR

} // namespace KIARA

#endif /* KIARA_COMPILER_IRREPLACER_HPP_INCLUDED */
