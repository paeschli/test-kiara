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
 * CodeGen.cpp
 *
 *  Created on: 30.01.2013
 *      Author: Dmitri Rubinstein
 */
#define KIARA_COMPILER_LIB
// ssize_t is defined by LLVM's DataTypes.h
#define _SSIZE_T_DEFINED
#include "CodeGen.hpp"
#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#else
#include "llvm/Module.h"
#include "llvm/Function.h"
#endif
#include "llvm/Analysis/Verifier.h"
#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
#include "llvm/IR/TypeBuilder.h"
#else
#include "llvm/Support/TypeBuilder.h"
#endif
#include "llvm/Support/raw_ostream.h"
#include <KIARA/DB/Attributes.hpp>
#include <KIARA/Utils/VarGuard.hpp>
#include <KIARA/LLVM/Utils.hpp>
#include <KIARA/Compiler/IRUtils.hpp>

// #define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

namespace KIARA
{

namespace Compiler
{

CodeGenException::CodeGenException(const std::string &arg)
    : Exception(arg)
{
}

CodeGen::CodeGen(llvm::LLVMContext &context, World &world)
    : world_(world)
    , context_(context)
    , compilerUnit(0)
    , builder_(context)
    , namedValues_()
    , llvmTypeMap_()
    , llvmDefs_()
    , genReference_()
{
    initLLVMDefs();
    initContext();
}

CodeGen::~CodeGen()
{
	// delete compilerUnit;
}

CodeGen::CodeGen(CompilerUnit *moduleCompilerUnit, World &world)
    : world_(world)
    , context_(moduleCompilerUnit->getContext())
    , compilerUnit(moduleCompilerUnit)
    , builder_(moduleCompilerUnit->getContext())
    , namedValues_()
    , llvmDefs_()
    , genReference_()
{
    initLLVMDefs();
    initContext();
}

void CodeGen::initLLVMDefs()
{
    std::ostringstream oss;
    std::string llvmTypeName;

#define DEF_LLVM_TYPE2(symName, typeName)                                               \
    llvmTypeName = llvmToString(getLLVMType(world_.c_type<typeName>(), VALUE_TYPE));    \
    oss <<"%" KIARA_STRINGIZE(symName) " = type "                                       \
        << llvmTypeName << "\n";                                                        \
    llvmPPVars_[KIARA_STRINGIZE(symName)] = llvmTypeName;

#define DEF_LLVM_TYPE(typeName) DEF_LLVM_TYPE2(typeName, typeName)


    DEF_LLVM_TYPE(char);
    //DEF_LLVM_TYPE(wchar_t);
    DEF_LLVM_TYPE2(schar, signed char);
    DEF_LLVM_TYPE2(uchar, unsigned char);

    DEF_LLVM_TYPE(short);
    DEF_LLVM_TYPE2(ushort, unsigned short);

    DEF_LLVM_TYPE(int);
    DEF_LLVM_TYPE2(uint, unsigned int);

    DEF_LLVM_TYPE(long);
    DEF_LLVM_TYPE2(ulong, unsigned long);

    DEF_LLVM_TYPE2(longlong, long long);
    DEF_LLVM_TYPE2(ulonglong, unsigned long long);

    DEF_LLVM_TYPE(float);
    DEF_LLVM_TYPE(double);
    DEF_LLVM_TYPE2(longdouble, long double);

    DEF_LLVM_TYPE(int8_t);
    DEF_LLVM_TYPE(uint8_t);
    DEF_LLVM_TYPE(int16_t);
    DEF_LLVM_TYPE(uint16_t);
    DEF_LLVM_TYPE(int32_t);
    DEF_LLVM_TYPE(uint32_t);
    DEF_LLVM_TYPE(int64_t);
    DEF_LLVM_TYPE(uint64_t);

    DEF_LLVM_TYPE(size_t);
    DEF_LLVM_TYPE(ssize_t);

    DFC_DEBUG("init llvm types: " << oss.str());

    llvmTypeName = llvmToString(
            getLLVMType(world_.type_c_char_ptr(), VALUE_TYPE));
    oss <<"%char_ptr = type "
        << llvmTypeName << "\n";
    llvmPPVars_["char_ptr"] = llvmTypeName;

    llvmTypeName = llvmToString(
            getLLVMType(world_.type_c_void_ptr(), VALUE_TYPE));
    oss <<"%void_ptr = type "
        << llvmTypeName << "\n";
    llvmPPVars_["void_ptr"] = llvmTypeName;

    llvmDefs_ = oss.str();
}

void CodeGen::initContext()
{
    genReference_ = false;
}

llvm::Value * CodeGen::createArgValue(const IR::IRExpr::Ptr &argValue, const Type::Ptr &destType)
{
    VarGuard<bool> g(genReference_);
    genReference_ = false;

    llvm::Value *result = 0;
    llvm::Type *llvmDestType = getLLVMType(destType, VALUE_TYPE);


    World &world = destType->getWorld();

    Type::Ptr srcType = argValue->getExprType();
    if (canonicallyEqual(srcType, destType))
    {
        result = apply(argValue);
        if (!result)
            DFC_THROW_EXCEPTION(CodeGenException,
                    "Could not create LLVM value for type '"<<
                    IR::IRUtils::getTypeName(argValue->getExprType()));
    }
    // nullptr -> ptr(*)
    else if (IR::PrimLiteral::isNullPtr(argValue) && getTypeAs<PtrType>(destType))
    {
        return llvm::Constant::getNullValue(llvmDestType);
    }
    // ptr(*) -> ptr(void)
    else if (getTypeAs<PtrType>(srcType) && canonicallyEqual(destType, world.type_c_void_ptr()))
    {
        result = apply(argValue);
        if (result->getType() != llvmDestType)
        {
            result = builder_.CreateBitCast(result, llvmDestType, "ptr_to_void_ptr");
        }
    }
    else if (canonicallyEqual(srcType, world.type_string())) // string literal
    {
        // string literal is compatible with char* and void*
        if (canonicallyEqual(destType, world.type_c_char_ptr()) || canonicallyEqual(destType, world.type_c_void_ptr()))
        {
            result = apply(argValue);
            llvm::Value *zero = getInt32(0);
            llvm::Value *args[] = { zero, zero };
            result = builder_.CreateInBoundsGEP(result, llvm::ArrayRef<llvm::Value*>(args, args+2), result->getName());
        }
    }
    else if (PtrType::Ptr pty = getTypeAs<PtrType>(destType))
    {
        // array(T) -> ptr(T)
        ArrayType::Ptr aty = getTypeAs<ArrayType>(srcType);
        if (aty && canonicallyEqual(aty->getElementType(), pty->getElementType()))
        {
            result = apply(argValue);
        }
        // array(T, N) -> ptr(T)
        else if (FixedArrayType::Ptr faty = getTypeAs<FixedArrayType>(srcType))
        {
            if (canonicallyEqual(faty->getElementType(), pty->getElementType()))
                result = apply(argValue);
        }
    }
    else
    {
        // check for lvalue (pass variable of type T to reference of type T)
        RefType::Ptr refTy = getTypeAs<RefType>(destType);

        // DEBUG
//        if (refTy)
//        {
//            DFC_DEBUG("REF TY: "<<IR::IRUtils::getTypeName(refTy)<<" "<<refTy->getElementType().get());
//            DFC_DEBUG("SRC TY: "<<IR::IRUtils::getTypeName(srcType)<<" "<<srcType.get());
//            DFC_DEBUG("OBJ TY: "<<argValue->getType().getName());
//            DFC_IFDEBUG(argValue->dump());
//
//            if (dyn_cast<FunctionType>(srcType))
//                DFC_DEBUG("FUN TY");
//        }

        if (refTy && canonicallyEqual(refTy->getElementType(), srcType))
        {
            if (IR::IRExpr::Ptr ref = argValue->getReference())
            {
                genReference_ = true;
                result = apply(argValue);
            }
        }
    }

    // void return type can only be passed as unit : struct {}
    if (result && result->getType()->isVoidTy())
        result = getUnitValue();
    return result;
}

llvm::Type * CodeGen::declareType(const Type::Ptr &type)
{
    return getLLVMType(type, VALUE_TYPE);
}

llvm::Value *CodeGen::visit(IR::MemRef &expr)
{
    if (llvm::Value *value = getEmittedValue(expr))
        return value;
    return addEmittedValue(expr, emit(expr));
}

llvm::Value *CodeGen::visit(IR::PrimLiteral &expr)
{
    if (llvm::Value *value = getEmittedValue(expr))
        return value;
    return addEmittedValue(expr, emit(expr));
}

llvm::Value *CodeGen::visit(IR::SymbolExpr &expr)
{
    if (llvm::Value *value = getEmittedValue(expr))
        return value;
    return addEmittedValue(expr, emit(expr));
}

llvm::Value *CodeGen::visit(IR::DefExpr &expr)
{
    // this cannot be cached because of different code
    // depending on genReference_
    return emit(expr);
}

llvm::Value *CodeGen::visit(IR::CallExpr &expr)
{
    if (llvm::Value *value = getEmittedValue(expr))
        return value;
    return addEmittedValue(expr, emit(expr));
}

llvm::Value *CodeGen::visit(IR::IfExpr &expr)
{
    if (llvm::Value *value = getEmittedValue(expr))
        return value;
    return addEmittedValue(expr, emit(expr));
}

llvm::Value *CodeGen::visit(IR::LoopExpr &expr)
{
    if (llvm::Value *value = getEmittedValue(expr))
        return value;
    return addEmittedValue(expr, emit(expr));
}

llvm::Value *CodeGen::visit(IR::ForExpr &expr)
{
    if (llvm::Value *value = getEmittedValue(expr))
        return value;
    return addEmittedValue(expr, emit(expr));
}

llvm::Value *CodeGen::visit(IR::LetExpr &expr)
{
    if (llvm::Value *value = getEmittedValue(expr))
        return value;
    return addEmittedValue(expr, emit(expr));
}

llvm::Value *CodeGen::visit(IR::BlockExpr &expr)
{
    if (llvm::Value *value = getEmittedValue(expr))
        return value;
    return addEmittedValue(expr, emit(expr));
}

llvm::Value *CodeGen::visit(IR::BreakExpr &expr)
{
    return emit(expr);
}

llvm::Value * CodeGen::visit(IR::Prototype &expr)
{
    return emit(expr);
}

llvm::Value *CodeGen::visit(IR::Function &expr)
{
    // this cannot be cached because of different code
    // depending on genReference_
    clearValueCache();
    llvm::Value *result = emit(expr);
    clearValueCache();
    return result;
}

llvm::Value *CodeGen::visit(IR::ExternFunction &expr)
{
    clearValueCache();
    llvm::Value *result = emit(expr);
    clearValueCache();
    return result;
}

llvm::Value *CodeGen::visit(IR::Intrinsic &expr)
{
    clearValueCache();
    llvm::Value *result = emit(expr);
    clearValueCache();
    return result;
}

llvm::Value *CodeGen::visit(IR::FunctionDeclaration &expr)
{
    clearValueCache();
    llvm::Value *result = emit(expr);
    clearValueCache();
    return result;
}

llvm::Value *CodeGen::emit(IR::DefExpr &expr, EmitMode emitMode)
{
    if (!builder_.GetInsertBlock())
        return 0;

    // Look this variable up in the function.
    llvm::Value *V = namedValues_[expr.getName()];
    if (V == 0)
        DFC_THROW_EXCEPTION(CodeGenException,
                "Unknown variable name : "<<expr.getName());

    // if we should generate reference, return pointer
    if (emitMode == EMIT_REFERENCE)
        return V;

    // Load the value.
    assert(emitMode == EMIT_VALUE);
    return builder_.CreateLoad(V, expr.getName().c_str());
}

llvm::Value *CodeGen::emit(IR::MemRef &expr)
{
    IR::IRExpr::Ptr value = expr.getValue();
    if (IR::DefExpr::Ptr defExpr = dyn_cast<IR::DefExpr>(value))
        return emit(*defExpr, EMIT_REFERENCE);

    if (IR::FunctionDefinition::Ptr funcDef = dyn_cast<IR::FunctionDefinition>(value))
    {
        if (llvm::Function *F = compilerUnit->requestCallee(funcDef->getMangledName()))
            return F;
        return apply(value);
    }

    DFC_THROW_EXCEPTION(CodeGenException,
            "Unsupported memory reference to type: "<<IR::IRUtils::getTypeName(value->getExprType()));
    KIARA_UNREACHABLE("exception");
}

void CodeGen::migrateTypes()
{
	for (LLVMTypeMap::iterator itType = llvmTypeMap_.begin(); itType != llvmTypeMap_.end(); ++itType) {
		llvmTypeMap_[itType->first] = compilerUnit->migrateToTop(itType->second);
	}
}

llvm::Value *CodeGen::emit(IR::PrimLiteral &expr)
{
    if (!builder_.GetInsertBlock())
        return 0;

    switch (expr.getPrimTypeKind())
    {
        case PRIMTYPE_c_int8_t:
        case PRIMTYPE_i8:
            return llvm::ConstantInt::getSigned(llvm::Type::getInt8Ty(context_), expr.get<int8_t>());
        case PRIMTYPE_c_uint8_t:
        case PRIMTYPE_u8:
            return llvm::ConstantInt::get(llvm::Type::getInt8Ty(context_), expr.get<uint8_t>());
        case PRIMTYPE_c_int16_t:
        case PRIMTYPE_i16:
            return llvm::ConstantInt::getSigned(llvm::Type::getInt16Ty(context_), expr.get<int16_t>());
        case PRIMTYPE_c_uint16_t:
        case PRIMTYPE_u16:
            return llvm::ConstantInt::get(llvm::Type::getInt16Ty(context_), expr.get<uint16_t>());
        case PRIMTYPE_c_int32_t:
        case PRIMTYPE_i32:
            return llvm::ConstantInt::getSigned(llvm::Type::getInt32Ty(context_), expr.get<int32_t>());
        case PRIMTYPE_c_uint32_t:
        case PRIMTYPE_u32:
            return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), expr.get<uint32_t>());
        case PRIMTYPE_c_int64_t:
        case PRIMTYPE_i64:
            return llvm::ConstantInt::getSigned(llvm::Type::getInt64Ty(context_), expr.get<int64_t>());
        case PRIMTYPE_c_uint64_t:
        case PRIMTYPE_u64:
            return llvm::ConstantInt::get(llvm::Type::getInt64Ty(context_), expr.get<uint64_t>());
        case PRIMTYPE_c_float:
        case PRIMTYPE_float:
            return llvm::ConstantFP::get(context_, llvm::APFloat(expr.get<float>()));
        case PRIMTYPE_c_double:
        case PRIMTYPE_double:
            return llvm::ConstantFP::get(context_, llvm::APFloat(expr.get<double>()));
        case PRIMTYPE_boolean:
        case PRIMTYPE_c_bool:
            return expr.get<bool>() ?
                    llvm::ConstantInt::getTrue(context_) :
                    llvm::ConstantInt::getFalse(context_);
        case PRIMTYPE_string:
            return getString(expr.get<const char *>());
        default:
            KIARA_UNREACHABLE("Unknown type ID");
            break;
    }
}

