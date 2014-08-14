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
 * Evaluator.hpp
 *
 *  Created on: 12.03.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMPILER_LLVM_EVALUATOR_HPP_INCLUDED
#define KIARA_COMPILER_LLVM_EVALUATOR_HPP_INCLUDED

#include <KIARA/Compiler/Config.hpp>
#include <KIARA/Compiler/IR.hpp>
#include <KIARA/Compiler/Scope.hpp>
#include <KIARA/Compiler/Compiler.hpp>
#include <KIARA/Compiler/LLVM/CompilerUnit.hpp>
#include <map>
#include <string>
#include <boost/shared_ptr.hpp>

namespace llvm
{
    class LLVMContext;
    class ExecutionEngine;
    class FunctionPassManager;
    class Value;
    class PassManager;
    class Module;
}

namespace KIARA
{

namespace Compiler
{

class CodeGen;
class LangParser;


class KIARA_COMPILER_API Evaluator : public Compiler
{
public:
    Evaluator(World &world, llvm::LLVMContext &context);
    Evaluator(LangParser &parser);
    Evaluator(LangParser &parser, llvm::LLVMContext &context);
    ~Evaluator();

    World & getWorld() const { return world_; }

    Scope::Ptr getTopScope() { return topScope_; }

    llvm::Module * getModule();

    void mainLoop()
    {
        runRepl(parser_);
    }

    llvm::Value * compile(const Object::Ptr &object);
    void * getPointerToFunction(llvm::Value *value);

    void * getPointerToFunction(const std::string &mangledName);

    bool optimizeModule();
    bool optimizeFunction(llvm::Value *value);

    void writeModule(const std::string &fileName);
    void includeFile(const std::string &fileName);
    bool loadModule(const std::string &fileName, std::string &errorMsg);
    bool loadModule(const std::string &fileName);

    void loadPlugin(const std::string &fileName);

    bool linkNativeFunc(const std::string &funcName, void * nativeFunc);

    bool linkNativeFuncOrReplaceWithImpl(const std::string &funcName, void * nativeFunc, const std::string &implFuncName);

    bool isFunctionGenerated(const std::string &funcName);

    static void addSymbol(const std::string &symbolName, void *symbolValue);

private:
    World &world_;
    Scope::Ptr topScope_;
    LangParser *parser_;
    llvm::ExecutionEngine *executionEngine_;
    CompilerUnit *compilerUnit;
    CodeGen *codegen_;

    void runRepl(LangParser *parser);

    void init(LangParser *parser, llvm::LLVMContext *context);
    bool parse(CompilationContext &ctx, LangParser *parser);
    void handleDefinition(const IR::FunctionDefinition::Ptr &funcDef);
    void handleExtern(const IR::ExternFunction::Ptr &externFunc);
    void handleFunction(const IR::Function::Ptr &F);

    void handleExpression(const IR::IRExpr::Ptr &expr);
};


} // namespace Compiler

} // namespace KIARA

#endif /* KIARA_COMPILER_EVALUATOR_HPP_INCLUDED */
