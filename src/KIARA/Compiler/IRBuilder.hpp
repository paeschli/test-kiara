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
 * IRBuilder.hpp
 *
 *  Created on: 17.04.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMPILER_IRBUILDER_HPP_INCLUDED
#define KIARA_COMPILER_IRBUILDER_HPP_INCLUDED

#include "Config.hpp"
#include "IR.hpp"
#include "Scope.hpp"
#include "Mangler.hpp"
#include <KIARA/Utils/ArrayRef.hpp>
#include <stack>

namespace KIARA
{

namespace Compiler
{

inline KIARA::IR::Prototype::Ptr createCFuncProto(
    const std::string &name,
    const Type::Ptr &returnType, ArrayRef<KIARA::IR::Prototype::Arg> args, KIARA::World &world)
{
    KIARA::IR::Prototype::Ptr proto = new KIARA::IR::Prototype(name, name, returnType, args, world);
    proto->setAttribute("C", "true");
    return proto;
}

inline KIARA::IR::Prototype::Ptr createMangledFuncProto(
    const std::string &name,
    const Type::Ptr &returnType, ArrayRef<KIARA::IR::Prototype::Arg> args, KIARA::World &world)
{
    return new KIARA::IR::Prototype(
        name,
        KIARA::Compiler::Mangler::getMangledFuncName(name, args),
        returnType,
        args,
        world);
}

inline KIARA::IR::Prototype::Ptr createMangledFuncProto(
    const std::string &name,
    const Type::Ptr &returnType, ArrayRef<KIARA::IR::Prototype::Arg> args)
{
    return new KIARA::IR::Prototype(
        name,
        KIARA::Compiler::Mangler::getMangledFuncName(name, args),
        returnType,
        args,
        returnType->getWorld());
}

inline KIARA::IR::Prototype::Ptr createOperatorProto(
    const std::string &name,
    const Type::Ptr &returnType, ArrayRef<KIARA::IR::Prototype::Arg> args, unsigned prec)
{
    return new KIARA::IR::Prototype(
        name,
        KIARA::Compiler::Mangler::getMangledFuncName(name, args),
        returnType,
        args,
        returnType->getWorld(),
        true,
        prec);
}

class KIARA_COMPILER_API IRBuilderException : public Exception
{
public:
    explicit IRBuilderException(const std::string &arg);
};

class KIARA_COMPILER_API IRBuilder
{
public:

    IRBuilder(World &world);
    IRBuilder(const Scope::Ptr &scope);

    World & getWorld() { return world_; }

    bool hasScope() const
    {
        return !scopes_.empty();
    }

    const Scope::Ptr & getScope() const
    {
        return scopes_.top();
    }

    void setScope(const Scope::Ptr & scope)
    {
        if (scopes_.empty())
            scopes_.push(scope);
        else
            scopes_.top() = scope;
    }

    void pushScope(const std::string &newName)
    {
        scopes_.push(new Scope(getWorld(), newName, scopes_.top()));
    }

    void pushScope(const Scope::Ptr &scope)
    {
        scopes_.push(scope);
    }

    void popScope()
    {
        scopes_.pop();
    }

    class ScopeGuard
    {
    public:

        ScopeGuard(const Scope::Ptr &scope, IRBuilder *builder)
            : builder_(*builder)
        { builder_.pushScope(scope); }

        ScopeGuard(const Scope::Ptr &scope, IRBuilder &builder)
            : builder_(builder)
        { builder_.pushScope(scope); }

        ScopeGuard(const std::string &name, IRBuilder *builder)
            : builder_(*builder)
        { builder_.pushScope(name); }

        ScopeGuard(const std::string &name, IRBuilder &builder)
            : builder_(builder)
        { builder_.pushScope(name); }

        ScopeGuard(const char *name, IRBuilder *builder)
            : builder_(*builder)
        { builder_.pushScope(name); }

        ScopeGuard(const char *name, IRBuilder &builder)
            : builder_(builder)
        { builder_.pushScope(name); }

        ~ScopeGuard() { builder_.popScope(); }

