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
 * remove_asm.cpp
 *
 *  Created on: 31.10.2012
 *      Author: Dmitri Rubinstein
 */

#include <KIARA/Core/Exception.hpp>
#include <KIARA/LLVM/Utils.hpp>

// LLVM-specific headers
#include "llvm/Config/llvm-config.h"
#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
#include <llvm/IR/Module.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/DataLayout.h>
#else
#include <llvm/Module.h>
#include <llvm/Instructions.h>
#include <llvm/Target/TargetData.h>
#endif
#include <llvm/Transforms/Utils/BuildLibCalls.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/InstIterator.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Host.h>

#include <iostream>
#include <set>

using namespace std;
using namespace KIARA;

// help

void showUsage()
{
    cout<<"Remove function definitions containing asm instructions"<<endl
        <<"Usage:"<<endl
        <<"kiara-remove-asm [options] bc-filename\n"
        <<"\n"
        <<"  -h    | -help | --help         : prints this and exit\n"
        <<"  -v                             : verbose output\n"
        <<"  -a                             : annotate each function\n"
        <<"  -d SYMBOL                      : delete global symbol\n"
        <<"  -o filename                    : output to this file\n"
        <<endl;
}

// main

#define ARG(str) (!strcmp(argv[i], str))
#define ARG_STARTS_WITH(str,len) (!strncmp(argv[i], str, len))
#define ARG_CONTAINS(str) strstr(argv[i], str)
#define APP_ERROR(msg) { std::cerr<<msg<<std::endl; exit(1); }

class LibraryInit
{
public:

    LibraryInit()
    {
        llvmInitialize();
    }

    ~LibraryInit()
    {
        llvmShutdown();
    }
};

void deleteFunction(llvm::Function *f, bool verbose)
{
    if (verbose)
        std::cerr<<f->getName().str();

    f->deleteBody();

    if (f->getNumUses() == 0)
    {
        if (verbose)
            std::cerr<<" Deleted function"<<std::endl;
        f->eraseFromParent();
    }
    else
    {
        if (verbose)
            std::cerr<<" Deleted body"<<std::endl;
    }
}

