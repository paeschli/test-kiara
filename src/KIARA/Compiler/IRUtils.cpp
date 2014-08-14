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
 * IRUtils.cpp
 *
 *  Created on: 19.02.2013
 *      Author: Dmitri Rubinstein
 */
#define KIARA_COMPILER_LIB
#include "IRUtils.hpp"
#include "IR.hpp"
#include "Mangler.hpp"
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <DFC/Utils/StrUtils.hpp>

namespace KIARA
{

namespace IR
{

std::string IRUtils::getObjectName(const Object::Ptr &object)
{
    if (Type::Ptr ty = dyn_cast<Type>(object))
        return getTypeName(ty);
    std::ostringstream oss;
    oss << *object;
    return oss.str();
}

std::string IRUtils::getTypeName(const Type::Ptr &type)
{
    if (!type)
        return "<NULL>";

    World &world = type->getWorld();

    if (type == VoidType::get(world))
        return "void";

#define RET_CTYPE_NAME(name, exprType)          \
        if (type == world.c_type<exprType>())   \
            return KIARA_STRINGIZE(name)

    RET_CTYPE_NAME(char, char);
    RET_CTYPE_NAME(schar, signed char);
    RET_CTYPE_NAME(uchar, unsigned char);
    RET_CTYPE_NAME(int, int);
    RET_CTYPE_NAME(uint, unsigned int);
    RET_CTYPE_NAME(long, long);
    RET_CTYPE_NAME(ulong, unsigned long);
    RET_CTYPE_NAME(longlong, long long);
    RET_CTYPE_NAME(ulonglong, unsigned long long);
    RET_CTYPE_NAME(size_t, size_t);
    RET_CTYPE_NAME(ssize_t, ssize_t);
    RET_CTYPE_NAME(int8_t, int8_t);
    RET_CTYPE_NAME(uint8_t, uint8_t);
    RET_CTYPE_NAME(int32_t, int32_t);
    RET_CTYPE_NAME(uint32_t, uint32_t);
    RET_CTYPE_NAME(int64_t, int64_t);
    RET_CTYPE_NAME(uint64_t, uint64_t);
    RET_CTYPE_NAME(float, float);
    RET_CTYPE_NAME(double, double);

    if (type == AnyType::get(world))
        return "any";
    if (type == TypeType::get(world))
        return "type";
    if (type == world.type_boolean()) // TODO should be better use c_type<bool> ?
        return "boolean";
    if (type == world.type_string())
        return "string_literal";
    if (PtrType::Ptr pty = DFC::safe_object_cast<PtrType>(type))
    {
        return "ptr(" + getTypeName(pty->getElementType()) + ")";
    }
    if (RefType::Ptr rty = DFC::safe_object_cast<RefType>(type))
    {
        return "ref(" + getTypeName(rty->getElementType()) + ")";
    }
    if (ArrayType::Ptr aty = dyn_cast<ArrayType>(type))
    {
        return "array(" + getTypeName(aty->getElementType()) + ")";
    }
    if (FixedArrayType::Ptr faty = dyn_cast<FixedArrayType>(type))
    {
        return "array(" + getTypeName(faty->getElementType()) + "," +
                boost::lexical_cast<std::string>(faty->getArraySize()) + ")";
    }
    if (SymbolType::Ptr sty = dyn_cast<SymbolType>(type))
    {
        return "symbol(" + DFC::StrUtils::quoted(sty->getSymbol()) + ")";
    }
    if (StructType::Ptr sty = dyn_cast<StructType>(type))
    {
        return type->getFullTypeName();
    }
    if (FunctionType::Ptr fty = dyn_cast<FunctionType>(type))
    {
        std::string buf = "fn(";
        size_t numParams = fty->getNumParams();
        for (size_t i = 0; i < numParams; ++i)
        {
            buf += getTypeName(fty->getParamType(i));
            if (i != numParams-1)
                buf += ",";
        }
        buf+=") -> ";
        buf+=getTypeName(fty->getReturnType());
        return buf;
    }

    std::ostringstream oss;
    oss << type->getFullTypeName();

#undef RET_CTYPE_NAME
    return oss.str();
}

std::string IRUtils::getArgumentTypeNames(ArrayRef<IR::IRExpr::Ptr> args)
{
    std::ostringstream oss;
    typedef ArrayRef<IR::IRExpr::Ptr>::const_iterator Iter;
    for (Iter begin = args.begin(), it = begin, end = args.end(); it != end; ++it)
    {
        if (it != begin)
            oss <<", ";
        oss << IR::IRUtils::getTypeName((*it)->getExprType());
    }
    return oss.str();
}

Type::Ptr IRUtils::makeVariableType(const Expr::Ptr &argExpr)
{
    Type::Ptr exprType = argExpr->getExprType();
    // make from literal types variable types
    if (exprType == exprType->getWorld().type_string())
        return exprType->getWorld().type_c_char_ptr(); // FIXME make array type !
    return exprType;
}

DefExpr::Ptr IRUtils::createVariable(const std::string &name, const Type::Ptr &valueType)
{
    return new DefExpr(name, valueType, /*hasAddress=*/true);
}

#define ERROR_MESSAGE(message)                                      \
do {                                                                \
    if (errorMsg)                                                   \
    {                                                               \
       std::ostringstream msgs;                                     \
       msgs << message;                                             \
       *errorMsg = msgs.str();                                      \
    }                                                               \
    return 0;                                                       \
} while(false)

#define ERROR_MESSAGE_AT(message, loc)                              \
do {                                                                \
    if (errorMsg)                                                   \
    {                                                               \
       std::ostringstream msgs;                                     \
       if (loc)                                                     \
           msgs << *loc << ": ";                                    \
       msgs << message;                                             \
       *errorMsg = msgs.str();                                      \
    }                                                               \
    return 0;                                                       \
} while(false)

bool IRUtils::addObjectToScope(
        const std::string &name,
        const Object::Ptr &object,
        const Compiler::Scope::Ptr &scope,
        std::string *errorMsg)
{
    Object::Ptr obj = scope->lookupObject(name);
    if (obj)
        ERROR_MESSAGE("Symbol '"<<name<<"' is already bound to : "<<*obj);
    scope->addObject(name, object);
    return true;
}

bool IRUtils::addFunctionToScope(
        const IR::FunctionDefinition::Ptr &func,
        const Compiler::Scope::Ptr &scope,
        std::string *errorMsg)
{
    std::string funcName = func->getName();
    std::string mangledName = func->getMangledName();
    std::pair<Object::Ptr, const Compiler::Scope *> objAndScope = scope->lookupObjectAndScope(funcName);
    if (!objAndScope.first)
    {
        // new symbol, create overloaded function map
        Compiler::OverloadedObjectMap::Ptr funcMap = new Compiler::OverloadedObjectMap(func->getWorld(), funcName);
        funcMap->addObject(mangledName, func);
        scope->addObject(funcName, funcMap);
        return true;
    }

    Compiler::OverloadedObjectMap::Ptr funcMap = dyn_cast<Compiler::OverloadedObjectMap>(objAndScope.first);
    if (!funcMap)
        ERROR_MESSAGE("Scope contains already '"<<funcName<<"' symbol and it is not a function");
    if (funcMap->lookupObject(mangledName))
        ERROR_MESSAGE("Function '"<<funcName<<"' is already overloaded with signature : "<<*func->getProto());

    if (objAndScope.second != scope)
    {
        // function was defined in a different scope, make a copy and
        // add new overload
        funcMap = new Compiler::OverloadedObjectMap(funcMap);
        scope->addObject(funcName, funcMap);
    }

    funcMap->addObject(mangledName, func);

    return true;
}

void IRUtils::addDefaultTypesToScope(const Compiler::Scope::Ptr &scope)
{
    World &world = scope->getWorld();

    // register types
    addObjectToScope(
            IR::IRUtils::getTypeName(VoidType::get(world)),
            VoidType::get(world), scope);

#define REG_TYPE(name, type)                                \
    addObjectToScope(name, type, scope)

#define REG_CTYPE(name, ctype)                              \
    REG_TYPE(KIARA_STRINGIZE(name), world.c_type<ctype>())

    REG_CTYPE(char, char);
    REG_CTYPE(schar, signed char);
    REG_CTYPE(uchar, unsigned char);
    REG_CTYPE(int, int);
    REG_CTYPE(uint, unsigned int);
    REG_CTYPE(long, long);
    REG_CTYPE(ulong, unsigned long);
    REG_CTYPE(longlong, long long);
    REG_CTYPE(ulonglong, unsigned long long);
    REG_CTYPE(size_t, size_t);
    REG_CTYPE(ssize_t, ssize_t);
    REG_CTYPE(int8_t, int8_t);
    REG_CTYPE(uint8_t, uint8_t);
    REG_CTYPE(int16_t, int16_t);
    REG_CTYPE(uint16_t, uint16_t);
    REG_CTYPE(int32_t, int32_t);
    REG_CTYPE(uint32_t, uint32_t);
    REG_CTYPE(int64_t, int64_t);
    REG_CTYPE(uint64_t, uint64_t);
    REG_CTYPE(float, float);
    REG_CTYPE(double, double);

    REG_TYPE("any", AnyType::get(world));
    REG_TYPE("type", TypeType::get(world));
    REG_TYPE("boolean", PrimType::getBooleanType(world));
#undef REG_CTYPE
#undef REG_TYPE
}

} // namespace IR

} // namespace KIARA
