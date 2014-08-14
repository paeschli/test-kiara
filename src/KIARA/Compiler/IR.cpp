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
 * IR.cpp
 *
 *  Created on: 30.01.2013
 *      Author: Dmitri Rubinstein
 */
#define KIARA_COMPILER_LIB
#include "IR.hpp"
#include "IRUtils.hpp"
#include "PrettyPrinter.hpp"
#include <DFC/Base/Core/ObjectFactory.hpp>
#include <DFC/Base/Core/ObjectMacros.hpp>
#include <DFC/Base/Utils/StaticInit.hpp>
#include <DFC/Utils/StrUtils.hpp>
#include <boost/assert.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/mem_fn.hpp>
#include <algorithm>

namespace KIARA
{

namespace IR
{

// RTTI
DFC_DEFINE_ABSTRACT_TYPE(KIARA::IR::IRExpr)
DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::IR::MemRef)
DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::IR::PrimLiteral)
DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::IR::ListLiteral)
DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::IR::TypeExpr)
DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::IR::DefExpr)
DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::IR::SymbolExpr)
DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::IR::CallExpr)
DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::IR::IfExpr)
DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::IR::LoopExpr)
DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::IR::ForExpr)
DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::IR::LetExpr)
DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::IR::BlockExpr)
DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::IR::BreakExpr)
DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::IR::Prototype)
DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::IR::TypeDefinition)
DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::IR::FunctionDefinition)
DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::IR::FunctionDeclaration)
DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::IR::Function)
DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::IR::ExternFunction)
DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::IR::Intrinsic)

/// IRExpr

IRExpr::IRExpr(const KIARA::Type::Ptr &exprType)
    : Expr(exprType)
{
}

IRExpr::IRExpr(const KIARA::Type::Ptr &exprType, KIARA::World &world)
    : Expr(exprType, world)
{
}

void IRExpr::print(std::ostream &out) const
{
    PrettyPrinter::print(out, this);
}

// MemRef

static inline RefType::Ptr getRefType(const IRExpr::Ptr &value)
{
    Type::Ptr valueTy = value->getExprType();
    if (RefType::Ptr refTy = valueTy->getAs<RefType>())
        return refTy;
    return RefType::get(valueTy);
}

static inline IRExpr::Ptr removeRef(const IRExpr::Ptr &value)
{
    if (MemRef::Ptr memRef = dyn_cast<MemRef>(value))
        return memRef->getValue();
    return value;
}

MemRef::MemRef(const IRExpr::Ptr &value)
    : IRExpr(getRefType(value), value->getWorld())
    , value_(removeRef(value))
{
}

bool MemRef::replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr)
{
    bool success = InheritedType::replaceExpr(oldExpr, newExpr);
    if (value_ == oldExpr)
    {
        value_ = dyn_cast<IRExpr>(newExpr);
        success |= true;
    }
    return success;
}

void MemRef::gcUnlinkRefs()
{
    InheritedType::gcUnlinkRefs();
    gcUnlinkChild(value_);
}

void MemRef::gcApplyToChildren(const CollectorCallback &callback)
{
    InheritedType::gcApplyToChildren(callback);
    gcApply(value_, callback);
}

// PrimLiteral

PrimLiteral::PrimLiteral(const PrimLiteral &value)
    : IRExpr(value.getExprType())
    , value_(value.value_)
{
}

PrimLiteral::PrimLiteral(const PrimLiteral::Ptr &valueType)
    : IRExpr(valueType->getExprType())
{
}

#define _KIARA_PRIMLITERAL_CONSTR(T)                                \
PrimLiteral::PrimLiteral(T v, KIARA::World &w)                      \
    : IRExpr(w.c_type<T>(), w)                                      \
    , value_(v)                                                     \
{                                                                   \
}

_KIARA_PRIMLITERAL_CONSTR(int8_t)
_KIARA_PRIMLITERAL_CONSTR(uint8_t)
_KIARA_PRIMLITERAL_CONSTR(int16_t)
_KIARA_PRIMLITERAL_CONSTR(uint16_t)
_KIARA_PRIMLITERAL_CONSTR(int32_t)
_KIARA_PRIMLITERAL_CONSTR(uint32_t)
_KIARA_PRIMLITERAL_CONSTR(int64_t)
_KIARA_PRIMLITERAL_CONSTR(uint64_t)
_KIARA_PRIMLITERAL_CONSTR(float)
_KIARA_PRIMLITERAL_CONSTR(double)
_KIARA_PRIMLITERAL_CONSTR(bool)

