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
 * CallSiteFiner.cpp
 *
 *  Created on: May 19, 2010
 *      Author: Dmitri Rubinstein
 */

#define KIARA_LIB
#define DEBUG_TYPE "call-site-finder"
#include <KIARA/Common/Config.hpp>
#include <KIARA/LLVM/CallSiteFinder.hpp>

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

#include <llvm/Support/CallSite.h>
#include <llvm/Support/raw_ostream.h>

#include <KIARA/LLVM/EnableLLVMDebug.hpp>

#include <iostream>

namespace KIARA
{

char CallSiteFinder::ID = 0;

CallSiteFinder::~CallSiteFinder() { }

bool CallSiteFinder::runOnFunction(llvm::Function &F)
{
    for (llvm::Function::iterator b = F.begin(), be = F.end(); b != be; ++b)
    {
        for (llvm::BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; ++i)
        {
            llvm::CallSite cs = llvm::CallSite(i);
            if (cs)
            {
                // We know we've encountered a call instruction, so we
                // need to determine if it's a call to the
                // function pointed to by func or not.
                if (cs.getCalledFunction() == func)
                    foundCallSites.push_back(cs);
            }
        }
    }

    return true;
}

char CallSiteFinderOfSet::ID = 0;

CallSiteFinderOfSet::~CallSiteFinderOfSet() { }

bool CallSiteFinderOfSet::runOnFunction(llvm::Function &F)
{
    for (llvm::Function::iterator b = F.begin(), be = F.end(); b != be; ++b)
    {
        for (llvm::BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; ++i)
        {
            llvm::CallSite cs = llvm::CallSite(i);
            if (cs)
            {
                // We know we've encountered a call instruction, so we
                // need to determine if it's a call to the
                // function in the set or not.
                if (funcSet.find(cs.getCalledFunction()) != funcSet.end())
                    foundCallSites.push_back(cs);
            }
        }
    }

    return true;
}

} // namespace KIARA
