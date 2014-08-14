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
 * CompilerUnit.hpp
 *
 *  Created on: Jul 11, 2013
 *      Author: Simon Moll
 */

#ifndef KIARA_COMPILER_LLVM_COMPILERUNIT_HPP_INCLUDED
#define KIARA_COMPILER_LLVM_COMPILERUNIT_HPP_INCLUDED


#include <KIARA/Compiler/Config.hpp>
#include <KIARA/LLVM/SymbolMangler.hpp>
#include <map>
#include <string>

namespace llvm
{
    class LLVMContext;

    class Module;
    class Value;
    class Function;
    class GlobalVariable;

    class Type;
    class StructType;
    class FunctionType;

    class PassManager;
}

namespace KIARA
{

namespace Compiler
{
	typedef std::map<llvm::Type*,llvm::Type*> LLVMTypeMap;
	typedef std::map<std::string, llvm::StructType*> NamedTypeMap;

	class CompilerUnit
	{
	protected:
		CompilerUnit() {}

	public:

		virtual ~CompilerUnit() {}

		virtual llvm::LLVMContext & getContext() = 0;

		// Returns a module that can be modified (extended)
		// Users should use compilerUnit methods to request already defined symbols
		virtual llvm::Module * getActiveModule() = 0;

		// looks up the global value or forces compilation
		virtual void* requestPointerToFunction(llvm::Function * func) = 0;
		virtual void* requestPointerToFunction(const std::string & funcName) = 0;
		// void* requestPointerToFunction(const std::string & funcName);

	    virtual void* requestPointerToGlobal(llvm::GlobalVariable * gv) = 0;
	    virtual void* requestPointerToGlobal(const std::string & gvName)  = 0;

	    virtual bool isFunctionGenerated(const std::string & funcName) = 0;

	    virtual bool replaceFunction(const std::string &oldName, const std::string &newName) = 0;

	    // use this to register host functions etc with the compilerUnit
	    virtual bool registerExternalFunction(const std::string & symbolName, void * symbolPtr) = 0;
	    virtual bool registerExternalGlobal(const std::string & symbolName, void * symbolPtr) = 0;

	    virtual void registerTrackedType(const std::string & typeName, llvm::StructType * type) = 0;
	    // create a new struct type and makes sure its LLVM type will be tracked
		virtual llvm::StructType * declareType(const std::string & typeName) = 0;
	    // Returns a callable function object
	    // this will insert a declaration in the current top module, if necessary
	    // will return 0, if the function does not occur
	    virtual llvm::Function * requestCallee(const std::string & funcName) = 0;

	    virtual llvm::Function * getFunction(const std::string & funcName) = 0;

	    // migrates a type to the top module
	    virtual llvm::Type * migrateToTop(llvm::Type* type) { return type; }

	    // retrieval (for inspection only)
	    virtual llvm::StructType * getTypeByName(const std::string & typeName) = 0;

	    // Optimization
        virtual bool optimizeActiveModule() = 0;

        virtual bool optimizeFunction(llvm::Function * f) = 0;

	    // "Links in" a new module
	    virtual void addModule(llvm::Module * module) = 0;

	    virtual llvm::Function * createFunction(llvm::FunctionType * type, int linkage, std::string name) = 0;

	    virtual void event_activeModuleModified() { }
	};
}

}




#endif /* KIARA_COMPILER_COMPILERUNIT_HPP_ */