llvm::Value *CodeGen::emit(IR::SymbolExpr &expr)
{
    if (!builder_.GetInsertBlock())
        return 0;

    //return getStringPtr(builder_, expr.getName(), "symbol_"+expr.getName());
    return getString(expr.getName(), "symbol_"+expr.getName());
}

llvm::Value *CodeGen::emit(IR::DefExpr &expr)
{
    return emit(expr, genReference_ ? EMIT_REFERENCE : EMIT_VALUE);
}

llvm::Value *CodeGen::emit(IR::CallExpr &expr)
{
    if (!builder_.GetInsertBlock())
        return 0;

    llvm::Value *calleeV = 0;
    llvm::Type *llvmReturnTy = 0;
    llvm::FunctionType *llvmFuncTy = 0;
    FunctionType::Ptr funcTy;
    std::string funcName;

    if (IR::FunctionDefinition::Ptr func = expr.getCalledFunction())
    {
        funcName = "function '"+func->getName()+"' (mangled : '"+func->getMangledName()+"')";

        // Look up the name in the global module table.
        llvm::Function *calleeF = compilerUnit->requestCallee(func->getMangledName());

        if (!calleeF)
        {
            // if called function is external one we can just regenerate its declaration
            // since it was possibly just optimized away
            if (isa<IR::ExternFunction>(func))
            {
                calleeF = llvm::dyn_cast<llvm::Function>(apply(func));
            }
            if (!calleeF)
                DFC_THROW_EXCEPTION(CodeGenException, funcName<<" not found");
        }

        // If argument mismatch error.
        if (calleeF->arg_size() != expr.getNumArgs())
            DFC_THROW_EXCEPTION(CodeGenException,
                    "Incorrect # arguments passed to "<<calleeF->getName().str()
                    <<", expected "<<calleeF->arg_size()<<" != "<<expr.getNumArgs());

        llvmFuncTy = calleeF->getFunctionType();
        llvmReturnTy = calleeF->getReturnType();
        calleeV = calleeF;
        funcTy = func->getFunctionType();
    }
    else
    {
        IR::IRExpr::Ptr callee = expr.getCallee();
        if (!callee)
            DFC_THROW_EXCEPTION(CodeGenException,
                    "no callee specified in CallExpr");

        PtrType::Ptr pty = getExprTypeAs<PtrType>(callee);
        if (!pty)
            DFC_THROW_EXCEPTION(CodeGenException,
                    "type of CallExpr callee is not a pointer to the function type");

        funcTy = dyn_cast<FunctionType>(pty->getElementType());
        if (!funcTy)
            DFC_THROW_EXCEPTION(CodeGenException,
                    "type of CallExpr callee is not a pointer to the function type");
        calleeV = apply(callee);
        if (!calleeV)
            DFC_THROW_EXCEPTION(CodeGenException,
                    "could not compile function pointer for indirect function call");
        llvmFuncTy =
                llvm::cast<llvm::FunctionType>(
                        llvm::cast<llvm::PointerType>(calleeV->getType())->getElementType());
        llvmReturnTy =
                llvmFuncTy->getReturnType();
        funcName = "indirect function call";
    }

    std::vector<llvm::Value*> ArgsV;
    for (unsigned i = 0, e = expr.getNumArgs(); i != e; ++i)
    {
        llvm::Value *value = createArgValue(expr.getArg(i), funcTy->getParamType(i));
        if (!value)
            DFC_THROW_EXCEPTION(CodeGenException,
                    "In "<<funcName<<"'):"
                    " Could not compile "<<(i+1)<<"-th argument, type mismatch: "
                    " argument type "<<IR::IRUtils::getTypeName(expr.getArg(i)->getExprType())
                    <<" is incompatible with"
                    " parameter type "<<IR::IRUtils::getTypeName(funcTy->getParamType(i)));

        // FIXME add type check
        llvm::Type *paramType = llvmFuncTy->getParamType(i);
        if (paramType != value->getType())
        {
            DFC_THROW_EXCEPTION(CodeGenException,
                    "Incompatible parameter and argument types in expression : "
                    <<expr<<" : expected "<<llvmToString(paramType)
                    <<" != "<<llvmToString(value->getType())<<" of value "<<llvmToString(value));
        }

        DFC_DEBUG("VALUE "<<i<<":");
        DFC_IFDEBUG(value->dump());

        ArgsV.push_back(value);
    }

    DFC_IFDEBUG(calleeV->dump());

    return builder_.CreateCall(calleeV, ArgsV, (llvmReturnTy->isVoidTy() ? "" : "calltmp"));
}

