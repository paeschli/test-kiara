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
 * PrettyPrinter.hpp
 *
 *  Created on: 25.06.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMPILER_PRETTYPRINTER_HPP_INCLUDED
#define KIARA_COMPILER_PRETTYPRINTER_HPP_INCLUDED

#include <KIARA/Compiler/Config.hpp>
#include <KIARA/Compiler/IR.hpp>
#include <KIARA/Utils/IndentingStreambuf.hpp>
#include <ostream>
#include <sstream>

namespace KIARA
{

namespace IR
{

class KIARA_COMPILER_API PrettyPrinter : public IR::ExprVisitor<PrettyPrinter>
{
public:
    typedef ExprVisitor<PrettyPrinter> InheritedType;
    using InheritedType::visit;

    PrettyPrinter();

    PrettyPrinter(std::ostream &out);

    static std::ostream & print(std::ostream &out, const Object::Ptr &object)
    {
        PrettyPrinter pp(out);
        pp.apply(object);
        return out;
    }

    static std::ostream & print(std::ostream &out, const Object* object)
    {
        PrettyPrinter pp(out);
        pp.apply(const_cast<Object*>(object));
        return out;
    }

    static std::string toString(const Object::Ptr &object)
    {
        if (!object)
            return "NULL";
        std::ostringstream oss;
        PrettyPrinter pp(oss);
        pp.apply(object);
        return oss.str();
    }

    void visit(Type &type);
    void visit(IR::PrimLiteral &expr);
    void visit(IR::TypeExpr &expr);
    void visit(IR::DefExpr &expr);
    void visit(IR::MemRef &expr);
    void visit(IR::CallExpr &expr);
    void visit(IR::SymbolExpr &expr);
    void visit(IR::IfExpr &expr);
    void visit(IR::LoopExpr &expr);
    void visit(IR::ForExpr &expr);
    void visit(IR::LetExpr &expr);
    void visit(IR::BlockExpr &expr);
    void visit(IR::BreakExpr &expr);
    void visit(IR::Prototype &proto);
    void visit(IR::Function &func);
    void visit(IR::ExternFunction &func);
    void visit(IR::Intrinsic &func);
    void visit(IR::FunctionDeclaration &decl);
    void visit(IR::TypeDefinition &decl);

private:

    void printObject(const Object::Ptr &obj);

    void printBody(const Object::Ptr &obj);

    bool isMemRef_; // true if we are currently printing a MemRef

    std::ostream *out_;
    IndentingStreambuf isb_;
};

} // namespace IR

} // namespace KIARA

#endif /* PRETTYPRINTER_HPP_ */
