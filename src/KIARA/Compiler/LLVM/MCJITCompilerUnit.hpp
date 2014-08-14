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
 * MCJITCompilerUnit.hpp
 *
 *  Created on: Aug 9, 2013
 *      Author: Simon Moll
 */

#ifndef KIARA_COMPILER_LLVM_MCJITCOMPILERUNIT_HPP_INCLUDED
#define KIARA_COMPILER_LLVM_MCJITCOMPILERUNIT_HPP_INCLUDED

#include <KIARA/Compiler/Config.hpp>
#include <KIARA/Compiler/IR.hpp>
#include <KIARA/Compiler/Scope.hpp>
#include <KIARA/Compiler/Compiler.hpp>
#include "CompilerUnit.hpp"
#include "Optimizer.hpp"

#include <map>
#include <string>
#include <vector>

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
    class JITMemoryManager;
    class ExecutionEngine;

}

namespace KIARA
{

namespace Compiler
{

class ExternalSymbolManager;

class MCJITCompilerUnit : public CompilerUnit
{
public:
    void dump() const;

    // migrates a type to the top module
    llvm::Type * migrateToTop(llvm::Type*);

    MCJITCompilerUnit(llvm::Module * activeModule);

    ~MCJITCompilerUnit();

    llvm::LLVMContext & getContext();

#if 0
    void setModule(llvm::Module * newModule);
#endif

    // Returns a module that can be modified (extended)
    // Users should use compilerUnit methods to request already defined symbols
    llvm::Module * getActiveModule();

    // should not be used
    // llvm::ExecutionEngine & getActiveEngine();

    // looks up the global value or forces compilation
    void* requestPointerToFunction(llvm::Function * func);
    void* requestPointerToFunction(const std::string & funcName);
    // void* requestPointerToFunction(const std::string & funcName);

    void* requestPointerToGlobal(llvm::GlobalVariable * gv);
    void* requestPointerToGlobal(const std::string & gvName);

    bool isFunctionGenerated(const std::string & funcName);

    bool replaceFunction(const std::string &oldName, const std::string &newName);

    // use this to register host functions etc with the compilerUnit
    bool registerExternalFunction(const std::string & symbolName, void * symbolPtr);
    bool registerExternalGlobal(const std::string & symbolName, void * symbolPtr);

    // if a type is registered, the CompilerUnit will be able to map the name to a valid TypeName for any later module on the top
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

    // "Links in" a new module updating the topTypeMap
    // if one of the trackedTypes is found in the module, the entry in trackedTypes will be updates as appropriate
    void addModule(llvm::Module * module);

    llvm::Function * createFunction(llvm::FunctionType * type, int linkage, std::string name);

    void event_activeModuleModified();

private:
    class JitStage
    {
    public:
        // mangle all global names as necessary
        void updateGlobalNames();

        JitStage(llvm::Module * stageModule, CompilerUnit & cu);

        ~JitStage();
        // the actual dtor
        void terminate();

        llvm::Module & getModule() const;
        llvm::ExecutionEngine & getEngine() const;
        llvm::JITMemoryManager * getManager() const;

        llvm::StructType * getTypeByName(const std::string & typeName) const;

        // will create an execution engine and compile the module
        void finalize(NamedTypeMap & trackedTypes);
        bool isFinalized() const;

        void * getPointerToGlobal(const std::string & globalName);
        void dump() const;

    private:
        typedef std::map<std::string, llvm::Function*> FunctionIndex;

        llvm::Module * module;
        llvm::ExecutionEngine * engine;
        llvm::JITMemoryManager * manager;
        FunctionIndex globalGetters;

        // Actual type name linkage (Only the top module may have a proper LLVM type name table)
        NamedTypeMap typeNames;

        void initGlobalGetters();
    };
    typedef std::vector<JitStage> JitStageVec;

    JitStage & getTop();
    llvm::ExecutionEngine & getTopEngine();
    llvm::Module & getTopModule();

    // Look for the named symbol in the stacked stages

    // allowDeclarations: will return the first declaring/defining stage, defaults to 0
    // !allowDeclarations: will return the first defining stages, defaults to a declaring stage or 0
    JitStage * findStageForFunction(const std::string & fncName, llvm::Function *& oFunc, bool allowDeclarations = false);
    const JitStage * findStageForGlobal(const std::string & gvName, bool AllowInternal = true, bool allowDeclarations = false) const;
    const JitStage * findStageForType(const std::string & typeName) const;

    // will push a new JitStage
    void pushStage(llvm::Module * initModule);
    void requestUnfinishedTop(); // afterwards, the top will be an unfinished module

    llvm::FunctionType * migrateFunctionToTop(llvm::FunctionType*);

    // used for type resolution when migrating symbols to the top module
    static void LinkTypes(llvm::Type * oldType, llvm::Type * newType, LLVMTypeMap * toNewTypeMap = 0, LLVMTypeMap * toOldTypeMap = 0);
    static void LinkFunctions(llvm::Function * oldFunc, llvm::Function * newFunc, LLVMTypeMap * toNewTypeMap = 0, LLVMTypeMap * toOldTypeMap = 0);
    static void LinkValues(llvm::Value * oldValue, llvm::Value * newValue, LLVMTypeMap * toNewTypeMap = 0, LLVMTypeMap * toOldTypeMap = 0);

    void* requestPointerToFunction(JitStage * stage, llvm::Function * func);

    // used to identify tracked type names
    NamedTypeMap trackedTypeMap;

    // Used to migrate symbols from stashed modules to the top module
    LLVMTypeMap topTypeMap;

    SymbolMangler mangler;
    JitStageVec jitStages;
    ExternalSymbolManager * symbolMan;

    // config
    llvm::LLVMContext & context;
    std::string hostTriple;
    Optimizer optimizer_;
};

} // namespace Compiler

} // namespace KIARA


#endif /* KIARA_COMPILER_MCJITCOMPILERUNIT_HPP_ */
