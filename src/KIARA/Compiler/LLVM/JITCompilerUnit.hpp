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
 * JITCompilerUnit.hpp
 *
 *  Created on: Aug 9, 2013
 *      Author: Simon Moll
 */

#ifndef KIARA_COMPILER_LLVM_JITCOMPILERUNIT_HPP_INCLUDED
#define KIARA_COMPILER_LLVM_JITCOMPILERUNIT_HPP_INCLUDED

#include <KIARA/Compiler/Config.hpp>
#include <KIARA/Compiler/IR.hpp>
#include <KIARA/Compiler/Scope.hpp>
#include <KIARA/Compiler/Compiler.hpp>
#include "CompilerUnit.hpp"
#include "Optimizer.hpp"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/ValueHandle.h"
#include <string>
#include <map>
#include <set>

namespace llvm
{
    class LLVMContext;
    class FunctionPassManager;

    class Module;
    class Value;
    class Function;
    class GlobalVariable;

    class Type;
    class StructType;

    class PassManager;
    class ExecutionEngine;
}

namespace KIARA
{

namespace Compiler
{

	class JITCompilerUnit : public CompilerUnit
	{
	public:

		// migrates a type to the top module

		JITCompilerUnit(llvm::Module * activeModule);

		~JITCompilerUnit();

		llvm::LLVMContext & getContext();

		// Returns a module that can be modified (extended)
		// Users should use compilerUnit methods to request already defined symbols
		llvm::Module * getActiveModule();

		// looks up the global value or forces compilation
		void* requestPointerToFunction(llvm::Function * func);
		void* requestPointerToFunction(const std::string & funcName);
		// void* requestPointerToFunction(const std::string & funcName);

	    void* requestPointerToGlobal(llvm::GlobalVariable * gv);
	    void* requestPointerToGlobal(const std::string & gvName) ;

	    bool isFunctionGenerated(const std::string & funcName);

        bool replaceFunction(const std::string &oldName, const std::string &newName);

	    // use this to register host functions etc with the compilerUnit
	    bool registerExternalFunction(const std::string & symbolName, void * symbolPtr);
	    bool registerExternalGlobal(const std::string & symbolName, void * symbolPtr);

	    void registerTrackedType(const std::string & typeName, llvm::StructType * type);

		// create a new struct type and makes sure its LLVM type will be tracked

		llvm::StructType * declareType(const std::string & typeName);

	    // Returns a callable function object
	    // this will insert a declaration in the current top module, if necessary
	    // will return 0, if the function does not occur
	    llvm::Function * requestCallee(const std::string & funcName);

	    llvm::Function * getFunction(const std::string & funcName);

	    // retrieval (for inspection only)
	    llvm::StructType * getTypeByName(const std::string & typeName);

	    // Optimization
        bool optimizeActiveModule();

        bool optimizeFunction(llvm::Function * f);

	    // "Links in" a new module
	    // Resolved opaque types are stored in the typeMap
	    void addModule(llvm::Module * module);

	    llvm::Function * createFunction(llvm::FunctionType * type, int rawLinkage, std::string name);

	private:

        void findAllUsedFunctions(
            llvm::Function *func,
            std::set<llvm::Function*> &funcSet,
            llvm::SmallVectorImpl<llvm::AssertingVH<llvm::Function> > &funcList);

	    void registerCompiledFunction(llvm::Function *func, void *addr);

        llvm::Module * jitModule;
        llvm::ExecutionEngine * jitEngine;
        Optimizer optimizer_;
        NamedTypeMap trackedTypeMap_;
        typedef std::map<void *, llvm::SmallVector<llvm::AssertingVH<llvm::Function>, 2> > CompiledFuncMap;
        CompiledFuncMap compiledFuncs_;
	};

} // namespace Compiler

} // namespace KIARA

#endif /* KIARA_COMPILER_JITCOMPILERUNIT_HPP_ */
