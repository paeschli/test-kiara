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
 * MCJITCompilerUnit.cpp
 *
 *  Created on: Aug 9, 2013
 *      Author: Simon Moll, Dmitri Rubinstein
 */

#define KIARA_COMPILER_LIB
// ssize_t is defined by LLVM's DataTypes.h
#define _SSIZE_T_DEFINED

#include "MCJITCompilerUnit.hpp"

#include "llvm/Config/llvm-config.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"

#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/Passes.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/ObjectCache.h"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/DerivedTypes.h"
#include <llvm/Support/Host.h>

#include <KIARA/LLVM/JITMemoryManagers.hpp>
#include <KIARA/LLVM/KDirectiveProcessor.hpp>
#include "OptimizationConfig.hpp"

// #define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

typedef unsigned int uint;

namespace KIARA
{
namespace Compiler
{


llvm::StructType * MCJITCompilerUnit::JitStage::getTypeByName(const std::string & typeName) const
{
	if (isFinalized()) {
		NamedTypeMap::const_iterator itType = typeNames.find(typeName);
		if (itType == typeNames.end())
			return 0;
		else
			return itType->second;

	} else {
		return module->getTypeByName(typeName);
	}
}


// MCJITCompilerUnit::JitStage

void MCJITCompilerUnit::JitStage::updateGlobalNames()
{
	SymbolMangler mangler;

	/// Mangle symbol names
	for (llvm::Module::global_iterator itGlobal = module->global_begin(); itGlobal != module->global_end(); ++itGlobal)
	{
		std::string name = itGlobal->getName().str();
		if (!name.empty()) {
			std::string mangledName = mangler.getNameWithPrefix(name);
			itGlobal->setName(mangledName);
		}
	}

	for (llvm::Module::iterator itFun = module->begin(); itFun != module->end(); ++itFun)
	{
		std::string name = itFun->getName().str();
		if (!name.empty()) {
			std::string mangledName = mangler.getNameWithPrefix(name);
			itFun->setName(mangledName);
			DFC_DEBUG("Renamed " << name << " to " << mangledName );
			assert(itFun->getName() == mangledName && "could not mangle name");
		}
	}

	DFC_DEBUG("Stage after global value update:");
	//DFC_IFDEBUG( dump() );
}

MCJITCompilerUnit::JitStage::JitStage(llvm::Module * stageModule, CompilerUnit & cu)
	: module(stageModule)
	, engine(0)
	, manager(new StagingMemoryManager(cu))
{
#ifdef _WIN32
    // this is required currently on Windows for MCJIT
    // since COFF format is only supported for emitting.
    if (!llvm::StringRef(module->getTargetTriple()).endswith("-elf"))
        module->setTargetTriple(module->getTargetTriple()+"-elf");
#endif
	updateGlobalNames();
}


MCJITCompilerUnit::JitStage::~JitStage()
{}

void MCJITCompilerUnit::JitStage::terminate()
{
	if (engine) {
		engine->runStaticConstructorsDestructors(true);
		engine->removeModule(module);
		delete engine;
		engine = 0;
	} else {
		delete manager;
		manager = 0;
	}

	delete module;
	module = 0;
}

void MCJITCompilerUnit::JitStage::dump() const
{
	llvm::errs() << "[JitStage, finalized=" << (isFinalized() ? "1":"0") << "]\n";
	module->dump();
	llvm::errs() << "[End-of-JitStage]\n";
}

bool MCJITCompilerUnit::JitStage::isFinalized() const { return engine != 0; }
llvm::Module & MCJITCompilerUnit::JitStage::getModule() const { return *module; }
llvm::ExecutionEngine & MCJITCompilerUnit::JitStage::getEngine() const { return *engine; }


void MCJITCompilerUnit::JitStage::initGlobalGetters()
{
	typedef llvm::Module::GlobalListType GlobalList;

	for (llvm::Module::global_iterator itGlobal = module->global_begin(); itGlobal != module->global_end(); ++itGlobal) {
		llvm::GlobalVariable & global = *itGlobal;
		std::string globalName = global.getName().str();

		llvm::PointerType * globalPtrType = global.getType();

		llvm::FunctionType * getterType = llvm::FunctionType::get(globalPtrType, llvm::ArrayRef<llvm::Type*>(), false);

		// void getGlobalRaw() { return <global>; }
		llvm::Function * getterFunc =
				llvm::Function::Create(getterType, llvm::GlobalValue::InternalLinkage, "getGlobalRaw_"+globalName, module);
		llvm::BasicBlock * entryBlock =
				llvm::BasicBlock::Create(module->getContext(), "entry", getterFunc, 0);
		llvm::ReturnInst::Create(module->getContext(), &global, entryBlock);

		// register function
		globalGetters[globalName] = getterFunc;
	}
}

typedef void* (GlobalGetterFunc)();

void * MCJITCompilerUnit::JitStage::getPointerToGlobal(const std::string & globalName)
{
	assert(isFinalized());
	FunctionIndex::const_iterator itGetter = globalGetters.find(globalName);
	if (itGetter == globalGetters.end()) {
		return 0;
	}

	llvm::Function * getterFunc = itGetter->second;
	assert(getterFunc);
	GlobalGetterFunc * getGlobalRaw = reinterpret_cast<GlobalGetterFunc*>(engine->getPointerToFunction(getterFunc));

	assert(getGlobalRaw);
	return getGlobalRaw();
}

void MCJITCompilerUnit::registerTrackedType(const std::string & typeName, llvm::StructType * type)
{
	trackedTypeMap[typeName] = type;
}

// will compile the module
void MCJITCompilerUnit::JitStage::finalize(NamedTypeMap & trackedTypes)
{
	if (isFinalized())
		return;

	// create getters for global variable
	initGlobalGetters();

	DFC_DEBUG("Finalizing stage module:" );
	//DFC_IFDEBUG(module->dump() );
	DFC_DEBUG("[End-Of-Module]" );

	// Move tracked types out of the way (and preserve the original mapping in the typeName map)
	for (NamedTypeMap::iterator itPair = trackedTypes.begin(); itPair != trackedTypes.end(); ++itPair) {
		std::string typeName = itPair->first;
		// llvm::Type * currentLLVMType = itPair->second;

		llvm::StructType * conflictType = module->getTypeByName(typeName);
		if (conflictType) {
			typeNames[typeName] = conflictType;
			conflictType->setName(typeName + ".s");
		}
	}

// Create the JIT.  This takes ownership of the module.
	llvm::EngineBuilder engineBuilder(module);
	bool useMCJIT = true;
	std::string errorMsg;

    // Configure for the host target
    std::string tripleText = llvm::sys::getProcessTriple(); //llvm::sys::getDefaultTargetTriple();
	llvm::Triple triple(tripleText);
#ifdef _WIN32
	triple.setEnvironment(llvm::Triple::ELF); // this must be set for windows
#endif
	std::string cpuName = llvm::sys::getHostCPUName();

	DFC_DEBUG("[MCJIT] target \"" << triple.getTriple() << "\" on \"" << cpuName << "\"");

	// translate host CPU features
	typedef llvm::StringMap<bool>::const_iterator StringIter;
	llvm::StringMap<bool> cpuFeatures;
	llvm::sys::getHostCPUFeatures(cpuFeatures);
	llvm::SmallVector<std::string, 8> featureFlags;
	for (StringIter it = cpuFeatures.begin(); it != cpuFeatures.end(); ++it) {
		std::string featureName = it->first();
		std::string flag = "";

		DFC_DEBUG("[MCJIT] has " << featureName << " " << it->second);
		if (! it->second) {
			flag = "-";
		}
		flag += featureName;
	}

	//engineBuilder.selectTarget(triple, "", cpuName, featureFlags); // FIXME why this was needed at all ?

// enable MCJIT
	engineBuilder.setUseMCJIT(useMCJIT);
	if (useMCJIT)
	{
		engineBuilder.setJITMemoryManager(manager);
	}

// set optimization options

	engineBuilder.setOptLevel(JIT_OPTIMIZATION_LEVEL);
	engineBuilder.setErrorStr(&errorMsg);

	llvm::TargetOptions Options;
	Options.JITExceptionHandling = true;
	Options.JITEmitDebugInfo = true;
	Options.JITEmitDebugInfoToDisk = false;

	// Options.PositionIndependentExecutable = true;

	engineBuilder.setTargetOptions(Options);

	engine = engineBuilder.create();
	if (!engine) {
		DFC_THROW_EXCEPTION(Exception, "Could not create ExecutionEngine: "<<errorMsg);
	}

	// Following code is for testing of JIT events
	//engine->RegisterJITEventListener(
	//    MyJITEventListener::create());

	// compile..
	engine->DisableLazyCompilation(true);
	engine->finalizeObject();

	// initialize the module
	engine->runStaticConstructorsDestructors(false);
}




// CompilerUnit

MCJITCompilerUnit::JitStage & MCJITCompilerUnit::getTop() {
	assert(!jitStages.empty());
	return jitStages.back();
}

llvm::JITMemoryManager * MCJITCompilerUnit::JitStage::getManager() const { return manager; }
llvm::ExecutionEngine & MCJITCompilerUnit::getTopEngine() { return getTop().getEngine(); }
llvm::Module & MCJITCompilerUnit::getTopModule() { return getTop().getModule(); }

void MCJITCompilerUnit::dump() const
{
	JitStageVec::const_iterator itStart = jitStages.begin();
	JitStageVec::const_iterator itEnd = jitStages.end();
	JitStageVec::const_iterator itStage;

	DFC_DEBUG("[MCJitCompilerUnit]" );
	for (itStage = itStart; itStage != itEnd; ++itStage) {
		itStage->dump();
	}
	DFC_DEBUG( "[End-Of-MCJitCompilerUnit]" );
}


const MCJITCompilerUnit::JitStage * MCJITCompilerUnit::findStageForGlobal(const std::string & gvName, bool AllowInternal, bool allowDeclarations) const
{
	JitStageVec::const_iterator itStart = jitStages.begin();
	JitStageVec::const_iterator itEnd = jitStages.end();
	JitStageVec::const_iterator itStage;

	const MCJITCompilerUnit::JitStage * declaringStage = 0;

	for (itStage = itStart; itStage != itEnd; ++itStage) {
		const JitStage & stage = *itStage;
		llvm::GlobalVariable * gv = stage.getModule().getGlobalVariable(gvName, AllowInternal);

		if (gv) {
			if (allowDeclarations || !gv->isDeclaration()) {
				return &stage;
			} else {
				declaringStage = &stage;
			}
		}
	}

	return declaringStage;
}

const MCJITCompilerUnit::JitStage * MCJITCompilerUnit::findStageForType(const std::string & typeName) const
{
	JitStageVec::const_iterator itStart = jitStages.begin();
	JitStageVec::const_iterator itEnd = jitStages.end();
	JitStageVec::const_iterator itStage;

	const MCJITCompilerUnit::JitStage * declaringStage = 0;

	for (itStage = itStart; itStage != itEnd; ++itStage) {
		const JitStage & stage = *itStage;
		llvm::Type * stageType = stage.getTypeByName(typeName);
		if (stageType) {
			declaringStage = &stage;
		}
	}

	return declaringStage;
}

MCJITCompilerUnit::JitStage *
MCJITCompilerUnit::findStageForFunction(const std::string & fncName, llvm::Function *& oFunc, bool allowDeclarations)
{
	JitStageVec::iterator itStart = jitStages.begin();
	JitStageVec::iterator itEnd = jitStages.end();
	JitStageVec::iterator itStage;

	MCJITCompilerUnit::JitStage * declaringStage = 0;

	for (itStage = itStart; itStage != itEnd; ++itStage) {
		JitStage & stage = *itStage;
		llvm::Function * func = stage.getModule().getFunction(fncName);

		if (func) {
			// always take a definition
			if (!func->isDeclaration()) {
				oFunc = func;
				return &stage;
			} else if (allowDeclarations && func->isDeclaration()) {
				declaringStage = &stage;
				oFunc = func;
				return &stage;
			}
		}
	}

	if (declaringStage && allowDeclarations) {
		return declaringStage;
	} else {
		return 0;
	}
}

void MCJITCompilerUnit::pushStage(llvm::Module * initModule)
{
    if (!jitStages.empty())
    {
        if (!getTop().isFinalized())
            getTop().finalize(trackedTypeMap);
    }

    jitStages.push_back(JitStage(initModule, *this));
    optimizer_.setModule(initModule);
}

void MCJITCompilerUnit::requestUnfinishedTop()
{
    if (getTop().isFinalized())
    {
        llvm::Module *M = new llvm::Module("kiara-jit", context);
        M->setTargetTriple(hostTriple);
        pushStage(M);
    }
}

MCJITCompilerUnit::MCJITCompilerUnit(llvm::Module * activeModule)
	: CompilerUnit()
    , trackedTypeMap()
    , topTypeMap()
    , mangler()
	, jitStages()
	, symbolMan(new ExternalSymbolManager())
	, context(activeModule->getContext())
	, hostTriple(activeModule->getTargetTriple())
    , optimizer_()
{
	assert(activeModule);

	jitStages.push_back(JitStage(activeModule, *this));
	optimizer_.setModule(activeModule);

	DFC_DEBUG( "[CU] CompilerUnit created!" );
}

MCJITCompilerUnit::~MCJITCompilerUnit()
{
	for (JitStageVec::iterator itStage = jitStages.begin(); itStage != jitStages.end(); ++itStage)
		itStage->terminate();

	DFC_DEBUG( "[CU] CompilerUnit terminated!" );
	delete symbolMan;
}

llvm::LLVMContext & MCJITCompilerUnit::getContext()
{
	return context;
}

#if 0
void MCJITCompilerUnit::setModule(llvm::Module * newModule)
{
	assert(&newModule->getContext() == &context);
	jitStages.push_back(JitStage(newModule));
	optimizer_.setModule(newModule);
}

llvm::ExecutionEngine & MCJITCompilerUnit::getActiveEngine()
{
	getTop().finalize(trackedTypeMap);
	return getTopEngine();
}
#endif


void MCJITCompilerUnit::event_activeModuleModified()
{
	getTop().updateGlobalNames();
}

llvm::Module * MCJITCompilerUnit::getActiveModule()
{
    assert(!jitStages.empty());
	requestUnfinishedTop();
	return &getTopModule();
}

bool MCJITCompilerUnit::isFunctionGenerated(const std::string & funcName)
{
	llvm::Function * func = 0;
	const JitStage * stage = findStageForFunction(funcName, func, false);
	if (!stage || !stage->isFinalized())
		return false;

	assert(func);
	return stage->getEngine().getPointerToGlobalIfAvailable(func);
}

void* MCJITCompilerUnit::requestPointerToFunction(const std::string & funcName)
{
	// External scope
	if (void * externalFuncPtr = symbolMan->getPointerToNamedFunction(funcName, false)) {
		return externalFuncPtr;
	}

	// JIT-scope
	llvm::Function * func = 0;
	JitStage * stage = findStageForFunction(funcName, func, false);
	if (!stage) {
		return 0;
	}

	return requestPointerToFunction(stage, func);
}

void *
MCJITCompilerUnit::requestPointerToFunction(JitStage * stage, llvm::Function * func)
{
	// compile as necessary
	if (!stage->isFinalized())
	    stage->finalize(trackedTypeMap);

	assert(&stage->getModule() == func->getParent());

	return stage->getEngine().getPointerToFunction(func);
}

void *
MCJITCompilerUnit::requestPointerToFunction(llvm::Function * func)
{
	assert(func);
	std::string funcName = func->getName();
	DFC_DEBUG( "requestPointerToFunction(" << funcName << ")" );

	// External scope
	if (void * externalFuncPtr = symbolMan->getPointerToNamedFunction(funcName, false)) {
		return externalFuncPtr;
	}


	DFC_DEBUG( "[CU] pointer to function: " << funcName );
	llvm::Function * unusedFunc = 0;
	JitStage * stage = const_cast<JitStage*>(findStageForFunction(funcName, unusedFunc, false));

	if (!stage) {
		// a global symbol?
		return symbolMan->getPointerToNamedFunction(funcName, false);
	}

	return requestPointerToFunction(stage, func);

}

void* MCJITCompilerUnit::requestPointerToGlobal(const std::string & gvName)
{
	// External scope
	if (void * externalGlobalPtr = symbolMan->getPointerToNamedGlobal(gvName, false)) {
		return externalGlobalPtr;
	}

	// JIT-scope
	const JitStage * stage = findStageForGlobal(gvName);

	if (!stage)
		return 0;

	llvm::GlobalVariable *  gv = stage->getModule().getGlobalVariable(gvName, true);
	return requestPointerToGlobal(gv); // TODO: sloppy reuse
}

void* MCJITCompilerUnit::requestPointerToGlobal(llvm::GlobalVariable * gv)
{
	assert(gv);
	std::string gvName = gv->getName();
	DFC_DEBUG( "[CU] pointer to gv: " << gvName );

	// External scope
	if (void * externalGlobalPtr = symbolMan->getPointerToNamedGlobal(gvName, false)) {
		return externalGlobalPtr;
	}

	// JIT-scope
	JitStage * stage = const_cast<JitStage*>(findStageForGlobal(gvName));

	if (!stage) {
		return symbolMan->getPointerToNamedGlobal(gvName, false);
	}

	if (!stage->isFinalized())
	    stage->finalize(trackedTypeMap);

	assert(&stage->getModule() == gv->getParent());

	return stage->getPointerToGlobal(gvName);
}

bool MCJITCompilerUnit::replaceFunction(const std::string &oldName, const std::string &newName)
{
    DFC_DEBUG("replaceFunction: "<<oldName<<" -> "<<newName);

    if (oldName.empty() || newName.empty())
        return false;

    return llvmReplaceFunctionCalls(getActiveModule(),
        mangler.getNameWithPrefix(oldName), mangler.getNameWithPrefix(newName));
}

bool MCJITCompilerUnit::registerExternalFunction(const std::string & funcName, void * funcPtr)
{
	symbolMan->registerFunctionSymbol(mangler.getNameWithPrefix(funcName), funcPtr);
	return true; // TODO: detect conflicts
}

bool MCJITCompilerUnit::registerExternalGlobal(const std::string & globalName, void * globalPtr)
{
	symbolMan->registerGlobalSymbol(mangler.getNameWithPrefix(globalName), globalPtr);
	return true; // TODO: detect conflicts
}

bool MCJITCompilerUnit::optimizeActiveModule()
{
    if (getTop().isFinalized())
        return false;

    assert(&getTop().getModule() == optimizer_.getModule());

    return optimizer_.optimizeModule();
}

bool MCJITCompilerUnit::optimizeFunction(llvm::Function * f)
{
    if (getTop().isFinalized() || &getTop().getModule() != f->getParent())
        return false;

    return optimizer_.optimizeFunction(f);
}

llvm::Function * MCJITCompilerUnit::getFunction(const std::string & rawFuncName)
{
	std::string funcName = mangler.getNameWithPrefix(rawFuncName);
	llvm::Function * func = 0;
	findStageForFunction(funcName, func, true);
	return func;
}

llvm::FunctionType * MCJITCompilerUnit::migrateFunctionToTop(llvm::FunctionType* oldFuncType)
{
	llvm::Type * retType = migrateToTop(oldFuncType->getReturnType());
	std::vector<llvm::Type*> paramVec;

	for(uint i = 0; i <  oldFuncType->getNumParams(); ++i) {
		paramVec.push_back(migrateToTop(oldFuncType->getParamType(i)));
	}

	return llvm::FunctionType::get(retType, paramVec, oldFuncType->isVarArg());
}

llvm::Type * MCJITCompilerUnit::migrateToTop(llvm::Type* oldType)
{
	LLVMTypeMap::iterator it = topTypeMap.find(oldType);
	if (it == topTypeMap.end()) {
		return oldType;
	} else {
		return it->second;
	}
}

llvm::Function * MCJITCompilerUnit::requestCallee(const std::string & rawFuncName)
{
	std::string funcName = mangler.getNameWithPrefix(rawFuncName);
	// already in the top module?
	llvm::Function * topFunc = getTopModule().getFunction(funcName);
	if (topFunc) {
		return topFunc;
	}

	// otw, somewhere down in the stack?
	llvm::Function * origFunc = 0;
	const JitStage * stage = findStageForFunction(funcName, origFunc, true);
	assert (&getTop() != stage && "inconsistent");

	if (!stage) {
		DFC_DEBUG( "(!!) Preparing for error as " << rawFuncName << " could not be found!" );
		//DFC_IFDEBUG( dump() );
		return 0;
	}
	requestUnfinishedTop();

	// Migrate the function type to the top module
	llvm::FunctionType * migratedType = migrateFunctionToTop(origFunc->getFunctionType());

	llvm::Function * calleeDecl = llvm::Function::Create(migratedType,
										  llvm::Function::ExternalLinkage, // force external linkage to enable symbol resolution through external manager
										  funcName,
										  &getTopModule());

	return calleeDecl;
}

llvm::StructType * MCJITCompilerUnit::declareType(const std::string & typeName) {
	llvm::StructType * declType = llvm::StructType::create(context, typeName);
	registerTrackedType(typeName, declType);
	return declType;
}

llvm::StructType * MCJITCompilerUnit::getTypeByName(const std::string & typeName)
{
	// check if this is a tracked type
	NamedTypeMap::const_iterator itNamed = trackedTypeMap.find(typeName);
	if (itNamed != trackedTypeMap.end()) {
		return itNamed->second;
	}

	// otherwise look for it in the stacked modules
	const JitStage * declStage = findStageForType(typeName);

	if (declStage) {
		llvm::StructType * namedType = declStage->getTypeByName(typeName);
		llvm::Type * migratedType = migrateToTop(namedType);
		llvm::StructType * topType = llvm::cast<llvm::StructType>(migratedType);

		trackedTypeMap[typeName] = topType; // make sure this type will be tracked properly in the future
		return topType;
	}

	return 0;
}

void MCJITCompilerUnit::LinkTypes(llvm::Type * oldType, llvm::Type * newType, LLVMTypeMap * toNewTypeMap, LLVMTypeMap * toOldTypeMap)
{
	if (oldType != newType) {
		if (toOldTypeMap) (*toOldTypeMap)[newType] = oldType;
		if (toNewTypeMap) (*toNewTypeMap)[oldType] = newType;
	}
}

void MCJITCompilerUnit::LinkFunctions(llvm::Function * oldFunc, llvm::Function * newFunc, LLVMTypeMap * toNewTypeMap, LLVMTypeMap * toOldTypeMap)
{
	llvm::FunctionType * oldFuncType = oldFunc->getFunctionType();
	llvm::FunctionType * newFuncType = newFunc->getFunctionType();

	assert(oldFuncType->isVarArg() == newFuncType->isVarArg());
	assert(oldFuncType->getNumParams() == newFuncType->getNumParams());

	for(uint iParam = 0; iParam < oldFuncType->getNumParams(); ++iParam)
	{
		llvm::Type * oldParamType = oldFuncType->getParamType(iParam);
		llvm::Type * paramType = newFuncType->getParamType(iParam);

		LinkTypes(oldParamType, paramType, toNewTypeMap, toOldTypeMap);
	}

	LinkTypes(oldFuncType->getReturnType(), newFuncType->getReturnType(), toNewTypeMap, toOldTypeMap);
}

void MCJITCompilerUnit::LinkValues(llvm::Value * oldValue, llvm::Value * newValue, LLVMTypeMap * toNewTypeMap, LLVMTypeMap * toOldTypeMap)
{
	LinkTypes(oldValue->getType(), newValue->getType(), toNewTypeMap, toOldTypeMap);
}

llvm::Function * MCJITCompilerUnit::createFunction(llvm::FunctionType * type, int rawLinkage, std::string name)
{
	llvm::GlobalValue::LinkageTypes linkage = (llvm::GlobalValue::LinkageTypes) rawLinkage;
	std::string mangledName = mangler.getNameWithPrefix(name);
	return llvm::Function::Create(type, linkage, mangledName, getActiveModule());
}

void MCJITCompilerUnit::addModule(llvm::Module * module)
{
    getTop().finalize(trackedTypeMap); // FIXME: without this tests fail on x86 32-bit architecture

    const bool createNewStage = getTop().isFinalized();
    if (createNewStage)
    {
        // link types

        // Link global variable types
        for (llvm::Module::global_iterator itGlobal = module->global_begin(); itGlobal != module->global_end(); ++itGlobal)
        {
            llvm::GlobalVariable * gv = itGlobal;
            std::string globalName = gv->getName();
            const JitStage * declaringStage = findStageForGlobal(globalName, true , true);

            if (declaringStage) {
                llvm::GlobalVariable * oldGV = declaringStage->getModule().getNamedGlobal(globalName);
                LinkValues(oldGV, gv, &topTypeMap, 0);
            }
        }

        // Link function types
        for (llvm::Module::iterator itFun = module->begin(); itFun != module->end(); ++itFun)
        {
            llvm::Function * fun = itFun;
            llvm::Function * unusedFunc = 0;
            const JitStage * declaringStage = findStageForFunction(fun->getName(), unusedFunc, true);

            if (declaringStage) {
                llvm::Function * oldFun = declaringStage->getModule().getFunction(fun->getName());
                LinkFunctions(oldFun, fun, &topTypeMap, 0);
            }
        }

        // Link explicitly tracked types
        for (NamedTypeMap::iterator itPair = trackedTypeMap.begin(); itPair != trackedTypeMap.end(); ++itPair) {
            std::string typeName = itPair->first;
            llvm::StructType * oldType = itPair->second;
            llvm::StructType * currType = module->getTypeByName(typeName);

            if (currType) {
                LinkTypes(oldType, currType, &topTypeMap);
                trackedTypeMap[typeName] = currType;
            }
        }
    }
    else
    {
        std::string errorMsg;
        llvm::Module *activeModule = getActiveModule();
        if (!llvmLinkModules(activeModule, module, errorMsg))
        {
            llvm::errs() << "[MCJITCompilerUnit] linker failed with error " << errorMsg << "\n";
            abort();
        }
        module = activeModule;
    }

    // process directives embedded in the LLVM module
    {
        KDirectiveProcessor kdp(*module);

        kdp.applyCommands(*module);

        for (KDirectiveProcessor::TypeMap::const_iterator it = kdp.getTypeMap().begin(),
            end = kdp.getTypeMap().end(); it != end; ++it)
        {
            if (llvm::isa<llvm::StructType>(it->second))
                registerTrackedType(it->first, llvm::cast<llvm::StructType>(it->second));
        }
        DFC_IFDEBUG(kdp.dump());
    }

    if (createNewStage)
    {
        jitStages.push_back(JitStage(module, *this));
        optimizer_.setModule(module);
    }
}

} // namespace Compiler
} // namespace KIARA