PrimLiteral::PrimLiteral(NullPtrTag tag, KIARA::World &w)
    : IRExpr(w.type_c_nullptr(), w)
    , value_(static_cast<void*>(0))
{
}

PrimLiteral::PrimLiteral(const char * v, KIARA::World &w)
    : IRExpr(w.type_string(), w)
    , value_(v)
{
}

PrimLiteral::PrimLiteral(const std::string & v, KIARA::World &w)
    : IRExpr(w.type_string(), w)
    , value_(v)
{
}

void PrimLiteral::destroyValue()
{
    value_.clear();
}

void PrimLiteral::gcUnlinkRefs()
{
    // We need to destroy value before unlinking, since
    // destruction depends on the valueType, which will be
    // set to NULL after unlinking.
    destroyValue();
    InheritedType::gcUnlinkRefs();
}

PrimLiteral::~PrimLiteral()
{
    destroyValue();
}

PrimLiteral::Ptr PrimLiteral::getZero(const KIARA::Type::Ptr &type)
{
    PrimType::Ptr ty = dyn_cast<PrimType>(type->getCanonicalType());
    if (!ty)
        return 0;
    switch (ty->primtype_kind())
    {
        case PRIMTYPE_i8: return new PrimLiteral(static_cast<int8_t>(0), type->getWorld());
        case PRIMTYPE_u8: return new PrimLiteral(static_cast<uint8_t>(0), type->getWorld());
        case PRIMTYPE_i16: return new PrimLiteral(static_cast<int16_t>(0), type->getWorld());
        case PRIMTYPE_u16: return new PrimLiteral(static_cast<uint16_t>(0), type->getWorld());
        case PRIMTYPE_i32: return new PrimLiteral(static_cast<int32_t>(0), type->getWorld());
        case PRIMTYPE_u32: return new PrimLiteral(static_cast<uint32_t>(0), type->getWorld());
        case PRIMTYPE_i64: return new PrimLiteral(static_cast<int64_t>(0), type->getWorld());
        case PRIMTYPE_u64: return new PrimLiteral(static_cast<uint64_t>(0), type->getWorld());
        case PRIMTYPE_float: return new PrimLiteral(static_cast<float>(0), type->getWorld());
        case PRIMTYPE_double: return new PrimLiteral(static_cast<double>(0), type->getWorld());
        case PRIMTYPE_boolean: return new PrimLiteral(false, type->getWorld());
        case PRIMTYPE_string: break;
        case PRIMTYPE_c_int8_t: return new PrimLiteral(static_cast<int8_t>(0), type->getWorld());
        case PRIMTYPE_c_uint8_t: return new PrimLiteral(static_cast<uint8_t>(0), type->getWorld());
        case PRIMTYPE_c_int16_t: return new PrimLiteral(static_cast<int16_t>(0), type->getWorld());
        case PRIMTYPE_c_uint16_t: return new PrimLiteral(static_cast<uint16_t>(0), type->getWorld());
        case PRIMTYPE_c_int32_t: return new PrimLiteral(static_cast<int32_t>(0), type->getWorld());
        case PRIMTYPE_c_uint32_t: return new PrimLiteral(static_cast<uint32_t>(0), type->getWorld());
        case PRIMTYPE_c_int64_t: return new PrimLiteral(static_cast<int64_t>(0), type->getWorld());
        case PRIMTYPE_c_uint64_t: return new PrimLiteral(static_cast<uint64_t>(0), type->getWorld());
        case PRIMTYPE_c_float: return new PrimLiteral(static_cast<float>(0), type->getWorld());
        case PRIMTYPE_c_double: return new PrimLiteral(static_cast<double>(0), type->getWorld());
        case PRIMTYPE_c_longdouble: break; // FIXME not supported yet
        case PRIMTYPE_c_bool: return new PrimLiteral(false, type->getWorld());
        case PRIMTYPE_c_nullptr: return getNullPtr(type->getWorld());
    }
    return 0;
}

bool PrimLiteral::equals(const KIARA::Object::Ptr &other) const
{
    const ThisType::Ptr o = DFC::safe_object_cast<ThisType>(other);
    if (!o)
        return false;
    return *this == *o;
}

/// ListLiteral

ListLiteral::ListLiteral(const Type::Ptr &valueType)
    : IRExpr(valueType, valueType->getWorld())
{
}

