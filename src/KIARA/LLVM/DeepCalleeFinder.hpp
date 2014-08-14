/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2012  German Research Center for Artificial Intelligence (DFKI)
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
 * DeepCalleeFinder.hpp
 *
 *  Created on: Aug 26, 2010
 *      Author: Simon Moll
 */

#ifndef KIARA_LLVM_DEEPCALLEEFINDER_HPP_INCLUDED
#define KIARA_LLVM_DEEPCALLEEFINDER_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <set>
#include <llvm/Pass.h>

namespace llvm
{
    class Function;
}

namespace KIARA
{

class DeepCalleeFinder : public llvm::ModulePass
{
public:

    typedef std::set<llvm::Function*> FunctionSet;

    static char ID; // Pass identification, replacement for typeid

    DeepCalleeFinder(const FunctionSet &initialSet) :
        llvm::ModulePass(ID),
        deepCallees(initialSet)
    {}

    bool runOnModule(llvm::Module &M);

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;

    const FunctionSet &getDeepCallees() const { return deepCallees; }

private:
    FunctionSet deepCallees;
};

} // namespace KIARA

#endif /* KIARA_LLVM_HPP_INCLUDED */
