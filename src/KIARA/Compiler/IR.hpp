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
 * IR.hpp
 *
 *  Created on: 30.01.2013
 *      Author: Dmitri Rubinstein
 */
#ifndef KIARA_COMPILER_IR_HPP_INCLUDED
#define KIARA_COMPILER_IR_HPP_INCLUDED

#include "Config.hpp"
#include <KIARA/DB/Expr.hpp>
#include <KIARA/DB/World.hpp>
#include <KIARA/Core/Exception.hpp>
#include <KIARA/Common/TypeTraits.hpp>
#include <KIARA/Utils/ArrayRef.hpp>
#include <KIARA/Utils/TypedBox.hpp>
#include <KIARA/Utils/StringAttributeHolder.hpp>
#include "SourceLocation.hpp"
#include <boost/scoped_ptr.hpp>
#include <boost/lexical_cast.hpp>

namespace KIARA
{

namespace IR
{

class IRExpr;
class DefExpr;
class SymbolExpr;
class CallExpr;
class IfExpr;
class ForExpr;
class LetExpr;
class FunctionDefinition;
class Function;
class Prototype;

class LocationHolder
{
public:
    LocationHolder() : loc_() { }

    void setLocation(const Compiler::SourceLocation &loc)
    {
        if (loc_)
            *loc_ = loc;
        else
            loc_.reset(new Compiler::SourceLocation(loc));
    }

    void setLocation(const Compiler::SourceLocation *loc)
    {
        if (loc)
            setLocation(*loc);
        else
            resetLocation();
    }

    void resetLocation()
    {
        loc_.reset();
    }

    const Compiler::SourceLocation * getLocation() const
    {
        return loc_.get();
    }

private:
    boost::scoped_ptr<Compiler::SourceLocation> loc_;
};

/// IRExpr - Base class for all expression nodes.
class KIARA_COMPILER_API IRExpr : public KIARA::Expr, public LocationHolder
{
    DFC_DECLARE_ABSTRACT_TYPE(IRExpr, Expr)
public:

    virtual ~IRExpr()
    {
    }

    // Returns memory reference to the expression when expression
    // has an address (is a lvalue).
    // Returned object is always of type MemRef.
    virtual IRExpr::Ptr getReference() { return IRExpr::Ptr(); }

    virtual bool hasAddress() const { return false; }

    virtual void print(std::ostream &out) const;

protected:
    IRExpr(const KIARA::Type::Ptr &exprType);
    IRExpr(const KIARA::Type::Ptr &exprType, KIARA::World &world);
};

/// Represents memory reference to the lvalue
class KIARA_COMPILER_API MemRef : public IRExpr
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(MemRef, IRExpr)
public:

    MemRef(const IRExpr::Ptr &value);

    virtual IRExpr::Ptr getReference() { return this; }

    const IRExpr::Ptr & getValue() const { return value_; }

    virtual bool replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr);

protected:
    virtual void gcUnlinkRefs();
    virtual void gcApplyToChildren(const CollectorCallback &callback);
private:
    IRExpr::Ptr value_;
};