ListLiteral::ListLiteral(const ListLiteral &other)
    : IRExpr(other.getExprType(), other.getWorld())
    , value_(other.value_)
{
}

ListLiteral::~ListLiteral()
{
}

bool ListLiteral::equals(const Object::Ptr &other) const
{
    const ThisType::Ptr o = DFC::safe_object_cast<ThisType>(other);
    if (!o)
        return false;
    return *this == *o;
}

bool ListLiteral::operator==(const ListLiteral &other) const
{
    if (!canonicallyEqual(getExprType(), other.getExprType()))
        return false;
    if (value_.size() != other.value_.size())
        return false;
    typedef std::vector<IRExpr::Ptr>::const_iterator Iter;
    for (Iter it1 = value_.begin(), it2 = other.value_.begin(), end1 = value_.end();
            it1 != end1; ++it1, ++it2)
    {
        const IRExpr::Ptr &v1 = *it1;
        const IRExpr::Ptr &v2 = *it2;
        if (!v1->equals(v2))
            return false;
    }
    return true;
}

void ListLiteral::print(std::ostream &out) const
{
    out << "[";
    typedef std::vector<IRExpr::Ptr>::const_iterator Iter;
    for (Iter begin = value_.begin(), it = begin, end = value_.end(); it != end; ++it)
    {
        if (it != begin)
            out << ", ";
        (*it)->printRepr(out);
    }
    out << "]";
}

void ListLiteral::gcUnlinkRefs()
{
    InheritedType::gcUnlinkRefs();
    gcUnlinkChildren(value_.begin(), value_.end());
}

void ListLiteral::gcApplyToChildren(const CollectorCallback &callback)
{
    InheritedType::gcApplyToChildren(callback);
    gcApply(value_.begin(), value_.end(), callback);
}

// TypeExpr

TypeExpr::TypeExpr(const Type::Ptr &typeValue)
    : IRExpr(TypeType::get(typeValue->getWorld()))
    , typeValue_(typeValue)
{
}

void TypeExpr::gcUnlinkRefs()
{
    InheritedType::gcUnlinkRefs();
    gcUnlinkChild(typeValue_);
}

void TypeExpr::gcApplyToChildren(const CollectorCallback &callback)
{
    InheritedType::gcApplyToChildren(callback);
    gcApply(typeValue_, callback);
}

static std::string makeUniqueNameFromPointer(const char *prefix, void *p)
{
    std::ostringstream oss;
    oss<<prefix<<std::hex<<reinterpret_cast<uintptr_t>(p);
    return oss.str();
}

// DefExpr

DefExpr::DefExpr(const std::string &name, const Type::Ptr &type, bool hasAddress)
    : IRExpr(type)
    , name_(name)
    , hasAddress_(hasAddress)
{
    if (name_.empty())
    {
        name_ = makeUniqueNameFromPointer("$var_", this);
    }
}

IRExpr::Ptr DefExpr::getReference()
{
    if (!memRef_ && hasAddress_)
        memRef_ = new MemRef(this);
    return memRef_;
}

bool DefExpr::replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr)
{
    bool success = InheritedType::replaceExpr(oldExpr, newExpr);
    if (memRef_ == oldExpr)
    {
        memRef_ = dyn_cast<MemRef>(newExpr);
        success |= true;
    }
    return success;
}

void DefExpr::gcUnlinkRefs()
{
    InheritedType::gcUnlinkRefs();
    gcUnlinkChild(memRef_);
}

void DefExpr::gcApplyToChildren(const CollectorCallback &callback)
{
    InheritedType::gcApplyToChildren(callback);
    gcApply(memRef_, callback);
}

// SymbolExpr

SymbolExpr::SymbolExpr(const std::string &name, World &world)
    : IRExpr(SymbolType::get(world, name))
    , name_(name)
    , scope_()
{
}

SymbolExpr::SymbolExpr(const std::string &name, const Object::Ptr &scope)
    : IRExpr(SymbolType::get(scope->getWorld(), name))
    , name_(name)
    , scope_(scope)
{
}

void SymbolExpr::gcUnlinkRefs()
{
    InheritedType::gcUnlinkRefs();
    gcUnlinkChild(scope_);
}

void SymbolExpr::gcApplyToChildren(const CollectorCallback &callback)
{
    InheritedType::gcApplyToChildren(callback);
    gcApply(scope_, callback);
}

// IfExpr

