/*
 * llvmInlining.cpp
 *
 *  Created on: Mar 5, 2010
 *      Author: Ralf Karrenberg, Dmitri Rubinstein
 */

#define KIARA_LIB
#define DEBUG_TYPE "kiara-inliner"
#include <KIARA/Common/Config.hpp>
#include "Inlining.hpp"

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instructions.h>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/Support/CallSite.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/Support/Debug.h>

#include "EnableLLVMDebug.hpp"
//#undef DEBUG
//#define DEBUG(X) X

namespace KIARA
{

bool llvmInlineAllFunctionCalls(llvm::Function * f)
{
    assert (f && "function cannot be NULL");

    DEBUG(llvm::dbgs() << "inlineFunctionCalls(function '"<<f->getName()<<"')"; );

    bool callsInlined = false;
    bool functionChanged = true;
    while (functionChanged)
    {
        functionChanged = false;
        for (llvm::Function::iterator BB=f->begin(); BB!=f->end(); ++BB)
        {
            bool blockChanged = false;
            for (llvm::BasicBlock::iterator I=BB->begin(); !blockChanged && I!=BB->end();)
            {
                if (!llvm::isa<llvm::CallInst>(I))
                {
                    ++I;
                    continue;
                }
                llvm::CallInst * call = llvm::cast<llvm::CallInst>(I++);
                llvm::Function * callee = call->getCalledFunction();
                if (!callee)
                {
                    //DEBUG(llvm::errs() << "ERROR: could not inline function call: " << *call;);
                    continue;
                }
                if (callee->getAttributes().hasAttrSomewhere(llvm::Attribute::NoInline))
                {
                    DEBUG(llvm::dbgs() << "    function '" << callee->getName().str() << "' has attribute 'no inline', ignored call!\n"; );
                    continue;
                }
                if (callee->isDeclaration())
                    continue;

                // we need to save name of a function as it might be deleted by
                // llvm::InlineFunction
                const std::string calleeName = callee->getName().str();

                llvm::InlineFunctionInfo IFI(NULL, NULL); //new TargetData(mod));
                blockChanged = llvm::InlineFunction(call, IFI);

                DEBUG(
                    if (blockChanged)
                        llvm::dbgs() << "  inlined call to function '" << calleeName << "'\n";
                    else
                        llvm::errs() << "  inlining of call to function '" << calleeName << "' FAILED!\n";
                );
                functionChanged |= blockChanged;
                callsInlined |= blockChanged;
            }
        }
    }
    DEBUG( llvm::dbgs() << "done.\n"; );
    return callsInlined;
}

static inline void removeFnAttr(llvm::Function *func, llvm::Attribute::AttrKind Attr)
{
    func->setAttributes(
        func->getAttributes().removeAttribute(
            func->getContext(), llvm::AttributeSet::FunctionIndex, Attr));
}

bool llvmInlineFunction(llvm::Function * func)
{
	bool success = false;
	assert(func);

	removeFnAttr(func, llvm::Attribute::NoInline);
	func->addFnAttr(llvm::Attribute::AlwaysInline);

	for(llvm::Value::use_iterator itUse = func->use_begin(); itUse != func->use_end(); ++itUse)
	{
		llvm::User * user = *itUse;

		if (llvm::CallInst * call = llvm::dyn_cast<llvm::CallInst>(user))
		{
            llvm::InlineFunctionInfo IFI(NULL, NULL); //new TargetData(mod));
            success |= llvm::InlineFunction(call, IFI);
		}
	}

	return success;
}

bool llvmInlineFunctionCallsInModule(llvm::Function *f, llvm::Module *mod)
{
    assert (f && mod);

    bool inlinedSomething = false;

    //scan through and identify all call sites ahead of time so that we only
    //inline call sites in the original functions, not call sites that result
    //from inlining other functions.
    //TODO: exploit Uses instead of iterating over all instructions to find calls
    //in contrast to BasicInliner we know the function we are going to inline
    std::vector<llvm::CallSite> CallSites;
    llvm::SmallPtrSet<llvm::Function *, 8> DeadFunctions;

    bool Changed = false;
    do
    {
        Changed = false;

        for (llvm::Module::iterator F = mod->begin(); F != mod->end(); ++F)
        {
            for (llvm::Function::iterator B = F->begin(); B != F->end(); ++B)
            {
                for (llvm::BasicBlock::iterator I = B->begin(); I != B->end(); ++I)
                {
                    llvm::CallSite CS = llvm::CallSite(I);
                    if (CS.getInstruction() && CS.getCalledFunction() &&
                            !CS.getCalledFunction()->isDeclaration())
                        CallSites.push_back(CS);
                }
            }
        }

        //iterate over all function-calls and insert code of f
        //at all its occurrences
        for (unsigned i=0; i != CallSites.size() && !CallSites.empty(); ++i)
        {
            llvm::CallSite CS = CallSites[i];
            if (llvm::Function* Callee = CS.getCalledFunction())
            {
		//aggressive inlining
		removeFnAttr(Callee, llvm::Attribute::NoInline);
		Callee->addFnAttr(llvm::Attribute::AlwaysInline);

                // Eliminate calls that are never inlinable.
                if (Callee->isDeclaration() ||
                        CS.getInstruction()->getParent()->getParent() == Callee)
                {
                    CallSites.erase(CallSites.begin() + i);
                    --i;
                    if (Callee->getName().str() == f->getName().str())
                    {
                        llvm::errs() << "WARNING: Function '" << f->getName().str() << "' can not be inlined!\n";
                    }
                    continue;
                }

                if (Callee->getName().str() == f->getName().str()) //TODO: is that comparison enough?
                {
                    llvm::InlineFunctionInfo IFI(NULL, NULL); //new TargetData(mod));

                    if (llvm::InlineFunction(CS, IFI)) //2nd NULL = TargetData*
                    {
                        if (Callee->use_empty() && Callee->hasInternalLinkage())
                            DeadFunctions.insert(Callee);
                        Changed = true;
                        CallSites.erase(CallSites.begin() + i);
                        --i;
                    }
                }

            }

        } //for all CS

        inlinedSomething |= Changed;

    } while (Changed);


    //remove completely inlined functions from module.
    for (llvm::SmallPtrSet<llvm::Function *, 8 >::iterator I = DeadFunctions.begin(),
             E = DeadFunctions.end(); I != E; ++I)
    {
        llvm::Function *D = *I;
        mod->getFunctionList().remove(D);
        //D->eraseFromParent(); //TODO: test this instead...
    }

    //    outs() << "done.\n";
    return inlinedSomething;
}

} // namespace KIARA