bool CodeGen::isTypesEqual(llvm::Type *a, llvm::Type *b)
{
    return (a == b ||
            (a == getUnitType() && b->isVoidTy()) ||
            (a->isVoidTy() && b == getUnitType()));
}

llvm::Value *CodeGen::emit(IR::IfExpr &expr)
{
    if (!builder_.GetInsertBlock())
        return 0;

    llvm::Value *condVal = apply(expr.getCond());
    if (!condVal)
        DFC_THROW_EXCEPTION(CodeGenException,
                "no condition expression");

    llvm::Type *condType = condVal->getType();
    // Convert condition to a bool by comparing equal to zero

    condVal = createCompareWithZero(condVal, "ifcond");
    if (!condVal)
        DFC_THROW_EXCEPTION(CodeGenException,
                "condition of type "<<*(expr.getExprType())
                <<" (LLVM : "<<llvmToString(condType)
                <<") cannot be converted to boolean");

    llvm::Function *thisFunc = builder_.GetInsertBlock()->getParent();

    // Create blocks for the then and else cases.  Insert the 'then' block at the
    // end of the function.
    llvm::BasicBlock *thenBB = llvm::BasicBlock::Create(context_, "then", thisFunc);
    llvm::BasicBlock *elseBB = llvm::BasicBlock::Create(context_, "else");
    llvm::BasicBlock *mergeBB = 0;

    builder_.CreateCondBr(condVal, thenBB, elseBB);

    // Emit then value.
    builder_.SetInsertPoint(thenBB);

    bool retVoidType = expr.getExprType() == VoidType::get(expr.getWorld());
    if (!retVoidType)
    {
        if (!expr.getThen())
            DFC_THROW_EXCEPTION(CodeGenException, "no 'then' expression in 'if' with a non-void result");
        if (!expr.getElse())
            DFC_THROW_EXCEPTION(CodeGenException, "no 'else' expression in 'if' with a non-void result");
    }

    llvm::Value *thenVal = 0;
    if (expr.getThen())
    {
        thenVal = apply(expr.getThen());
        if (thenVal == 0)
            DFC_THROW_EXCEPTION(CodeGenException, "no code for 'then' expression generated");
        if (retVoidType)
            thenVal = getUnitValue();
    }
    else
        thenVal = getUnitValue();

    BOOST_ASSERT(thenVal != 0);

    // CodeGen of 'Then' can change the current block, update ThenBB for the PHI.
    thenBB = builder_.GetInsertBlock();

    if (thenBB)
    {
        mergeBB = llvm::BasicBlock::Create(context_, "endif");
        // jump to merge after 'then'
        builder_.CreateBr(mergeBB);
    }

    // Emit else block.
    thisFunc->getBasicBlockList().push_back(elseBB);
    builder_.SetInsertPoint(elseBB);

    llvm::Value *elseVal = 0;
    if (expr.getElse())
    {
        elseVal = apply(expr.getElse());
        if (elseVal == 0)
            DFC_THROW_EXCEPTION(CodeGenException, "no code for 'else' expression generated");
        if (retVoidType)
            elseVal = getUnitValue();
    }
    else
        elseVal = getUnitValue();

    BOOST_ASSERT(elseVal != 0);

    if (thenVal->getType()->isVoidTy())
        thenVal = getUnitValue();
    if (elseVal->getType()->isVoidTy())
        elseVal = getUnitValue();

    if (!isTypesEqual(thenVal->getType(), elseVal->getType()))
        DFC_THROW_EXCEPTION(CodeGenException, "in expression '"<<expr<<"' are incompatible LLVM types: "
                <<llvmToString(thenVal->getType())<<" != "
                <<llvmToString(elseVal->getType())
                <<", value1: "<<llvmToString(thenVal)
                <<", value2: "<<llvmToString(elseVal));

    // CodeGen of 'Else' can change the current block, update ElseBB for the PHI.
    llvm::BasicBlock *currentBB = builder_.GetInsertBlock();
    elseBB = currentBB;

    if (currentBB && !currentBB->getTerminator())
    {
        if (!mergeBB)
            mergeBB = llvm::BasicBlock::Create(context_, "endif");
        builder_.CreateBr(mergeBB);

        // Emit merge block.
        thisFunc->getBasicBlockList().push_back(mergeBB);
        builder_.SetInsertPoint(mergeBB);
    }
    else
    {
        if (mergeBB && mergeBB->getParent() == 0)
        {
            // Emit merge block.
            thisFunc->getBasicBlockList().push_back(mergeBB);
            builder_.SetInsertPoint(mergeBB);
        }
    }

    if (thenBB && elseBB)
    {
        llvm::PHINode *PN = builder_.CreatePHI(thenVal->getType(), 2);

        if (!thenVal->getType()->isVoidTy())
            PN->setName("iftmp");

        PN->addIncoming(thenVal, thenBB);
        PN->addIncoming(elseVal, elseBB);
        return PN;
    }

    //if (!currentBB)
    //    return 0;

    return getUnitValue();
}