static inline Type::Ptr getMostSpecializedType(const Type::Ptr &t1, const Type::Ptr &t2)
{
    if (t1 && !t2)
        return VoidType::get(t1->getWorld());
    if (t2 && !t1)
        return VoidType::get(t2->getWorld());
    if (canonicallyEqual(t1, AnyType::get(t1->getWorld())))
        return t2;
    if (canonicallyEqual(t2, AnyType::get(t2->getWorld())))
        return t1;
    if (canonicallyEqual(t1, t2))
        return t1;
    return VoidType::get(t1->getWorld());
}

static inline Type::Ptr getIfType(const IRExpr::Ptr &then, const IRExpr::Ptr &_else, World &world)
{
    Type::Ptr thenTy, elseTy;
    if (then)
        thenTy = then->getExprType();
    if (_else)
        elseTy = _else->getExprType();
    return getMostSpecializedType(thenTy, elseTy);
}

IfExpr::IfExpr(const IRExpr::Ptr &cond, const IRExpr::Ptr &then, const IRExpr::Ptr &_else)
    : IRExpr(getIfType(then, _else, cond->getWorld()), cond->getWorld())
    , cond_(cond), then_(then), else_(_else)
{
}

IfExpr::IfExpr(const IRExpr::Ptr &cond, const IRExpr::Ptr &then, const IRExpr::Ptr &_else, KIARA::World &world)
    : IRExpr(getIfType(then, _else, world), world)
    , cond_(cond), then_(then), else_(_else)
{
}

bool IfExpr::replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr)
{
    bool success = InheritedType::replaceExpr(oldExpr, newExpr);
    if (cond_ == oldExpr)
    {
        cond_ = dyn_cast<IRExpr>(newExpr);
        success |= true;
    }
    if (then_ == oldExpr)
    {
        then_ = dyn_cast<IRExpr>(newExpr);
        success |= true;
    }
    if (else_ == oldExpr)
    {
        else_ = dyn_cast<IRExpr>(newExpr);
        success |= true;
    }
    return success;
}

void IfExpr::gcUnlinkRefs()
{
    InheritedType::gcUnlinkRefs();
    gcUnlinkChild(cond_);
    gcUnlinkChild(then_);
    gcUnlinkChild(else_);
}

void IfExpr::gcApplyToChildren(const CollectorCallback &callback)
{
    InheritedType::gcApplyToChildren(callback);
    gcApply(cond_, callback);
    gcApply(then_, callback);
    gcApply(else_, callback);
}

// LoopExpr

LoopExpr::LoopExpr(const IRExpr::Ptr &body)
    : IRExpr(VoidType::get(body->getWorld()), body->getWorld())
    , body_(body)
{
}

bool LoopExpr::replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr)
{
    bool success = InheritedType::replaceExpr(oldExpr, newExpr);
    if (body_ == oldExpr)
    {
        body_ = dyn_cast<IRExpr>(newExpr);
        success |= true;
    }
    return success;
}

void LoopExpr::gcUnlinkRefs()
{
    InheritedType::gcUnlinkRefs();
    gcUnlinkChild(body_);
}

void LoopExpr::gcApplyToChildren(const CollectorCallback &callback)
{
    InheritedType::gcApplyToChildren(callback);
    gcApply(body_, callback);
}

// ForExpr

ForExpr::ForExpr(
        const std::string &varName,
        const Type::Ptr &varType,
        const IRExpr::Ptr &start,
        const IRExpr::Ptr &end,
        const IRExpr::Ptr &step,
        const IRExpr::Ptr &body)
    : IRExpr(body->getExprType(), body->getWorld())
    , var_(new DefExpr(varName, varType, true))
    , start_(start)
    , end_(end)
    , step_(step)
    , body_(body)
{
}

ForExpr::ForExpr(
        const DefExpr::Ptr &var,
        const IRExpr::Ptr &start,
        const IRExpr::Ptr &end,
        const IRExpr::Ptr &step,
        const IRExpr::Ptr &body)
    : IRExpr(body->getExprType(), body->getWorld())
    , var_(var)
    , start_(start)
    , end_(end)
    , step_(step)
    , body_(body)
{
}


bool ForExpr::replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr)
{
    bool success = InheritedType::replaceExpr(oldExpr, newExpr);
    if (var_ == oldExpr)
    {
        var_ = dyn_cast<DefExpr>(newExpr);
        success |= true;
    }
    if (start_ == oldExpr)
    {
        start_ = dyn_cast<IRExpr>(newExpr);
        success |= true;
    }
    if (end_ == oldExpr)
    {
        end_ = dyn_cast<IRExpr>(newExpr);
        success |= true;
    }
    if (step_ == oldExpr)
    {
        step_ = dyn_cast<IRExpr>(newExpr);
        success |= true;
    }
    if (body_ == oldExpr)
    {
        body_ = dyn_cast<IRExpr>(newExpr);
        success |= true;
    }
    return success;
}