/// PrimLiteral - Expression class for numeric literals like "1.0".
class KIARA_COMPILER_API PrimLiteral: public IRExpr
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(PrimLiteral, IRExpr)
public:

    struct NullPtrTag { };

    explicit PrimLiteral(const PrimLiteral &value);
    explicit PrimLiteral(const PrimLiteral::Ptr &valueType);

    PrimLiteral(int8_t v, KIARA::World &w);
    PrimLiteral(uint8_t v, KIARA::World &w);
    PrimLiteral(int16_t v, KIARA::World &w);
    PrimLiteral(uint16_t v, KIARA::World &w);
    PrimLiteral(int32_t v, KIARA::World &w);
    PrimLiteral(uint32_t v, KIARA::World &w);
    PrimLiteral(int64_t v, KIARA::World &w);
    PrimLiteral(uint64_t v, KIARA::World &w);
    PrimLiteral(float v, KIARA::World &w);
    PrimLiteral(double v, KIARA::World &w);
    PrimLiteral(bool v, KIARA::World &w);
    PrimLiteral(NullPtrTag, KIARA::World &w);
    PrimLiteral(const char * v, KIARA::World &w);
    PrimLiteral(const std::string & v, KIARA::World &w);

    ~PrimLiteral();

    static PrimLiteral::Ptr getZero(const KIARA::Type::Ptr &type);

    static PrimLiteral::Ptr getNullPtr(KIARA::World &w)
    {
        return new PrimLiteral(NullPtrTag(), w);
    }

    bool isNullPtr() const
    {
        return (value_.getType() == BT_VOIDPTR && value_.get_ptr() == 0);
    }

    static bool isNullPtr(const IRExpr::Ptr &value)
    {
        PrimLiteral::Ptr v = dyn_cast<PrimLiteral>(value);
        return v && v->isNullPtr();
    }

    bool isInteger() const
    {
        switch (value_.getType())
        {
            case BT_INT8_T:
            case BT_UINT8_T:
            case BT_INT16_T:
            case BT_UINT16_T:
            case BT_INT32_T:
            case BT_UINT32_T:
            case BT_INT64_T:
            case BT_UINT64_T:
                return true;
            default:
                break;
        }
        return false;
    }

    template <class T>
    bool hasIntegerValueOf(T value) const
    {
        switch (value_.getType())
        {
            case BT_INT8_T:
                return value_.get_i8() == value;
            case BT_UINT8_T:
                return value_.get_u8() == value;
            case BT_INT16_T:
                return value_.get_i16() == value;
            case BT_UINT16_T:
                return value_.get_u16() == value;
            case BT_INT32_T:
                return value_.get_i32() == value;
            case BT_UINT32_T:
                return value_.get_u32() == value;
            case BT_INT64_T:
                return value_.get_i64() == value;
            case BT_UINT64_T:
                return value_.get_u64() == value;
            default:
                break;
        }
        return false;
    }

    const KIARA::Type::Ptr & getValueType() const { return getExprType(); }

    KIARA::PrimType::Ptr getPrimType() const
    {
        return DFC::object_cast<KIARA::PrimType>(getValueType()->getCanonicalType());
    }

    KIARA::PrimTypeKind getPrimTypeKind() const
    {
        return getPrimType()->primtype_kind();
    }

    template <class T>
    T get() const
    {
        typedef typename KIARA::normalize_type<T>::type NormType;
        const KIARA::Type::Ptr nt = getWorld().c_type<NormType>();
        const KIARA::Type::Ptr & vt = getValueType();
        // either value type is equal to native type
        // or value type is string and native type is char*
        if (getValueType() != nt &&
            !(vt == getWorld().type_string() && nt == getWorld().type_c_char_ptr()))
        {
            DFC_THROW_EXCEPTION(KIARA::Exception, "Type mismatch");
        }
        return value_.get<NormType>();
    }

    int64_t getAsInt() const
    {
        switch (value_.getType())
        {
            case BT_INT8_T:
                return value_.get_i8();
            case BT_UINT8_T:
                return value_.get_u8();
            case BT_INT16_T:
                return value_.get_i16();
            case BT_UINT16_T:
                return value_.get_u16();
            case BT_INT32_T:
                return value_.get_i32();
            case BT_UINT32_T:
                return value_.get_u32();
            case BT_INT64_T:
                return value_.get_i64();
            case BT_UINT64_T:
                return value_.get_u64();
            case BT_FLOAT:
                return value_.get_float();
            case BT_DOUBLE:
                return value_.get_double();
            case BT_BOOL:
                return value_.get_bool();
            case BT_STRING:
                return boost::lexical_cast<int64_t>(value_.get_string());
            default:
                break;
        }
        return -1;
    }

    virtual bool equals(const Object::Ptr &other) const;

    bool operator==(const PrimLiteral &other) const
    {
        return (getValueType() == other.getValueType() &&
                value_ == other.value_);
    }

protected:

    void destroyValue();

    virtual void gcUnlinkRefs();

private:
    KIARA::TypedBox value_;

    const PrimLiteral &operator=(const PrimLiteral &);
};

