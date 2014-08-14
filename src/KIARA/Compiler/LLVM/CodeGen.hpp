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
 * CodeGen.hpp
 *
 *  Created on: 30.01.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMPILER_LLVM_CODEGEN_HPP_INCLUDED
#define KIARA_COMPILER_LLVM_CODEGEN_HPP_INCLUDED

#include <KIARA/Compiler/Config.hpp>
#include "llvm/Config/llvm-config.h"
#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
#include "llvm/IR/Function.h"
#else
#include "llvm/Function.h"
#endif
#include "llvm/PassManager.h"
#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
#include "llvm/IR/IRBuilder.h"
#else
#include "llvm/Support/IRBuilder.h"
#endif
#include "llvm/Support/CFG.h"
#include "llvm/Support/ValueHandle.h"
#include <KIARA/Compiler/IR.hpp>
#include <KIARA/Compiler/LLVM/CompilerUnit.hpp>
#include <KIARA/Compiler/Preprocessor.hpp>
#include <KIARA/Core/Exception.hpp>
#include <deque>

namespace KIARA
{

namespace Compiler
{

class KIARA_COMPILER_API CodeGenException : public Exception
{
public:

    explicit CodeGenException(const std::string &arg);

};

//===----------------------------------------------------------------------===//
// Code Generation
//===----------------------------------------------------------------------===//

class KIARA_COMPILER_API CodeGen : public IR::ExprVisitor<CodeGen, llvm::Value*>
{
public:

    typedef ExprVisitor<CodeGen, llvm::Value*> InheritedType;
    using InheritedType::visit;

    CodeGen(llvm::LLVMContext &context, World &world);

    CodeGen(CompilerUnit* compilerUnit, World &world);

    ~CodeGen();

    llvm::LLVMContext & getContext() const { return context_; }

    llvm::Function * createFunction(const std::string &name, IR::Prototype &proto);

    llvm::Type * declareType(const Type::Ptr &type);

    llvm::Value *visit(IR::MemRef &expr);
    llvm::Value *visit(IR::PrimLiteral &expr);
    llvm::Value *visit(IR::SymbolExpr &expr);
    llvm::Value *visit(IR::DefExpr &expr);
    llvm::Value *visit(IR::CallExpr &expr);
    llvm::Value *visit(IR::IfExpr &expr);
    llvm::Value *visit(IR::LoopExpr &expr);
    llvm::Value *visit(IR::ForExpr &expr);
    llvm::Value *visit(IR::LetExpr &expr);
    llvm::Value *visit(IR::BlockExpr &expr);
    llvm::Value *visit(IR::BreakExpr &expr);
    llvm::Value *visit(IR::Prototype &proto);
    llvm::Value *visit(IR::Function &func);
    llvm::Value *visit(IR::ExternFunction &func);
    llvm::Value *visit(IR::Intrinsic &func);
    llvm::Value *visit(IR::FunctionDeclaration &decl);
#if 0
    void insertTrackedTypes(NamedTypeMap & trackedTypes);
#endif
    void createArgumentAllocas(IR::Prototype &proto, llvm::Function *F);

    llvm::Value * getString(const std::string &str, const llvm::Twine &name = "");

    llvm::Value * getStringPtr(
            llvm::IRBuilder<> &builder,
            const std::string &str,
            const llvm::Twine &name = "");

    llvm::Value * getStringPtr(
            const std::string &str,
            const llvm::Twine &name = "",
            llvm::Instruction *insertBefore = 0);

    void clearStringCache();

    static llvm::Type * getLLVMTypeCtx(KIARA::PrimTypeKind primTypeKind, llvm::LLVMContext &ctx);

    llvm::Type * getLLVMType(KIARA::PrimTypeKind primTypeKind) const
    {
        return getLLVMTypeCtx(primTypeKind, getContext());
    }

    enum TypeMode
    {
        VALUE_TYPE,     // variables and function parameters
        ARGUMENT_TYPE,  // passed arguments
        RETURN_TYPE     // returned values
    };

    llvm::Type * getLLVMType(const KIARA::Type::Ptr & type, TypeMode typeMode)
    {
        if (type)
            return getLLVMType(*type, typeMode);
        return 0;
    }

    llvm::Type * getLLVMType(const KIARA::Type & type, TypeMode typeMode);