void ForExpr::gcUnlinkRefs()
{
    InheritedType::gcUnlinkRefs();
    gcUnlinkChild(var_);
    gcUnlinkChild(start_);
    gcUnlinkChild(end_);
    gcUnlinkChild(step_);
    gcUnlinkChild(body_);
}

void ForExpr::gcApplyToChildren(const CollectorCallback &callback)
{
    InheritedType::gcApplyToChildren(callback);
    gcApply(var_, callback);
    gcApply(start_, callback);
    gcApply(end_, callback);
    gcApply(step_, callback);
    gcApply(body_, callback);
}

// LetExpr

LetExpr::LetExpr(const DefExpr::Ptr &var,
                 const IRExpr::Ptr &initValue,
                 const IRExpr::Ptr &body)
    : IRExpr(body->getExprType(), body->getWorld())
    , var_(var)
    , initValue_(initValue)
    , body_(body)
{
}

LetExpr::LetExpr(const std::string &varName,
                 const IRExpr::Ptr &initValue,
                 const IRExpr::Ptr &body)
    : IRExpr(body->getExprType(), body->getWorld())
    , var_(new DefExpr(varName, initValue->getExprType(), true))
    , body_(body)
{
}

bool LetExpr::replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr)
{
    bool success = InheritedType::replaceExpr(oldExpr, newExpr);
    if (var_ == oldExpr)
    {
        var_ = dyn_cast<DefExpr>(newExpr);
        success |= true;
    }
    if (initValue_ == oldExpr)
    {
        initValue_ = dyn_cast<IRExpr>(newExpr);
        success |= true;
    }
    if (body_ == oldExpr)
    {
        body_ = dyn_cast<IRExpr>(newExpr);
        success |= true;
    }
    return success;
}

void LetExpr::gcUnlinkRefs()
{
    InheritedType::gcUnlinkRefs();
    gcUnlinkChild(var_);
    gcUnlinkChild(initValue_);
    gcUnlinkChild(body_);
}

void LetExpr::gcApplyToChildren(const CollectorCallback &callback)
{
    InheritedType::gcApplyToChildren(callback);
    gcApply(var_, callback);
    gcApply(initValue_, callback);
    gcApply(body_, callback);
}

// BlockExpr

BlockExpr::BlockExpr(ArrayRef<IRExpr::Ptr> exprList, const std::string &name)
    : IRExpr(exprList.back()->getExprType(), exprList.back()->getWorld())
    , name_(name)
    , exprList_(exprList.begin(), exprList.end())
{
}

BlockExpr::BlockExpr(World &world, const std::string &name)
    : IRExpr(world.type_void(), world), name_(name)
{
}

BlockExpr::BlockExpr(ArrayRef<IRExpr::Ptr> exprList, World &world, const std::string &name)
    : IRExpr(exprList.empty() ? VoidType::get(world) : exprList.back()->getExprType(), world)
    , name_(name)
    , exprList_(exprList.begin(), exprList.end())
{
}

void BlockExpr::setName(const std::string &name)
{
    name_ = name;
}

void BlockExpr::setExprList(ArrayRef<IRExpr::Ptr> exprList)
{
    exprList_.clear();
    exprList_.insert(exprList_.begin(), exprList.begin(), exprList.end());
    update();
}

void BlockExpr::addExpr(const IRExpr::Ptr &expr)
{
    exprList_.push_back(expr);
    update();
}

void BlockExpr::update()
{
    if (exprList_.empty())
        setExprType(Type::Ptr());
    else
    {
        if (exprList_.back())
            setExprType(exprList_.back()->getExprType());
    }
}

bool BlockExpr::replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr)
{
    bool success = InheritedType::replaceExpr(oldExpr, newExpr);
    for (ExprList::iterator it = exprList_.begin(), end = exprList_.end(); it != end; ++it)
    {
        if (*it == oldExpr)
        {
            *it = dyn_cast<IRExpr>(newExpr);
            success |= true;
        }
    }
    for (ExprList::iterator it = breakList_.begin(), end = breakList_.end(); it != end; ++it)
    {
        if (*it == oldExpr)
        {
            *it = dyn_cast<IRExpr>(newExpr);
            success |= true;
        }
    }
    return success;
}