    private:
        IRBuilder &builder_;
    };

    IR::PrimLiteral::Ptr createLiteral(int8_t v) { return new IR::PrimLiteral(v, getWorld()); }
    IR::PrimLiteral::Ptr createLiteral(uint8_t v) { return new IR::PrimLiteral(v, getWorld()); }
    IR::PrimLiteral::Ptr createLiteral(int16_t v) { return new IR::PrimLiteral(v, getWorld()); }
    IR::PrimLiteral::Ptr createLiteral(uint16_t v) { return new IR::PrimLiteral(v, getWorld()); }
    IR::PrimLiteral::Ptr createLiteral(int32_t v) { return new IR::PrimLiteral(v, getWorld()); }
    IR::PrimLiteral::Ptr createLiteral(uint32_t v) { return new IR::PrimLiteral(v, getWorld()); }
    IR::PrimLiteral::Ptr createLiteral(int64_t v) { return new IR::PrimLiteral(v, getWorld()); }
    IR::PrimLiteral::Ptr createLiteral(uint64_t v) { return new IR::PrimLiteral(v, getWorld()); }
    IR::PrimLiteral::Ptr createLiteral(float v) { return new IR::PrimLiteral(v, getWorld()); }
    IR::PrimLiteral::Ptr createLiteral(double v) { return new IR::PrimLiteral(v, getWorld()); }
    IR::PrimLiteral::Ptr createLiteral(bool v) { return new IR::PrimLiteral(v, getWorld()); }
    IR::PrimLiteral::Ptr createLiteral(const char * v) { return new IR::PrimLiteral(v, getWorld()); }
    IR::PrimLiteral::Ptr createLiteral(const std::string & v) { return new IR::PrimLiteral(v, getWorld()); }

    static bool createAddressOfCode(
            const Type::Ptr &exprType,
            std::vector<KIARA::IR::IRExpr::Ptr> &expressions,
            const Scope::Ptr &scope,
            std::string *errorMsg = 0);

    bool createAddressOfCode(
            const Type::Ptr &exprType,
            std::vector<KIARA::IR::IRExpr::Ptr> &expressions,
            std::string *errorMsg = 0)
    {
        return createAddressOfCode(exprType, expressions, getScope(), errorMsg);
    }

    static bool createDereferenceCode(
            const Type::Ptr &exprType,
            std::vector<KIARA::IR::IRExpr::Ptr> &expressions,
            const Scope::Ptr &scope,
            std::string *errorMsg = 0);

    bool createDereferenceCode(
            const Type::Ptr &exprType,
            std::vector<KIARA::IR::IRExpr::Ptr> &expressions,
            std::string *errorMsg = 0)
    {
        return createDereferenceCode(exprType, expressions, getScope(), errorMsg);
    }

    static bool createStructCode(
            const StructType::Ptr &structType,
            std::vector<KIARA::IR::IRExpr::Ptr> &expressions,
            const Scope::Ptr &scope,
            std::string *errorMsg = 0);

    bool createStructCode(
            const StructType::Ptr &structType,
            std::vector<KIARA::IR::IRExpr::Ptr> &expressions,
            std::string *errorMsg = 0)
    {
        return createStructCode(structType, expressions, getScope(), errorMsg);
    }

    static bool createCastCode(
            const Type::Ptr &srcType,
            const Type::Ptr &destType,
            std::string &castFuncName,
            std::vector<KIARA::IR::IRExpr::Ptr> &expressions,
            const Scope::Ptr &scope,
            std::string *errorMsg = 0);

    static bool createArrayIndexCode(
            const Type::Ptr &arrayType,
            const Type::Ptr &indexType,
            std::vector<KIARA::IR::IRExpr::Ptr> &expressions,
            const Scope::Ptr &scope,
            std::string *errorMsg = 0);

    static bool createAssignCode(
            const Type::Ptr &destType,
            const Type::Ptr &srcType,
            std::vector<KIARA::IR::IRExpr::Ptr> &expressions,
            const Scope::Ptr &scope,
            std::string *errorMsg = 0);

