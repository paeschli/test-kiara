/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2012, 2013  German Research Center for Artificial Intelligence (DFKI)
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
 * Utils.hpp
 *
 *  Created on: 25.11.2011
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_LLVM_UTILS_HPP_INCLUDED
#define KIARA_LLVM_UTILS_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <KIARA/Common/stdint.h>
#include <vector>
#include <set>
#include <string>

namespace llvm
{
    class Module;
    class LLVMContext;
    class Type;
    class PointerType;
    class Value;
    class Function;
    class ExecutionEngine;
} // namespace llvm

namespace KIARA
{

KIARA_API void llvmInitialize();

/// Call this function after llvmInitialize
KIARA_API void llvmInitializeJIT();

KIARA_API void llvmShutdown();

KIARA_API void llvmDeleteModule(llvm::Module *module);

/// This function returns false when failed (different to llvm::LinkModules) !!!
KIARA_API bool llvmLinkModules(
        llvm::Module *dest, llvm::Module *src,
        std::string &errorMsg);

/// This function maps specified module function to the native pointer
KIARA_API bool llvmLinkNativeFunc(
        llvm::Module * module,
        llvm::ExecutionEngine * engine,
        const std::string &funcName,
        void * nativeFunc);

/// This function returns true if function was codegen'd or linked
KIARA_API bool llvmIsFunctionGenerated(
        llvm::Module * module,
        llvm::ExecutionEngine * engine,
        const std::string &funcName);

/// Returns true and saves a string value of a constant string expression argument
/// to valueStr. Returns false if value is not a constant expression
KIARA_API bool llvmGetConstStringArg(const llvm::Value *value, std::string &valueStr);

/// Returns true and saves a signed integer value of a constant integer expression argument
/// to valueInt. Returns false if value is not a constant integer expression or
/// if its bit width is more than 64.
KIARA_API bool llvmGetConstIntArg(const llvm::Value *value, int64_t &valueInt);

/// Returns true and saves an unsigned integer value of a constant integer expression argument
/// to valueInt. Returns false if value is not a constant integer expression or
/// if its bit width is more than 64.
KIARA_API bool llvmGetConstUIntArg(const llvm::Value *value, uint64_t &valueInt);

/// Returns true and saves a float value of a constant float expression argument
/// to valueFloat. Returns false if value is not a constant float expression.
KIARA_API bool llvmGetConstFloatArg(const llvm::Value *value, double &valueFloat);

KIARA_API llvm::Module * llvmLoadModuleFromFile(
        const std::string &fileName, std::string &errorMsg,
        llvm::LLVMContext *context = 0);

KIARA_API bool llvmWriteModuleToFile(
        llvm::Module *module, const std::string &fileName,
        std::string &errorMsg);

KIARA_API bool llvmWriteModuleToStdout(
        llvm::Module *module,
        std::string &errorMsg);

KIARA_API bool llvmWriteModuleAssemblyToFile(
        const llvm::Module *module, const std::string &fileName,
        std::string &errorMsg);

KIARA_API bool llvmParseAssemblyString(
        const char *asmString, llvm::Module &module, std::string &errorMsg);

KIARA_API bool llvmExtParseAssemblyString(
        const char *asmString, llvm::Module &module, std::string &errorMsg);

KIARA_API llvm::Module * llvmParseAssemblyString(
        const char *asmString, std::string &errorMsg, llvm::LLVMContext &context);

KIARA_API llvm::Module * llvmParseBitcodeString(
        const char *name, const char *start, size_t size,
        std::string &errorMsg, llvm::LLVMContext &context);

KIARA_API llvm::LLVMContext & llvmGetGlobalContext();

KIARA_API bool llvmHasParent(llvm::Value *value);

/// converts LLVM's Value to a string representation
KIARA_API std::string llvmToString(const llvm::Value *value);

/// converts LLVM's Type to a string representation
KIARA_API std::string llvmToString(llvm::Type *type);

/// Returns name of the type
KIARA_API std::string llvmGetTypeName(llvm::Type *type);

/// Returns type identifier that can be used in LLVM assembly source
KIARA_API std::string llvmGetTypeIdentifier(llvm::Type *type);

KIARA_API llvm::Type * llvmUpCast(llvm::PointerType *ptype);

KIARA_API const llvm::Type * llvmUpCast(const llvm::PointerType *ptype);

/// Returns true if the module is corrupt
KIARA_API bool llvmVerifyModule(llvm::Module *module, std::string &errorMsg);

KIARA_API void llvmRemoveUnusedFunctionsExcept(llvm::Module *module,
                                               const std::vector<std::string> &functionNames);

KIARA_API void llvmRemoveUnusedFunctionsExcept(llvm::Module *module,
                                               const std::set<llvm::Function*> &functions);

KIARA_API void llvmRemoveUnusedGlobalVariables(llvm::Module *module);

KIARA_API void llvmRemoveVoidFunctionCall(llvm::Function *fun);

/// Load LLVM plugin, uses LLVM's PluginLoader class
KIARA_API void llvmLoadPlugin(const std::string &fileName);

/// Get number of loaded LLVM plugins
KIARA_API unsigned int llvmGetNumPlugins();

/// Get name of loaded LLVM plugin with the specified index
KIARA_API const std::string llvmGetPlugin(unsigned int num);

KIARA_API void llvmAnnotateFunctionEntersAndExits(llvm::Module *module, const std::string &funcToCall);

/// Replace all calls to function oldName with calls to function newName
/// Returns true if module was changed
KIARA_API bool llvmReplaceFunctionCalls(llvm::Module *module, const std::string &oldName, const std::string &newName);

KIARA_API bool llvmMarkFunctionsWithAlwaysInline(llvm::Module *module);

/// If removeAlwaysInlineAttr is true, AlwaysInline attribute will be removed and NoInline added
/// If removeAlwaysInlineAttr is false, NoInline will be only set on functions without AlwaysInline attribute
KIARA_API bool llvmMarkFunctionsWithNoInline(llvm::Module *module, bool removeAlwaysInlineAttr = true);

} // namespace KIARA

#endif /* KIARA_LLVM_UTILS_HPP_INCLUDED */
