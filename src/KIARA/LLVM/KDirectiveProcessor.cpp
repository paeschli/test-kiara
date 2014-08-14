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
 * KDirectiveProcessor.cpp
 *
 *  Created on: Dec 30, 2013
 *      Author: Dmitri Rubinstein
 */

#define KIARA_LIB
#include <KIARA/Common/Config.hpp>

#include "KDirectiveProcessor.hpp"
#include <KIARA/LLVM/CallSiteFinder.hpp>
#include <KIARA/LLVM/Utils.hpp>

#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Constants.h>
#include <llvm/PassManager.h>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <iostream>

/// extern "C" void ar_info*(void);
/// contains informational calls
#define KIARA_OP_INFO_PREFIX "kiara_info"

/// KIARA directive prefix
#define KIARA_DR_PREFIX "kiara_"

#define KIARA_DR_EXPORT_TYPE_PREFIX "kiara_export_type_"
#define KIARA_DR_COMMAND "kiara_command"

namespace KIARA
{

KDirectiveProcessor::KDirectiveProcessor(llvm::Module &module)
    : module_(module)
{
    process();
}

KDirectiveProcessor::~KDirectiveProcessor()
{
}

void KDirectiveProcessor::process()
{
    // iterate over all info functions, extract AnySL-metadata and
    // then delete them

    CallSiteFinderOfSet::FunctionSet fset;

    for (llvm::Module::iterator it = module_.begin(),
            e = module_.end(); it != e; ++it)
    {
        if (it->getName().startswith(KIARA_OP_INFO_PREFIX) && !it->isDeclaration())
        {
            analyzeInfoFunc(*it);
            fset.insert(it);
        }
    }

    CallSiteFinderOfSet *finder = new CallSiteFinderOfSet(fset);

    llvm::PassManager passes;
    passes.add(finder);
    passes.run(module_);

    const CallSiteFinderOfSet::CallSiteVec &callSites = finder->getFoundCallSites();

    // remove all calls to info functions
    for(CallSiteFinderOfSet::CallSiteVec::const_iterator it = callSites.begin(),
        et = callSites.end(); it != et; ++it)
    {
        (*it)->removeFromParent();
    }

    // remove all info functions
    for (CallSiteFinderOfSet::FunctionSet::const_iterator it = fset.begin(),
            et = fset.end(); it != et; ++it)
    {
        (*it)->eraseFromParent();
    }

    // analyze metadata
    llvm::NamedMDNode *kiaraMD = module_.getNamedMetadata("kiara");
    if (kiaraMD)
    {
        for (unsigned int i = 0; i < kiaraMD->getNumOperands(); ++i)
        {
            llvm::MDNode *mdnode = kiaraMD->getOperand(i);
            KDirective kd;
            if (kd.initFromMDNode(mdnode))
                processDirective(kd);
        }
    }
}

void KDirectiveProcessor::addExportTypeMapping(const std::string &typeName, llvm::Type *type)
{
    typeMap_[typeName] = type;
}

void KDirectiveProcessor::analyzeInfoFunc(const llvm::Function &arinfo)
{
    for (llvm::Function::const_iterator BB=arinfo.begin(), BBE=arinfo.end(); BB!=BBE; ++BB)
    {
        for (llvm::BasicBlock::const_iterator I=BB->begin(), IE=BB->end(); I!=IE; )
        {
            if (!llvm::isa<llvm::CallInst>(I))
            {
                ++I;
                continue;
            }
            const llvm::CallInst* call = llvm::cast<llvm::CallInst>(I++);
            const llvm::Function* callee = call->getCalledFunction();
            if (!callee)
            {
                //outs() << "call can not be analyzed: " << *call;
                continue;
            }

            const std::string calleeName = callee->getName().str();

            // create directive if calleeName starts with "kiara_"
            if (callee->getName().startswith(KIARA_DR_PREFIX))
            {
                KDirective kd(calleeName);
                kd.addArgumentsFromCall(call);
                processDirective(kd);
            }
        }
    }
}

void KDirectiveProcessor::processDirective(const KDirective &directive)
{
    if (boost::algorithm::starts_with(directive.getName(), KIARA_DR_EXPORT_TYPE_PREFIX))
    {
        processExportTypeDirective(directive);
    }
    else if (directive.getName() == KIARA_DR_COMMAND)
    {
        processCommandDirective(directive);
    }
}

bool KDirectiveProcessor::processExportTypeDirective(const KDirective &directive, std::string *errorMsg)
{
    // kiara_export_type(Null of Type*, TypeName as string)

    if (directive.getNumArguments() != 2)
    {
        if (errorMsg)
            *errorMsg =
                "Incorrect number of arguments in " +
                boost::lexical_cast<std::string>(directive) +
                ": " +
                boost::lexical_cast<std::string>(directive.getNumArguments()) +
                ", should be two.";
        return false;
    }

    if (directive.getArgument(0).getType() != KDArgument::NULL_PTR_ARG)
    {
        if (errorMsg)
            *errorMsg = "first argument to " + boost::lexical_cast<std::string>(directive) +
                        " directive is not a null pointer constant.";
        return false;
    }

    if (directive.getArgument(1).getType() != KDArgument::STRING_ARG)
    {
        if (errorMsg)
            *errorMsg = "second argument to " + boost::lexical_cast<std::string>(directive) +
                        " directive is not a string constant.";
        return false;
    }

    // 0 - Type*

    // keep the pointer when handling arrays
    llvm::PointerType * ptrType = directive.getArgument(0).getNullPtr().type;
    llvm::Type *etype = ptrType->getElementType();
    std::string typeSymbol = directive.getArgument(1).getString();

    if (etype)
    {
        addExportTypeMapping(typeSymbol, etype);
    }

    return true;
}

bool KDirectiveProcessor::processCommandDirective(const KDirective &directive, std::string *errorMsg)
{
    // kiara_command(string)

    if (directive.getNumArguments() != 1)
    {
        if (errorMsg)
            *errorMsg =
                "Incorrect number of arguments in " +
                boost::lexical_cast<std::string>(directive) +
                ": " +
                boost::lexical_cast<std::string>(directive.getNumArguments()) +
                ", should be one.";
        return false;
    }

    if (directive.getArgument(0).getType() != KDArgument::STRING_ARG)
    {
        if (errorMsg)
            *errorMsg = "first argument to " + boost::lexical_cast<std::string>(directive) +
                        " directive is not a string constant.";
        return false;
    }

    commands_.push_back(directive.getArgument(0).getString());

    return true;
}

void KDirectiveProcessor::applyCommands(llvm::Module &module)
{
    for (StringList::const_iterator it = commands_.begin(), end = commands_.end(); it != end; ++it)
    {
        if (*it == "llvmMarkFunctionsWithAlwaysInline")
            llvmMarkFunctionsWithAlwaysInline(&module);
        else if (*it == "llvmMarkFunctionsWithNoInlineAndRemoveAlwaysInlineAttr")
            llvmMarkFunctionsWithNoInline(&module, true);
        else if (*it == "llvmMarkFunctionsWithNoInline")
            llvmMarkFunctionsWithNoInline(&module, false);
        else if (*it == "llvmAnnotateFunctionEntersAndExitsWithPuts")
            llvmAnnotateFunctionEntersAndExits(&module, "puts");
    }
}

void KDirectiveProcessor::dump() const
{
    std::cerr<<"TypeMap {"<<std::endl;
    for (TypeMap::const_iterator it = typeMap_.begin(), end = typeMap_.end(); it != end; ++it)
    {
        std::cerr << "  " << it->first << " : " << llvmToString(it->second) << " PTR " << it->second << std::endl;
    }
    std::cerr<<"}"<<std::endl;
    std::cerr<<"Commands ["<<std::endl;
    for (StringList::const_iterator it = commands_.begin(), end = commands_.end(); it != end; ++it)
    {
        std::cerr << "  " << *it << std::endl;
    }
    std::cerr<<"]"<<std::endl;
}

} // namespace KIARA
