/*
 * Optimizer.cpp
 *
 *  Created on: Mar 25, 2014
 *      Author: Dmitri Rubinstein
 */
#define KIARA_COMPILER_LIB
// ssize_t is defined by LLVM's DataTypes.h
#define _SSIZE_T_DEFINED

#include "Optimizer.hpp"
#include "OptimizationConfig.hpp"

// LLVM
#include "llvm/Config/llvm-config.h"
#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
#include "llvm/IR/Module.h"
#else
#include "llvm/Module.h"
#endif
#include "llvm/PassManager.h"
#include "llvm/ADT/Triple.h"
#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
#include "llvm/IR/DataLayout.h"
#else
#include "llvm/Target/TargetData.h"
#endif
#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Vectorize.h"
#include "llvm/Analysis/Passes.h"

#include <KIARA/LLVM/Inlining.hpp>

namespace KIARA
{

namespace Compiler
{

Optimizer::Optimizer(llvm::Module *module)
    : fpm_(0)
    , mpm_(0)
    , module_(0)
{
    if (module)
        setModule(module);
}

Optimizer::~Optimizer()
{
    reset();
}

void Optimizer::reset()
{
    delete fpm_;
    fpm_ = 0;
    delete mpm_;
    mpm_ = 0;
    module_ = 0;
}

void Optimizer::setModule(llvm::Module *module)
{
    reset();
    module_ = module;
    if (!module)
        return;

    mpm_ = new llvm::PassManager;
    llvm::TargetLibraryInfo *TLI = new llvm::TargetLibraryInfo(llvm::Triple(module->getTargetTriple()));
    mpm_->add(TLI);
#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
    mpm_->add(new llvm::DataLayout(module));
#else
    mpm_->add(new llvm::TargetData(module));
#endif

#ifndef DISABLE_OPTIMIZATION
    {
        fpm_ = new llvm::FunctionPassManager(module);
        llvm::TargetLibraryInfo *TLI = new llvm::TargetLibraryInfo(llvm::Triple(module->getTargetTriple()));
        fpm_->add(TLI);
#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
        fpm_->add(new llvm::DataLayout(module));
#else
        fpm_->add(new llvm::TargetData(module));
#endif
    }
#endif

    llvm::PassManagerBuilder builder;
#ifdef DISABLE_OPTIMIZATION
    builder.OptLevel = 0;
    builder.Inliner = 0;
#else
    builder.OptLevel = 3;
    builder.Inliner = llvm::createAlwaysInlinerPass();
#endif
    if (fpm_)
    {
#ifndef CUSTOM_FUNCTION_OPTIMIZATION
        builder.populateFunctionPassManager(*fpm_);
#else
        fpm_->add(llvm::createTypeBasedAliasAnalysisPass());
        fpm_->add(llvm::createBasicAliasAnalysisPass());

        fpm_->add(llvm::createInstructionCombiningPass());
        fpm_->add(llvm::createCFGSimplificationPass());
        fpm_->add(llvm::createSROAPass());

        fpm_->add(llvm::createEarlyCSEPass());
        fpm_->add(llvm::createLowerExpectIntrinsicPass());

        fpm_->add(llvm::createSimplifyLibCallsPass());
        fpm_->add(llvm::createJumpThreadingPass());         // Thread jumps.
        fpm_->add(llvm::createCorrelatedValuePropagationPass()); // Propagate conditionals
        fpm_->add(llvm::createCFGSimplificationPass());     // Merge & remove BBs
        fpm_->add(llvm::createInstructionCombiningPass());  // Combine silly seq's

        fpm_->add(llvm::createTailCallEliminationPass());   // Eliminate tail calls
        fpm_->add(llvm::createCFGSimplificationPass());     // Merge & remove BBs
        fpm_->add(llvm::createReassociatePass());           // Reassociate expressions
        fpm_->add(llvm::createLoopRotatePass());            // Rotate Loop
        fpm_->add(llvm::createLICMPass());                  // Hoist loop invariants
        fpm_->add(llvm::createLoopUnswitchPass(/* optimize for size */false));
        fpm_->add(llvm::createInstructionCombiningPass());
        fpm_->add(llvm::createIndVarSimplifyPass());        // Canonicalize indvars
        fpm_->add(llvm::createLoopIdiomPass());             // Recognize idioms like memset.
        fpm_->add(llvm::createLoopDeletionPass());          // Delete dead loops

        fpm_->add(llvm::createLoopVectorizePass());

        fpm_->add(llvm::createLoopUnrollPass());          // Unroll small loops
        fpm_->add(llvm::createGVNPass());                 // Remove redundancies
        fpm_->add(llvm::createMemCpyOptPass());             // Remove memcpy / form memset
        fpm_->add(llvm::createSCCPPass());                  // Constant prop with SCCP

        // Run instcombine after redundancy elimination to exploit opportunities
        // opened up by them.
        fpm_->add(llvm::createInstructionCombiningPass());
        fpm_->add(llvm::createJumpThreadingPass());         // Thread jumps
        fpm_->add(llvm::createCorrelatedValuePropagationPass());
        fpm_->add(llvm::createDeadStoreEliminationPass());  // Delete dead stores

        // following passes seems to not optimize much
#if 0
        fpm_->add(llvm::createSLPVectorizerPass());     // Vectorize parallel scalar chains.

        fpm_->add(llvm::createBBVectorizePass());
        fpm_->add(llvm::createInstructionCombiningPass());
        fpm_->add(llvm::createGVNPass());                   // Remove redundancies

        // BBVectorize may have significantly shortened a loop body; unroll again.
        fpm_->add(llvm::createLoopUnrollPass());

        fpm_->add(llvm::createAggressiveDCEPass());         // Delete dead instructions
        fpm_->add(llvm::createCFGSimplificationPass());     // Merge & remove BBs
        fpm_->add(llvm::createInstructionCombiningPass());  // Clean up after everything.
#endif
#endif
    }
    builder.populateModulePassManager(*mpm_);
}

bool Optimizer::optimizeModule()
{
    if (mpm_ && module_)
        return mpm_->run(*module_);
    return false;
}

bool Optimizer::optimizeFunction(llvm::Function *func)
{
    bool functionChanged = false;
    if (func)
    {
        functionChanged |= llvmInlineAllFunctionCalls(func);
        if (fpm_)
        {
            fpm_->doInitialization();
            functionChanged |= fpm_->run(*func);
            fpm_->doFinalization();
        }
    }
    return functionChanged;
}

} // namespace Compiler

} // namespace KIARA
