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
 * DeepCalleeFinder.cpp
 *
 *  Created on: Mar 16, 2010
 *      Author: Simon Moll
 */

#define KIARA_LIB
#define DEBUG_TYPE "function-call-finder"
#include <KIARA/Common/Config.hpp>
#include <KIARA/LLVM/DeepCalleeFinder.hpp>
#include <KIARA/LLVM/Utils.hpp>

#include <llvm/Pass.h>
#include "llvm/Config/llvm-config.h"
#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#else
#include <llvm/Function.h>
#include <llvm/Module.h>
#include <llvm/Support/IRBuilder.h>
#endif
#include <llvm/Analysis/CallGraph.h>
#include <llvm/ADT/SCCIterator.h>

#include <llvm/Support/CallSite.h>
#include <llvm/Support/raw_ostream.h>

#include <KIARA/LLVM/EnableLLVMDebug.hpp>

#include <iostream>
#include <stack>

namespace KIARA
{

char DeepCalleeFinder::ID = 0;

// run - Print out SCCs in the call graph for the specified module.
bool DeepCalleeFinder::runOnModule(llvm::Module &M)
{
    std::stack< llvm::Function* > nodeStack;

	for(FunctionSet::iterator itRootFunc = deepCallees.begin(); itRootFunc != deepCallees.end(); ++itRootFunc)
	{
		nodeStack.push(*itRootFunc);
	}

	while ( !nodeStack.empty() )
	{
		llvm::Function * caller = nodeStack.top();
		nodeStack.pop();

		for (llvm::Function::iterator itBlock = caller->begin(); itBlock != caller->end(); itBlock++)
		{
			for(llvm::BasicBlock::iterator itInst = itBlock->begin(); itInst != itBlock->end(); ++itInst)
			{

			    llvm::CallSite CS = llvm::CallSite(itInst);

			    if (!CS)
			        continue;

			    llvm::Function * callee = CS.getCalledFunction();

			    if (callee)
			    {
			        if (deepCallees.insert(callee).second)
			        {
			            nodeStack.push(callee);
			        }
			    }
			}
		}
	}

	return false;
}

// getAnalysisUsage - This pass requires the CallGraph.
void DeepCalleeFinder::getAnalysisUsage(llvm::AnalysisUsage &AU) const
{
    AU.setPreservesAll();
}

} // namespace KIARA