/// ListLiteral - Expression class for lists like [1, 2, 3].
class KIARA_API ListLiteral : public IRExpr
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(ListLiteral, IRExpr)
public:

    ListLiteral(const ListLiteral &value);
    ListLiteral(const Type::Ptr &valueType);

    template <class Iter>
    ListLiteral(Iter begin, Iter end, const Type::Ptr &valueType)
        : IRExpr(valueType, valueType->getWorld()), value_(begin, end)
    {
    }

    template <class Container>
    ListLiteral(const Container &container, const Type::Ptr &valueType)
        : IRExpr(valueType, valueType->getWorld()), value_(container.begin(), container.end())
    {
    }

    ~ListLiteral();

    size_t getSize() const { return value_.size(); }

    void setValue(const std::vector<IRExpr::Ptr> &value)
    {
        value_ = value;
    }

    IRExpr::Ptr getElementAt(size_t index) const { return value_[index]; }

    void setElementAt(size_t index, const IRExpr::Ptr &element)
    {
        value_[index] = element;
    }

    bool operator==(const ListLiteral &other) const;

    virtual bool equals(const Object::Ptr &other) const;

    virtual void print(std::ostream &out) const;

protected:

    virtual void gcUnlinkRefs();
    virtual void gcApplyToChildren(const CollectorCallback &callback);

private:
    std::vector<IRExpr::Ptr> value_;

    const ListLiteral &operator=(const ListLiteral &);
};

/// TypeExpr - Expression class for representing types used as function
//  arguments: sizeof(int)
class KIARA_COMPILER_API TypeExpr: public IRExpr
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(TypeExpr, IRExpr)
public:

    TypeExpr(const Type::Ptr &typeValue);

    Type::Ptr getTypeValue() const { return typeValue_; }

protected:
    virtual void gcUnlinkRefs();
    virtual void gcApplyToChildren(const CollectorCallback &callback);
private:
    Type::Ptr typeValue_;
};

/// DefExpr - Expression class for referencing a variable, like "a".
class KIARA_COMPILER_API DefExpr: public IRExpr
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(DefExpr, IRExpr)
public:

    DefExpr(const std::string &name, const Type::Ptr &type, bool hasAddress);

    const std::string &getName() const { return name_; }

    virtual IRExpr::Ptr getReference();

    virtual bool hasAddress() const { return hasAddress_; }

    virtual bool replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr);

protected:
    virtual void gcUnlinkRefs();
    virtual void gcApplyToChildren(const CollectorCallback &callback);
private:
    std::string name_;
    bool hasAddress_;
    MemRef::Ptr memRef_;
};

/// SymbolExpr - Expression class for referencing a symbol in a scope.
class KIARA_COMPILER_API SymbolExpr: public IRExpr
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(SymbolExpr, IRExpr)
public:

    SymbolExpr(const std::string &name, KIARA::World &world);

    SymbolExpr(const std::string &name, const Object::Ptr &scope);

    const std::string &getName() const { return name_; }

    const Object::Ptr & getScope() const { return scope_; }

protected:
    virtual void gcUnlinkRefs();
    virtual void gcApplyToChildren(const CollectorCallback &callback);
private:
    std::string name_;
    Object::Ptr scope_;
};

/// IfExpr - Expression class for if/then/else.
class KIARA_COMPILER_API IfExpr: public IRExpr
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(IfExpr, IRExpr)
public:

    IfExpr(const IRExpr::Ptr &cond, const IRExpr::Ptr &then, const IRExpr::Ptr &_else);

    IfExpr(const IRExpr::Ptr &cond, const IRExpr::Ptr &then, const IRExpr::Ptr &_else, KIARA::World &world);

    const IRExpr::Ptr & getCond() const { return cond_; }
    const IRExpr::Ptr & getThen() const { return then_; }
    const IRExpr::Ptr & getElse() const { return else_; }

    virtual bool replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr);

protected:
    virtual void gcUnlinkRefs();
    virtual void gcApplyToChildren(const CollectorCallback &callback);
private:
    IRExpr::Ptr cond_;
    IRExpr::Ptr then_;
    IRExpr::Ptr else_;
};

/// LoopExpr - Expression class for infinite loop.
class KIARA_COMPILER_API LoopExpr: public IRExpr
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(LoopExpr, IRExpr)
public:

    LoopExpr(const IRExpr::Ptr &body);

    const IRExpr::Ptr & getBody() const { return body_; }

    virtual bool replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr);

protected:
    virtual void gcUnlinkRefs();
    virtual void gcApplyToChildren(const CollectorCallback &callback);
private:
    IRExpr::Ptr body_;
};