llvm::Value *CodeGen::emit(IR::LoopExpr &expr)
{
    if (!builder_.GetInsertBlock())
        return 0;

    llvm::Function *func = builder_.GetInsertBlock()->getParent();

    // Make the new basic block for the loop header, inserting after current
    // block.
    llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(context_, "loop", func);

    // Insert an explicit fall through from the current block to the LoopBB.
    builder_.CreateBr(LoopBB);

    // Start insertion in LoopBB.
    builder_.SetInsertPoint(LoopBB);

    // Emit the body of the loop.  This, like any other expr, can change the
    // current BB.  Note that we ignore the value computed by the body, but don't
    // allow an error.
    llvm::Value *body = apply(expr.getBody());
    if (body == 0)
        DFC_THROW_EXCEPTION(CodeGenException, "LoopExpr: could not compile loop body");

    if (builder_.GetInsertBlock())
    {
        // Insert the conditional branch into the end of LoopEndBB.
        builder_.CreateBr(LoopBB);
    }

    // Any new code will be inserted in AfterBB.
    //builder_.SetInsertPoint(AfterBB);

    builder_.ClearInsertionPoint();

    return getUnitValue();
}


llvm::Value *CodeGen::emit(IR::ForExpr &expr)
{
    if (!builder_.GetInsertBlock())
        return 0;

    // Output this as:
    //   var = alloca double
    //   ...
    //   start = startexpr
    //   store start -> var
    //   goto loop
    // loop:
    //   ...
    //   bodyexpr
    //   ...
    // loopend:
    //   step = stepexpr
    //   endcond = endexpr
    //
    //   curvar = load var
    //   nextvar = curvar + step
    //   store nextvar -> var
    //   br endcond, loop, endloop
    // outloop:

    llvm::Function *func = builder_.GetInsertBlock()->getParent();

    llvm::Type *varType = getLLVMType(expr.getVarType(), VALUE_TYPE);
    if (!varType)
        DFC_THROW_EXCEPTION(CodeGenException, "No variable type in for loop : "<<expr);

    // Create an alloca for the variable in the entry block.
    llvm::AllocaInst *Alloca = createEntryBlockAlloca(
            func, expr.getVarName(), varType);

    // Emit the start code first, without 'variable' in scope.
    llvm::Value *StartVal = apply(expr.getStart());
    if (StartVal == 0)
        DFC_THROW_EXCEPTION(CodeGenException, "ForExpr: could not compile start value");

    // Store the value into the alloca.
    builder_.CreateStore(StartVal, Alloca);

    // Make the new basic block for the loop header, inserting after current
    // block.
    llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(context_, "loop", func);

    // Insert an explicit fall through from the current block to the LoopBB.
    builder_.CreateBr(LoopBB);

    // Start insertion in LoopBB.
    builder_.SetInsertPoint(LoopBB);

    // Within the loop, the variable is defined equal to the PHI node.  If it
    // shadows an existing variable, we have to restore it, so save it now.
    llvm::AllocaInst *OldVal = namedValues_[expr.getVarName()];
    namedValues_[expr.getVarName()] = Alloca;

    // Emit the body of the loop.  This, like any other expr, can change the
    // current BB.  Note that we ignore the value computed by the body, but don't
    // allow an error.
    if (apply(expr.getBody()) == 0)
        DFC_THROW_EXCEPTION(CodeGenException, "ForExpr: could not compile loop body");

    // Emit the step value.
    llvm::Value *StepVal;
    if (expr.getStep())
    {
        StepVal = apply(expr.getStep());
        if (StepVal == 0)
            DFC_THROW_EXCEPTION(CodeGenException, "ForExpr: could not compile step expression");
    }
    else
    {
        // If not specified, use 1.0.
        StepVal = createOne(varType);
    }

    // Compute the end condition.
    llvm::Value *EndCond = apply(expr.getEnd());
    if (EndCond == 0)
        DFC_THROW_EXCEPTION(CodeGenException, "ForExpr: could not compile end condition");

    // Reload, increment, and restore the alloca.  This handles the case where
    // the body of the loop mutates the variable.
    llvm::Value *CurVar = builder_.CreateLoad(Alloca, expr.getVarName().c_str());
    // FIXME replace by user operator
    llvm::Value *NextVar = createAdd(CurVar, StepVal, false, "nextvar");
    builder_.CreateStore(NextVar, Alloca);

    // Convert condition to a bool by comparing equal to 0.0.
    llvm::Type *endTy = EndCond->getType();
    EndCond = createCompareWithZero(EndCond, "loopcond");
    if (!EndCond)
        DFC_THROW_EXCEPTION(CodeGenException,
                "condition type "<<*(expr.getEnd()->getExprType())
                <<" (LLVM : "<<llvmToString(endTy)
                <<") cannot be converted to boolean");

    // Create the "after loop" block and insert it.
    llvm::BasicBlock *AfterBB = llvm::BasicBlock::Create(context_, "afterloop", func);

    // Insert the conditional branch into the end of LoopEndBB.
    builder_.CreateCondBr(EndCond, LoopBB, AfterBB);

    // Any new code will be inserted in AfterBB.
    builder_.SetInsertPoint(AfterBB);

    // Restore the unshadowed variable.
    if (OldVal)
        namedValues_[expr.getVarName()] = OldVal;
    else
        namedValues_.erase(expr.getVarName());

    // for expr returns 0.0 when possible, otherwise undef
    llvm::Type *resultTy = getLLVMType(expr.getBody()->getExprType(), VALUE_TYPE);

    llvm::Value *result;
    if (resultTy->isVoidTy())
        result = getUnitValue(); // llvm::UndefValue::get(resultTy) ?
    else
        result = llvm::Constant::getNullValue(resultTy);
    return result;
}