    IR::BlockExpr::Ptr createBlock(const IR::IRExpr::Ptr &expr1,
                                   const std::string &name = "")
    {
        IR::IRExpr::Ptr exprList[] = { expr1 };
        return new IR::BlockExpr(exprList, name);
    }

    IR::BlockExpr::Ptr createBlock(
            const IR::IRExpr::Ptr &expr1, const IR::IRExpr::Ptr &expr2,
            const std::string &name = "")
    {
        IR::IRExpr::Ptr exprList[] = { expr1, expr2 };
        return new IR::BlockExpr(exprList, name);
    }

    IR::BlockExpr::Ptr createBlock(
            const IR::IRExpr::Ptr &expr1, const IR::IRExpr::Ptr &expr2, const IR::IRExpr::Ptr &expr3,
            const std::string &name = "")
    {
        IR::IRExpr::Ptr exprList[] = { expr1, expr2, expr3 };
        return new IR::BlockExpr(exprList);
    }

    IR::DefExpr::Ptr createVariable(const std::string &name, const Type::Ptr &valueType);

    IR::DefExpr::Ptr createVariableInScope(const std::string &name, const Type::Ptr &valueType,
                                           std::string *errorMsg = 0)
    {
        IR::DefExpr::Ptr var = createVariable(name, valueType);
        if (addVariableToScope(var, errorMsg))
            return var;
        return 0;
    }

    //IR::IRExpr::Ptr getReference(const IR::IRExpr::Ptr &expr);

    IR::IRExpr::Ptr getDereference(const IR::IRExpr::Ptr &expr,
                                   std::string *errorMsg = 0);

    // numConversions is incremented for each data type conversion
    IR::IRExpr::Ptr convertValue(
            const IR::IRExpr::Ptr &value,
            const Type::Ptr &destType,
            int *numConversions = 0,
            std::string *errorMsg = 0);

    enum CallMode
    {
        CM_DEFAULT              = 0,
        CM_NO_TYPE_CONVERSION   = 1
    };

    IR::CallExpr::Ptr createCall(
            const std::string &name,
            const SourceLocation *location = 0,
            const std::string &callMethod = "__call__",
            CallMode mode = CM_DEFAULT,
            std::string *errorMsg = 0)
    {
        return createCall(name, ArrayRef<IR::IRExpr::Ptr>(), location, callMethod, mode, errorMsg);
    }

    IR::CallExpr::Ptr createCall(
            const std::string &name,
            const IR::IRExpr::Ptr &arg1,
            const SourceLocation *location = 0,
            const std::string &callMethod = "__call__",
            CallMode mode = CM_DEFAULT,
            std::string *errorMsg = 0)
    {
        IR::IRExpr::Ptr args[] = {
            arg1
        };
        return createCall(name, args, location, callMethod, mode, errorMsg);
    }

    IR::CallExpr::Ptr createCall(
            const std::string &name,
            const IR::IRExpr::Ptr &arg1,
            const IR::IRExpr::Ptr &arg2,
            const SourceLocation *location = 0,
            const std::string &callMethod = "__call__",
            CallMode mode = CM_DEFAULT,
            std::string *errorMsg = 0)
    {
        IR::IRExpr::Ptr args[] = {
            arg1,
            arg2
        };
        return createCall(name, args, location, callMethod, mode, errorMsg);
    }

    IR::CallExpr::Ptr createCall(
            const std::string &name,
            const IR::IRExpr::Ptr &arg1,
            const IR::IRExpr::Ptr &arg2,
            const IR::IRExpr::Ptr &arg3,
            const SourceLocation *location = 0,
            const std::string &callMethod = "__call__",
            CallMode mode = CM_DEFAULT,
            std::string *errorMsg = 0)
    {
        IR::IRExpr::Ptr args[] = {
            arg1,
            arg2,
            arg3
        };
        return createCall(name, args, location, callMethod, mode, errorMsg);
    }