/// ForExpr - Expression class for for/in.
class KIARA_COMPILER_API ForExpr: public IRExpr
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(ForExpr, IRExpr)
public:

    ForExpr(
            const std::string &varName,
            const Type::Ptr &varType,
            const IRExpr::Ptr &start,
            const IRExpr::Ptr &end,
            const IRExpr::Ptr &step,
            const IRExpr::Ptr &body);

    ForExpr(
            const DefExpr::Ptr &var,
            const IRExpr::Ptr &start,
            const IRExpr::Ptr &end,
            const IRExpr::Ptr &step,
            const IRExpr::Ptr &body);

    const DefExpr::Ptr &getVar() const { return var_; }
    const std::string & getVarName() const { return var_->getName(); }
    const Type::Ptr & getVarType() const { return var_->getExprType(); }
    const IRExpr::Ptr & getStart() const { return start_; }
    const IRExpr::Ptr & getEnd() const { return end_; }
    const IRExpr::Ptr & getStep() const { return step_; }
    const IRExpr::Ptr & getBody() const { return body_; }

    virtual bool replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr);

protected:
    virtual void gcUnlinkRefs();
    virtual void gcApplyToChildren(const CollectorCallback &callback);
private:
    DefExpr::Ptr var_;
    IRExpr::Ptr start_;
    IRExpr::Ptr end_;
    IRExpr::Ptr step_;
    IRExpr::Ptr body_;
};

/// LetExpr - Expression class for var/in
class KIARA_COMPILER_API LetExpr: public IRExpr
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(LetExpr, IRExpr)
public:

    LetExpr(const DefExpr::Ptr &var,
            const IRExpr::Ptr &initValue,
            const IRExpr::Ptr &body);

    LetExpr(const std::string &varName,
            const IRExpr::Ptr &initValue,
            const IRExpr::Ptr &body);

    const DefExpr::Ptr & getVar() const { return var_; }

    const IRExpr::Ptr & getInitValue() const { return initValue_; }

    const IRExpr::Ptr & getBody() const { return body_; }

    virtual bool replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr);

protected:
    virtual void gcUnlinkRefs();
    virtual void gcApplyToChildren(const CollectorCallback &callback);
private:
    DefExpr::Ptr var_;
    IRExpr::Ptr initValue_;
    IRExpr::Ptr body_;
};

/// BlockExpr - sequence of expressions, last expression is returned
class KIARA_COMPILER_API BlockExpr: public IRExpr
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(BlockExpr, IRExpr)
    friend class BreakExpr;
public:

    typedef std::vector<IRExpr::Ptr> ExprList;

    BlockExpr(ArrayRef<IRExpr::Ptr> exprList, const std::string &name = "");
    BlockExpr(World &world, const std::string &name = "");
    BlockExpr(ArrayRef<IRExpr::Ptr> exprList, World &world, const std::string &name = "");

    const std::string & getName() const { return name_; }

    void setName(const std::string &name);

    const ExprList & getExprList() const { return exprList_; }

    ExprList & getExprList() { return exprList_; }

    const ExprList & getBreakList() const { return breakList_; }

    bool isBreakTarget() const { return !breakList_.empty(); }

    void setExprList(ArrayRef<IRExpr::Ptr> exprList);

    size_t getExprListSize() const { return exprList_.size(); }

    void setExprListSize(size_t newSize)
    {
        exprList_.resize(newSize);
        update();
    }

    void setExprAt(size_t index, const IRExpr::Ptr &expr)
    {
        exprList_[index] = expr;
        // FIXME should we update ?
    }

    void addExpr(const IRExpr::Ptr &expr);

    void addExpr(const IRExpr::Ptr &expr1, const IRExpr::Ptr &expr2)
    {
        addExpr(expr1);
        addExpr(expr2);
    }

    void addExpr(const IRExpr::Ptr &expr1, const IRExpr::Ptr &expr2, const IRExpr::Ptr &expr3)
    {
        addExpr(expr1);
        addExpr(expr2);
        addExpr(expr3);
    }

    void addExpr(const IRExpr::Ptr &expr1, const IRExpr::Ptr &expr2, const IRExpr::Ptr &expr3, const IRExpr::Ptr &expr4)
    {
        addExpr(expr1);
        addExpr(expr2);
        addExpr(expr3);
        addExpr(expr4);
    }

    void update();

    virtual bool replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr);

protected:
    virtual void gcUnlinkRefs();
    virtual void gcApplyToChildren(const CollectorCallback &callback);
