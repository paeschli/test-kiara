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
 * CodeGenPhase.cpp
 *
 *  Created on: 31.05.2013
 *      Author: Dmitri Rubinstein
 */

#define KIARA_COMPILER_LIB
// ssize_t is defined by LLVM's DataTypes.h
#define _SSIZE_T_DEFINED

#include "CodeGenPhase.hpp"
#include <KIARA/Compiler/LangParser.hpp>
#include <KIARA/Compiler/PrettyPrinter.hpp>
#include "CodeGen.hpp"
#include "llvm/Config/llvm-config.h"
#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
#include "llvm/IR/Function.h"
#else
#include "llvm/Function.h"
#endif
#include "llvm/Support/Casting.h"
#include <KIARA/Utils/VarGuard.hpp>

// #define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

namespace KIARA
{

namespace Compiler
{

// CodeGenPhase

CodeGenPhase::CodeGenPhase(CodeGen &codegen, Compiler &compiler)
    : CompilerPhase(compiler)
    , codegen_(codegen)
    , uid_(0)
{
}

void CodeGenPhase::runPhase(CompilationContext &ctx, const Object::Ptr &object)
{
    // clear CodeGen data
    ctx.clear("codegen.llvm.expr");
    ctx.clear("codegen.llvm.def");
    ctx.clear("codegen.llvm.value");

    if (object)
    {
        // handle top level objects
        if (IR::FunctionDefinition::Ptr funcDef = dyn_cast<IR::FunctionDefinition>(object))
        {
            if (codegen_.isSupported(funcDef))
            {
                // definition
		llvm::Value * retValue = codegen_.apply(funcDef);
                if (llvm::Function *LF = llvm::cast_or_null<llvm::Function>(retValue))
                {
                    ctx.set("codegen.llvm.def", LLVMObject(LF));
                    ctx.set("codegen.llvm.value", LLVMObject(LF));
                    DFC_DEBUG("Read function definition:");
                    DFC_IFDEBUG(funcDef->print(std::cerr));
                    DFC_IFDEBUG("Converted to LLVM:");
                    DFC_IFDEBUG(LF->dump());
                    //parser_.installOperator(F);
                }
                else
                {
                    // FIXME report this as an error
                    std::cerr<<"error: Could not compile definition: "<<std::endl;
                    funcDef->print(std::cerr);
                    std::cerr<<std::endl;
                }
            }
        }
        else if (const IR::IRExpr::Ptr expr = dyn_cast<IR::IRExpr>(object))
        {
            // expression
            // Make an anonymous proto, generate and execute function
            World &world = expr->getWorld();

            std::string uname = genUidName();
            IR::Prototype::Ptr proto = new IR::Prototype(
                    uname, 0, std::vector<IR::Prototype::Arg>(), world);
            IR::Function::Ptr func = new IR::Function(proto, expr);
            if (llvm::Function *LF = llvm::cast_or_null<llvm::Function>(codegen_.apply(func)))
            {
                ctx.set("codegen.llvm.expr", LLVMObject(LF));
                ctx.set("codegen.llvm.value", LLVMObject(LF));
            }
            else
            {
                // TODO dump F
                std::cerr<<"error: Could not compile top level expr"<<std::endl;
            }
        }
        else if (const IR::TypeDefinition::Ptr typeDef = dyn_cast<IR::TypeDefinition>(object))
        {
            codegen_.declareType(typeDef->getDefinedType());
        }
        else
        {
            // FIXME report this as an error
            std::cerr<<"Compiler: Unknown object : "<<*object<<" (type is "<<object->getTypeName()<<")"<<std::endl;
        }
    }
}

std::string CodeGenPhase::genUidName()
{
    return "UName"+boost::lexical_cast<std::string>(uid_++);
}

} // namespace Compiler

} // namespace KIARA

