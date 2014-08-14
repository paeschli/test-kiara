/*
 * Optimizer.hpp
 *
 *  Created on: Mar 25, 2014
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMPILER_LLVM_OPTIMIZER_HPP_INCLUDED
#define KIARA_COMPILER_LLVM_OPTIMIZER_HPP_INCLUDED


namespace llvm
{
class LLVMContext;
class FunctionPassManager;
class PassManager;
class Module;
class Function;
}

namespace KIARA
{
namespace Compiler
{

class Optimizer
{
public:

    Optimizer(llvm::Module *module = 0);
    ~Optimizer();

    void reset();

    llvm::Module * getModule() const { return module_; }
    void setModule(llvm::Module *module);

    // optimizeModule returns true if module was modified
    bool optimizeModule();

    // optimizeFunction returns true if function was modified
    bool optimizeFunction(llvm::Function *func);

private:
    llvm::FunctionPassManager *fpm_;
    llvm::PassManager *mpm_;
    llvm::Module *module_;
};

} // namespace Compiler
} // namespace KIARA

#endif /* KIARA_COMPILER_LLVM_OPTIMIZER_HPP_INCLUDED */