void BlockExpr::gcUnlinkRefs()
{
    InheritedType::gcUnlinkRefs();
    gcUnlinkChildren(exprList_.begin(), exprList_.end());
    gcUnlinkChildren(breakList_.begin(), breakList_.end());
}

void BlockExpr::gcApplyToChildren(const CollectorCallback &callback)
{
    InheritedType::gcApplyToChildren(callback);
    gcApply(exprList_.begin(), exprList_.end(), callback);
    gcApply(breakList_.begin(), breakList_.end(), callback);
}

void BlockExpr::addBreak(const IRExpr::Ptr &expr)
{
    breakList_.push_back(expr);
    if (name_.empty())
    {
        name_ = makeUniqueNameFromPointer("$block_", this);
    }
}

void BlockExpr::removeBreak(const IRExpr::Ptr &expr)
{
    ExprList::iterator it = std::find(breakList_.begin(), breakList_.end(), expr);
    if (it != breakList_.end())
    {
        breakList_.erase(it);
    }
}

// BreakExpr

// LetExpr

BreakExpr::BreakExpr(const IRExpr::Ptr &block,
                     const IRExpr::Ptr &value)
    : IRExpr(value ? value->getExprType() : VoidType::get(block->getWorld()))
    , block_(block)
    , value_(value)
{
    if (BlockExpr::Ptr b = dyn_cast<BlockExpr>(block_))
    {
        b->addBreak(this);
    }
}

BreakExpr::BreakExpr(const IRExpr::Ptr &block)
    : IRExpr(VoidType::get(block->getWorld()))
    , block_(block)
    , value_()
{
    if (BlockExpr::Ptr b = dyn_cast<BlockExpr>(block_))
    {
        b->addBreak(this);
    }
}

BreakExpr::~BreakExpr()
{
    if (BlockExpr::Ptr b = dyn_cast<BlockExpr>(block_))
    {
        b->removeBreak(this);
    }
}

bool BreakExpr::replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr)
{
    bool success = InheritedType::replaceExpr(oldExpr, newExpr);
    if (block_ == oldExpr)
    {
        block_ = dyn_cast<IRExpr>(newExpr);
        success |= true;
    }
    if (value_ == oldExpr)
    {
        value_ = dyn_cast<IRExpr>(newExpr);
        success |= true;
    }
    return success;
}

void BreakExpr::gcUnlinkRefs()
{
    InheritedType::gcUnlinkRefs();
    gcUnlinkChild(block_);
    gcUnlinkChild(value_);
}

void BreakExpr::gcApplyToChildren(const CollectorCallback &callback)
{
    InheritedType::gcApplyToChildren(callback);
    gcApply(block_, callback);
    gcApply(value_, callback);
}

// Prototype

Prototype::Prototype(
        const std::string &name,
        const Type::Ptr &returnType, ArrayRef<Arg> args,
        KIARA::World &world,
        bool isoperator, unsigned prec)
    : Object(world)
    , name_(name)
    , mangledName_(name)
    , functionType_()
    , returnType_(returnType)
    , args_(args.begin(), args.end())
    , isOperator_(isoperator), precedence_(prec)
{
    computeFunctionType();
}

Prototype::Prototype(
        const std::string &name,
        const std::string &mangledName,
        const Type::Ptr &returnType, ArrayRef<Arg> args,
        KIARA::World &world,
        bool isoperator, unsigned prec)
    : Object(world)
    , name_(name)
    , mangledName_(mangledName)
    , functionType_()
    , returnType_(returnType)
    , args_(args.begin(), args.end())
    , isOperator_(isoperator), precedence_(prec)
{
    computeFunctionType();
}

void Prototype::setMangledName(const std::string &name)
{
    mangledName_ = name;
}

void Prototype::setReturnType(const Type::Ptr &type)
{
    if (canonicallyEqual(type, returnType_))
        return;

    if (returnType_ && !canonicallyEqual(returnType_, AnyType::get(type->getWorld())))
        DFC_THROW_EXCEPTION(Exception, "Return type is already set to "
                <<IR::IRUtils::getTypeName(returnType_)<<", cannot change to "
                <<IR::IRUtils::getTypeName(type));

    returnType_ = type;
    computeFunctionType();
}