    static llvm::Value * getDefaultValue(llvm::Type *type);

    llvm::Value * getUndefValue(llvm::Type *type);

    llvm::Type * getInt8Ty() const
    {
        return llvm::Type::getInt8Ty(getContext());
    }

    llvm::Type * getInt32Ty() const
    {
        return llvm::Type::getInt32Ty(getContext());
    }

    llvm::PointerType * getCharPtrTy() const
    {
        return llvm::PointerType::get(getInt8Ty(), 0);
    }

    llvm::PointerType * getCharArrayPtrTy(size_t numElements)
    {
        return llvm::PointerType::getUnqual(llvm::ArrayType::get(getInt8Ty(), numElements));
    }

    llvm::PointerType * getVoidPtrTy() const
    {
        return getCharPtrTy();
    }

    llvm::ConstantInt * getInt32(uint64_t value)
    {
        return getInt32Ctx(value, getContext());
    }

    static llvm::ConstantInt * getInt32Ctx(uint64_t value, llvm::LLVMContext &ctx)
    {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), value);
    }

    llvm::ConstantInt * getSignedInt32(int64_t value)
    {
        return getSignedInt32Ctx(value, getContext());
    }

    static llvm::ConstantInt * getSignedInt32Ctx(int64_t value, llvm::LLVMContext &ctx)
    {
        return llvm::ConstantInt::getSigned(llvm::Type::getInt32Ty(ctx), value);
    }

    llvm::Value * getEmptyStructValue() const
    {
        return llvm::UndefValue::get(llvm::StructType::get(getContext()));
    }

    llvm::Type * getUnitType() const
    {
        return llvm::StructType::get(getContext());
    }

    llvm::Value * getUnitValue() const
    {
        return llvm::UndefValue::get(getUnitType());
    }

    bool isUnitValue(llvm::Value *value) const
    {
        return value && (value->getType() == getUnitType() && llvm::isa<llvm::UndefValue>(value));
    }

    llvm::Type * voidToUnitType(llvm::Type *ty)
    {
        return ty->isVoidTy() ? getUnitType() : ty;
    }

    llvm::Type * unitToVoidType(llvm::Type *ty)
    {
        return ty == getUnitType() ? llvm::Type::getVoidTy(getContext()) : ty;
    }

    bool hasPredecessors(llvm::BasicBlock *block)
    {
        llvm::pred_iterator PI = llvm::pred_begin(block), E = llvm::pred_end(block);
        return (PI != E);
    }

    bool isTypesEqual(llvm::Type *a, llvm::Type *b);

    void emitAssemblyString(const std::string &llvmAssembly);

    /** Returns true if CodeGen can generate code for the function */
    bool isSupported(const IR::FunctionDefinition::Ptr &funcDef) const;

    // Updates the internal type caches to refer to the latest version of types
    // this should be used, whenever a new module is linked into the CompilerUnit
    void migrateTypes();

private:
    typedef std::map<std::string, llvm::WeakVH> StringCacheMap;
    typedef std::map<Type::Ptr, llvm::Type*> LLVMTypeMap;
    typedef std::map<const IR::IRExpr *, llvm::WeakVH> LLVMValueMap;

    struct BlockInfo
    {
        IR::IRExpr::Ptr expr;
        llvm::BasicBlock *nextBasicBlock;
        typedef std::vector<std::pair<llvm::Value*, llvm::BasicBlock *> > BreakSiteList;
        BreakSiteList breakSites;

        BlockInfo(const IR::IRExpr::Ptr &expr, llvm::BasicBlock *nextBasicBlock)
            : expr(expr), nextBasicBlock(nextBasicBlock), breakSites()
        { }
    };
    typedef std::deque<BlockInfo*> BlockDeque;

    World &world_;
    llvm::LLVMContext &context_;
    CompilerUnit *compilerUnit;
    StringCacheMap stringCache_;
    llvm::IRBuilder<> builder_;
    std::map<std::string, llvm::AllocaInst*> namedValues_;
    LLVMTypeMap llvmTypeMap_;
    LLVMValueMap llvmValueMap_;
    BlockDeque blockDeque_;
    std::string llvmDefs_;
    Preprocessor::VariableMap llvmPPVars_;

    // code generation context
    // following member influence code generation in visit methods
    // their value must be saved before modified
    bool genReference_; // visit should return reference (pointer to data) if possible

