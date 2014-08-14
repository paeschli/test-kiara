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
 * JITMemoryManagers.cpp
 *
 *  Created on: Jul 22, 2013
 *      Author: Simon Moll
 */

#include "JITMemoryManagers.hpp"
#include <KIARA/Compiler/LLVM/CompilerUnit.hpp>

#define _SSIZE_T_DEFINED
#include "llvm/Config/llvm-config.h"
#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#else
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#endif

#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/Passes.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/ObjectCache.h"
#include "llvm/ExecutionEngine/ObjectImage.h"

#include "llvm/Support/raw_ostream.h"


#if 0
#define DEBUG_MAN(X) X
#else
#define DEBUG_MAN(X) static_cast<void>(0)
#endif


#if 0
#define RAISE(EXCEPTION, STREAMOPS) \
		DFC_THROW_EXCEPTION(EXCEPTION, STREAMOPS);
#else
#include <csignal>
#define RAISE(EXCEPTION, STREAMOPS) { \
		llvm::errs() << STREAMOPS; \
		raise(SIGINT); } // causes a break in gdb
#endif


// Internal classes

// KIARA::Compiler::ExternalSymbolManager

KIARA::Compiler::ExternalSymbolManager::ExternalSymbolManager()
: llvm::SectionMemoryManager()
, mangler()
{}

KIARA::Compiler::ExternalSymbolManager::~ExternalSymbolManager() {}

void KIARA::Compiler::ExternalSymbolManager::dump() const
{
	llvm::errs() << "[External functions]\n";
	for (SymbolMap::const_iterator itSymbol = externFunctions.begin(); itSymbol != externFunctions.end(); ++itSymbol) {
		llvm::errs() << itSymbol->first << "\n";
	}
	llvm::errs() << "[External globals]\n";
	for (SymbolMap::const_iterator itSymbol = externGlobals.begin(); itSymbol != externGlobals.end(); ++itSymbol) {
		llvm::errs() << itSymbol->first << "\n";
	}
}

void KIARA::Compiler::ExternalSymbolManager::registerFunctionSymbol(const std::string &name, void* ptr)
{
	DEBUG_MAN( llvm::errs() << "[EMM] Registered external function " << name << "\n" );
	externFunctions[mangler.getNameWithPrefix(name)] = ptr;
}

void KIARA::Compiler::ExternalSymbolManager::registerGlobalSymbol(const std::string &name, void* ptr)
{
	DEBUG_MAN( llvm::errs() << "[EMM] Registered external global " << name << "\n" );
	externGlobals[mangler.getNameWithPrefix(name)] = ptr;
}

void* KIARA::Compiler::ExternalSymbolManager::getPointerToNamedGlobal(const std::string & Name, bool AbortOnFailure)
{
	SymbolMap::iterator itSymbol = externGlobals.find(mangler.getNameWithPrefix(Name));
	if (itSymbol != externGlobals.end()) {
		return itSymbol->second;
	} else {
		if (AbortOnFailure) {
			RAISE(Exception, "Could not find symbol: "<< Name);
		}

		DEBUG_MAN( llvm::errs() << "SharedSymbolManager: could not resolve symbol " << Name << "\n" );
		DEBUG_MAN( dump() );
		return 0;
	}
}

// MemoryManager-interface
void * KIARA::Compiler::ExternalSymbolManager::getPointerToNamedFunction(const std::string &Name, bool AbortOnFailure)
{
	DEBUG_MAN( llvm::errs() << "[EMM] pointer to: " << Name << "\n" );

	SymbolMap::iterator itSymbol = externFunctions.find(mangler.getNameWithPrefix(Name));
	if (itSymbol != externFunctions.end()) {
		DEBUG_MAN( llvm::errs() << "\tyes!\n" );
		return itSymbol->second;
	} else {
		if (AbortOnFailure) {
			RAISE(Exception, "Could not find symbol: "<< Name);
		}

		DEBUG_MAN( llvm::errs() << "ExternalSymbolManager: could not resolve symbol " << Name << "\n" );
		DEBUG_MAN( dump() );
		return 0;
	}
}




// KIARA::Compiler::StagingMemoryManager

KIARA::Compiler::StagingMemoryManager::StagingMemoryManager(CompilerUnit & compilerUnit)
	: llvm::SectionMemoryManager()
	, mangler()
	, unit(compilerUnit)
{}

KIARA::Compiler::StagingMemoryManager::~StagingMemoryManager() {}

void * KIARA::Compiler::StagingMemoryManager::getPointerToNamedFunction(const std::string &Name, bool AbortOnFailure)
{
	// librar symbol look-up
	DEBUG_MAN( llvm::errs() << "[SMM] get pointer to: " << Name << "\n" );
	void * fncPointer = SectionMemoryManager::getPointerToNamedFunction(Name, false);

	if (fncPointer) {
		DEBUG_MAN( llvm::errs() << "found here!\n" );
		return fncPointer;
	}

	// JIT-scope look up
	if (void * ptr = unit.requestPointerToFunction(Name)) {
		return ptr;

	// fall-back solution
	} else if (void * ptr = unit.requestPointerToFunction(Name)) {
		return ptr;
	}
	if (AbortOnFailure) {
		RAISE(Exception, "Could not find symbol: "<< Name);
	}
	return 0;
}

KIARA::Compiler::MyJITEventListener::MyJITEventListener() { }

void KIARA::Compiler::MyJITEventListener::NotifyObjectEmitted(const llvm::ObjectImage &Obj)
{
    llvm::errs() << "NotifyObjectEmitted:\n";
    //Obj.registerWithDebugger();
}

void KIARA::Compiler::MyJITEventListener::NotifyFreeingObject(const llvm::ObjectImage &Obj)
{
    llvm::errs() << "NotifyFreeingObject:\n";
    //Obj.deregisterWithDebugger();
}

KIARA::Compiler::MyJITEventListener * KIARA::Compiler::MyJITEventListener::create()
{
    return new MyJITEventListener();
}

#undef RAISE