const FunctionType::Ptr & Prototype::computeFunctionType()
{
    if (!functionType_ && returnType_)
    {
        typedef std::vector<Arg>::const_iterator Iter;
        for (Iter it = args_.begin(), end = args_.end(); it != end; ++it)
        {
            if (!it->second)
                return functionType_;
        }
        functionType_ = FunctionType::get(returnType_, args_);
    }
    return functionType_;
}

void Prototype::gcUnlinkRefs()
{
    InheritedType::gcUnlinkRefs();

    gcUnlinkChild(functionType_);
    gcUnlinkChild(returnType_);
    gcUnlinkChildren(GCObject::map_values_tag(), args_.begin(), args_.end());
}

void Prototype::gcApplyToChildren(const CollectorCallback &callback)
{
    InheritedType::gcApplyToChildren(callback);
    gcApply(functionType_, callback);
    gcApply(returnType_, callback);
    gcApply(GCObject::map_values_tag(), args_.begin(), args_.end(), callback);
}

void Prototype::print(std::ostream &out) const
{
    PrettyPrinter::print(out, this);
}

// TypeDefinition

TypeDefinition::TypeDefinition(
        const Type::Ptr &type)
    : Object(type->getWorld())
    , name_(type->getFullTypeName())
    , type_(type)
{
}

TypeDefinition::TypeDefinition(
        const std::string &name,
        const Type::Ptr &type)
    : Object(type->getWorld())
    , name_(name)
    , type_(type)
{
}

void TypeDefinition::gcUnlinkRefs()
{
    InheritedType::gcUnlinkRefs();
    gcUnlinkChild(type_);
}

void TypeDefinition::gcApplyToChildren(const CollectorCallback &callback)
{
    InheritedType::gcApplyToChildren(callback);
    gcApply(type_, callback);
}

void TypeDefinition::print(std::ostream &out) const
{
    PrettyPrinter::print(out, this);
}

// FunctionDefinition

FunctionDefinition::FunctionDefinition(
        const Prototype::Ptr &proto,
        KIARA::World &world)
    : IRExpr(proto->getFunctionType(), world)
    , proto_(proto)
    , args_(proto_->getNumArgs())
{
    const size_t numArgs = proto->getNumArgs();
    for (size_t i = 0; i < numArgs; ++i)
    {
        args_[i] = IR::IRUtils::createVariable(proto->getArg(i).first, proto->getArg(i).second);
    }
}

IRExpr::Ptr FunctionDefinition::getReference()
{
    if (!memRef_)
        memRef_ = new MemRef(this);
    return memRef_;
}

bool FunctionDefinition::replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr)
{
    bool success = InheritedType::replaceExpr(oldExpr, newExpr);
    // FIXME should proto_ be replaced as well ?
    for (std::vector<DefExpr::Ptr>::iterator it = args_.begin(), end = args_.end(); it != end; ++it)
    {
        if (*it == oldExpr)
        {
            *it = dyn_cast<DefExpr>(newExpr);
            success |= true;
        }
    }
    if (memRef_ == oldExpr)
    {
        memRef_ = dyn_cast<MemRef>(newExpr);
        success |= true;
    }
    return success;
}

void FunctionDefinition::gcUnlinkRefs()
{
    InheritedType::gcUnlinkRefs();
    gcUnlinkChild(proto_);
    gcUnlinkChildren(args_.begin(), args_.end());
    gcUnlinkChild(memRef_);
}

void FunctionDefinition::gcApplyToChildren(const CollectorCallback &callback)
{
    InheritedType::gcApplyToChildren(callback);
    gcApply(proto_, callback);
    gcApply(args_.begin(), args_.end(), callback);
    gcApply(memRef_, callback);
}

// FunctionDeclaration

FunctionDeclaration::FunctionDeclaration(
        const Prototype::Ptr &proto,
        bool isExtern)
    : FunctionDefinition(proto, proto->getWorld())
    , extern_(isExtern)
{
}

// Function

Function::Function(
        const Prototype::Ptr &proto,
        const IRExpr::Ptr &body)
    : FunctionDefinition(proto, proto->getWorld())
    , body_(body)
{
    if (body_)
        proto->setReturnType(body_->getExprType());
}

void Function::setBody(const IRExpr::Ptr &body)
{
    body_ = body;
    if (body)
        getProto()->setReturnType(body_->getExprType());
}

bool Function::replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr)
{
    bool success = InheritedType::replaceExpr(oldExpr, newExpr);
    if (body_ == oldExpr)
    {
        body_ = dyn_cast<IRExpr>(newExpr);
        success |= true;
    }
    return success;
}