#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
/// EmitPutS - Emit a call to the puts function.  This assumes that Str is
/// some pointer.
llvm::CallInst *MyEmitPutS(llvm::Value *Str, llvm::IRBuilder<> &B, const llvm::DataLayout *TD) {
  llvm::Module *M = B.GetInsertBlock()->getParent()->getParent();
  llvm::AttributeSet AS[2];
  AS[0] = llvm::AttributeSet::get(M->getContext(), 1, llvm::Attribute::NoCapture);
  AS[1] = llvm::AttributeSet::get(M->getContext(), llvm::AttributeSet::FunctionIndex,
				  llvm::Attribute::NoUnwind);

  llvm::Value *PutS = M->getOrInsertFunction("kr_puts",
					     llvm::AttributeSet::get(M->getContext(), AS),
					     B.getInt32Ty(),
					     B.getInt8PtrTy(),
					     NULL);
  llvm::CallInst *CI = B.CreateCall(PutS, CastToCStr(Str, B), "kr_puts");
  if (const llvm::Function *F = llvm::dyn_cast<llvm::Function>(PutS->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());
  return CI;
}
#else
/// EmitPutS - Emit a call to the puts function.  This assumes that Str is
/// some pointer.
llvm::CallInst * MyEmitPutS(llvm::Value *Str, llvm::IRBuilder<> &B, const llvm::TargetData *TD) {
  llvm::Module *M = B.GetInsertBlock()->getParent()->getParent();
  llvm::AttributeWithIndex AWI[2];
  AWI[0] = llvm::AttributeWithIndex::get(1, llvm::Attribute::NoCapture);
  AWI[1] = llvm::AttributeWithIndex::get(~0u, llvm::Attribute::NoUnwind);

  llvm::Value *PutS = M->getOrInsertFunction("kr_puts", llvm::AttrListPtr::get(AWI, 2),
                                       B.getInt32Ty(),
                                       B.getInt8PtrTy(),
                                       NULL);
  llvm::CallInst *CI = B.CreateCall(PutS, llvm::CastToCStr(Str, B), "kr_puts");
  if (const llvm::Function *F = llvm::dyn_cast<llvm::Function>(PutS->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());
  return CI;
}
#endif

void annotateFuncs(llvm::Module *module)
{
    std::string Error;
    std::string hostTriple = llvm::sys::getDefaultTargetTriple();

    const llvm::Target *hostTarget =
        llvm::TargetRegistry::lookupTarget(hostTriple, Error);
    if (!hostTarget)
    {
        llvm::errs()<<"Error: "<<Error<<"\n";
        return;
    }

    llvm::TargetOptions options;
    llvm::TargetMachine *tm =
            hostTarget->createTargetMachine(hostTriple, "", "", options);
    if (!tm)
    {
        llvm::errs()<<"Error: Could not create target machine\n";
        return;
    }

#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
    const llvm::DataLayout* dataLayout = tm->getDataLayout();
    if (!dataLayout)
    {
        llvm::errs()<<"Error: Could not get data layout\n";
        return;
    }
#else
    const llvm::TargetData* targetData = tm->getTargetData();
    if (!targetData)
    {
        llvm::errs()<<"Error: Could not get target data\n";
        return;
    }
#endif

    std::string text;
    for (llvm::Module::iterator F = module->begin(), FE = module->end(); F != FE; ++F)
    {
        if (F->isDeclaration())
            continue;

        llvm::BasicBlock *BB = F->begin();
        llvm::Instruction *I1 = BB->begin();

        //llvm::errs() << F->getName() << "\n";
        //llvm::errs() << BB->getName() << "\n";
        //llvm::errs() << I1->getName() << "\n";

        if (I1->hasName() &&
            (I1->getName().find("print_annotation") != llvm::StringRef::npos))
            continue;
        llvm::IRBuilder<> Builder(BB, BB->begin());
        text = F->getName();
        text.append("() entered");
        llvm::Value *textValue = Builder.CreateGlobalStringPtr(text);
#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
        llvm::CallInst *CI = MyEmitPutS(textValue, Builder, dataLayout);
#else
        llvm::CallInst *CI = MyEmitPutS(textValue, Builder, targetData);
#endif

        CI->setName("print_annotation");

        for (llvm::Function::iterator BBI = F->begin(), BBE = F->end(); BBI != BBE; ++BBI)
        {
            llvm::TerminatorInst *TI = BBI->getTerminator();
            if (!TI)
                continue;
            if (llvm::ReturnInst *RI = llvm::dyn_cast<llvm::ReturnInst>(TI))
            {
                Builder.SetInsertPoint(RI);
                text = F->getName();
                text.append("() left");
                llvm::Value *textValue = Builder.CreateGlobalStringPtr(text);
#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
		llvm::CallInst *CI = MyEmitPutS(textValue, Builder, dataLayout);
#else
                llvm::CallInst *CI = MyEmitPutS(textValue, Builder, targetData);
#endif
                CI->setName("print_annotation");
            }
            else if (llvm::UnreachableInst *UI = llvm::dyn_cast<llvm::UnreachableInst>(TI))
            {
                Builder.SetInsertPoint(UI);
                text = F->getName();
                text.append("() left");
                llvm::Value *textValue = Builder.CreateGlobalStringPtr(text);
#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
                llvm::CallInst *CI = MyEmitPutS(textValue, Builder, dataLayout);
#else
                llvm::CallInst *CI = MyEmitPutS(textValue, Builder, targetData);
#endif
                CI->setName("print_annotation");
            }
        }
    }
}

void processModule(llvm::Module *module, const std::vector<const char *> &delSymbols, bool annotate, bool verbose)
{
    std::set<llvm::Function*> asmFuncs;
    for (llvm::Module::iterator F = module->begin(), FE = module->end(); F != FE; ++F)
    {
        // F is a pointer to a Function instance
        if (F->isDeclaration())
        {
            if (F->hasDLLImportLinkage())
                F->setLinkage(llvm::GlobalValue::ExternalLinkage);
            continue;
        }

        for (llvm::inst_iterator I = llvm::inst_begin(F), E = llvm::inst_end(F); I != E; ++I)
        {
            if (llvm::CallInst *CI = llvm::dyn_cast<llvm::CallInst>(&*I))
            {
                if (CI->isInlineAsm())
                {
                    asmFuncs.insert(F);
                    break;
                }
            }
        }
    }

    for (std::set<llvm::Function*>::iterator it = asmFuncs.begin(), end = asmFuncs.end();
            it != end; ++it)
    {
        deleteFunction(*it, verbose);
    }

    // Delete symbols
    for (std::vector<const char *>::const_iterator it = delSymbols.begin(), end = delSymbols.end();
            it != end; ++it)
    {
        if (llvm::Function *func = module->getFunction(*it))
        {
            deleteFunction(func, verbose);
        }
        else if (llvm::GlobalVariable *gvar = module->getNamedGlobal(*it))
        {
            if (gvar->getNumUses() == 0)
            {
                gvar->eraseFromParent();
                if (verbose)
                    std::cerr<<*it<<" Deleted global variable"<<std::endl;
            }
        }
    }

    if (annotate)
        annotateFuncs(module);
}

int main(int argc, char *argv[])
{
    const char *outputFile = 0;
    bool verbose = false;
    bool annotate = false;
    std::vector<const char *> files;
    std::vector<const char *> delSymbols;

    //typedef std::vector<const char *>::iterator Iter;

    // parse command line
    int i;
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (ARG("-help") || ARG("--help") || ARG("-h"))
            {
                showUsage();
                exit(0);
            }
            else if (ARG("-v"))
            {
                verbose = true;
            }
            else if (ARG("-a"))
            {
                annotate = true;
            }
            else if (ARG("-o"))
            {
                if (++i < argc)
                {
                    outputFile = argv[i];
                }
                else
                {
                    APP_ERROR("-o option require argument");
                }
            }
            else if (ARG("-d"))
            {
                if (++i < argc)
                {
                    delSymbols.push_back(argv[i]);
                }
                else
                {
                    APP_ERROR("-d option require argument");
                }
            }
            else if (ARG("--"))
            {
                i++;
                break;
            }
            else
            {
                APP_ERROR("Unknown option "<<argv[i]);
            }
        }
        else
        {
            // no '-' prefix, assume this is a file name
            files.push_back(argv[i]);
        }
    }

    // read rest files
    for (; i < argc; i++)
    {
        files.push_back(argv[i]);
    }

    LibraryInit init;
    if (files.size() == 0)
    {
        cerr<<"No file specified\n"<<endl;
        showUsage();
        exit(1);
    }

    if (files.size() > 1)
    {
        cerr<<"Only a single file is allowed\n"<<endl;
        showUsage();
        exit(1);
    }

    llvm::Module *module = 0;

    const char *infile = files[0];

    try
    {
        std::string errorMsg;
        module = llvmLoadModuleFromFile(infile, errorMsg);

        if (!module)
        {
            cerr<<"Could not load module from file : "<<infile<<endl;
            cerr<<errorMsg<<endl;
            exit(1);
        }
        processModule(module, delSymbols, annotate, verbose);
    }
    catch (std::exception &e)
    {
        cerr<<"Error: "<<e.what()<<endl;
        return 1;
    }
    std::string errorMsg;
    if (outputFile)
    {
        llvmWriteModuleToFile(module, outputFile, errorMsg);
    }
    else
    {
        llvmWriteModuleToStdout(module, errorMsg);
    }

    return 0;
}