llvm::Value *CodeGen::emit(IR::LetExpr &expr)
{
    if (!builder_.GetInsertBlock())
        return 0;

    llvm::AllocaInst * oldBinding;

    llvm::Function *func = builder_.GetInsertBlock()->getParent();

    // Register variable and emit its initializer.
    const IR::DefExpr::Ptr &var = expr.getVar();
    const IR::IRExpr::Ptr &init = expr.getInitValue();

    const Type::Ptr varType = var->getExprType();
    if (!varType)
        DFC_THROW_EXCEPTION(CodeGenException,
                "No type for variable : "<<var->getName());

    Type::Ptr initType;
    if (init)
        initType = init->getExprType();
    else
        initType = varType;

    if (!initType)
        DFC_THROW_EXCEPTION(CodeGenException, "No type for initializer");

    llvm::Type *llvmVarType = getLLVMType(varType, VALUE_TYPE);
    if (!llvmVarType)
        DFC_THROW_EXCEPTION(CodeGenException,
                "Could not create LLVM type from "<<*varType<<" type");

    llvm::Type *llvmInitType = getLLVMType(initType, VALUE_TYPE);
    if (!llvmInitType)
        DFC_THROW_EXCEPTION(CodeGenException,
                "Could not create LLVM type from "<<*initType<<" type");

    if (llvmVarType != llvmInitType)
        DFC_THROW_EXCEPTION(CodeGenException,
                "LLVM types mismatch : "
                <<llvmToString(llvmVarType)<<" != "
                <<llvmToString(llvmInitType)<<" type");

    // Emit the initializer before adding the variable to scope, this prevents
    // the initializer from referencing the variable itself, and permits stuff
    // like this:
    //  var a = 1 in
    //    var a = a in ...   # refers to outer 'a'.
    llvm::Value *initVal;
    if (init)
    {
        initVal = createArgValue(init, varType);
        if (!initVal)
            DFC_THROW_EXCEPTION(CodeGenException,
                    "Could not create initialization value for variable."
                    " Variable type: "<<IR::IRUtils::getTypeName(varType)
                    <<", Initialization value: "<<*init<<" of type: "
                    <<IR::IRUtils::getTypeName(init->getExprType()));
    }
    else
    {
        // If not specified, use 0. // FIXME use undef ?
        initVal = llvm::Constant::getNullValue(llvmVarType);
    }

    llvm::AllocaInst *allocaVal = createEntryBlockAlloca(func, var->getName(), llvmVarType);
    builder_.CreateStore(initVal, allocaVal);

    // Remember the old variable binding so that we can restore the binding when
    // we unrecurse.
    // FIXME use var pointer instead of name as a key
    oldBinding = namedValues_[var->getName()];

    // Remember this binding.
    namedValues_[var->getName()] = allocaVal;

    DFC_DEBUG("Overwrite binding "<<var->getName()<<" from "<<llvmToString(oldBinding)<<" to "<<llvmToString(allocaVal));

    // CodeGen the body, now that all vars are in scope.
    llvm::Value *bodyVal = apply(expr.getBody());
    if (!bodyVal)
        DFC_THROW_EXCEPTION(CodeGenException, "no body expression generated");

    // Pop all our variables from scope.
    namedValues_[var->getName()] = oldBinding;

    // Return the body computation.
    return bodyVal;
}

llvm::Value *CodeGen::emit(IR::BlockExpr &expr)
{
    if (!builder_.GetInsertBlock())
        return 0;

    llvm::Function *thisFunc = builder_.GetInsertBlock()->getParent();
    llvm::BasicBlock *thisBB = llvm::BasicBlock::Create(context_, "begin_block_"+expr.getName(), thisFunc);
    llvm::BasicBlock *nextBB = llvm::BasicBlock::Create(context_, "end_block_"+expr.getName());
    llvm::Value *result = 0;

    builder_.CreateBr(thisBB);
    builder_.SetInsertPoint(thisBB);

    BlockInfo binfo(&expr, nextBB);
    blockDeque_.push_back(&binfo);

    const IR::BlockExpr::ExprList &exprList = expr.getExprList();

    llvm::Type *exprType = getLLVMType(expr.getExprType(), RETURN_TYPE);

    exprType = exprType ? voidToUnitType(exprType) : getUnitType();

    if (exprList.empty())
        result = getUndefValue(exprType);
    else
    {
        for (IR::BlockExpr::ExprList::const_iterator it = exprList.begin(),
                 end = exprList.end(); it != end; ++it)
        {
            llvm::Value *tmp = apply(*it);
            if (tmp)
                result = tmp;
        }
        if (result && !isTypesEqual(result->getType(), exprType))
        {
            DFC_THROW_EXCEPTION(CodeGenException,
                                "LLVM type mismatch in block expression: "<<
                                llvmToString(result->getType())<<" != "
                                <<llvmToString(exprType));
        }
    }

    blockDeque_.pop_back();

    thisBB = builder_.GetInsertBlock();
    if (!thisBB && !hasPredecessors(nextBB))
    {
        delete nextBB;
        result = getUnitValue();
    }
    else
    {
        if (thisBB)
            builder_.CreateBr(nextBB);

        thisFunc->getBasicBlockList().push_back(nextBB);
        // code generation can change the current block
        builder_.SetInsertPoint(nextBB);

        if (!binfo.breakSites.empty())
        {
            DFC_DEBUG("ADD PHI TY "<<llvmToString(exprType));

            // FIXME add check that all types are the same
            size_t numResults = (result ? 1 : 0) + binfo.breakSites.size();
            llvm::PHINode *PN = builder_.CreatePHI(exprType, numResults, "blocktmp");
            for (BlockInfo::BreakSiteList::iterator it = binfo.breakSites.begin(),
                     end = binfo.breakSites.end(); it != end; ++it)
            {
                DFC_DEBUG("ADD TY "<<llvmToString(it->first));

                if (PN->getType() != it->first->getType())
                {
                    DFC_IFDEBUG(thisFunc->dump());

                    DFC_THROW_EXCEPTION(CodeGenException,
                                        "Type mismatch in break target: "<<
                                        llvmToString(PN->getType())<<" != "
                                        <<llvmToString(it->first->getType()));
                }

                PN->addIncoming(it->first, it->second);
            }
            if (result && thisBB)
            {
                if (result->getType()->isVoidTy())
                    result = getUnitValue();

                PN->addIncoming(result, thisBB);
            }
            result = PN;
        }
    }

    return result;
}

llvm::Value *CodeGen::emit(IR::BreakExpr &expr)
{
    if (!builder_.GetInsertBlock())
        return 0;

    if (!expr.getBlock())
        DFC_THROW_EXCEPTION(CodeGenException,
                "Could not generate break, no block argument specified");
    for (BlockDeque::reverse_iterator it = blockDeque_.rbegin(),
            end = blockDeque_.rend(); it != end; ++it)
    {
        DFC_DEBUG("Break: Block PTR "<<(*it)->expr.get()<<" "<<(*it)->expr->toString());
        if ((*it)->expr == expr.getBlock())
        {
            BlockInfo *binfo = *it;
            llvm::Value *retVal;
            if (expr.getValue())
                retVal = apply(expr.getValue());
            else
                retVal = getUnitValue();
            llvm::BasicBlock *thisBlock = builder_.GetInsertBlock();
            //llvm::Function *thisFunc = thisBlock->getParent();
            binfo->breakSites.push_back(std::make_pair(retVal, thisBlock));

            DFC_DEBUG("BREAK SITES ADD "<<binfo->breakSites.size());

            builder_.CreateBr(binfo->nextBasicBlock);
            builder_.ClearInsertionPoint();
            //llvm::BasicBlock *nextBB = llvm::BasicBlock::Create(getContext(), "afterBreak", thisFunc);
            //builder_.SetInsertPoint(nextBB);
            //return getUnitValue();
            return retVal;
        }
    }
    DFC_THROW_EXCEPTION(CodeGenException, "Break outside of block");
    KIARA_UNREACHABLE("exception");
}

