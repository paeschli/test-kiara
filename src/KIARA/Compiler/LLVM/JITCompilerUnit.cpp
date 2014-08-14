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
 * JITCompilerUnit.cpp
 *
 *  Created on: Aug 9, 2013
 *      Author: Simon Moll
 */


#define KIARA_COMPILER_LIB
// ssize_t is defined by LLVM's DataTypes.h
#define _SSIZE_T_DEFINED

#include "JITCompilerUnit.hpp"

#include "llvm/Config/llvm-config.h"
#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#else
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#endif
#include "llvm/PassManager.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/Analysis/Passes.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/ObjectCache.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Support/CallSite.h"

// #include <KIARA/LLVM/JITMemoryManagers.hpp>
#include <KIARA/LLVM/Utils.hpp>
#include <KIARA/LLVM/KDirectiveProcessor.hpp>
#include "OptimizationConfig.hpp"

// #define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

namespace KIARA
{

namespace Compiler
{

// CompilerUnit

JITCompilerUnit::JITCompilerUnit(llvm::Module * initModule)
	: CompilerUnit()
	, jitModule(initModule)
	, jitEngine(0)
    , optimizer_()
    , trackedTypeMap_()
    , compiledFuncs_()
{
	assert(jitModule);

	optimizer_.setModule(jitModule);

	// Create the JIT.  This takes ownership of the module.
	llvm::EngineBuilder engineBuilder(jitModule);
	std::string errorMsg;

	engineBuilder.setUseMCJIT(false);
    engineBuilder.setOptLevel(JIT_OPTIMIZATION_LEVEL);
    engineBuilder.setErrorStr(&errorMsg);

	llvm::TargetOptions Options;
    Options.JITExceptionHandling = true;
	Options.JITEmitDebugInfo = true;
	Options.JITEmitDebugInfoToDisk = false;

	// Options.PositionIndependentExecutable = true;

	engineBuilder.setTargetOptions(Options);

	jitEngine = engineBuilder.create();
	if (!jitEngine) {
		DFC_THROW_EXCEPTION(Exception, "Could not create ExecutionEngine: "<<errorMsg);
	}

	// compile..
	jitEngine->DisableLazyCompilation();

	// initialize the module
	jitEngine->runStaticConstructorsDestructors(false);
}

JITCompilerUnit::~JITCompilerUnit()
{
    if (jitEngine)
    {
        jitEngine->runStaticConstructorsDestructors(true);
        jitEngine->removeModule(jitModule);
        delete jitEngine;
        jitEngine = 0;
    }
}

llvm::LLVMContext & JITCompilerUnit::getContext()
{
	return jitModule->getContext();
}

llvm::Module * JITCompilerUnit::getActiveModule()
{
	return jitModule;
}

bool JITCompilerUnit::isFunctionGenerated(const std::string & funcName)
{
	return jitEngine->getPointerToGlobalIfAvailable(jitModule->getFunction(funcName));
}

bool JITCompilerUnit::replaceFunction(const std::string &oldName, const std::string &newName)
{
    DFC_DEBUG("replaceFunction: "<<oldName<<" -> "<<newName);

    return llvmReplaceFunctionCalls(jitModule, oldName, newName);
}

void* JITCompilerUnit::requestPointerToFunction(const std::string & funcName)
{
    llvm::Function *func = jitModule->getFunction(funcName);
    if (!func)
        return 0;
    void *addr = jitEngine->getPointerToFunction(func);

    if (addr)
        registerCompiledFunction(func, addr);

	return addr;
}

void* JITCompilerUnit::requestPointerToFunction(llvm::Function * func)
{
    void *addr = jitEngine->getPointerToFunction(func);
    if (addr)
        registerCompiledFunction(func, addr);

	return addr;
}

void* JITCompilerUnit::requestPointerToGlobal(const std::string & gvName)
{
	return jitEngine->getPointerToGlobal(jitModule->getGlobalVariable(gvName, true));
}

void* JITCompilerUnit::requestPointerToGlobal(llvm::GlobalVariable * gv)
{
	return jitEngine->getPointerToGlobal(gv);
}

bool JITCompilerUnit::registerExternalFunction(const std::string & funcName, void * funcPtr)
{
    llvm::Function *funcDecl = jitModule->getFunction(funcName);
    if (!funcDecl)
        return false;
    jitEngine->updateGlobalMapping(funcDecl, funcPtr);
    return true;
}

bool JITCompilerUnit::registerExternalGlobal(const std::string & globalName, void * globalPtr)
{
    llvm::GlobalVariable *varDecl = jitModule->getGlobalVariable(globalName);
    if (!varDecl)
        return false;
    jitEngine->updateGlobalMapping(varDecl, globalPtr);
    return true;
}

void JITCompilerUnit::registerTrackedType(const std::string & typeName, llvm::StructType * type)
{
    assert(&type->getContext() == &jitModule->getContext());
    assert(type != 0);

    DFC_DEBUG("ADD TYPE " << typeName << " RESULT " << llvmToString(type) << " PTR " << type);
    assert(!type->isLiteral());

    trackedTypeMap_[typeName] = type;
}

bool JITCompilerUnit::optimizeActiveModule()
{
    return optimizer_.optimizeModule();
}

bool JITCompilerUnit::optimizeFunction(llvm::Function * f)
{
	return optimizer_.optimizeFunction(f);
}

llvm::Function * JITCompilerUnit::getFunction(const std::string & funcName)
{
	return jitModule->getFunction(funcName);
}

llvm::Function * JITCompilerUnit::requestCallee(const std::string & funcName)
{
	return jitModule->getFunction(funcName);
}

llvm::StructType * JITCompilerUnit::declareType(const std::string & typeName)
{
    llvm::StructType * declType = llvm::StructType::create(jitModule->getContext(), typeName);
    registerTrackedType(typeName, declType);
    return declType;
}

llvm::StructType * JITCompilerUnit::getTypeByName(const std::string & typeName)
{
    // check if this is a tracked type
    NamedTypeMap::const_iterator it = trackedTypeMap_.find(typeName);
    if (it != trackedTypeMap_.end())
    {
#if 0
        if (!it->second->hasName())
        {
            it->second->setName(typeName + ".X");
        }
#endif
        assert(it->second->hasName());

        DFC_DEBUG("REQUEST " << typeName << " RESULT " << llvmToString(it->second) << " PTR " << it->second);

        return it->second;
    }

    llvm::StructType *sty = jitModule->getTypeByName(typeName);

    if (sty)
    {
        assert(sty->hasName());
        DFC_DEBUG("REQUEST " << typeName << " RESULT " << llvmToString(sty) << " PTR " << sty);
    }
    else
    {
        DFC_DEBUG("REQUEST " << typeName << " RESULT NULL");
    }

    return sty;
}

void JITCompilerUnit::addModule(llvm::Module * module)
{
	std::string errorMsg;
	if (!llvmLinkModules(jitModule, module, errorMsg))
	{
	    llvm::errs() << "[JITCompilerUnit] linker failed with error " << errorMsg << "\n";
		abort();
	}

    // process directives embedded in the LLVM module
    {
        KDirectiveProcessor kdp(*jitModule);

        kdp.applyCommands(*jitModule);

        for (KDirectiveProcessor::TypeMap::const_iterator it = kdp.getTypeMap().begin(),
            end = kdp.getTypeMap().end(); it != end; ++it)
        {
            if (llvm::isa<llvm::StructType>(it->second))
                registerTrackedType(it->first, llvm::cast<llvm::StructType>(it->second));
        }
        DFC_IFDEBUG(kdp.dump());
    }
}

llvm::Function * JITCompilerUnit::createFunction(llvm::FunctionType * type, int rawLinkage, std::string name)
{
	llvm::GlobalValue::LinkageTypes linkage = (llvm::GlobalValue::LinkageTypes) rawLinkage;
	return llvm::Function::Create(type, linkage, name, getActiveModule());
}

void JITCompilerUnit::findAllUsedFunctions(
    llvm::Function *func,
    std::set<llvm::Function*> &funcSet,
    llvm::SmallVectorImpl<llvm::AssertingVH<llvm::Function> > &funcList)
{
    // check if we already visited func
    if (funcSet.find(func) != funcSet.end())
        return;

    funcSet.insert(func);
    funcList.push_back(func);

    for (llvm::Function::iterator b = func->begin(), be = func->end(); b != be; ++b)
    {
        for (llvm::BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; ++i)
        {
            llvm::CallSite cs = llvm::CallSite(i);
            if (cs && cs.getCalledFunction())
            {
                findAllUsedFunctions(cs.getCalledFunction(), funcSet, funcList);
            }
        }
    }
}

void JITCompilerUnit::registerCompiledFunction(llvm::Function *func, void *addr)
{
    std::set<llvm::Function*> funcSet;
    llvm::SmallVector<llvm::AssertingVH<llvm::Function>, 2> &funcList = compiledFuncs_[addr];
    findAllUsedFunctions(func, funcSet, funcList);

    // fix all non-external functions so they are never deleted when inlined
    for (llvm::SmallVector<llvm::AssertingVH<llvm::Function>, 2>::iterator it = funcList.begin(),
        end = funcList.end(); it != end; ++it)
    {
        llvm::Function *func = (*it);
        if (func->hasLocalLinkage())
        {
            llvm::StringRef funcName = func->getName();
            llvm::SmallString<256> uniqueName(funcName.begin(), funcName.end());
            llvm::raw_svector_ostream(uniqueName) << '_' << (void*)func;
            func->setName(uniqueName.str());
            func->setLinkage(llvm::GlobalValue::ExternalLinkage);
        }
    }
}

} // namespace Compiler

} // namespace KIARA