private:
    std::string name_;
    ExprList exprList_;
    ExprList breakList_;

    void addBreak(const IRExpr::Ptr &expr);
    void removeBreak(const IRExpr::Ptr &expr);
};

/// BreakExpr - Expression class for break
class KIARA_COMPILER_API BreakExpr: public IRExpr
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(BreakExpr, IRExpr)
public:

    BreakExpr(const IRExpr::Ptr &block,
              const IRExpr::Ptr &value);

    BreakExpr(const IRExpr::Ptr &block);

    ~BreakExpr();

    const IRExpr::Ptr & getValue() const { return value_; }

    const IRExpr::Ptr & getBlock() const { return block_; }

    virtual bool replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr);

protected:
    virtual void gcUnlinkRefs();
    virtual void gcApplyToChildren(const CollectorCallback &callback);
private:
    IRExpr::Ptr block_;
    IRExpr::Ptr value_;
};


/// Prototype - This class represents the "prototype" for a function,
/// which captures its argument names as well as if it is an operator.
class KIARA_COMPILER_API Prototype : public KIARA::Object, public StringAttributeHolder, public LocationHolder
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(Prototype, KIARA::Object)
public:

    typedef std::pair<std::string, Type::Ptr> Arg;

    Prototype(const std::string &name, const Type::Ptr &returnType, ArrayRef<Arg> args, KIARA::World &world,
              bool isOperator = false, unsigned prec = 0);

    Prototype(const std::string &name,
              const std::string &mangledName,
              const Type::Ptr &returnType, ArrayRef<Arg> args, KIARA::World &world,
              bool isOperator = false, unsigned prec = 0);

    bool isUnaryOp() const
    {
        return isOperator_ && args_.size() == 1;
    }

    bool isBinaryOp() const
    {
        return isOperator_ && args_.size() == 2;
    }

    void setMangledName(const std::string &name);

    const std::string & getName() const { return name_; }

    const std::string & getMangledName() const { return mangledName_; }

    const std::string & getOperatorName() const
    {
        assert(isUnaryOp() || isBinaryOp());
        return name_;
    }

    unsigned getBinaryPrecedence() const
    {
        return precedence_;
    }

    const FunctionType::Ptr & getFunctionType() const
    {
        return functionType_;
    }

    // return type can only be set if it is not known yet
    void setReturnType(const Type::Ptr &type);

    const Type::Ptr & getReturnType() const
    {
        return returnType_;
    }

    const std::vector<Arg> & getArgs() const
    {
        return args_;
    }

    size_t getNumArgs() const { return args_.size(); }
    const Arg & getArg(size_t index) const { return args_[index]; }
    const std::string & getArgName(size_t index) const { return getArg(index).first; }
    const Type::Ptr & getArgType(size_t index) const { return getArg(index).second; }

    virtual void print(std::ostream &out) const;

protected:
    virtual void gcUnlinkRefs();
    virtual void gcApplyToChildren(const CollectorCallback &callback);
private:
    std::string name_;
    std::string mangledName_;
    FunctionType::Ptr functionType_;
    Type::Ptr returnType_;
    std::vector<Arg> args_;
    bool isOperator_;
    unsigned precedence_;  // Precedence if a binary op.

    const FunctionType::Ptr & computeFunctionType();
};

/// TypeDefinition - This is a type definition class.
class KIARA_COMPILER_API TypeDefinition : public KIARA::Object
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(TypeDefinition, KIARA::Object)
public:

    TypeDefinition(const Type::Ptr &type);

    TypeDefinition(const std::string &name, const Type::Ptr &type);

    const Type::Ptr & getDefinedType() const { return type_; }

    const std::string & getDefinedTypeName() const { return name_; }

    virtual void print(std::ostream &out) const;

protected:
    virtual void gcUnlinkRefs();
    virtual void gcApplyToChildren(const CollectorCallback &callback);
private:
    std::string name_;
    Type::Ptr type_;
};

/// FunctionDefinition - This is a function definition base class.
class KIARA_COMPILER_API FunctionDefinition : public IR::IRExpr
{
    DFC_DECLARE_ABSTRACT_TYPE(FunctionDefinition, IR::IRExpr)
public:

    FunctionDefinition(
            const Prototype::Ptr &proto,
            KIARA::World &world);

    FunctionType::Ptr getFunctionType() const { return proto_->getFunctionType(); }