    void initLLVMDefs();
    void initContext();

    enum EmitMode
    {
        EMIT_VALUE,
        EMIT_REFERENCE
    };

    llvm::Value *emit(IR::DefExpr &expr, EmitMode emitMode);

    // uncached versions of visit
    llvm::Value *emit(IR::MemRef &expr);
    llvm::Value *emit(IR::PrimLiteral &expr);
    llvm::Value *emit(IR::SymbolExpr &expr);
    llvm::Value *emit(IR::DefExpr &expr);
    llvm::Value *emit(IR::CallExpr &expr);
    llvm::Value *emit(IR::IfExpr &expr);
    llvm::Value *emit(IR::LoopExpr &expr);
    llvm::Value *emit(IR::ForExpr &expr);
    llvm::Value *emit(IR::LetExpr &expr);
    llvm::Value *emit(IR::BlockExpr &expr);
    llvm::Value *emit(IR::BreakExpr &expr);
    llvm::Value *emit(IR::Prototype &proto);
    llvm::Value *emit(IR::Function &func);
    llvm::Value *emit(IR::ExternFunction &func);
    llvm::Value *emit(IR::Intrinsic &func);
    llvm::Value *emit(IR::FunctionDeclaration &decl);

    // Note: Value cache will currently not work with nested functions

    llvm::Value * getEmittedValue(const IR::IRExpr &expr) const
    {
        return getEmittedValue(&expr);
    }

    llvm::Value * getEmittedValue(const IR::IRExpr *expr) const
    {
        LLVMValueMap::const_iterator it = llvmValueMap_.find(expr);
        if (it != llvmValueMap_.end())
            return it->second;
        return 0;
    }

    llvm::Value * addEmittedValue(const IR::IRExpr &expr, llvm::Value *value)
    {
        return addEmittedValue(&expr, value);
    }

    llvm::Value * addEmittedValue(const IR::IRExpr *expr, llvm::Value *value)
    {
        if (expr && value)
            llvmValueMap_[expr] = value;
        return value;
    }

    void clearValueCache()
    {
        llvmValueMap_.clear();
    }

    // create llvm::Value for passing an argument argValue to the parameter with the destType type.
    llvm::Value * createArgValue(const IR::IRExpr::Ptr &argValue, const Type::Ptr &destType);

    /// CreateEntryBlockAlloca - Create an alloca instruction in the entry block of
    /// the function.  This is used for mutable variables etc.
    static llvm::AllocaInst *createEntryBlockAlloca(
            llvm::Function *function, const std::string &varName, llvm::Type *type)
    {
        llvm::IRBuilder<> tmpB(&function->getEntryBlock(), function->getEntryBlock().begin());
        return tmpB.CreateAlloca(type, 0, varName.c_str());
    }

    llvm::Value * createCompareWithZero(llvm::Value *value, const llvm::Twine &name = "")
    {
        llvm::Type *ty = value->getType();
        if (ty == llvm::Type::getInt1Ty(getContext()))
            return value;
        else if (ty->isIntegerTy())
            return builder_.CreateICmpNE(value, llvm::ConstantInt::get(ty, 0), name);
        else if (ty->isFloatingPointTy())
            return builder_.CreateFCmpUNE(value, llvm::ConstantFP::get(ty, 0.0), name);
        else
            return 0;
    }

    static llvm::Value * createOne(llvm::Type *type)
    {
        if (type->isFloatingPointTy())
            return llvm::ConstantFP::get(type, 1.0);
        else if (type->isIntegerTy())
            return llvm::ConstantInt::get(type, 1);
        else
            return 0;
    }

    llvm::Value * createAdd(llvm::Value *arg1, llvm::Value *arg2, bool signedInts = false,
                            const llvm::Twine &name = "")
    {
        llvm::Type *ty = arg1->getType();
        if (ty != arg2->getType())
            return 0;
        else if (ty->isIntegerTy())
            return builder_.CreateAdd(arg1, arg2, name, false, signedInts);
        else if (ty->isFloatingPointTy())
            return builder_.CreateFAdd(arg1, arg2, name);
        else
            return 0;
    }

};

} // namespace Compiler

} // namespace KIARA

#endif /* KIARA_LANG_CODEGEN_HPP_INCLUDED */