void Function::gcUnlinkRefs()
{
    InheritedType::gcUnlinkRefs();
    gcUnlinkChild(body_);
}

void Function::gcApplyToChildren(const CollectorCallback &callback)
{
    InheritedType::gcApplyToChildren(callback);
    gcApply(body_, callback);
}

// ExternFunction

ExternFunction::ExternFunction(const Prototype::Ptr &proto)
    : FunctionDefinition(proto, proto->getWorld())
{
}

// Intrinsic

// CallIRExpr

static inline Type::Ptr getFunctionReturnType(const IRExpr::Ptr &expr)
{
    if (FunctionDefinition::Ptr func = dyn_cast<FunctionDefinition>(expr))
        return func->getProto()->getReturnType();
    Type::Ptr ty = expr->getExprType();
    PtrType::Ptr pty = ty->getAs<PtrType>();
    if (!pty)
        return 0;
    FunctionType::Ptr fty = pty->getElementType()->getAs<FunctionType>();
    if (!fty)
        return 0;
    return fty->getReturnType();
}

CallExpr::CallExpr(
        const IRExpr::Ptr &callee,
        const IRExpr::Ptr &operand)
    : IRExpr(getFunctionReturnType(callee), callee->getWorld())
    , callee_(callee)
    , args_(1, operand)
{
}

CallExpr::CallExpr(
        const IRExpr::Ptr &callee,
        const IRExpr::Ptr &lhs,
        const IRExpr::Ptr &rhs)
    : IRExpr(getFunctionReturnType(callee), callee->getWorld())
    , callee_(callee)
    , args_()
{
    args_.push_back(lhs);
    args_.push_back(rhs);
}

CallExpr::CallExpr(
        const IRExpr::Ptr &callee,
        std::vector<IRExpr::Ptr> &args)
    : IRExpr(getFunctionReturnType(callee), callee->getWorld())
    , callee_(callee)
    , args_(args)
{
}

CallExpr::CallExpr(const IRExpr::Ptr &callee, ArrayRef<IRExpr::Ptr> args)
    : IRExpr(getFunctionReturnType(callee), callee->getWorld())
    , callee_(callee)
    , args_(args.begin(), args.end())
{
}

bool CallExpr::replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr)
{
    bool success = InheritedType::replaceExpr(oldExpr, newExpr);
    for (std::vector<IRExpr::Ptr>::iterator it = args_.begin(), end = args_.end(); it != end; ++it)
    {
        if (*it == oldExpr)
        {
            *it = dyn_cast<IRExpr>(newExpr);
            success |= true;
        }
    }
    if (callee_ == oldExpr)
    {
        callee_ = dyn_cast<IRExpr>(newExpr);
        success |= true;
    }
    return success;
}

void CallExpr::gcUnlinkRefs()
{
    InheritedType::gcUnlinkRefs();
    gcUnlinkChild(callee_);
    gcUnlinkChildren(args_.begin(), args_.end());
}

void CallExpr::gcApplyToChildren(const CollectorCallback &callback)
{
    InheritedType::gcApplyToChildren(callback);
    gcApply(callee_, callback);
    gcApply(args_.begin(), args_.end(), callback);
}

DFC_STATIC_INIT_FUNC {
    DFC_REGISTER_TYPE(MemRef);
    DFC_REGISTER_TYPE(PrimLiteral);
    DFC_REGISTER_TYPE(ListLiteral);
    DFC_REGISTER_TYPE(TypeExpr);
    DFC_REGISTER_TYPE(DefExpr);
    DFC_REGISTER_TYPE(SymbolExpr);
    DFC_REGISTER_TYPE(IfExpr);
    DFC_REGISTER_TYPE(LoopExpr);
    DFC_REGISTER_TYPE(ForExpr);
    DFC_REGISTER_TYPE(LetExpr);
    DFC_REGISTER_TYPE(BlockExpr);
    DFC_REGISTER_TYPE(BreakExpr);
    DFC_REGISTER_TYPE(Prototype);
    DFC_REGISTER_TYPE(Function);
    DFC_REGISTER_TYPE(TypeDefinition);
    DFC_REGISTER_TYPE(FunctionDefinition);
    DFC_REGISTER_TYPE(FunctionDeclaration);
    DFC_REGISTER_TYPE(ExternFunction);
    DFC_REGISTER_TYPE(Intrinsic);
    DFC_REGISTER_TYPE(CallExpr);
}

} // namespace IR

} // namespace KIARA
