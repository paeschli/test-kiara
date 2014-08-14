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
 * Evaluator.cpp
 *
 *  Created on: 12.03.2013
 *      Author: Dmitri Rubinstein
 */

#define KIARA_COMPILER_LIB
// ssize_t is defined by LLVM's DataTypes.h
#define _SSIZE_T_DEFINED

#include "Evaluator.hpp"
#include <KIARA/Compiler/IRUtils.hpp>

// LLVM
#include "llvm/Config/llvm-config.h"
#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
#include "llvm/IR/Module.h"
#else
#include "llvm/Module.h"
#endif
#include "llvm/ADT/Triple.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Analysis/Passes.h"
#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
#include "llvm/IR/DataLayout.h"
#else
#include "llvm/Target/TargetData.h"
#endif
#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/LinkAllPasses.h"
#include "llvm/Linker.h"

#include <DFC/Base/Core/ObjectFactory.hpp>
#include <DFC/Base/Core/ObjectMacros.hpp>
#include <DFC/Base/Utils/StaticInit.hpp>
#include <DFC/Base/Utils/FileSystem.hpp>
#include <KIARA/Impl/Core.hpp>
#include <KIARA/Core/Exception.hpp>
#include <KIARA/LLVM/Utils.hpp>
#include <KIARA/DB/LibraryConfiguration.hpp>
#include <boost/scoped_ptr.hpp>

// Lang
#include "CodeGen.hpp"
#include "CodeGenPhase.hpp"
#include <KIARA/Compiler/LangParser.hpp>
#include <KIARA/Compiler/SubstituteBuiltins.hpp>
#include <KIARA/Compiler/Mangler.hpp>
#include <KIARA/Compiler/Token.hpp>
#include <KIARA/Compiler/Lexer.hpp>


#include <KIARA/Compiler/LLVM/MCJITCompilerUnit.hpp>
#include <KIARA/Compiler/LLVM/JITCompilerUnit.hpp>
#include "OptimizationConfig.hpp"

// Embed Bitcode
static unsigned char bootstrapBitcode[] = {
    // kiara_llvm_bootstrap_data.h is generated
    // from kiara_llvm_bootstrap.c by the build process
    #include <KIARA/Bitcode/kiara_llvm_bootstrap_data.h>
};

