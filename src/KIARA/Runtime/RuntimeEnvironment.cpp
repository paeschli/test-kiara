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
 * RuntimeEnvironment.cpp
 *
 *  Created on: Dec 29, 2013
 *      Author: Dmitri Rubinstein
 */
#define KIARA_LIB
#include "RuntimeEnvironment.hpp"
#include "RuntimeContext.hpp"

#ifdef HAVE_LLVM
#include "KIARA/Compiler/IRUtils.hpp"
#include "KIARA/Compiler/LLVM/Evaluator.hpp"
#include "KIARA/LLVM/Utils.hpp"
#endif

// #define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

namespace KIARA
{

RuntimeEnvironment::RuntimeEnvironment(RuntimeContext &context)
    : context_(context)
{ }

RuntimeEnvironment::~RuntimeEnvironment()
{ }

InterpreterRuntimeEnvironment::InterpreterRuntimeEnvironment(InterpreterRuntimeContext &context)
    : RuntimeEnvironment(context)
{

}

InterpreterRuntimeEnvironment::~InterpreterRuntimeEnvironment()
{

}

bool InterpreterRuntimeEnvironment::startInitialization(std::string *errorMsg)
{
    return true;
}

bool InterpreterRuntimeEnvironment::finishInitialization(std::string *errorMsg)
{
    return true;
}

bool InterpreterRuntimeEnvironment::loadModule(const std::string &name, std::string *errorMsg)
{
    // TODO what should we do here ?
    return false;
}

bool InterpreterRuntimeEnvironment::loadComponent(const std::string &name, std::string *errorMsg)
{
    // TODO what should we do here ?
    return false;
}

bool InterpreterRuntimeEnvironment::registerExternalFunction(const std::string & symbolName, void * symbolPtr)
{
    // TODO what should we do here ?
    return false;
}

Compiler::Scope::Ptr InterpreterRuntimeEnvironment::getTopScope()
{
    // TODO what should we do here ?
    return 0;
}

void * InterpreterRuntimeEnvironment::compileFunction(
    const IR::Function::Ptr &func,
    IRGenContext &genCtx,
    const std::string &infoHint,
    std::string *errorMsg)
{
    // TODO what should we do here ?
    return 0;
}

void * InterpreterRuntimeEnvironment::requestPointerToFunction(const std::string &funcName, std::string *errorMsg)
{
    // TODO what should we do here ?
    return 0;
}

#ifdef HAVE_LLVM

LLVMRuntimeEnvironment::LLVMRuntimeEnvironment(LLVMRuntimeContext &context)
    : RuntimeEnvironment(context)
    , evaluator_(0)
{
    evaluator_ = new KIARA::Compiler::Evaluator(
        context.getWorld(),
        context.getLLVMContext()/*KIARA::llvmGetGlobalContext()*/);

    // All types that are also defined in KL files must be added to the top scope, otherwise they will be defined twice
    KIARA::IR::IRUtils::addObjectToScope(context.getContextType()->getTypeName(), context.getContextType(), evaluator_->getTopScope());
    KIARA::IR::IRUtils::addObjectToScope(context.getConnectionType()->getTypeName(), context.getConnectionType(), evaluator_->getTopScope());
    KIARA::IR::IRUtils::addObjectToScope(context.getMessageType()->getTypeName(), context.getMessageType(), evaluator_->getTopScope());
    KIARA::IR::IRUtils::addObjectToScope(context.getFuncObjType()->getTypeName(), context.getFuncObjType(), evaluator_->getTopScope());
    KIARA::IR::IRUtils::addObjectToScope(context.getServiceFuncObjType()->getTypeName(), context.getServiceFuncObjType(), evaluator_->getTopScope());
    KIARA::IR::IRUtils::addObjectToScope(context.getUserTypeType()->getTypeName(), context.getUserTypeType(), evaluator_->getTopScope());
    KIARA::IR::IRUtils::addObjectToScope(context.getDBufferType()->getTypeName(), context.getDBufferType(), evaluator_->getTopScope());
    KIARA::IR::IRUtils::addObjectToScope(context.getBinaryStreamType()->getTypeName(), context.getBinaryStreamType(), evaluator_->getTopScope());
}

bool LLVMRuntimeEnvironment::startInitialization(std::string *errorMsg)
{
    if (!includeFile("stdlib.kl", errorMsg))
        return false;

    return true;
}

bool LLVMRuntimeEnvironment::finishInitialization(std::string *errorMsg)
{
    if (!includeFile("api.kl", errorMsg))
        return false;

    evaluator_->optimizeModule();

    return true;
}

bool LLVMRuntimeEnvironment::loadComponent(const std::string &name, std::string *errorMsg)
{
    return loadModule(name+"_kp.bc", errorMsg);
}

bool LLVMRuntimeEnvironment::registerExternalFunction(const std::string & symbolName, void * symbolPtr)
{
    functionLinkMap_[symbolName] = symbolPtr;
    return evaluator_->linkNativeFunc(symbolName, symbolPtr);
}

Compiler::Scope::Ptr LLVMRuntimeEnvironment::getTopScope()
{
    return evaluator_->getTopScope();
}

void * LLVMRuntimeEnvironment::compileFunction(
    const IR::Function::Ptr &func,
    IRGenContext &genCtx,
    const std::string &infoHint,
    std::string *errorMsg)
{
    DFC_DEBUG("Compile function: "<<func->getName());
    for (std::vector<KIARA::IR::IRExpr::Ptr>::const_iterator it = genCtx.expressions.begin(),
            end = genCtx.expressions.end(); it != end; ++it)
    {
        evaluator_->compile(*it);
    }

    llvm::Value *llvmFunc = evaluator_->compile(func);

    DFC_IFDEBUG(evaluator_->writeModule(infoHint+"_NO_OPT.bc"));

    evaluator_->optimizeFunction(llvmFunc);
    // evaluator_->optimizeModule();

    DFC_IFDEBUG(evaluator_->writeModule(infoHint+".bc"));

    functionLinkMap_.insert(genCtx.functionLinkMap.begin(), genCtx.functionLinkMap.end());

    for (KIARA::FunctionLinkMap::iterator it = functionLinkMap_.begin(),
        end = functionLinkMap_.end(); it != end; ++it)
    {
        // FIXME This code assumes that functions are never overridden
        if (!evaluator_->isFunctionGenerated(it->first))
        {
            evaluator_->linkNativeFuncOrReplaceWithImpl(it->first, it->second.funcPtr, it->second.funcName);
        }
    }

    DFC_DEBUG("LLVM FUNC: "<<KIARA::llvmToString(llvmFunc));

    void *funcPtr = evaluator_->getPointerToFunction(llvmFunc);
    return funcPtr;
}

void * LLVMRuntimeEnvironment::requestPointerToFunction(const std::string &funcName, std::string *errorMsg)
{
    return evaluator_->getPointerToFunction(funcName);
}

bool LLVMRuntimeEnvironment::includeFile(const std::string &fileName, std::string *errorMsg)
{
    std::string path = getRuntimeContext().findPath(fileName, errorMsg);
    if (path.empty())
    {
        if (errorMsg)
            *errorMsg = "Could not find include '" + fileName + "' file";
        return false;
    }

    // FIXME: pass errorMsg to includeFile
    evaluator_->includeFile(path);
    return true;
}

bool LLVMRuntimeEnvironment::loadModule(const std::string &fileName, std::string *errorMsg)
{
    std::string path = getRuntimeContext().findPath(fileName, errorMsg);
    if (path.empty())
    {
        if (errorMsg)
            *errorMsg = "Could not find '" + fileName + "' module";
        return false;
    }

    std::string tmpErrorMsg;
    if (!evaluator_->loadModule(path, tmpErrorMsg))
    {
        if (errorMsg)
            errorMsg->swap(tmpErrorMsg);
        return false;
    }
    return true;
}

bool LLVMRuntimeEnvironment::loadPluginLibrary(const std::string &libName, std::string *errorMsg)
{
    std::string fileName;
#ifdef _WIN32
    fileName = libName + ".dll";
#else
    fileName = "lib" + libName + ".so";
#endif

    std::string path = getRuntimeContext().findPath(fileName, errorMsg);
    if (path.empty())
    {
        if (errorMsg)
            *errorMsg = "Could not find '" + fileName + "' plugin library";
        return false;
    }

    evaluator_->loadPlugin(path);
    return true;
}

bool LLVMRuntimeEnvironment::writeModule(const std::string &fileName, std::string *errorMsg)
{
    // FIXME propagate error
    evaluator_->writeModule(fileName);
    return true;
}

LLVMRuntimeEnvironment::~LLVMRuntimeEnvironment()
{
    delete evaluator_;
}

#endif

} // namespace KIARA