/// CreateArgumentAllocas - Create an alloca for each argument and register the
/// argument in the symbol table so that references to it will succeed.
void CodeGen::createArgumentAllocas(IR::Prototype &proto, llvm::Function *F)
{
    DFC_DEBUG("createArgumentAllocas for proto "<<proto);

    llvm::Function::arg_iterator AI = F->arg_begin();
    const std::vector<IR::Prototype::Arg> & args = proto.getArgs();
    for (unsigned idx = 0, e = args.size(); idx != e; ++idx, ++AI)
    {
        // Create an alloca for this variable.
        llvm::AllocaInst *Alloca = createEntryBlockAlloca(F, args[idx].first, AI->getType());

        // Store the initial value into the alloca.
        builder_.CreateStore(AI, Alloca);

        // Add arguments to variable symbol table.
        namedValues_[args[idx].first] = Alloca;

        DFC_DEBUG("Create argument alloca for "<<args[idx].first<<" : "<<llvmToString(Alloca)
                  <<" from type "<<IR::IRUtils::getTypeName(args[idx].second));
    }
}

llvm::Value * CodeGen::getString(const std::string &str, const llvm::Twine &name)
{
    // try to find in cache
    StringCacheMap::const_iterator it = stringCache_.find(str);
    if (it != stringCache_.end())
    {
        if (llvm::Value *value = it->second)
            return value;
    }

    llvm::Constant *strConstant = llvm::ConstantDataArray::getString(getContext(), str, true);

    llvm::GlobalVariable *gv = new llvm::GlobalVariable(
            *compilerUnit->getActiveModule(), // TODO do not use getActiveModule
            strConstant->getType(),
            true,
            llvm::GlobalValue::InternalLinkage,
            strConstant, "" /*, 0, false*/);
    gv->setName(name);

    // store in cache
    stringCache_[str] = gv;

    return gv;
}

llvm::Value * CodeGen::getStringPtr(
        llvm::IRBuilder<> &builder,
        const std::string &str,
        const llvm::Twine &name)
{
    llvm::Value *gv = getString(str, name);
    llvm::Value *zero = getInt32(0);
    llvm::Value *args[] = { zero, zero };
    return builder.CreateInBoundsGEP(gv, llvm::ArrayRef<llvm::Value*>(args, args+2), name);
}

llvm::Value * CodeGen::getStringPtr(
        const std::string &str,
        const llvm::Twine &name,
        llvm::Instruction *insertBefore)
{
    llvm::Value *gv = getString(str, name);
    llvm::Value *zero = getInt32(0);
    llvm::Value *args[] = { zero, zero };
    return llvm::GetElementPtrInst::CreateInBounds(
            gv, llvm::ArrayRef<llvm::Value*>(args, args+2), name, insertBefore);
}

void CodeGen::clearStringCache()
{
    stringCache_.clear();
}

llvm::Type * CodeGen::getLLVMTypeCtx(PrimTypeKind primTypeKind, llvm::LLVMContext &ctx)
{
    switch (primTypeKind)
    {
        case PRIMTYPE_c_int8_t:
        case PRIMTYPE_c_uint8_t:
        case PRIMTYPE_i8:
        case PRIMTYPE_u8:
            return llvm::Type::getInt8Ty(ctx);
        case PRIMTYPE_c_int16_t:
        case PRIMTYPE_c_uint16_t:
        case PRIMTYPE_i16:
        case PRIMTYPE_u16:
            return llvm::Type::getInt16Ty(ctx);
        case PRIMTYPE_c_int32_t:
        case PRIMTYPE_c_uint32_t:
        case PRIMTYPE_i32:
        case PRIMTYPE_u32:
            return llvm::Type::getInt32Ty(ctx);
        case PRIMTYPE_c_int64_t:
        case PRIMTYPE_c_uint64_t:
        case PRIMTYPE_i64:
        case PRIMTYPE_u64:
            return llvm::Type::getInt64Ty(ctx);
        case PRIMTYPE_c_float:
        case PRIMTYPE_float:
            return llvm::Type::getFloatTy(ctx);
        case PRIMTYPE_c_double:
        case PRIMTYPE_double:
            return llvm::Type::getDoubleTy(ctx);
        case PRIMTYPE_boolean:
            return llvm::Type::getInt1Ty(ctx);
        case PRIMTYPE_c_longdouble:
            return llvm::Type::getX86_FP80Ty(ctx); // FIXME what to do on non-x86 platform
        case PRIMTYPE_c_nullptr:
            return llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(ctx));
        case PRIMTYPE_string:
            return llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(ctx));
        default:
            break;
    }
    KIARA_UNREACHABLE("Unknown type ID");
}

static bool isExplicitLayout(const StructType &structType)
{
    const size_t numElements = structType.getNumElements();
    for (size_t i = 0; i < numElements; ++i)
    {
        if (structType.getElementDataAt(i).hasAttributeValue<NativeOffsetAttr>())
            return true;
    }
    return false;
}

#if 0
void CodeGen::insertTrackedTypes(NamedTypeMap & trackedTypes)
{
	for (LLVMTypeMap::iterator itPair = llvmTypeMap_.begin(); itPair != llvmTypeMap_.end(); ++itPair) {
		//Type::Ptr kiaraType = itPair->first;
		llvm::Type * llvmType = itPair->second;
		if (llvm::isa<llvm::StructType>(llvmType)) {
			trackedTypes[llvmType->getStructName()] =llvm::cast<llvm::StructType>(llvmType);
		}
	}
}
#endif