    IR::CallExpr::Ptr createCall(
        const std::string &name,
        ArrayRef<IR::IRExpr::Ptr> args,
        const SourceLocation *location = 0,
        const std::string &callMethod = "__call__",
        CallMode mode = CM_DEFAULT,
        std::string *errorMsg = 0);

    IR::CallExpr::Ptr createCall(
        const std::string &name,
        const Scope::Ptr &scope,
        ArrayRef<IR::IRExpr::Ptr> args,
        const SourceLocation *location = 0,
        const std::string &callMethod = "__call__",
        CallMode mode = CM_DEFAULT,
        std::string *errorMsg = 0);

    IR::IRExpr::Ptr lookupExpr(const std::string &name);

    Type::Ptr lookupType(const std::string &name) const;

    IR::FunctionDefinition::Ptr lookupFunction(const std::string &name)
    {
        return dyn_cast<IR::FunctionDefinition>(lookupExpr(name));
    }

    bool addVariableToScope(const IR::DefExpr::Ptr &var, std::string *errorMsg = 0);

    bool addObjectToScope(
            const std::string &name,
            const Object::Ptr &object,
            std::string *errorMsg = 0);

    bool hasFunctionInScope(
            const IR::Prototype::Ptr &funcProto)
    {
        return hasFunctionInScope(funcProto, getScope());
    }

    static bool hasFunctionInScope(
            const IR::Prototype::Ptr &funcProto,
            const Scope::Ptr &scope)
    {
        return getFunctionFromScope(funcProto, scope) != 0;
    }

    IR::FunctionDefinition::Ptr getFunctionFromScope(
                const IR::Prototype::Ptr &funcProto)
    {
        return getFunctionFromScope(funcProto, getScope());
    }

    static IR::FunctionDefinition::Ptr getFunctionFromScope(
            const IR::Prototype::Ptr &funcProto,
            const Scope::Ptr &scope);

    bool addFunctionToScope(
            const IR::FunctionDefinition::Ptr &func,
            std::string *errorMsg = 0)
    {
        return addFunctionToScope(func, getScope(), errorMsg);
    }

    static bool addFunctionToScope(
            const IR::FunctionDefinition::Ptr &func,
            const Scope::Ptr &scope,
            std::string *errorMsg = 0);