    const Prototype::Ptr &getProto() const { return proto_; }

    const std::string & getName() const { return proto_->getName(); }

    const std::string & getMangledName() const { return proto_->getMangledName(); }

    size_t getNumArgs() const { return args_.size(); }

    const std::string & getArgName(size_t index) const { return proto_->getArg(index).first; }

    const Type::Ptr & getArgType(size_t index) const { return proto_->getArg(index).second; }

    const DefExpr::Ptr & getArg(size_t index) const { return args_[index]; }

    virtual IRExpr::Ptr getReference();

    virtual bool hasAddress() const { return true; }

    virtual bool replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr);

protected:
    virtual void gcUnlinkRefs();
    virtual void gcApplyToChildren(const CollectorCallback &callback);
private:
    Prototype::Ptr proto_;
    std::vector<DefExpr::Ptr> args_;
    MemRef::Ptr memRef_;
};

/// FunctionDeclaration - This class represents external function definition.
class KIARA_COMPILER_API FunctionDeclaration : public FunctionDefinition
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(FunctionDeclaration, FunctionDefinition)
public:

    FunctionDeclaration(const Prototype::Ptr &proto, bool isExtern = false);

    bool isExtern() const { return extern_; }

private:
    bool extern_;
};


/// Function - This class represents a function definition itself.
class KIARA_COMPILER_API Function : public FunctionDefinition
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(Function, FunctionDefinition)
public:

    Function(
            const Prototype::Ptr &proto,
            const IRExpr::Ptr &body = 0);

    const IRExpr::Ptr &getBody() const { return body_; }

    void setBody(const IRExpr::Ptr &body);

    virtual bool replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr);

protected:
    virtual void gcUnlinkRefs();
    virtual void gcApplyToChildren(const CollectorCallback &callback);
private:
    IRExpr::Ptr body_;
};

/// ExternFunction - This class represents external function definition.
class KIARA_COMPILER_API ExternFunction : public FunctionDefinition
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(ExternFunction, FunctionDefinition)
public:

    ExternFunction(const Prototype::Ptr &proto);
};

/// Intrinsic - This class represents intrinsic function definition.
class KIARA_COMPILER_API Intrinsic : public FunctionDefinition
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(Intrinsic, FunctionDefinition)
public:

    Intrinsic(
            const Prototype::Ptr &proto,
            const std::string &body,
            KIARA::World &world)
        : FunctionDefinition(proto, world)
        , body_(body)
    {
    }

    const std::string &getBody() const { return body_; }

private:
    std::string body_;
};

/// CallExpr - Expression class for function calls.
class KIARA_COMPILER_API CallExpr: public IRExpr
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(CallExpr, IRExpr)
public:

    CallExpr(const IRExpr::Ptr &callee, const IRExpr::Ptr &operand);

    CallExpr(const IRExpr::Ptr &callee,
             const IRExpr::Ptr &lhs,
             const IRExpr::Ptr &rhs);

    CallExpr(const IRExpr::Ptr &callee, std::vector<IRExpr::Ptr> &args);

    CallExpr(const IRExpr::Ptr &callee, ArrayRef<IRExpr::Ptr> args);

    FunctionDefinition::Ptr getCalledFunction() const
    {
        return dyn_cast<FunctionDefinition>(callee_);
    }

    const IRExpr::Ptr & getCallee() const { return callee_; }
    const std::vector<IRExpr::Ptr> &getArgs() const { return args_; }
    size_t getNumArgs() const { return args_.size(); }
    const IRExpr::Ptr & getArg(size_t index) const { return args_[index]; }

    virtual bool replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr);

protected:
    virtual void gcUnlinkRefs();
    virtual void gcApplyToChildren(const CollectorCallback &callback);
private:
    IRExpr::Ptr callee_;
    std::vector<IRExpr::Ptr> args_;
};

template <typename SubClass, typename RetType=void>
class ExprVisitor
{
public:

    // Generic apply method - Allow application to all types in a range
    template<class Iterator>
    void apply(Iterator start, Iterator end)
    {
        while (start != end)
            apply(*start++);
    }

    // Forward functions with pointer arguments to versions with references
    RetType apply(const DFC::PointerTraits<KIARA::Object>::Ptr &type)
    {
        if (type)
            return apply(*type);
        DFC_THROW_EXCEPTION(KIARA::Exception, "NULL pointer");
        KIARA_UNREACHABLE("exception");
    }