llvm::Type * CodeGen::getLLVMType(const Type & type, TypeMode typeMode)
{
    Type::Ptr tmp = type.getCanonicalType();
    const Type & canonicalType = *tmp;

    const DFC::ObjectType &typeId = canonicalType.getType();
    if (typeId == DFC_STATIC_TYPE(PrimType))
        return getLLVMType(static_cast<const PrimType&>(canonicalType).primtype_kind());
    if (typeId == DFC_STATIC_TYPE(VoidType))
        // map void type to empty struct / unit if it is function argument
        return typeMode == RETURN_TYPE ? llvm::Type::getVoidTy(getContext()) : llvm::StructType::get(getContext());
    if (typeId == DFC_STATIC_TYPE(PtrType) || typeId == DFC_STATIC_TYPE(RefType))
    {
        Type::Ptr elemType;
        if (typeId == DFC_STATIC_TYPE(PtrType))
            elemType = static_cast<const PtrType&>(canonicalType).getElementType();
        else
            elemType = static_cast<const RefType&>(canonicalType).getElementType();

        if (elemType == elemType->getWorld().type_void())
            return getVoidPtrTy();
        llvm::Type *llvmElemType = getLLVMType(elemType, typeMode);
        return llvm::PointerType::getUnqual(llvmElemType);
    }
    if (typeId == DFC_STATIC_TYPE(ArrayType) || typeId == DFC_STATIC_TYPE(FixedArrayType))
    {
        // FIXME this should possibly work only in ARGUMENT_TYPE mode
        Type::Ptr elemType;
        if (typeId == DFC_STATIC_TYPE(ArrayType))
            elemType = static_cast<const ArrayType&>(canonicalType).getElementType();
        else
            elemType = static_cast<const FixedArrayType&>(canonicalType).getElementType();

        if (elemType == elemType->getWorld().type_void())
            DFC_THROW_EXCEPTION(CodeGenException,
                    "element type of an array cannot be 'void'");

        llvm::Type *llvmElemType = getLLVMType(elemType, typeMode);
        return llvm::PointerType::getUnqual(llvmElemType);
    }
    if (typeId == DFC_STATIC_TYPE(StructType))
    {
        const StructType &structType = static_cast<const StructType&>(canonicalType);
        StructType::Ptr typePtr = const_cast<StructType*>(&structType);
        LLVMTypeMap::iterator it = llvmTypeMap_.find(typePtr);
        if (it != llvmTypeMap_.end())
            return it->second;

        llvm::StructType *llvmStructType = 0;
        bool packed = false;

        std::string llvmTypeName = IR::IRUtils::getTypeName(typePtr);

        // Explicit layout means that LLVM struct type stay opaque
        // and access to the members is done by computing offsets
        const bool explicitLayout = isExplicitLayout(structType);

        if (structType.isUnique())
        {
            llvm::Type * foundType = compilerUnit->getTypeByName(llvmTypeName);
            if (!foundType)
            {
                std::string tmp = typePtr->getAttributeAsString("llvmName");
                if (tmp.empty())
                    llvmTypeName = "struct." + llvmTypeName; // for clang compatibility
                else
                    llvmTypeName = tmp;
                foundType = compilerUnit->getTypeByName(llvmTypeName);
            }

            assert(!foundType || llvm::isa<llvm::StructType>(foundType));

            llvmStructType = foundType ? llvm::dyn_cast<llvm::StructType>(foundType) : 0;
            if (llvmStructType)
            {
                if (structType.isOpaque() || explicitLayout)
                {
                    llvmTypeMap_[typePtr] = llvmStructType;
                    return llvmStructType;
                }
                DFC_THROW_EXCEPTION(CodeGenException,
                                    "Redefinition of the non-opaque struct type: "<<llvmTypeName);
            }

            llvmStructType = compilerUnit->declareType(llvmTypeName);
            llvmTypeMap_[typePtr] = llvmStructType;
        }

        // FIXME there is no recursion detection, infinite loop is possible

        size_t numElements = structType.getNumElements();
        std::vector<llvm::Type*> elements;

        if (!explicitLayout)
        {
            elements.reserve(numElements);
            for (size_t i = 0; i < numElements; ++i)
            {
                elements.push_back(getLLVMType(structType.getElementAt(i), VALUE_TYPE));
            }
        }

        if (llvmStructType)
        {
            if (!structType.isOpaque())
                llvmStructType->setBody(elements, packed);
        }
        else
        {
            if (structType.isUnique()) {
                llvmStructType = llvm::StructType::create(
                        context_,
                        elements,
                        llvmTypeName,
                        packed);
                compilerUnit->registerTrackedType(llvmTypeName, llvmStructType);
            } else
                llvmStructType = llvm::StructType::get(
                        context_,
                        elements,
                        packed);
            llvmTypeMap_[typePtr] = llvmStructType;
        }
        return llvmStructType;
    }
    if (typeId == DFC_STATIC_TYPE(FunctionType))
    {
        const FunctionType &funcType = static_cast<const FunctionType&>(canonicalType);
        FunctionType::Ptr typePtr = const_cast<FunctionType*>(&funcType);
        LLVMTypeMap::iterator it = llvmTypeMap_.find(typePtr);
        if (it != llvmTypeMap_.end())
            return it->second;

        llvm::FunctionType *llvmFuncType = 0;

        // FIXME there is no recursion detection, infinite loop is possible

        size_t numElements = funcType.getNumParams();
        std::vector<llvm::Type*> elements;
        elements.reserve(numElements);
        for (size_t i = 0; i < numElements; ++i)
        {
            llvm::Type *ty = getLLVMType(funcType.getParamType(i), ARGUMENT_TYPE);
            if (llvm::StructType *sty = llvm::dyn_cast<llvm::StructType>(ty))
            {
                if (sty->isOpaque())
                    DFC_THROW_EXCEPTION(CodeGenException,
                            "opaque type cannot be argument of a function: "<<
                            IR::IRUtils::getTypeName(funcType.getParamType(i)));
            }
            elements.push_back(ty);
        }

        llvm::Type *returnType = getLLVMType(funcType.getReturnType(), RETURN_TYPE);

        llvmFuncType = llvm::FunctionType::get(returnType, elements, /*isVarArg=*/false);

        llvmTypeMap_[typePtr] = llvmFuncType;
        return llvmFuncType;
    }
    if (typeId == DFC_STATIC_TYPE(SymbolType))
    {
        // FIXME this does make sense only at compile time
        //       since symbol(X) type has only a single value.
        size_t len = static_cast<const SymbolType&>(canonicalType).getSymbol().length();
        return getCharArrayPtrTy(len+1); // +1 for trailing zero
    }

    DFC_THROW_EXCEPTION(Exception, "Unhandled type: "<<canonicalType.getType().getName());
    KIARA_UNREACHABLE("exception");
}

llvm::Value * CodeGen::getDefaultValue(llvm::Type *type)
{
    //llvm::LLVMContext &ctx = type->getContext();
    if (type->isIntegerTy())
        return llvm::ConstantInt::get(type, 0);
    if (type->isFloatTy())
        return llvm::ConstantFP::get(type, 0.0);
    return llvm::UndefValue::get(type);
}

llvm::Value * CodeGen::getUndefValue(llvm::Type *type)
{
    if (!type || type->isVoidTy())
        return getUnitValue();
    return llvm::UndefValue::get(type);
}

llvm::Function * CodeGen::createFunction(const std::string &name, IR::Prototype &proto)
{
    // Make the function type:  double(double,double) etc.
    std::vector<llvm::Type*> argTypes(proto.getNumArgs());
    for (size_t i = 0; i < proto.getNumArgs(); ++i)
    {
        argTypes[i] = getLLVMType(proto.getArgType(i), VALUE_TYPE);
    }
    llvm::Type *returnType = getLLVMType(proto.getReturnType(), RETURN_TYPE);
    if (!returnType)
        DFC_THROW_EXCEPTION(CodeGenException,
                            "No return type in prototype: "<<proto<<" at "
                            <<source_location(proto.getLocation()));
    llvm::FunctionType *FT = llvm::FunctionType::get(returnType, argTypes, false);

    llvm::GlobalValue::LinkageTypes linkage = llvm::Function::ExternalLinkage;
    if (proto.hasAttribute("internal"))
        linkage = llvm::Function::InternalLinkage;

    if (llvm::Function * existingFunc = compilerUnit->getFunction(name))
    {
        std::string errorMsg;

        if (FT != existingFunc->getFunctionType())
        {
            DFC_THROW_EXCEPTION(CodeGenException,
                "redefinition of function '"<<name<<"' with a different type: "
                <<llvmToString(FT)<<", existing has type: "
                <<llvmToString(existingFunc->getFunctionType()));
        }
        else
            return existingFunc;

        assert(!existingFunc);
#if 0
        return compilerUnit->requestCallee(name); // TODO: works only for calls
#endif
    }

    llvm::Function * F = compilerUnit->createFunction(FT, linkage, name);
    if (proto.hasAttribute("always_inline"))
        F->addFnAttr(llvm::Attribute::AlwaysInline);

    // If F conflicted, there was already something named 'Name'.  If it has a
    // body, don't allow redefinition or reextern.
#if 1
#if 0 // MCJITCompilerUnit uses aggressive mangling..
    assert(F->getName() == name && "compiler unit can not resolve name conflicts");
#endif
    // TODO re-open the conflicting module, modify the function, merge in all dependent stages and recompile it
#else
    if (F->getName() != name)
    {
        // Delete the one we just made and get the existing one.
        F->eraseFromParent();
        F = getModule()->getFunction(name);

        // If F already has a body, reject this.
        if (!F->empty())
        {
            DFC_THROW_EXCEPTION(CodeGenException,
                    "redefinition of function '"<<name<<"'");
        }

        // If F took a different number of args, reject.
        if (F->arg_size() != proto.getNumArgs())
        {
            DFC_THROW_EXCEPTION(CodeGenException,
                    "redefinition of function '"<<name<<"' with different # args");
        }
    }
#endif

    // Set names for all arguments.
    unsigned Idx = 0;
    for (llvm::Function::arg_iterator AI = F->arg_begin(); Idx != proto.getNumArgs(); ++AI, ++Idx)
        AI->setName(proto.getArgName(Idx));

    return F;
}

