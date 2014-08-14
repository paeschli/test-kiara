/*
 * llvmInlining.hpp
 *
 *  Created on: Mar 5, 2010
 *      Author: Ralf Karrenberg, Dmitri Rubinstein
 */

#ifndef KIARA_LLVM_INLINING_HPP_INCLUDED
#define KIARA_LLVM_INLINING_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>

namespace llvm
{
    class Function;
    class Module;
} // namespace llvm

namespace KIARA
{

//forces a function to be inlined
KIARA_API bool llvmInlineFunction(llvm::Function * func);

/// Inline all function calls in the specified function
KIARA_API bool llvmInlineAllFunctionCalls(llvm::Function * f);

/// Inline all calls to a specified function in a specified module.
KIARA_API bool llvmInlineFunctionCallsInModule(llvm::Function *f, llvm::Module *m);

} // namespace KIARA

#endif /* KIARA_LLVM_INLINING_HPP_INCLUDED */
