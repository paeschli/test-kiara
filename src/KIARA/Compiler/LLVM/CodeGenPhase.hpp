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
 * CodeGenPhase.hpp
 *
 *  Created on: 31.05.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMPILER_LLVM_CODEGENPHASE_HPP_INCLUDED
#define KIARA_COMPILER_LLVM_CODEGENPHASE_HPP_INCLUDED

#include <KIARA/Compiler/Config.hpp>
#include <KIARA/Compiler/Compiler.hpp>

namespace llvm
{
class Value;
}

namespace KIARA
{

namespace Compiler
{

struct LLVMObject
{
    llvm::Value *value;

    LLVMObject(llvm::Value *value = 0) : value(value) { }

    typedef llvm::Value * LLVMObject::*UnspecifiedBoolType;

    operator UnspecifiedBoolType () const
    {
        return value == 0 ? 0 : &LLVMObject::value;
    }
};

// CodeGenPhase

class CodeGenPhase : public CompilerPhase
{
public:

    typedef boost::shared_ptr<CodeGenPhase> Ptr;

    CodeGenPhase(CodeGen &codegen, Compiler &compiler);

    void runPhase(CompilationContext &ctx, const Object::Ptr &object);

protected:

    std::string genUidName();

private:
    CodeGen &codegen_;
    int uid_;
};

} // namespace Compiler

} // namespace KIARA

#endif /* KIARA_COMPILER_LLVM_CODEGENPHASE_HPP_INCLUDED */