llvm::Value * CodeGen::emit(IR::Prototype &proto)
{
    return createFunction(proto.getName(), proto);
}

llvm::Value *CodeGen::emit(IR::Function &func)
{
    if (genReference_)
    {
        if (llvm::Function *F = compilerUnit->requestCallee(func.getMangledName()))
            return F;
    }

    namedValues_.clear();

    IR::Prototype::Ptr proto = func.getProto();
    llvm::Function *llvmFunc = createFunction(func.getMangledName(), *proto);
    assert(llvmFunc != 0);

    // Create a new basic block to start insertion into.
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(context_, "entry", llvmFunc);
    builder_.SetInsertPoint(BB);

    // Add all arguments to the symbol table and create their allocas.
    createArgumentAllocas(*func.getProto(), llvmFunc);

    llvm::Value *retVal = apply(func.getBody());
    if (!retVal)
    {
        // Error reading body, remove function.
        llvmFunc->eraseFromParent();
        DFC_THROW_EXCEPTION(CodeGenException, "Could not compile body of a function '"<<func.getName()
                            <<" at "<<source_location(func.getLocation()));
    }

    // Finish off the function.
    if (llvmFunc->getReturnType()->isVoidTy())
        builder_.CreateRetVoid();
    else
    {
        if (retVal->getType() != llvmFunc->getReturnType())
        {
            DFC_IFDEBUG(llvmFunc->dump());

            DFC_THROW_EXCEPTION(CodeGenException,
                                "Type mismatch in function return: "<<
                                llvmToString(retVal)<<" != "
                                <<llvmToString(llvmFunc->getReturnType()));
        }

        builder_.CreateRet(retVal);
    }

    DFC_IFDEBUG(retVal->dump());
    DFC_IFDEBUG(llvmFunc->dump());

    // Validate the generated code, checking for consistency.
    verifyFunction(*llvmFunc);

    // Optimize the function.
    // compilerUnit->optimizeFunction(llvmFunc);

    return llvmFunc;
}

llvm::Value * CodeGen::emit(IR::ExternFunction &func)
{
    return createFunction(func.getMangledName(), *func.getProto());
}

void CodeGen::emitAssemblyString(const std::string &llvmAssembly)
{
    std::string assembly = llvmDefs_ + "\n";
//    for (LLVMTypeMap::iterator it = llvmTypeMap_.begin(), end = llvmTypeMap_.end();
//            it != end; ++it)
//    {
//        if (it->second->isStructTy())
//        {
//            llvm::StructType *sty = llvm::dyn_cast<llvm::StructType>(it->second);
//            if (sty->hasName())
//                assembly += llvmToString(sty) + "\n";
//        }
//    }

    assembly += llvmAssembly;
    Preprocessor::VariableMap ppvars(llvmPPVars_);
    Preprocessor pp(ppvars);
    assembly = pp.substVars(assembly);
    std::string errorMsg;
    if (!llvmExtParseAssemblyString(assembly.c_str(), *compilerUnit->getActiveModule(), errorMsg)) // TODO do not use getActiveModule
        DFC_THROW_EXCEPTION(CodeGenException,
                "could not parse LLVM assembly "<<errorMsg);
    compilerUnit->event_activeModuleModified();
}

bool CodeGen::isSupported(const IR::FunctionDefinition::Ptr &funcDef) const
{
    if (IR::Intrinsic::Ptr ir = dyn_cast<IR::Intrinsic>(funcDef))
        if (!ir->getProto()->hasAttribute("llvm"))
            return false;
    return true;
}

llvm::Value *CodeGen::emit(IR::Intrinsic &func)
{
    const IR::Prototype::Ptr &proto = func.getProto();
    if (!proto->hasAttribute("llvm"))
        return 0; // FIXME what to do here ?

    namedValues_.clear();

    std::string funcName = proto->getName();
    const std::string mangledFuncName = proto->getMangledName();

    Preprocessor::VariableMap ppvars(llvmPPVars_);
    ppvars["name"] = funcName;
    ppvars["mangledName"] = mangledFuncName;

    // check that parameter and return types are defined and generate
    // variables for substitution
    std::string idx;
    for (size_t i = 0; i < proto->getNumArgs(); ++i)
    {
        idx = boost::lexical_cast<std::string>(i);
        ppvars["argname"+idx] = proto->getArgName(i);
        ppvars["argtype"+idx] = llvmGetTypeIdentifier(
                getLLVMType(proto->getArgType(i), VALUE_TYPE));
    }
    llvm::Type *returnType = getLLVMType(proto->getReturnType(), RETURN_TYPE);
    ppvars["rettype"] = llvmGetTypeIdentifier(returnType);

    std::string errorMsg;
    std::string assembly = llvmDefs_ + "\n";
//    for (LLVMTypeMap::iterator it = llvmTypeMap_.begin(), end = llvmTypeMap_.end();
//            it != end; ++it)
//    {
//        if (it->second->isStructTy())
//        {
//            llvm::StructType *sty = llvm::dyn_cast<llvm::StructType>(it->second);
//            if (sty->hasName())
//                assembly += llvmToString(sty) + "\n";
//        }
//    }

    assembly += func.getBody();


    // preprocess assembly
    Preprocessor pp(ppvars);
    assembly = pp.substVars(assembly);

    DFC_DEBUG("-----------");
    DFC_DEBUG(assembly);

    // TODO: what if the intrinsic was already defined in another module?
    if (!llvmExtParseAssemblyString(assembly.c_str(), *compilerUnit->getActiveModule(), errorMsg)) // TODO do not use getActiveModule
        DFC_THROW_EXCEPTION(CodeGenException,
                "could not parse LLVM assembly for function "<<funcName<<" : "<<errorMsg);
    compilerUnit->event_activeModuleModified();

    llvm::Function *F = compilerUnit->getFunction(funcName);

    if (!F)
    {
        funcName = mangledFuncName;
        F = compilerUnit->getFunction(funcName);
    }

    if (!F)
        DFC_THROW_EXCEPTION(CodeGenException,
                "No LLVM function: '"<<func.getProto()->getName()
                <<"' (mangled : '"<<mangledFuncName
                <<"') specified by prototype");

    if (!F->isDeclaration())
    {
        // Validate the generated code, checking for consistency.
        verifyFunction(*F);

        if (proto->hasAttribute("always_inline"))
            F->addFnAttr(llvm::Attribute::AlwaysInline);

        // Optimize the function.
        // compilerUnit->optimizeFunction(F);
    }

    return F;
}

llvm::Value * CodeGen::emit(IR::FunctionDeclaration &decl)
{
    // FIXME this works only for "undef"
    return 0;
}

} // namespace Compiler

} // namespace KIARA