namespace KIARA
{

namespace Compiler
{

// Evaluator

Evaluator::Evaluator(World &world, llvm::LLVMContext &context)
    : world_(world)
    , topScope_()
    , parser_(0)
    , compilerUnit(0)
    , codegen_(0)
{
    init(0, &context);
}

Evaluator::Evaluator(LangParser &parser)
    : world_(parser.getWorld())
    , topScope_()
    , parser_(&parser)
    , compilerUnit(0)
    , codegen_(0)
{
    init(&parser, 0);
}

Evaluator::Evaluator(LangParser &parser, llvm::LLVMContext &context)
    : world_(parser.getWorld())
    , topScope_()
    , parser_(&parser)
    , compilerUnit(0)
    , codegen_(0)
{
    init(&parser, &context);
}

Evaluator::~Evaluator()
{
    removeAllPhases();

    delete compilerUnit;

    delete codegen_;
}

extern "C"
{
    void write_module_ctx(Evaluator *evaluator, const char *fileName)
    {
        evaluator->writeModule(fileName ? fileName : "");
    }

    void include_file_ctx(Evaluator *evaluator, const char *fileName)
    {
        evaluator->includeFile(fileName ? fileName : "");
    }

    void load_module_ctx(Evaluator *evaluator, const char *fileName)
    {
        evaluator->loadModule(fileName ? fileName : "");
    }
}

void Evaluator::init(LangParser *parser, llvm::LLVMContext *context)
{
    assert((parser ? &world_ == &parser->getWorld() : true));

    if (!context)
        context = &llvm::getGlobalContext();

    if (parser)
        topScope_ = parser->getScope();
    else
        topScope_ = new Scope(world_, "main");

    std::string errorMsg;

    // Make the module, which holds all the code.
    llvm::Module *module = KIARA::llvmParseBitcodeString(
        "kiara_jit",
        reinterpret_cast<char*>(bootstrapBitcode),
        sizeof(bootstrapBitcode)/sizeof(bootstrapBitcode[0]),
        errorMsg,
        *context);
    //llvm::Module *module = new llvm::Module("kiara_jit", *context);
    if (!module)
        DFC_THROW_EXCEPTION(Exception, "Could not load embedded bootstrap bitcode: "<<errorMsg);

    JITConfiguration jitConfig = KIARA::Impl::Global::getJITConfiguration();

    if (jitConfig.useLegacyJIT)
    {
        compilerUnit = new JITCompilerUnit(module);
    }
    else
    {
        compilerUnit = new MCJITCompilerUnit(module);
    }
    codegen_ = new CodeGen(compilerUnit, world_);

    // setup intrinsics
    // FIXME add support for all types or generic '=' operator

    addPhase(CompilerPhase::Ptr(new TransformPhase<SubstituteBuiltins>(*this)));
    addPhase(CompilerPhase::Ptr(new CodeGenPhase(*codegen_, *this)));

    IR::IRUtils::addDefaultTypesToScope(topScope_);

//    codegen_->emitAssemblyString(
//            "%Context = type opaque "
//            "@context = global %Context* null ");

    Type::Ptr contextType = StructType::create(world_, "Context");
    Type::Ptr contextTypePtr = PtrType::get(contextType);

    IR::Prototype::Ptr proto = new KIARA::IR::Prototype(
            "getContext", "getContext",
            contextTypePtr, ArrayRef<IR::Prototype::Arg>(), world_);
    proto->setAttribute("internal", "true");
    proto->setAttribute("llvm", "true");
    IR::Intrinsic::Ptr def = new IR::Intrinsic(proto,
                                               "@context = global %struct.Context* null "
                                               "define %struct.Context* @${mangledName}() nounwind uwtable readonly { "
                                               "entry: "
                                               "  %0 = load %struct.Context** @context "
                                               "  ret %struct.Context* %0 "
                                               "}",
                                               world_);
    CompilationContext ctx;
    emit(ctx, def);

    // FIXME scope should be separated from the parser
    IR::IRUtils::addObjectToScope(contextType->getTypeName(), contextType, topScope_);
    IR::IRUtils::addFunctionToScope(def, topScope_);

    llvm::GlobalVariable *contextVar = module->getGlobalVariable("context", true);
    assert(contextVar);

    //if ()
    {
        void **ctx = (void**) compilerUnit->requestPointerToGlobal(contextVar);
        *ctx = this;
    }

    /* FIXME: We cannot use registerExternalFunction because we do not create function declaration
     * in the LLVM module at this point
     */
#if 0
    compilerUnit->registerExternalFunction("write_module_ctx", (void*) &write_module_ctx);
    compilerUnit->registerExternalFunction("include_file_ctx", (void*) &include_file_ctx);
    compilerUnit->registerExternalFunction("load_module_ctx", (void*) &load_module_ctx);
#else
    addSymbol("write_module_ctx", (void*) &write_module_ctx);
    addSymbol("include_file_ctx", (void*) &include_file_ctx);
    addSymbol("load_module_ctx", (void*) &load_module_ctx);
#endif

#if 0
    if (parser) // FIXME scope should be separated from the parser
    {
        IR::Prototype::Arg args[] = {
                std::pair<std::string, KIARA::Type::Ptr>("a", world_.type_c_ref(world.type_c_double())),
                std::pair<std::string, KIARA::Type::Ptr>("b", world_.type_c_double())
        };
        IR::Prototype::Ptr proto = new KIARA::IR::Prototype(
                "=",
                Mangler::getMangledFuncName("=", ArrayRef<IR::Prototype::Arg>(args)),
                world_.type_c_double(), args, world_, true, 2);
        proto->setAttribute("internal", "true");
        IR::Intrinsic::Ptr def = new IR::Intrinsic(proto,
                                                   "define double @_KaSRdd(double* nocapture %var, double %value) nounwind uwtable { "
                                                   "  store double %value, double* %var "
                                                   "  ret double %value "
                                                   "}",
                                                   world_);
        parser->addDefinition(def);
        handleDefinition(def);
    }

    if (parser) // FIXME scope should be separated from the parser
    {
        IR::Prototype::Arg args[] = {
                std::pair<std::string, Type::Ptr>("a", world_.type_c_ref(world.type_c_int())),
                std::pair<std::string, Type::Ptr>("b", world_.type_c_int())
        };
        IR::Prototype::Ptr proto = new IR::Prototype(
                "=",
                Mangler::getMangledFuncName("=", ArrayRef<IR::Prototype::Arg>(args)),
                world_.type_c_int(), args, world_, true, 2);
        proto->setAttribute("internal", "true");
        IR::Intrinsic::Ptr def = new IR::Intrinsic(proto,
                                                   "define i32 @_KaSRii(i32* nocapture %var, i32 %value) nounwind uwtable { "
                                                   "  store i32 %value, i32* %var "
                                                   "  ret i32 %value "
                                                   "}",
                                                  world_);
        parser->addDefinition(def);
        handleDefinition(def);
    }
#endif
}

bool Evaluator::parse(CompilationContext &ctx, LangParser *parser)
{
    ctx.clear("parser.result");

    const Token &tok = parser->current();
    if (tok.isEof() || tok.isError())
    {
        ctx.endCompilation();
        return false;
    }

    Object::Ptr stmt = parser->parseStatement();
    if (!stmt)
    {
        ctx.endCompilation();
        return false;
    }

    ctx.set("parser.result", stmt);
    emit(ctx, stmt);
    return true;
}

/// top ::= definition | external | expression | ';'
void Evaluator::runRepl(LangParser *parser)
{
    if (!parser)
        return;

    CompilationContext ctx;

    while (!ctx.isEndOfCompilation())
    {
        ctx.clear();
        if (!parse(ctx, parser))
            break;

        // get result
        if (ctx.get<LLVMObject>("codegen.llvm.def", 0))
        {
            IR::FunctionDefinition::Ptr funcDef = dyn_cast<IR::FunctionDefinition>(ctx.get<Object::Ptr>("parser.result", Object::Ptr()));
            parser->installOperator(funcDef);
        }

        if (llvm::Function *llvmFun =
                llvm::cast_or_null<llvm::Function>(
                        ctx.get<LLVMObject>("codegen.llvm.expr", 0).value))
        {
            // JIT the function, returning a function pointer.
            void *FPtr = compilerUnit->requestPointerToFunction(llvmFun);
            if (llvmFun->getFunctionType()->getNumParams() == 0)
            {
                if (llvmFun->getReturnType()->isVoidTy())
                {
                    void (*FP)() = (void (*)())(intptr_t)FPtr;
                    FP();
                }
                else if (llvmFun->getReturnType()->isDoubleTy())
                {
                    double (*FP)() = (double (*)())(intptr_t)FPtr;
                    fprintf(stderr, "Evaluated to %f\n", FP());
                }
                else
                {
                    std::cerr<<"Cannot execute function with type : "<<llvmToString(llvmFun->getFunctionType())<<std::endl;
                }
            }
        }
    }
}

llvm::Value * Evaluator::compile(const Object::Ptr &object)
{
    CompilationContext ctx;
    emit(ctx, object);
    return ctx.get<LLVMObject>("codegen.llvm.value", 0).value;
}

llvm::Module * Evaluator::getModule()
{
    return compilerUnit->getActiveModule();
}

void * Evaluator::getPointerToFunction(llvm::Value *value)
{
    void * ptr = 0;
    if (llvm::Function *llvmFun = llvm::cast_or_null<llvm::Function>(value))
    {
        ptr = compilerUnit->requestPointerToFunction(llvmFun);
        codegen_->clearStringCache();
    }

    return ptr;
}

void * Evaluator::getPointerToFunction(const std::string &mangledName)
{
    void * ptr = compilerUnit->requestPointerToFunction(mangledName);
    codegen_->clearStringCache();
    return ptr;
}

bool Evaluator::optimizeModule()
{
    return compilerUnit->optimizeActiveModule();
}

bool Evaluator::optimizeFunction(llvm::Value *value)
{
    if (llvm::Function *f = llvm::cast_or_null<llvm::Function>(value))
        return compilerUnit->optimizeFunction(f);
    return false;
}

void Evaluator::writeModule(const std::string &fileName)
{
    llvm::Module *module = compilerUnit->getActiveModule();
    // TODO: link all modules together to yield a consistent dump

    std::string errorMsg;
    if (!llvmWriteModuleToFile(module, fileName, errorMsg))
    {
        std::cerr<<"Error: "<<errorMsg<<std::endl;
    }
}

#define DEFAULT_LEXER_PREFIX "=<>!+-*&|/%^"
#define DEFAULT_LEXER_SUFFIX "=<>&|"
#define DEFAULT_LEXER_CXX_COMMENT false

bool Evaluator::loadModule(const std::string &fileName, std::string &errorMsg)
{
    std::string fn;
    if (DFC::FileSystem::isAbsolutePath(fileName) || !parser_)
        fn = fileName;
    else
        fn = DFC::FileSystem::joinPaths(
                DFC::FileSystem::getDirname(parser_->getFileName()),
                fileName);

    NamedTypeMap updatedTypes;
#if 0
    codegen_->insertTrackedTypes(updatedTypes);
#endif

    // clean tracked type names
    codegen_->clearStringCache();

    // load the module to the CompilerUnit context
    llvm::Module * module = llvmLoadModuleFromFile(fn, errorMsg, &codegen_->getContext());
    if (!module)
        return false;

    // register the module and update all type reference
    compilerUnit->addModule(module);
    codegen_->migrateTypes();
    return true;
}

bool Evaluator::loadModule(const std::string &fileName)
{
    std::string errorMsg;
    bool success = loadModule(fileName, errorMsg);
    if (!success)
        std::cerr<<"Error: "<<errorMsg<<std::endl;
    return success;
}

void Evaluator::loadPlugin(const std::string &fileName)
{
    llvmLoadPlugin(fileName);
}

bool Evaluator::linkNativeFunc(const std::string &funcName, void * nativeFunc)
{
    return compilerUnit->registerExternalFunction(funcName, nativeFunc);
}

bool Evaluator::linkNativeFuncOrReplaceWithImpl(const std::string &funcName, void * nativeFunc, const std::string &implFuncName)
{
    if (compilerUnit->replaceFunction(funcName, implFuncName))
        return true;
    return compilerUnit->registerExternalFunction(funcName, nativeFunc);
}

bool Evaluator::isFunctionGenerated(const std::string &funcName)
{
    return compilerUnit->isFunctionGenerated(funcName);
}

void Evaluator::includeFile(const std::string &fileName)
{
    std::string fn;
    if (DFC::FileSystem::isAbsolutePath(fileName) || !parser_)
        fn = fileName;
    else
        fn = DFC::FileSystem::joinPaths(
                DFC::FileSystem::getDirname(parser_->getFileName()),
                fileName);
    std::ifstream in(fn.c_str());
    if (!in)
    {
        std::cerr<<"Error: Could not include file "<<fileName<<std::endl;
        return;
    }

    std::string prefix = parser_ ? parser_->getLexer()->getPrefix() : DEFAULT_LEXER_PREFIX;
    std::string suffix = parser_ ? parser_->getLexer()->getSuffix() : DEFAULT_LEXER_SUFFIX;
    bool cxxComment = parser_ ? parser_->getLexer()->getCXXComment() : DEFAULT_LEXER_CXX_COMMENT;

    Lexer lexer(in, prefix, suffix);
    lexer.setCXXComment(cxxComment);

    if (parser_)
    {
        LangParser::ParserState state = parser_->getState();
        parser_->initParser(lexer, fileName);
        mainLoop();
        parser_->setState(state);
    }
    else
    {
        KIARA::Compiler::LangParser parser(topScope_);
        parser.initParser(lexer, fileName);
        runRepl(&parser);
    }
}

void Evaluator::addSymbol(const std::string &symbolName, void *symbolValue)
{
    // FIXME move this to LLVM Utils
    llvm::sys::DynamicLibrary::AddSymbol(symbolName, symbolValue);
}

} // namespace Compiler

} // namespace KIARA
