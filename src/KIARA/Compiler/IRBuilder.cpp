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
 * IRBuilder.cpp
 *
 *  Created on: 17.04.2013
 *      Author: Dmitri Rubinstein
 */
#define KIARA_COMPILER_LIB
#include "IRBuilder.hpp"
#include "IRUtils.hpp"
#include "IR.hpp"
#include "Mangler.hpp"
#include <KIARA/DB/Attributes.hpp>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <DFC/Utils/StrUtils.hpp>
#include <KIARA/Compiler/PrettyPrinter.hpp>

//#define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

namespace KIARA
{

namespace Compiler
{

IRBuilderException::IRBuilderException(const std::string &arg)
    : Exception(arg)
{
}

IRBuilder::IRBuilder(World &world)
    : world_(world)
    , scopes_()
{
}

IRBuilder::IRBuilder(const Scope::Ptr &scope)
    : world_(scope->getWorld())
    , scopes_()
{
    scopes_.push(scope);
}

IR::DefExpr::Ptr IRBuilder::createVariable(const std::string &name, const Type::Ptr &valueType)
{
    return new IR::DefExpr(name, valueType, /*hasAddress=*/true);
}

#define ERROR_MESSAGE(message)                                      \
do {                                                                \
    if (errorMsg)                                                   \
    {                                                               \
       std::ostringstream msgs;                                     \
       msgs << message;                                             \
       *errorMsg = msgs.str();                                      \
    }                                                               \
    else                                                            \
    {                                                               \
        DFC_THROW_EXCEPTION(IRBuilderException, message);           \
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
    else                                                            \
    {                                                               \
        DFC_THROW_EXCEPTION(IRBuilderException,                     \
            source_location(loc)<<message);                         \
    }                                                               \
    return 0;                                                       \
} while(false)

bool IRBuilder::createAddressOfCode(
        const Type::Ptr &exprType,
        std::vector<KIARA::IR::IRExpr::Ptr> &expressions,
        const Scope::Ptr &scope,
        std::string *errorMsg)
{
    RefType::Ptr rty = exprType->getAs<RefType>();
    if (!rty)
        ERROR_MESSAGE("Type "<<IR::IRUtils::getTypeName(exprType)<<" is not a reference");


    Scope::Ptr topScope = scope->getTopScope();

    KIARA::IR::Prototype::Arg args[] = {
            KIARA::IR::Prototype::Arg("v", rty)
    };

    KIARA::IR::Prototype::Ptr proto = new KIARA::IR::Prototype(
            "__addressof__",
            Mangler::getMangledFuncName("__addressof__", args),
            PtrType::get(rty->getElementType()),
            args,
            scope->getWorld(),
            false);

    bool result = true;

    IR::FunctionDefinition::Ptr addressofDef = getFunctionFromScope(proto, scope);
    if (!addressofDef)
    {
        DFC_DEBUG("Proto of __addressof__ :");
        DFC_DEBUG(proto->toString());

        KIARA::IR::Intrinsic::Ptr func = new KIARA::IR::Intrinsic(
                proto,
                "define ${rettype} @$(quote ${mangledName})(${argtype0} %${argname0}) nounwind uwtable readnone { "
                "entry: "
                "  ret  ${rettype} %${argname0} "
                "}",
                scope->getWorld());
        proto->setAttribute("llvm", "true");
        proto->setAttribute("always_inline", "true");
        result &= addFunctionToScope(func, topScope, errorMsg);
        if (result)
        {
            expressions.push_back(func);
            addressofDef = func;
        }
    }
    else
        result &= true;

    if (addressofDef)
    {
        DFC_DEBUG("addressofDef:");
        DFC_DEBUG(addressofDef->toString());
    }

    proto = new KIARA::IR::Prototype(
            "&",
            Mangler::getMangledFuncName("&", args),
            PtrType::get(rty->getElementType()),
            args,
            scope->getWorld(),
            true);

    IR::FunctionDefinition::Ptr addressofOpDef = getFunctionFromScope(proto, scope);
    if (!addressofOpDef)
    {
        IR::Function::Ptr func = new KIARA::IR::Function(proto);
        proto->setAttribute("always_inline", "true");
        func->setBody(new IR::CallExpr(addressofDef, func->getArg(0)));
        result &= addFunctionToScope(func, topScope, errorMsg);
        if (result)
        {
            expressions.push_back(func);
            addressofOpDef = func;
        }
    }
    else
        result &= true;

    if (addressofOpDef)
    {
        DFC_DEBUG("addressofOpDef:");
        DFC_DEBUG(addressofOpDef->toString());
    }

    return result;
}

bool IRBuilder::createDereferenceCode(
        const Type::Ptr &exprType,
        std::vector<KIARA::IR::IRExpr::Ptr> &expressions,
        const Scope::Ptr &scope,
        std::string *errorMsg)
{
    RefType::Ptr rty;
    PtrType::Ptr pty;
    Type::Ptr elemType;
    if ((rty = exprType->getAs<RefType>()))
        elemType = rty->getElementType();
    else if ((pty = exprType->getAs<PtrType>()))
        elemType = pty->getElementType();
    else
        ERROR_MESSAGE("Type "<<IR::IRUtils::getTypeName(exprType)<<" is neither a reference nor pointer");

    //if (rty && !dyn_cast<PrimType>(rty->getElementType()))
    //    ERROR_MESSAGE("Can create dereference only for primitive types: "<<IR::IRUtils::getTypeName(exprType)<<" is no primitive type");

    Scope::Ptr topScope = scope->getTopScope();

    KIARA::IR::Prototype::Arg args[] = {
            KIARA::IR::Prototype::Arg("v", exprType)
    };

    // __deref__(ref(T)):T
    // __deref__(ptr(T)):ref(T)

    KIARA::IR::Prototype::Ptr proto = new KIARA::IR::Prototype(
            "__deref__",
            Mangler::getMangledFuncName("__deref__", args),
            rty ? elemType : RefType::get(elemType),
            args,
            scope->getWorld(),
            false);

    bool result = true;

    IR::FunctionDefinition::Ptr derefDef = getFunctionFromScope(proto, scope);
    if (!derefDef)
    {
        DFC_DEBUG("Proto of __deref__:");
        DFC_DEBUG(proto->toString());

        const char *body = rty ?
                // __deref__(ref(T)):T
                "define ${rettype} @$(quote ${mangledName})(${argtype0} %${argname0}) nounwind uwtable readonly { "
                "  %r = load ${argtype0} %${argname0} "
                "  ret ${rettype} %r"
                "}" :
                // __deref__(ptr(T)):ref(T)
                "define ${rettype} @$(quote ${mangledName})(${argtype0} nocapture %${argname0}) nounwind uwtable readonly { "
                "  ret ${rettype} %${argname0} "
                "} ";

        KIARA::IR::Intrinsic::Ptr func = new KIARA::IR::Intrinsic(
                proto,
                body,
                scope->getWorld());
        proto->setAttribute("llvm", "true");
        proto->setAttribute("always_inline", "true");

        result &= addFunctionToScope(func, topScope, errorMsg);
        if (result)
        {
            expressions.push_back(func);
            derefDef = func;
        }
    }
    else
        result &= true;

    if (derefDef)
    {
        DFC_DEBUG("derefDef:");
        DFC_DEBUG(derefDef->toString());
    }

    if (pty)
    {
        proto = new KIARA::IR::Prototype(
                "*",
                Mangler::getMangledFuncName("*", args),
                RefType::get(elemType),
                args,
                scope->getWorld(),
                true);

        IR::FunctionDefinition::Ptr derefOpDef = getFunctionFromScope(proto, scope);
        if (!derefOpDef)
        {
            IR::Function::Ptr func = new KIARA::IR::Function(proto);
            proto->setAttribute("always_inline", "true");

            func->setBody(new IR::CallExpr(derefDef, func->getArg(0)));
            result &= addFunctionToScope(func, topScope, errorMsg);
            if (result)
            {
                expressions.push_back(func);
                derefOpDef = func;
            }
        }
        else
            result &= true;

        if (derefOpDef)
        {
            DFC_DEBUG("derefOpDef: ");
            DFC_DEBUG(derefOpDef->toString());
        }
    }

    return result;
}


bool IRBuilder::createStructCode(
        const StructType::Ptr &structType,
        std::vector<KIARA::IR::IRExpr::Ptr> &expressions,
        const Scope::Ptr &scope,
        std::string *errorMsg)
{
    PtrType::Ptr pty = PtrType::get(structType);

    if (!createDereferenceCode(pty, expressions, scope, errorMsg))
        return false;

    RefType::Ptr rty = RefType::get(structType);

    if (!createDereferenceCode(rty, expressions, scope, errorMsg))
        return false;

    Scope::Ptr topScope = scope->getTopScope();

    KIARA::IR::Prototype::Ptr proto;
    IR::FunctionDefinition::Ptr memberAccess;
    KIARA::IR::Prototype::Arg args[2];
    for (size_t i = 0, numElems = structType->getNumElements(); i < numElems; ++i)
    {
        KIARA::Type::Ptr elemType = structType->getElementAt(i);
        const KIARA::ElementData &elemData = structType->getElementDataAt(i);
        const std::string &elemName = elemData.getName();
        const size_t elemOffset = elemData.getAttributeValue<NativeOffsetAttr>();
        const std::string elemOffsetStr = boost::lexical_cast<std::string>(elemOffset);

        SymbolType::Ptr elemNameSym = SymbolType::get(structType->getWorld(), elemName);

        args[0] = KIARA::IR::Prototype::Arg("s", rty);
        args[1] = KIARA::IR::Prototype::Arg("sym", elemNameSym);

        proto = new KIARA::IR::Prototype(
                ".",
                Mangler::getMangledFuncName(".", args),
                RefType::get(elemType),
                args,
                structType->getWorld(),
                true,
                80);

        if (!getFunctionFromScope(proto, scope))
        {
            KIARA::IR::Intrinsic::Ptr func = new KIARA::IR::Intrinsic(
                    proto,
                    "define ${rettype} @$(quote ${mangledName})(${argtype0} %${argname0}, ${argtype1} %${argname1}) nounwind uwtable readnone {"
                    "entry:"
                    "  %0 = bitcast ${argtype0} %${argname0} to i8* "
                    "  %mbr = getelementptr inbounds i8* %0, %size_t "+elemOffsetStr+" "
                    "  %r = bitcast i8* %mbr to ${rettype} "
                    "  ret ${rettype} %r"
                    "}",
                    structType->getWorld());
            proto->setAttribute("llvm", "true");
            proto->setAttribute("always_inline", "true");
            if (!addFunctionToScope(func, topScope, errorMsg))
                return false;
            expressions.push_back(func);

            DFC_DEBUG("memberAccess:");
            DFC_DEBUG(func->toReprString());
        }

        args[0] = KIARA::IR::Prototype::Arg("s", pty);
        args[1] = KIARA::IR::Prototype::Arg("sym", elemNameSym);

        proto = new KIARA::IR::Prototype(
                "->",
                Mangler::getMangledFuncName("->", args),
                RefType::get(elemType),
                args,
                structType->getWorld(),
                true,
                80);

        if (!getFunctionFromScope(proto, scope))
        {
            KIARA::IR::Intrinsic::Ptr func = new KIARA::IR::Intrinsic(
                    proto,
                    "define ${rettype} @$(quote ${mangledName})(${argtype0} %${argname0}, ${argtype1} %${argname1}) nounwind uwtable readnone {"
                    "entry:"
                    "  %0 = bitcast ${argtype0} %${argname0} to i8* "
                    "  %mbr = getelementptr inbounds i8* %0, %size_t "+elemOffsetStr+" "
                    "  %r = bitcast i8* %mbr to ${rettype} "
                    "  ret ${rettype} %r"
                    "}",
                    structType->getWorld());
            proto->setAttribute("llvm", "true");
            proto->setAttribute("always_inline", "true");
            if (!addFunctionToScope(func, topScope, errorMsg))
                return false;
            expressions.push_back(func);

            DFC_DEBUG("memberAccess:");
            DFC_DEBUG(func->toReprString());
        }
    }

    return true;
}

bool IRBuilder::createCastCode(
        const Type::Ptr &srcType,
        const Type::Ptr &destType,
        std::string &castFuncName,
        std::vector<KIARA::IR::IRExpr::Ptr> &expressions,
        const Scope::Ptr &scope,
        std::string *errorMsg)
{
    PtrType::Ptr srcPtrTy = srcType->getAs<PtrType>();
    PtrType::Ptr destPtrTy = destType->getAs<PtrType>();
    if (srcPtrTy && destPtrTy)
    {
        Scope::Ptr topScope = scope->getTopScope();

        KIARA::IR::Prototype::Arg args[] = {
                KIARA::IR::Prototype::Arg("src", srcPtrTy)
        };

        castFuncName = "to_";
        castFuncName += KIARA::IR::IRUtils::getTypeName(destPtrTy->getElementType());
        castFuncName += "_ptr";

        KIARA::IR::Prototype::Ptr proto = createMangledFuncProto(
                castFuncName,
                destPtrTy,
                args,
                scope->getWorld());

        IR::FunctionDefinition::Ptr funcDef = getFunctionFromScope(proto, scope);
        if (!funcDef)
        {
            DFC_DEBUG("Proto of "<<castFuncName<<":");
            DFC_DEBUG(proto->toString());

            KIARA::IR::Intrinsic::Ptr func = new KIARA::IR::Intrinsic(
                    proto,
                    "define ${rettype} @$(quote ${mangledName})(${argtype0} %${argname0}) nounwind uwtable readnone { "
                    "  %r = bitcast ${argtype0} %${argname0} to ${rettype} "
                    "  ret ${rettype} %r "
                    "} ",
                    scope->getWorld());
            proto->setAttribute("llvm", "true");
            proto->setAttribute("always_inline", "true");

            if (!addFunctionToScope(func, topScope, errorMsg))
                return false;

            expressions.push_back(func);
        }

        return true;
    }

    PrimType::Ptr srcPrimTy = srcType->getAs<PrimType>();
    PrimType::Ptr destPrimTy = destType->getAs<PrimType>();
    if (srcPrimTy && destPrimTy && srcPrimTy->isInteger() && destPrimTy->isInteger())
    {
        Scope::Ptr topScope = scope->getTopScope();

        KIARA::IR::Prototype::Arg args[] = {
                KIARA::IR::Prototype::Arg("src", srcPrimTy)
        };

        castFuncName = "to_";
        castFuncName += KIARA::IR::IRUtils::getTypeName(destPrimTy);

        KIARA::IR::Prototype::Ptr proto = createMangledFuncProto(
            castFuncName,
            destPrimTy,
            args,
            scope->getWorld());

        IR::FunctionDefinition::Ptr funcDef = getFunctionFromScope(proto, scope);
        if (!funcDef)
        {
            DFC_DEBUG("Proto of "<<castFuncName<<":");
            DFC_DEBUG(proto->toString());

            std::string conv;

			if (srcPrimTy->getByteSize() == destPrimTy->getByteSize())
				conv = "ret ${argtype0} %${argname0} ";
            else if (srcPrimTy->getByteSize() > destPrimTy->getByteSize())
                conv = "%r = trunc ${argtype0} %${argname0} to ${rettype} ret ${rettype} %r ";
            else if (srcPrimTy->isSignedInteger())
                conv = "%r = sext ${argtype0} %${argname0} to ${rettype} ret ${rettype} %r ";
            else
                conv = "%r = zext ${argtype0} %${argname0} to ${rettype} ret ${rettype} %r ";

            KIARA::IR::Intrinsic::Ptr func = new KIARA::IR::Intrinsic(
                    proto,
                    "define ${rettype} @$(quote ${mangledName})(${argtype0} %${argname0}) nounwind uwtable readnone { "
                    +conv+
                    "} ",
                    scope->getWorld());
            proto->setAttribute("llvm", "true");
            proto->setAttribute("always_inline", "true");

            if (!addFunctionToScope(func, topScope, errorMsg))
                return false;

            expressions.push_back(func);
        }

        return true;
    }

    return false;
}

bool IRBuilder::createArrayIndexCode(
    const Type::Ptr &arrayType,
    const Type::Ptr &indexType,
    std::vector<KIARA::IR::IRExpr::Ptr> &expressions,
    const Scope::Ptr &scope,
    std::string *errorMsg)
{
    PtrType::Ptr arrayPtrTy = arrayType->getAs<PtrType>();
    PrimType::Ptr indexTy = indexType->getAs<PrimType>();
    if (!arrayPtrTy || !indexTy)
        return false;
    if (!indexTy->isInteger())
        return false;

    Type::Ptr elemTy = arrayPtrTy->getElementType();
    RefType::Ptr resultTy = RefType::get(elemTy);

    Scope::Ptr topScope = scope->getTopScope();

    KIARA::IR::Prototype::Arg args[] = {
        KIARA::IR::Prototype::Arg("array", arrayPtrTy),
        KIARA::IR::Prototype::Arg("index", indexTy)
    };

    KIARA::IR::Prototype::Ptr proto = createMangledFuncProto(
        "__index__",
        resultTy,
        args,
        scope->getWorld());

    IR::FunctionDefinition::Ptr funcDef = getFunctionFromScope(proto, scope);
    if (!funcDef)
    {
        DFC_DEBUG("Proto of __index__:");
        DFC_DEBUG(proto->toString());

        KIARA::IR::Intrinsic::Ptr func;

        if (elemTy->hasAttributeValue<NativeSizeAttr>())
        {
            const std::string nativeSizeStr =
                boost::lexical_cast<std::string>(elemTy->getAttributeValue<NativeSizeAttr>());

            func = new KIARA::IR::Intrinsic(
                    proto,
                    "define ${rettype} @${mangledName}(${argtype0} nocapture %${argname0}, ${argtype1} %${argname1}) nounwind uwtable readonly { "
                    "entry: "
                    "  %0 = bitcast ${rettype} %${argname0} to i8* "
                    "  %mul = mul ${argtype1} %${argname1}, " + nativeSizeStr +
                    "  %add.ptr = getelementptr inbounds i8* %0, ${argtype1} %mul "
                    "  %1 = bitcast i8* %add.ptr to ${rettype} "
                    "  ret ${rettype} %1 "
                    "} ",
                    scope->getWorld());
        }
        else
        {
            func = new KIARA::IR::Intrinsic(
                    proto,
                    "define ${rettype} @${mangledName}(${argtype0} nocapture %${argname0}, ${argtype1} %${argname1}) nounwind uwtable readonly { "
                    "  %arrayidx = getelementptr inbounds ${rettype} %${argname0}, ${argtype1} %${argname1} "
                    "  ret ${rettype} %arrayidx "
                    "} ",
                    scope->getWorld());
        }
        proto->setAttribute("llvm", "true");
        proto->setAttribute("always_inline", "true");

        if (!addFunctionToScope(func, topScope, errorMsg))
            return false;

        expressions.push_back(func);
    }

    return true;
}

bool IRBuilder::createAssignCode(
    const Type::Ptr &destType,
    const Type::Ptr &srcType,
    std::vector<KIARA::IR::IRExpr::Ptr> &expressions,
    const Scope::Ptr &scope,
    std::string *errorMsg)
{
    RefType::Ptr varTy = destType->getAs<RefType>();
    if (!varTy)
        return false;

    Scope::Ptr topScope = scope->getTopScope();

    KIARA::IR::Prototype::Arg args[] = {
        KIARA::IR::Prototype::Arg("a", varTy),
        KIARA::IR::Prototype::Arg("b", srcType)
    };

    KIARA::IR::Prototype::Ptr proto = createOperatorProto(
        "=",
        varTy,
        args,
        2);

    IR::FunctionDefinition::Ptr funcDef = getFunctionFromScope(proto, scope);
    if (!funcDef)
    {
        DFC_DEBUG("Proto of "<<proto->getName()<<":");
        DFC_DEBUG(proto->toString());

        KIARA::IR::Intrinsic::Ptr func = new KIARA::IR::Intrinsic(
            proto,
            "define ${rettype} @${mangledName}(${argtype0} %${argname0}, ${argtype1} %${argname1}) nounwind uwtable { "
            "  store ${argtype1} %${argname1}, ${argtype0} %${argname0} "
            "  ret ${rettype} %${argname0} "
            "}",
            scope->getWorld());
        proto->setAttribute("llvm", "true");
        proto->setAttribute("always_inline", "true");

        if (!addFunctionToScope(func, topScope, errorMsg))
            return false;

        expressions.push_back(func);
    }

    return true;
}

IR::IRExpr::Ptr IRBuilder::getDereference(
        const IR::IRExpr::Ptr &expr,
        std::string *errorMsg)
{
    Type::Ptr ty = expr->getExprType();
    if (isa<RefType>(ty) || isa<PtrType>(ty))
    {
        std::vector<IR::IRExpr::Ptr> args;
        args.push_back(expr);
        return createCall(
                "__deref__", args, expr->getLocation(),
                "__call__", CM_NO_TYPE_CONVERSION,
                errorMsg);
    }
    return 0;
}

IR::IRExpr::Ptr IRBuilder::convertValue(
        const IR::IRExpr::Ptr &value,
        const Type::Ptr &destType,
        int *numConversions,
        std::string *errorMsg)
{
    World &world = value->getWorld();
    Type::Ptr srcType = value->getExprType();

    if (canonicallyEqual(srcType, destType))
        return value;

    if (canonicallyEqual(destType, AnyType::get(world)))
        return value;

    // check for lvalue (pass variable of type T to reference of type T)
    RefType::Ptr refTy = destType->getAs<RefType>();
    if (refTy && canonicallyEqual(refTy->getElementType(), srcType))
    {
        if (IR::IRExpr::Ptr ref = value->getReference())
            return ref;
    }

    // reference can be passed to value
    refTy = srcType->getAs<RefType>();
    if (refTy)
    {
        std::string derefErrorMsg;
        // try to make dereference call
        IR::IRExpr::Ptr deref = getDereference(value, &derefErrorMsg);
        if (deref)
        {
            DFC_DEBUG("Dereference of type " << KIARA::IR::PrettyPrinter::toString(refTy) << " TO VALUE " << KIARA::IR::PrettyPrinter::toString(deref));

            if (canonicallyEqual(refTy->getElementType(), destType))
                return deref;
            else
            {
                if (IR::IRExpr::Ptr newValue = convertValue(deref, destType, numConversions))
                {
                    DFC_DEBUG("Dereference conversion to value " << KIARA::IR::PrettyPrinter::toString(newValue));
                    return newValue;
                }
            }
        }
        else
        {
            DFC_DEBUG("Dereference error: "<<derefErrorMsg);
        }
    }

    // string constant to char pointer / void pointer
    if (canonicallyEqual(srcType, world.type_string()) &&
            (canonicallyEqual(destType, world.type_c_void_ptr()) ||
                canonicallyEqual(destType, world.type_c_char_ptr())))
        return value;

    IR::PrimLiteral::Ptr literalValue = dyn_cast<IR::PrimLiteral>(value);

    if (PtrType::Ptr pty = destType->getAs<PtrType>())
    {
        // null pointer to any pointer
        if (IR::PrimLiteral::isNullPtr(value))
            return value;

        // array(T) -> ptr(T)
        ArrayType::Ptr aty = srcType->getAs<ArrayType>();
        if (aty && (canonicallyEqual(aty->getElementType(), pty->getElementType())))
        {
            return value;
        }
        // array(T, N) -> ptr(T)
        else if (FixedArrayType::Ptr faty = srcType->getAs<FixedArrayType>())
        {
            if (canonicallyEqual(faty->getElementType(), pty->getElementType()))
                return value;
        }
    }

    // pointer to void pointer
    if (srcType->getAs<PtrType>() &&
            canonicallyEqual(destType, world.type_c_void_ptr()))
    {
        if (numConversions)
            ++(*numConversions);
        return value;
    }

    return 0;
}

IR::CallExpr::Ptr IRBuilder::createCall(
        const std::string &name,
        ArrayRef<IR::IRExpr::Ptr> args,
        const SourceLocation *location,
        const std::string &callMethod,
        CallMode callMode,
        std::string *errorMsg)
{
    return createCall(name, getScope(), args, location, callMethod, callMode, errorMsg);
}

namespace
{
struct FuncExprInfo
{
    int numConv; // number of conversions of all passed arguments
    IR::IRExpr::Ptr funcExpr; // expression to call
    FunctionType::Ptr funcType; // type of the function

    FuncExprInfo(
            int numConv,
            const IR::IRExpr::Ptr &funcExpr,
            const FunctionType::Ptr funcType)
        : numConv(numConv)
        , funcExpr(funcExpr)
        , funcType(funcType)
    { }
};
typedef std::vector<FuncExprInfo> FuncExprList;
}

inline FunctionType::Ptr getFunctionPtrType(const IR::IRExpr::Ptr &expr)
{
    if (PtrType::Ptr ptrType = expr->getExprType()->getAs<PtrType>())
        return ptrType->getElementType()->getAs<FunctionType>();
    return 0;
}

IR::CallExpr::Ptr IRBuilder::createCall(
        const std::string &name,
        const Scope::Ptr &scope,
        ArrayRef<IR::IRExpr::Ptr> args,
        const SourceLocation *location,
        const std::string &callMethod,
        CallMode callMode,
        std::string *errorMsg)
{
    // Debug
    DFC_DEBUG("LOOKUP FUNCTION: '"<<name<<"'");
    DFC_DEBUG(scope->toReprString());
#ifdef DFC_DEBUG_MODE
    for (ArrayRef<IR::IRExpr::Ptr>::const_iterator it = args.begin(), end = args.end();
         it != end; ++it)
    {
        std::string s = "ARG ";
        if (*it)
            s += (*it)->toString() + " of type " + IR::IRUtils::getTypeName((*it)->getExprType());
        else
            s += "NULL";
        DFC_DEBUG(s);
    }
#endif
    // End Debug

    Object::Ptr obj = scope->lookupObject(name);
    if (!obj)
        ERROR_MESSAGE_AT("no function '"<<name<<"' for arguments ("
                <<IR::IRUtils::getArgumentTypeNames(args)<<") declared",
                location);

    ArrayRef<IR::IRExpr::Ptr> callArgs;
    const std::string *callName = &name;
    std::vector<IR::IRExpr::Ptr> tmpArgs;
    IR::IRExpr::Ptr funcExpr;
    FunctionType::Ptr funcType;

    OverloadedObjectMap::Ptr funcMap = dyn_cast<OverloadedObjectMap>(obj);
    if (funcMap)
    {
        // overloaded function, funcMap contains candidates
        callArgs = args;
    }
    else
    {
        IR::IRExpr::Ptr expr = dyn_cast<IR::IRExpr>(obj);
        if (!expr)
            ERROR_MESSAGE_AT("'"<<name<<"' is not an expression: "
                    <<IR::IRUtils::getObjectName(obj)
                    <<" (internal type "<<obj->getTypeName()<<")",
                    location);

        funcType = getFunctionPtrType(expr);
        if (funcType)
        {
            // indirect function call
            funcExpr = expr;
            callArgs = args;
        }
        else
        {
            // lookup for the __call__ function
            obj = scope->lookupObject(callMethod);
            if (!obj)
                ERROR_MESSAGE_AT("'"<<name<<"' is not a function, and there is no "<<callMethod
                        <<" function: "
                        <<*expr<<" has type "<<IR::IRUtils::getTypeName(expr->getExprType()),
                        location);

            funcMap = dyn_cast<OverloadedObjectMap>(obj);
            if (!funcMap)
                ERROR_MESSAGE_AT("'"<<name<<"' is not a function, and "
                        <<callMethod<<" is not a function: "
                        <<*expr<<" has type "<<IR::IRUtils::getTypeName(expr->getExprType()),
                        location);

            tmpArgs.reserve(args.size()+1);
            tmpArgs.push_back(expr);
            tmpArgs.insert(tmpArgs.end(), args.begin(), args.end());
            callArgs = makeArrayRef(tmpArgs);
            callName = &callMethod;
        }
    }

    FuncExprList candidates;
    const size_t numArgs = callArgs.size();
    std::string calledFuncName = "'";
    calledFuncName += *callName;
    calledFuncName += "'";

    if (funcExpr)
    {
        candidates.push_back(FuncExprInfo(0, funcExpr, funcType));
    }
    else
    {
        std::string mn = Mangler::getMangledFuncName(*callName, callArgs);
        calledFuncName += " (mangled '";
        calledFuncName += mn;
        calledFuncName += "')";
        IR::FunctionDefinition::Ptr funcDef = dyn_cast<IR::FunctionDefinition>(funcMap->lookupObject(mn));
        funcExpr = funcDef;
        if (funcDef)
        {
            funcType = funcDef->getFunctionType();
            assert(funcType != 0);
            candidates.push_back(FuncExprInfo(0, funcDef, funcType));
        }
        else
        {
            // add all candidates for a call
            for (OverloadedObjectMap::const_iterator it = funcMap->begin(),
                    end = funcMap->end(); it != end; ++it)
            {
                funcDef = dyn_cast<IR::FunctionDefinition>(it->second);
                if (funcDef && funcDef->getNumArgs() == numArgs)
                {
                    funcType = funcDef->getFunctionType();
                    candidates.push_back(FuncExprInfo(0, funcDef, funcType));
                }
            }
        }
    }

    std::vector<IR::IRExpr::Ptr> newArgs;

    // check their arguments
    {
        FuncExprList tmp;
        std::vector<IR::IRExpr::Ptr> tmpArgs;
        for (FuncExprList::iterator it = candidates.begin(), end = candidates.end();
                it != end; ++it)
        {
            tmpArgs.clear();
            int numConv = 0;
            const IR::IRExpr::Ptr &candidate = it->funcExpr;
            const FunctionType::Ptr &candidateType = it->funcType;
            for (size_t i = 0; i < numArgs; ++i)
            {
                IR::IRExpr::Ptr arg;
                if (callMode != CM_NO_TYPE_CONVERSION)
                {
                    arg = convertValue(callArgs[i], candidateType->getParamType(i), &numConv, errorMsg);
                    DFC_IFDEBUG(if (!arg)) {
                        DFC_IFDEBUG(KIARA::IR::FunctionDefinition::Ptr candidateFunc =
                                    dyn_cast<KIARA::IR::FunctionDefinition>(candidate));
                        DFC_IFDEBUG(KIARA::Type::Ptr candidateArgType =
                                    candidateFunc ? candidateFunc->getArgType(i) : 0);
                        DFC_DEBUG("IRBuilder::createCall: ARGUMENT "<<i<<" CONVERSION FAILED call argument type: "
                                  <<KIARA::IR::PrettyPrinter::toString(callArgs[i]->getExprType())
                                  <<" (ptr: "<<(callArgs[i]->getExprType().get())<<"), candidate parameter type: "
                                  <<(candidateArgType ? KIARA::IR::PrettyPrinter::toString(candidateArgType) : "?")
                                  <<" (ptr: "<<(candidateArgType ? candidateArgType.get() : 0)<<")");
                    }
                }
                else
                {
                    Type::Ptr srcType = callArgs[i]->getExprType();
                    if (canonicallyEqual(srcType, candidateType->getParamType(i)))
                        arg = callArgs[i];
                    DFC_IFDEBUG(if (!arg)) {
                        DFC_IFDEBUG(KIARA::IR::FunctionDefinition::Ptr candidateFunc =
                                    dyn_cast<KIARA::IR::FunctionDefinition>(candidate));
                        DFC_IFDEBUG(KIARA::Type::Ptr candidateArgType =
                                    candidateFunc ? candidateFunc->getArgType(i) : 0);
                        DFC_DEBUG("IRBuilder::createCall: ARGUMENT "<<i<<" NOT COMPATIBLE call argument type: "
                                  <<KIARA::IR::PrettyPrinter::toString(callArgs[i]->getExprType())
                                  <<" (ptr: "<<(callArgs[i]->getExprType().get())<<"), candidate parameter type: "
                                  <<(candidateArgType ? KIARA::IR::PrettyPrinter::toString(candidateArgType) : "?")
                                  <<" (ptr: "<<(candidateArgType ? candidateArgType.get() : 0)<<")");
                   }
                }
                if (!arg)
                    break;
                tmpArgs.push_back(arg);
            }
            if (tmpArgs.size() == numArgs)
            {
                // selected compatible function
                tmp.push_back(FuncExprInfo(numConv, candidate, candidateType));
                // and move new arguments to newArgs when they are not set yet
                if (newArgs.size() == 0)
                    newArgs.swap(tmpArgs);
            }
        }

        candidates.swap(tmp);
    }

    if (candidates.size() == 0)
        funcExpr = 0;
    if (candidates.size() == 1)
        funcExpr = candidates[0].funcExpr;
    else if (candidates.size() > 1)
    {
        // try find candidate without any data conversions
        for (FuncExprList::iterator it = candidates.begin(), end = candidates.end();
                it != end; ++it)
        {
            if (it->numConv == 0)
            {
                funcExpr = it->funcExpr;
                break;
            }
        }

        if (!funcExpr)
            // FIXME output candidates
            ERROR_MESSAGE_AT("Multiple compatible function definitions for symbol \""
                    <<*callName<<"\" found",
                    location);
    }

    if (!funcExpr)
    {
        std::ostringstream oss;
        typedef ArrayRef<IR::IRExpr::Ptr>::const_iterator Iter;
        oss << "no matching function for call to "<<calledFuncName<<", "
                "arguments: ("<<IR::IRUtils::getArgumentTypeNames(callArgs)<< ")";
        if (funcMap->empty())
        {
            oss<<", no candidates !";
        }
        else
        {
            oss<<",\ncandidates:\n";
            for (OverloadedObjectMap::const_iterator it = funcMap->begin(),
                    end = funcMap->end(); it != end; ++it)
            {
                IR::FunctionDefinition::Ptr func =
                        dyn_cast<IR::FunctionDefinition>(it->second);
                oss<<*func<<"\n";
            }
        }

        ERROR_MESSAGE_AT(oss.str(), location);
    }

    if (IR::FunctionDefinition::Ptr funcDef = dyn_cast<IR::FunctionDefinition>(funcExpr))
    {
        if (!funcDef->getProto())
            ERROR_MESSAGE_AT("Missing proto", location);
        // FIXME Check argument compatibility between arguments and proto.
    }

    // FIXME Check argument compatibility between arguments and proto.

    IR::CallExpr::Ptr result;
    if (!newArgs.empty())
        result = new IR::CallExpr(funcExpr, newArgs);
    else
        result = new IR::CallExpr(funcExpr, callArgs);
    result->setLocation(location);
    return result;
}

IR::IRExpr::Ptr IRBuilder::lookupExpr(const std::string &name)
{
    if (!hasScope())
        return 0;
    Object::Ptr obj = getScope()->lookupObject(name);
    if (IR::IRExpr::Ptr resolvedExpr = dyn_cast<IR::IRExpr>(obj))
        return resolvedExpr;
    else if (OverloadedObjectMap::Ptr funcMap = dyn_cast<OverloadedObjectMap>(obj))
    {
        if (funcMap->getNumObjects() == 1)
            return dyn_cast<IR::IRExpr>(funcMap->begin()->second);
    }
    else if (Type::Ptr typeValue = dyn_cast<Type>(obj))
        return new IR::TypeExpr(typeValue);
    return 0;
}

Type::Ptr IRBuilder::lookupType(const std::string &name) const
{
    if (!hasScope())
        return 0;
    Object::Ptr obj = getScope()->lookupObject(name);
    return dyn_cast<Type>(obj);
}

bool IRBuilder::addVariableToScope(const IR::DefExpr::Ptr &var, std::string *errorMsg)
{
    return addObjectToScope(var->getName(), var, errorMsg);
}

bool IRBuilder::addObjectToScope(
        const std::string &name,
        const Object::Ptr &object,
        std::string *errorMsg)
{
    const Scope::Ptr &scope = getScope();
    Object::Ptr obj = scope->lookupObject(name);
    if (obj)
        ERROR_MESSAGE("Symbol '"<<name<<"' is already bound to : "<<*obj);
    scope->addObject(name, object);
    return true;
}

IR::FunctionDefinition::Ptr IRBuilder::getFunctionFromScope(
        const IR::Prototype::Ptr &funcProto,
        const Scope::Ptr &scope)
{
    std::string funcName = funcProto->getName();
    std::string mangledName = funcProto->getMangledName();
    std::pair<Object::Ptr, const Scope *> objAndScope = scope->lookupObjectAndScope(funcName);
    if (!objAndScope.first)
        return 0;
    OverloadedObjectMap::Ptr funcMap = dyn_cast<OverloadedObjectMap>(objAndScope.first);
    if (!funcMap)
        return 0;
    return dyn_cast<IR::FunctionDefinition>(funcMap->lookupObject(mangledName));
}

bool IRBuilder::addFunctionToScope(
        const IR::FunctionDefinition::Ptr &func,
        const Scope::Ptr &scope,
        std::string *errorMsg)
{
    std::string funcName = func->getName();
    std::string mangledName = func->getMangledName();
    std::pair<Object::Ptr, const Scope *> objAndScope = scope->lookupObjectAndScope(funcName);
    if (!objAndScope.first)
    {
        // new symbol, create overloaded function map
        OverloadedObjectMap::Ptr funcMap = new OverloadedObjectMap(func->getWorld(), funcName);
        funcMap->addObject(mangledName, func);
        scope->addObject(funcName, funcMap);
        return true;
    }

    OverloadedObjectMap::Ptr funcMap = dyn_cast<OverloadedObjectMap>(objAndScope.first);
    if (!funcMap)
        ERROR_MESSAGE("Scope contains already '"<<funcName<<"' symbol and it is not a function");
    if (funcMap->lookupObject(mangledName))
        ERROR_MESSAGE("Function '"<<funcName<<"' is already overloaded with signature : "<<*func->getProto());

    if (objAndScope.second != scope)
    {
        // function was defined in a different scope, make a copy and
        // add new overload
        funcMap = new OverloadedObjectMap(funcMap);
        scope->addObject(funcName, funcMap);
    }

    funcMap->addObject(mangledName, func);

    return true;
}

void IRBuilder::initFunctionScope(const IR::FunctionDefinition::Ptr &func,
                                  std::string *errorMsg)
{
    const size_t numArgs = func->getNumArgs();
    for (size_t i = 0; i < numArgs; ++i)
    {
        addVariableToScope(func->getArg(i));
    }
    addFunctionToScope(func, errorMsg);
}

} // namespace Compiler

} // namespace KIARA