    void initFunctionScope(
            const IR::FunctionDefinition::Ptr &func,
            std::string *errorMsg = 0);

private:
    World &world_;
    std::stack<Scope::Ptr> scopes_;
};


template <typename T>
inline KIARA::IR::PrimLiteral::Ptr Literal(const T &value, KIARA::Compiler::IRBuilder &builder)
{
    return builder.createLiteral(KIARA::normalize_value(value));
}

inline KIARA::IR::DefExpr::Ptr Var(const std::string &varName, const KIARA::Type::Ptr &type, KIARA::Compiler::IRBuilder &builder)
{
    return builder.createVariableInScope(varName, type);
}

inline KIARA::IR::DefExpr::Ptr Arg(KIARA::IR::Function::Ptr &func, size_t argIndex)
{
    return func->getArg(argIndex);
}

inline KIARA::IR::BlockExpr::Ptr Block(KIARA::World &world, const std::string &name = "")
{
    return new KIARA::IR::BlockExpr(world, name);
}

inline KIARA::IR::BlockExpr::Ptr Block(const KIARA::IR::IRExpr::Ptr &expr, const std::string &name = "")
{
    KIARA::IR::BlockExpr::Ptr block = new KIARA::IR::BlockExpr(expr->getWorld(), name);
    block->addExpr(expr);
    return block;
}

inline KIARA::IR::BlockExpr::Ptr Block(
    const KIARA::IR::IRExpr::Ptr &expr1,
    const KIARA::IR::IRExpr::Ptr &expr2,
    const std::string &name = "")
{
    KIARA::IR::BlockExpr::Ptr block = new KIARA::IR::BlockExpr(expr1->getWorld(), name);
    block->addExpr(expr1);
    block->addExpr(expr2);
    return block;
}

inline KIARA::IR::BlockExpr::Ptr Block(
    const KIARA::IR::IRExpr::Ptr &expr1,
    const KIARA::IR::IRExpr::Ptr &expr2,
    const KIARA::IR::IRExpr::Ptr &expr3,
    const std::string &name = "")
{
    KIARA::IR::BlockExpr::Ptr block = new KIARA::IR::BlockExpr(expr1->getWorld(), name);
    block->addExpr(expr1);
    block->addExpr(expr2);
    block->addExpr(expr3);
    return block;
}

inline KIARA::IR::BlockExpr::Ptr Block(
    const KIARA::IR::IRExpr::Ptr &expr1,
    const KIARA::IR::IRExpr::Ptr &expr2,
    const KIARA::IR::IRExpr::Ptr &expr3,
    const KIARA::IR::IRExpr::Ptr &expr4,
    const std::string &name = "")
{
    KIARA::IR::BlockExpr::Ptr block = new KIARA::IR::BlockExpr(expr1->getWorld(), name);
    block->addExpr(expr1);
    block->addExpr(expr2);
    block->addExpr(expr3);
    block->addExpr(expr4);
    return block;
}

inline KIARA::IR::BlockExpr::Ptr Block(
    const KIARA::IR::IRExpr::Ptr &expr1,
    const KIARA::IR::IRExpr::Ptr &expr2,
    const KIARA::IR::IRExpr::Ptr &expr3,
    const KIARA::IR::IRExpr::Ptr &expr4,
    const KIARA::IR::IRExpr::Ptr &expr5,
    const std::string &name = "")
{
    KIARA::IR::BlockExpr::Ptr block = new KIARA::IR::BlockExpr(expr1->getWorld(), name);
    block->addExpr(expr1);
    block->addExpr(expr2);
    block->addExpr(expr3);
    block->addExpr(expr4);
    block->addExpr(expr5);
    return block;
}

inline KIARA::IR::BlockExpr::Ptr Block(
    const KIARA::IR::IRExpr::Ptr &expr1,
    const KIARA::IR::IRExpr::Ptr &expr2,
    const KIARA::IR::IRExpr::Ptr &expr3,
    const KIARA::IR::IRExpr::Ptr &expr4,
    const KIARA::IR::IRExpr::Ptr &expr5,
    const KIARA::IR::IRExpr::Ptr &expr6,
    const std::string &name = "")
{
    KIARA::IR::BlockExpr::Ptr block = new KIARA::IR::BlockExpr(expr1->getWorld(), name);
    block->addExpr(expr1);
    block->addExpr(expr2);
    block->addExpr(expr3);
    block->addExpr(expr4);
    block->addExpr(expr5);
    block->addExpr(expr6);
    return block;
}

inline KIARA::IR::BlockExpr::Ptr Block(
    const KIARA::IR::IRExpr::Ptr &expr1,
    const KIARA::IR::IRExpr::Ptr &expr2,
    const KIARA::IR::IRExpr::Ptr &expr3,
    const KIARA::IR::IRExpr::Ptr &expr4,
    const KIARA::IR::IRExpr::Ptr &expr5,
    const KIARA::IR::IRExpr::Ptr &expr6,
    const KIARA::IR::IRExpr::Ptr &expr7,
    const std::string &name = "")
{
    KIARA::IR::BlockExpr::Ptr block = new KIARA::IR::BlockExpr(expr1->getWorld(), name);
    block->addExpr(expr1);
    block->addExpr(expr2);
    block->addExpr(expr3);
    block->addExpr(expr4);
    block->addExpr(expr5);
    block->addExpr(expr6);
    block->addExpr(expr7);
    return block;
}

inline KIARA::IR::BlockExpr::Ptr Block(
    const KIARA::IR::IRExpr::Ptr &expr1,
    const KIARA::IR::IRExpr::Ptr &expr2,
    const KIARA::IR::IRExpr::Ptr &expr3,
    const KIARA::IR::IRExpr::Ptr &expr4,
    const KIARA::IR::IRExpr::Ptr &expr5,
    const KIARA::IR::IRExpr::Ptr &expr6,
    const KIARA::IR::IRExpr::Ptr &expr7,
    const KIARA::IR::IRExpr::Ptr &expr8,
    const std::string &name = "")
{
    KIARA::IR::BlockExpr::Ptr block = new KIARA::IR::BlockExpr(expr1->getWorld(), name);
    block->addExpr(expr1);
    block->addExpr(expr2);
    block->addExpr(expr3);
    block->addExpr(expr4);
    block->addExpr(expr5);
    block->addExpr(expr6);
    block->addExpr(expr7);
    block->addExpr(expr8);
    return block;
}

inline KIARA::IR::BlockExpr::Ptr Block(
    const KIARA::IR::IRExpr::Ptr &expr1,
    const KIARA::IR::IRExpr::Ptr &expr2,
    const KIARA::IR::IRExpr::Ptr &expr3,
    const KIARA::IR::IRExpr::Ptr &expr4,
    const KIARA::IR::IRExpr::Ptr &expr5,
    const KIARA::IR::IRExpr::Ptr &expr6,
    const KIARA::IR::IRExpr::Ptr &expr7,
    const KIARA::IR::IRExpr::Ptr &expr8,
    const KIARA::IR::IRExpr::Ptr &expr9,
    const std::string &name = "")
{
    KIARA::IR::BlockExpr::Ptr block = new KIARA::IR::BlockExpr(expr1->getWorld(), name);
    block->addExpr(expr1);
    block->addExpr(expr2);
    block->addExpr(expr3);
    block->addExpr(expr4);
    block->addExpr(expr5);
    block->addExpr(expr6);
    block->addExpr(expr7);
    block->addExpr(expr8);
    block->addExpr(expr9);
    return block;
}

inline KIARA::IR::BlockExpr::Ptr NamedBlock(const std::string &name, KIARA::World &world)
{
    return new KIARA::IR::BlockExpr(world, name);
}

inline KIARA::IR::BlockExpr::Ptr NamedBlock(const std::string &name, const KIARA::IR::IRExpr::Ptr &expr)
{
    KIARA::IR::BlockExpr::Ptr block = new KIARA::IR::BlockExpr(expr->getWorld(), name);
    block->addExpr(expr);
    return block;
}

inline KIARA::IR::BlockExpr::Ptr NamedBlock(
    const std::string &name,
    const KIARA::IR::IRExpr::Ptr &expr1,
    const KIARA::IR::IRExpr::Ptr &expr2)
{
    KIARA::IR::BlockExpr::Ptr block = new KIARA::IR::BlockExpr(expr1->getWorld(), name);
    block->addExpr(expr1);
    block->addExpr(expr2);
    return block;
}

inline KIARA::IR::BlockExpr::Ptr NamedBlock(
    const std::string &name,
    const KIARA::IR::IRExpr::Ptr &expr1,
    const KIARA::IR::IRExpr::Ptr &expr2,
    const KIARA::IR::IRExpr::Ptr &expr3)
{
    KIARA::IR::BlockExpr::Ptr block = new KIARA::IR::BlockExpr(expr1->getWorld(), name);
    block->addExpr(expr1);
    block->addExpr(expr2);
    block->addExpr(expr3);
    return block;
}

inline KIARA::IR::BlockExpr::Ptr NamedBlock(
    const std::string &name,
    const KIARA::IR::IRExpr::Ptr &expr1,
    const KIARA::IR::IRExpr::Ptr &expr2,
    const KIARA::IR::IRExpr::Ptr &expr3,
    const KIARA::IR::IRExpr::Ptr &expr4)
{
    KIARA::IR::BlockExpr::Ptr block = new KIARA::IR::BlockExpr(expr1->getWorld(), name);
    block->addExpr(expr1);
    block->addExpr(expr2);
    block->addExpr(expr3);
    block->addExpr(expr4);
    return block;
}

inline KIARA::IR::LetExpr::Ptr Let(
    const KIARA::IR::DefExpr::Ptr &var,
    const KIARA::IR::IRExpr::Ptr &initValue,
    const KIARA::IR::IRExpr::Ptr &body)
{
    return new KIARA::IR::LetExpr(var, initValue, body);
}

inline KIARA::IR::LoopExpr::Ptr Loop(
    const KIARA::IR::IRExpr::Ptr &body)
{
    return new KIARA::IR::LoopExpr(body);
}

inline KIARA::IR::IfExpr::Ptr If(const KIARA::IR::IRExpr::Ptr &cond,
                                 const KIARA::IR::IRExpr::Ptr &then,
                                 const KIARA::IR::IRExpr::Ptr &_else)
{
    return new KIARA::IR::IfExpr(cond, then, _else);
}

inline KIARA::IR::IfExpr::Ptr If(const KIARA::IR::IRExpr::Ptr &cond,
                                 const KIARA::IR::IRExpr::Ptr &then)
{
    return new KIARA::IR::IfExpr(cond, then, 0);
}

inline KIARA::IR::BreakExpr::Ptr Break(const KIARA::IR::IRExpr::Ptr &block,
                                       const KIARA::IR::IRExpr::Ptr &value)
{
    return new KIARA::IR::BreakExpr(block, value);
}

inline KIARA::IR::BreakExpr::Ptr Break(const KIARA::IR::IRExpr::Ptr &block)
{
    return new KIARA::IR::BreakExpr(block);
}

inline KIARA::IR::Function::Ptr Function(const KIARA::IR::Prototype::Ptr &proto)
{
    return new KIARA::IR::Function(proto, 0);
}

inline KIARA::IR::Function::Ptr Function(const KIARA::IR::Prototype::Ptr &proto,
                                         const KIARA::IR::IRExpr::Ptr &body)
{
    return new KIARA::IR::Function(proto, body);
}

inline KIARA::IR::SymbolExpr::Ptr Symbol(const std::string &name, World &world)
{
    return new KIARA::IR::SymbolExpr(name, world);
}

inline KIARA::IR::SymbolExpr::Ptr Symbol(const std::string &name, IRBuilder &builder)
{
    return new KIARA::IR::SymbolExpr(name, builder.getWorld());
}

inline KIARA::IR::TypeExpr::Ptr EType(const KIARA::Type::Ptr &natType)
{
    return new KIARA::IR::TypeExpr(natType);
}

typedef KIARA::IR::IRExpr::Ptr TExpr;
typedef KIARA::IR::CallExpr::Ptr TCall;
typedef KIARA::IR::PrimLiteral::Ptr TLiteral;
typedef KIARA::IR::DefExpr::Ptr TVar;
typedef KIARA::IR::LetExpr::Ptr TLet;
typedef KIARA::IR::Function::Ptr TFunction;
typedef KIARA::IR::BlockExpr::Ptr TBlock;

class Callee
{
public:

    Callee(const std::string &name, KIARA::Compiler::IRBuilder &builder)
        : name(name), builder(builder)
    { }

    KIARA::IR::CallExpr::Ptr operator()()
    {
        return builder.createCall(name);
    }

    KIARA::IR::CallExpr::Ptr operator()(const KIARA::IR::IRExpr::Ptr &arg1)
    {
        return builder.createCall(name, arg1);
    }

    KIARA::IR::CallExpr::Ptr operator()(const KIARA::IR::IRExpr::Ptr &arg1,
                                        const KIARA::IR::IRExpr::Ptr &arg2)
    {
        return builder.createCall(name, arg1, arg2);
    }

    KIARA::IR::CallExpr::Ptr operator()(const KIARA::IR::IRExpr::Ptr &arg1,
                                        const KIARA::IR::IRExpr::Ptr &arg2,
                                        const KIARA::IR::IRExpr::Ptr &arg3)
    {
        return builder.createCall(name, arg1, arg2, arg3);
    }

    std::string name;
    KIARA::Compiler::IRBuilder &builder;
};

} // namespace Compiler

} // namespace KIARA

#endif /* KIARA_IRBUILDER_HPP_INCLUDED */
