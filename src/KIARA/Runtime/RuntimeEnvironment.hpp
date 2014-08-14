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
 * RuntimeEnvironment.hpp
 *
 *  Created on: Dec 29, 2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_RUNTIME_RUNTIMEENVIRONMENT_HPP_INCLUDED
#define KIARA_RUNTIME_RUNTIMEENVIRONMENT_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <KIARA/Compiler/Scope.hpp>
#include <KIARA/IRGen/IRGen.hpp>

#include <string>

#ifdef HAVE_LLVM
namespace KIARA
{
namespace Compiler
{
    class Evaluator;
}
}
#endif

namespace KIARA
{

class RuntimeContext;

class KIARA_API RuntimeEnvironment
{
public:
    virtual ~RuntimeEnvironment();

    RuntimeContext & getRuntimeContext() const { return context_; }

    virtual bool startInitialization(std::string *errorMsg = 0) = 0;

    virtual bool finishInitialization(std::string *errorMsg = 0) = 0;

    virtual bool loadModule(const std::string &name, std::string *errorMsg = 0) = 0;

    virtual bool loadComponent(const std::string &name, std::string *errorMsg = 0) = 0;

    virtual bool registerExternalFunction(const std::string & symbolName, void * symbolPtr) = 0;

    virtual Compiler::Scope::Ptr getTopScope() = 0;

    virtual bool isCompilationSupported() const = 0;

    virtual void * compileFunction(const IR::Function::Ptr &func, IRGenContext &genCtx,
                                   const std::string &infoHint = "",
                                   std::string *errorMsg = 0) = 0;

    virtual void * requestPointerToFunction(const std::string &funcName, std::string *errorMsg = 0) = 0;

protected:
    RuntimeEnvironment(RuntimeContext &context);
private:
    RuntimeContext &context_;
};

class InterpreterRuntimeContext;

class KIARA_API InterpreterRuntimeEnvironment : public RuntimeEnvironment
{
    friend class RuntimeContext;
    friend class InterpreterRuntimeContext;
public:

    virtual bool startInitialization(std::string *errorMsg = 0);

    virtual bool finishInitialization(std::string *errorMsg = 0);

    virtual bool loadModule(const std::string &name, std::string *errorMsg = 0);

    virtual bool loadComponent(const std::string &name, std::string *errorMsg = 0);

    virtual bool registerExternalFunction(const std::string & symbolName, void * symbolPtr);

    virtual Compiler::Scope::Ptr getTopScope();

    virtual bool isCompilationSupported() const { return false; }

    virtual void * compileFunction(const IR::Function::Ptr &func, IRGenContext &genCtx,
                                   const std::string &infoHint = "",
                                   std::string *errorMsg = 0);

    virtual void * requestPointerToFunction(const std::string &funcName, std::string *errorMsg = 0);

    virtual ~InterpreterRuntimeEnvironment();

private:
    InterpreterRuntimeEnvironment(InterpreterRuntimeContext &context);
};

#ifdef HAVE_LLVM

class LLVMRuntimeContext;

class KIARA_API LLVMRuntimeEnvironment : public RuntimeEnvironment
{
    friend class RuntimeContext;
    friend class LLVMRuntimeContext;
public:

    virtual ~LLVMRuntimeEnvironment();

    LLVMRuntimeContext & getLLVMRuntimeContext() const
    {
        return reinterpret_cast<LLVMRuntimeContext&>(getRuntimeContext());
    }

    virtual bool startInitialization(std::string *errorMsg = 0);

    virtual bool finishInitialization(std::string *errorMsg = 0);

    virtual bool loadModule(const std::string &fileName, std::string *errorMsg = 0);

    virtual bool loadComponent(const std::string &name, std::string *errorMsg = 0);

    virtual bool registerExternalFunction(const std::string & symbolName, void * symbolPtr);

    virtual Compiler::Scope::Ptr getTopScope();

    virtual bool isCompilationSupported() const { return true; }

    virtual void * compileFunction(const IR::Function::Ptr &func, IRGenContext &genCtx,
                                   const std::string &infoHint = "",
                                   std::string *errorMsg = 0);

    virtual void * requestPointerToFunction(const std::string &funcName, std::string *errorMsg = 0);

    bool includeFile(const std::string &fileName, std::string *errorMsg = 0);
    bool loadPluginLibrary(const std::string &libName, std::string *errorMsg = 0);
    bool writeModule(const std::string &fileName, std::string *errorMsg = 0);

private:
    KIARA::Compiler::Evaluator *evaluator_;
    FunctionLinkMap functionLinkMap_; // this map records all external functions

    LLVMRuntimeEnvironment(LLVMRuntimeContext &context);
};

#endif

} // namespace KIARA

#endif /* KIARA_RUNTIME_RUNTIMEENVIRONMENT_HPP_INCLUDED */