    RetType apply(KIARA::Object *type)
    {
        if (type)
            return apply(*type);
        DFC_THROW_EXCEPTION(KIARA::Exception, "NULL pointer");
        KIARA_UNREACHABLE("exception");
    }

    RetType apply(KIARA::Object &object)
    {
        const DFC::ObjectType &typeId = object.getType();

#define HANDLE_BASETYPE(TYPE)                               \
        if (TYPE *baseType = dyn_cast<TYPE>(&object))       \
            return static_cast<SubClass*>(this)->visit(*baseType);

#define HANDLE_TYPE(TYPE)                                   \
        if (typeId == DFC_STATIC_TYPE(TYPE))                \
            return static_cast<SubClass*>(this)->visit(static_cast<TYPE&>(object));
#include "IRDefs.hpp"
        DFC_THROW_EXCEPTION(KIARA::Exception, "Unhandled type: "<<object.getTypeName());
        KIARA_UNREACHABLE("exception");
    }

    // Fallback to the superclass
#define HANDLE_BASETYPE(TYPE)                                   \
    RetType visit(TYPE &object)                                 \
    {                                                           \
        DFC_THROW_EXCEPTION(KIARA::Exception,                   \
            "Support for object type: "                         \
            <<static_cast<DFC::Object&>(object).getTypeName()   \
            <<" was not implemented yet");                      \
        return RetType();                                       \
    }

#define HANDLE_SUBTYPE(TYPE)                                    \
    RetType visit(TYPE &object)                                 \
    {                                                           \
        return static_cast<SubClass*>(this)->visit(             \
                static_cast<TYPE::InheritedType&>(object));     \
    }
#include "IRDefs.hpp"

};

template <typename SubClass, typename ArgType, typename RetType=void>
class ExprVisitorWithArg
{
public:

    // Generic apply method - Allow application to all types in a range
    template<class Iterator>
    void apply(Iterator start, Iterator end, ArgType arg)
    {
        while (start != end)
            static_cast<SubClass*>(this)->apply(*start++, arg);
    }

    // Forward functions with pointer arguments to versions with references
    RetType apply(const DFC::PointerTraits<KIARA::Object>::Ptr &object, ArgType arg)
    {
        if (object)
            return static_cast<SubClass*>(this)->apply(*object, arg);
        DFC_THROW_EXCEPTION(KIARA::Exception, "NULL pointer");
    }

    RetType apply(KIARA::Object *object, ArgType arg)
    {
        if (object)
            return static_cast<SubClass*>(this)->apply(*object, arg);
        DFC_THROW_EXCEPTION(KIARA::Exception, "NULL pointer");
    }

    RetType apply(KIARA::Object &object, ArgType arg)
    {
        const DFC::ObjectType &typeId = object.getType();

#define HANDLE_BASETYPE(TYPE)                               \
        if (TYPE *baseType = dyn_cast<TYPE>(&object))       \
            return static_cast<SubClass*>(this)->visit(*baseType, arg);

#define HANDLE_TYPE(TYPE)                                       \
        if (typeId == DFC_STATIC_TYPE(TYPE))                    \
            return static_cast<SubClass*>(this)->visit(static_cast<TYPE&>(object), arg);

#include "IRDefs.hpp"
        DFC_THROW_EXCEPTION(KIARA::Exception, "Unhandled type: "<<object.getTypeName());
    }

    // Fallback to the superclass
#define HANDLE_BASETYPE(TYPE)                                   \
    RetType visit(TYPE &object, ArgType arg)                    \
    {                                                           \
        DFC_THROW_EXCEPTION(KIARA::Exception,                   \
            "Support for object type: "                         \
            <<static_cast<DFC::Object&>(object).getTypeName()   \
            <<" was not implemented yet");                      \
        return RetType();                                       \
    }

#define HANDLE_SUBTYPE(TYPE)                                    \
    RetType visit(TYPE &object, ArgType arg)                    \
    {                                                           \
        return static_cast<SubClass*>(this)->visit(             \
                static_cast<TYPE::InheritedType&>(object), arg);\
    }
#include "IRDefs.hpp"

};

} // namespace IR

} // namespace KIARA

#endif /* KIARA_COMPILER_IR_HPP_INCLUDED */
