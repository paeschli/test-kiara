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
 * Utils.cpp
 *
 *  Created on: 25.11.2011
 *      Author: Dmitri Rubinstein
 */

#define KIARA_LIB
#define DONT_GET_PLUGIN_LOADER_OPTION
#include <KIARA/Common/Config.hpp>

#include <KIARA/LLVM/Utils.hpp>
#include <KIARA/LLVM/DeepCalleeFinder.hpp>
#include <KIARA/LLVM/CallSiteFinder.hpp>
#include <DFC/Utils/StrUtils.hpp>

#include <KIARA/LLVM/Utils.hpp>

#include <llvm/PassRegistry.h>
#include <llvm/PassManager.h>
#include <llvm/InitializePasses.h>
#include "llvm/Config/llvm-config.h"
#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>
#else
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Type.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Instructions.h>
#endif
#include <llvm/Linker.h>
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/ADT/OwningPtr.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/PluginLoader.h>
#include <llvm/Support/Host.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/system_error.h>
#include "llvm/Support/DynamicLibrary.h"
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Transforms/Utils/BuildLibCalls.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/Assembly/Parser.h>
#include "AsmParser/Parser.h"

#include <cerrno>
#include <cstdio>

// For symbols
#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <conio.h>
//#include <WinSock2.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

namespace KIARA
{

void llvmInitialize()
{
    // Initialize LLVM

    // Initialize LLVM passes
    llvm::PassRegistry &Registry = *llvm::PassRegistry::getPassRegistry();
    llvm::initializeCore(Registry);
    llvm::initializeScalarOpts(Registry);
    llvm::initializeIPO(Registry);
    llvm::initializeAnalysis(Registry);
    llvm::initializeIPA(Registry);
    llvm::initializeTransformUtils(Registry);
    llvm::initializeInstCombine(Registry);
    llvm::initializeInstrumentation(Registry);
    llvm::initializeTarget(Registry);

    // Initialize targets first, so that --version shows registered targets.
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmPrinters();
    llvm::InitializeAllAsmParsers();
}

void llvmInitializeJIT()
{
    llvm::InitializeNativeTarget();

#ifdef _WIN32
    // Add symbols that are missing on Windows
    llvm::sys::DynamicLibrary::AddSymbol("malloc", (void*)(intptr_t)malloc);
    llvm::sys::DynamicLibrary::AddSymbol("calloc", (void*)(intptr_t)calloc);
    llvm::sys::DynamicLibrary::AddSymbol("realloc", (void*)(intptr_t)realloc);
    llvm::sys::DynamicLibrary::AddSymbol("strdup", (void*)(intptr_t)strdup);
    llvm::sys::DynamicLibrary::AddSymbol("free", (void*)(intptr_t)free);

    llvm::sys::DynamicLibrary::AddSymbol("fileno", (void*)(intptr_t)fileno);
    llvm::sys::DynamicLibrary::AddSymbol("open", (void*)(intptr_t)open);
    llvm::sys::DynamicLibrary::AddSymbol("close", (void*)(intptr_t)close);
    llvm::sys::DynamicLibrary::AddSymbol("isatty", (void*)(intptr_t)isatty);
    llvm::sys::DynamicLibrary::AddSymbol("strcmpi", (void*)(intptr_t)strcmpi);
    llvm::sys::DynamicLibrary::AddSymbol("lseek", (void*)(intptr_t)lseek);
    llvm::sys::DynamicLibrary::AddSymbol("read", (void*)(intptr_t)read);
    llvm::sys::DynamicLibrary::AddSymbol("write", (void*)(intptr_t)write);
    llvm::sys::DynamicLibrary::AddSymbol("access", (void*)(intptr_t)access);
    llvm::sys::DynamicLibrary::AddSymbol("mkdir", (void*)(intptr_t)mkdir);
    llvm::sys::DynamicLibrary::AddSymbol("getch", (void*)(intptr_t)getch);
    llvm::sys::DynamicLibrary::AddSymbol("_snprintf", (void*)(intptr_t)_snprintf);
#endif
}

void llvmShutdown()
{
    llvm::llvm_shutdown();
}

void llvmDeleteModule(llvm::Module *module)
{
    delete module;
}

bool llvmLinkModules(
        llvm::Module *dest, llvm::Module *src,
        std::string &errorMsg)
{
    return !llvm::Linker::LinkModules(dest, src, llvm::Linker::DestroySource, &errorMsg);
}

bool llvmLinkNativeFunc(
        llvm::Module * module,
        llvm::ExecutionEngine * engine,
        const std::string &funcName,
        void * nativeFunc)
{
    assert(nativeFunc != 0 && "null is not a valid native function");

    llvm::Function * funcDecl = module->getFunction(funcName);

    if (!funcDecl)
        return false;

    engine->addGlobalMapping(funcDecl, reinterpret_cast<void*>(nativeFunc));
    return true;
}

bool llvmIsFunctionGenerated(
        llvm::Module * module,
        llvm::ExecutionEngine * engine,
        const std::string &funcName)
{
    llvm::Function * funcDecl = module->getFunction(funcName);

    if (!funcDecl)
        return false;

    return engine->getPointerToGlobalIfAvailable(funcDecl) != 0;
}

bool llvmGetConstStringArg(const llvm::Value *value, std::string &valueStr)
{
    if (const llvm::MDString *mdstring = llvm::dyn_cast<llvm::MDString>(value))
    {
        valueStr = mdstring->getString();
        return true;
    }

//FIXME re-enabled other section as this function does not exist per LLVM 3.1
#if 0
    if (llvm::GetConstantStringInfo(value, valueStr))
        return true;

    return false;
#endif

//#if 0
    // possibly following is unnecessary as llvm::GetConstantStringInfo
    // already covers following case
    if (!llvm::isa<llvm::ConstantExpr>(value))
        return false;

    const llvm::ConstantExpr * constExpr = llvm::cast<llvm::ConstantExpr>(value);

    if (!llvm::isa<llvm::Constant>(constExpr->getOperand(0)))
        return false;
    if (!llvm::isa<llvm::Constant>(constExpr->getOperand(0)->getOperand(0)))
        return false;

    const llvm::Constant * stringConst = llvm::cast<llvm::Constant>(
            constExpr->getOperand(0)->getOperand(0));
    if (!llvm::isa<llvm::ConstantDataSequential>(stringConst))
    {
        valueStr = "";
        return true;
    }
    //assert (llvm::isa<llvm::ConstantArray>(stringConst)); //fails for [ 1x i8* ] zeroinitializer
    //FIXME
    const llvm::ConstantDataSequential * stringConstSeq = llvm::cast<llvm::ConstantDataSequential>(stringConst);
    valueStr = stringConstSeq->getAsString();
    return true;
//#endif
}

bool llvmGetConstIntArg(const llvm::Value *value, int64_t &valueInt)
{
    if (!llvm::isa<llvm::ConstantInt>(value))
        return false;

    const llvm::ConstantInt * constInt = llvm::cast<llvm::ConstantInt>(value);
    if (constInt->getBitWidth() > 64)
        return false; // constant is too big

    valueInt = constInt->getSExtValue();

    return true;
}

bool llvmGetConstUIntArg(const llvm::Value *value, uint64_t &valueInt)
{
    if (!llvm::isa<llvm::ConstantInt>(value))
        return false;

    const llvm::ConstantInt * constInt = llvm::cast<llvm::ConstantInt>(value);
    if (constInt->getBitWidth() > 64)
        return false; // constant is too big

    valueInt = constInt->getZExtValue();

    return true;
}

bool llvmGetConstFloatArg(const llvm::Value *value, double &valueFloat)
{
    if (!llvm::isa<llvm::ConstantFP>(value))
        return false;

    const llvm::ConstantFP * CFP = llvm::cast<llvm::ConstantFP>(value);

    const bool isDouble = &CFP->getValueAPF().getSemantics() == &llvm::APFloat::IEEEdouble;
    const bool isFloat = &CFP->getValueAPF().getSemantics() == &llvm::APFloat::IEEEsingle;

    if (isDouble || isFloat)
    {
        valueFloat = (isDouble ?
                CFP->getValueAPF().convertToDouble() :
                CFP->getValueAPF().convertToFloat());
    }
    else
    {
        bool ignored;
        llvm::APFloat apf(CFP->getValueAPF());
        apf.convert(llvm::APFloat::IEEEdouble, llvm::APFloat::rmNearestTiesToEven, &ignored);
        valueFloat = apf.convertToDouble();
    }

    return true;
}


llvm::Module * llvmLoadModuleFromFile(
        const std::string &fileName,
        std::string &errorMsg,
        llvm::LLVMContext *context)
{
    //create memory buffer for file
    llvm::OwningPtr<llvm::MemoryBuffer> fileBuffer;
    llvm::error_code e = llvm::MemoryBuffer::getFile(fileName.c_str(), fileBuffer);

    if (e)
    {
        errorMsg = "Error reading '" + fileName + "': " + e.message();
        return 0;
    }

    if (!fileBuffer)
    {
        errorMsg = "Error reading '" + fileName + "'";
        return 0;
    }

    if (fileBuffer->getBufferSize() & 3)
    {
        errorMsg = "Error: Bitcode stream should be a multiple of 4 bytes in length";
        return 0;
    }

    //parse file

    llvm::Module* m =
        llvm::ParseBitcodeFile(
                fileBuffer.get(),
                context ? *context : llvm::getGlobalContext(),
                &errorMsg);

    if (errorMsg != "")
    {
        errorMsg = "Error reading bitcode file: " + errorMsg;
        return 0;
    }

    if (!m)
    {
        errorMsg = "Error reading bitcode file";
        return 0;
    }

    return m;
}

bool llvmWriteModuleToFile(
        llvm::Module *module, const std::string &fileName,
        std::string &errorMsg)
{
    if (!module)
    {
        errorMsg = "NULL module";
        return false;
    }

    errorMsg = "";
    llvm::raw_fd_ostream file(fileName.c_str(), errorMsg, llvm::raw_fd_ostream::F_Binary);
    llvm::WriteBitcodeToFile(module, file);
    file.close();

    return errorMsg.length() == 0;
}

bool llvmWriteModuleToStdout(
        llvm::Module *module,
        std::string &errorMsg)
{
    if (!module)
    {
        errorMsg = "NULL module";
        return false;
    }

    llvm::WriteBitcodeToFile(module, llvm::outs());

    return true;
}

bool llvmWriteModuleAssemblyToFile(
        const llvm::Module *module,
        const std::string &fileName,
        std::string &errorMsg)
{
    if (!module)
    {
        errorMsg = "NULL module";
        return false;
    }

    llvm::raw_fd_ostream file(fileName.c_str(), errorMsg);
    module->print(file, NULL);
    file.close();

    return errorMsg.length() == 0;
}

bool llvmParseAssemblyString(
        const char *asmString, llvm::Module &module, std::string &errorMsg)
{
    llvm::SMDiagnostic error;
    if (!llvm::ParseAssemblyString(asmString, &module, error, module.getContext()))
    {
        llvm::raw_string_ostream os(errorMsg);
        error.print("", os);
        return false;
    }
    return true;
}

bool llvmExtParseAssemblyString(
        const char *asmString, llvm::Module &module, std::string &errorMsg)
{
    llvm::SMDiagnostic error;
    if (!llvm::kiara::ParseAssemblyString(asmString, &module, error, module.getContext()))
    {
        llvm::raw_string_ostream os(errorMsg);
        error.print("", os);
        return false;
    }
    return true;
}

KIARA_API llvm::Module * llvmParseAssemblyString(
        const char *asmString, std::string &errorMsg, llvm::LLVMContext &context)
{
    llvm::SMDiagnostic error;
    llvm::Module *module = llvm::ParseAssemblyString(asmString, 0, error, context);
    if (!module)
    {
        llvm::raw_string_ostream os(errorMsg);
        error.print("", os);
        return 0;
    }
    return module;
}

llvm::Module * llvmParseBitcodeString(
        const char *name, const char *start, size_t size,
        std::string &errorMsg, llvm::LLVMContext &context)
{
    llvm::Module *module = 0;
    llvm::StringRef memRef(start, size);

    llvm::MemoryBuffer *buffer = llvm::MemoryBuffer::getMemBuffer(memRef, name, false);
    if (buffer != 0)
    {
        module = llvm::ParseBitcodeFile(buffer, context, &errorMsg);
        delete buffer;
    }

    return module;
}


llvm::LLVMContext & llvmGetGlobalContext()
{
    return llvm::getGlobalContext();
}

bool llvmHasParent(llvm::Value *V)
{
    if (llvm::Instruction *I = llvm::dyn_cast<llvm::Instruction>(V))
        return I->getParent();
    else if (llvm::BasicBlock *BB = llvm::dyn_cast<llvm::BasicBlock>(V))
        return BB->getParent();
    else if (llvm::Function *F = llvm::dyn_cast<llvm::Function>(V))
        return F->getParent();
    else if (llvm::GlobalValue *GV = llvm::dyn_cast<llvm::GlobalValue>(V))
        return GV->getParent();
    else if (llvm::Argument *A = llvm::dyn_cast<llvm::Argument>(V))
        return A->getParent();
    else
    {
        assert(llvm::isa<llvm::Constant>(V) && "Unknown value type!");
        return false;
    }
    return false;
}

std::string llvmToString(const llvm::Value *value)
{
    if (value)
    {
        std::string r;
        llvm::raw_string_ostream ss(r);
        ss<<*value;
        return ss.str();
    }
    return "<null>";
}

std::string llvmToString(llvm::Type *type)
{
    if (type)
    {
        std::string r;
        llvm::raw_string_ostream ss(r);
        ss<<*type;
        return ss.str();
    }
    return "<null>";
}

KIARA_API std::string llvmGetTypeName(llvm::Type *type)
{
    if (!type)
        return "";
    llvm::StructType *sty = llvm::dyn_cast<llvm::StructType>(type);
    if (sty && sty->hasName())
        return sty->getName();
    return llvmToString(type);
}

KIARA_API std::string llvmGetTypeIdentifier(llvm::Type *type)
{
    if (!type)
        return "";
    llvm::StructType *sty = llvm::dyn_cast<llvm::StructType>(type);
    if (sty && sty->hasName())
        return ("%" + DFC::StrUtils::quoted(sty->getName().str()));
    return llvmToString(type);
}

llvm::Type * llvmUpCast(llvm::PointerType *ptype)
{
    return ptype;
}

const llvm::Type * llvmUpCast(const llvm::PointerType *ptype)
{
    return ptype;
}

bool llvmVerifyModule(llvm::Module * module, std::string &errorMsg)
{
    assert(module && "module must not be NULL");

    std::string moduleName = module->getModuleIdentifier();

    //check for references to other modules
    for (llvm::Module::iterator func = module->begin(), E = module->end();
         func != E;
         ++func)
    {
        for (llvm::Function::iterator block = func->begin();
             block != func->end();
             ++block)
        {
            for (llvm::BasicBlock::iterator inst = block->begin();
                 inst != block->end();
                 ++inst)
            {
                //calls
                if (llvm::isa<llvm::CallInst>(inst))
                {
                    llvm::CallInst * callInst = llvm::cast<llvm::CallInst>(inst);
                    llvm::Value * calledValue = callInst->getCalledValue();
                    if (!calledValue)
                    {
                        //errs() << "ERROR: " << func->getNameStr() << " has call to NULL "; callInst->dump();
                    }

                    if (llvm::isa<llvm::Function>(calledValue))
                    {
                        llvm::Function * callee = llvm::cast<llvm::Function>(calledValue);
                        llvm::Module * calleeMod = callee->getParent();

                        if (!calleeMod)
                        {
                            errorMsg = "function does not have a parent: " +llvmToString(callee);
                            return true;
                        }

                        std::string calleeModName = calleeMod->getModuleIdentifier();
                        std::string calleeName = callee->getName().str();

                        if (callee && (calleeMod != module))
                        {
                            //errs() << "ERROR: In module '" << moduleName << "': Called function '" << calleeName << "' is in module '" << calleeModName << "'.\n";
                            //inst->print(llvm::outs());
                            //exit(-1);
                            errorMsg = "Erroneous call: "+llvmToString(inst);
                            return true;
                        }
                    }
                }

                //globals
                for (unsigned iOperand = 0; iOperand < inst->getNumOperands(); ++iOperand)
                {
                    llvm::Value * operand = inst->getOperand(iOperand);

                    if (llvm::isa<llvm::GlobalVariable>(operand)) {
                        llvm::GlobalVariable * gv = llvm::cast<llvm::GlobalVariable>(operand);
                        if (gv->getParent() != module)
                        {
                            std::string parentModuleName = gv->getParent()->getModuleIdentifier();
                            //errs() << "ERROR: In module '" << moduleName << "': referencing global value '" << gv->getNameStr() << "' of module '" << parentModuleName << "'.\n";
                            exit(-1);
                        }
                    }
                }
            }
        }
    }

    // use LLVM's own verifier pass
    return llvm::verifyModule(*module, llvm::AbortProcessAction, &errorMsg);
}

void llvmRemoveUnusedFunctionsExcept(llvm::Module *module,
                                     const std::vector<std::string> &functionNames)
{
    DeepCalleeFinder::FunctionSet functions;
    typedef std::vector<std::string>::const_iterator Iter;
    for (Iter it = functionNames.begin(), e = functionNames.end(); it != e; ++it)
    {
        llvm::Function *func = module->getFunction(*it);
        if (func)
            functions.insert(func);
    }
    if (functions.size())
        llvmRemoveUnusedFunctionsExcept(module, functions);
}

void llvmRemoveUnusedFunctionsExcept(llvm::Module *module,
                                     const std::set<llvm::Function*> &functions)
{
    DeepCalleeFinder * callFinder = new DeepCalleeFinder(functions);

    llvm::PassManager pm;
    pm.add(callFinder);
    pm.run(*module);

    DeepCalleeFinder::FunctionSet preservedFuncs = callFinder->getDeepCallees();

    for(llvm::Module::iterator itFunc = module->begin(); itFunc != module->end();)
    {
        llvm::Function * func = itFunc++;

        if (preservedFuncs.find(func) == preservedFuncs.end())
        {
            func->dropAllReferences();
            func->removeFromParent();
        }
    }
}

void llvmRemoveUnusedGlobalVariables(llvm::Module *module)
{
    std::vector<llvm::GlobalVariable*> globals;
    for(llvm::iplist<llvm::GlobalVariable, llvm::ilist_traits<llvm::GlobalVariable> >::iterator it =
            module->getGlobalList().begin(),
            E = module->getGlobalList().end(); it != E; ++it)
    {
        if (it->getNumUses() > 0)
            continue;

        globals.push_back(it);
    }

    for (unsigned int i=0; i < globals.size(); i++)
        globals[i]->eraseFromParent();
}

void llvmRemoveVoidFunctionCall(llvm::Function *fun)
{
    // find all calls to the function
    CallSiteFinder *finder = new CallSiteFinder(fun);
    llvm::PassManager passes;
    passes.add(finder);
    passes.run(*(fun->getParent()));

    const CallSiteFinder::CallSiteVec &callSites = finder->getFoundCallSites();
    for (CallSiteFinder::CallSiteVec::const_iterator it = callSites.begin(),
        e = callSites.end(); it != e; ++it)
    {
        const llvm::CallSite &CS = *it;
        //Debug: llvm::outs()<<"Removing printf "<<*CS.getInstruction()<<"\n";

        llvm::Instruction *Call = CS.getInstruction();

        Call->eraseFromParent();
    }
}

void llvmLoadPlugin(const std::string &fileName)
{
    llvm::PluginLoader pl;
    pl = fileName;
}

unsigned int llvmGetNumPlugins()
{
    return llvm::PluginLoader::getNumPlugins();
}

const std::string llvmGetPlugin(unsigned int num)
{
    return std::string(llvm::PluginLoader::getPlugin(num));
}


#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
/// EmitPutS - Emit a call to the puts function.  This assumes that Str is
/// some pointer.
static llvm::CallInst *EmitLogFunc(llvm::StringRef Name, llvm::Value *Str, llvm::IRBuilder<> &B, const llvm::DataLayout *TD) {
  llvm::Module *M = B.GetInsertBlock()->getParent()->getParent();
  llvm::AttributeSet AS[2];
  AS[0] = llvm::AttributeSet::get(M->getContext(), 1, llvm::Attribute::NoCapture);
  AS[1] = llvm::AttributeSet::get(M->getContext(), llvm::AttributeSet::FunctionIndex,
                  llvm::Attribute::NoUnwind);

  llvm::Value *LogFun = M->getOrInsertFunction(Name,
                         llvm::AttributeSet::get(M->getContext(), AS),
                         B.getInt32Ty(),
                         B.getInt8PtrTy(),
                         NULL);
  llvm::CallInst *CI = B.CreateCall(LogFun, CastToCStr(Str, B), Name);
  if (const llvm::Function *F = llvm::dyn_cast<llvm::Function>(LogFun->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());
  return CI;
}
#else
/// EmitPutS - Emit a call to the puts function.  This assumes that Str is
/// some pointer.
static llvm::CallInst * EmitLogFunc(llvm::StringRef Name, llvm::Value *Str, llvm::IRBuilder<> &B, const llvm::TargetData *TD) {
  llvm::Module *M = B.GetInsertBlock()->getParent()->getParent();
  llvm::AttributeWithIndex AWI[2];
  AWI[0] = llvm::AttributeWithIndex::get(1, llvm::Attribute::NoCapture);
  AWI[1] = llvm::AttributeWithIndex::get(~0u, llvm::Attribute::NoUnwind);

  llvm::Value *LogFun = M->getOrInsertFunction(Name, llvm::AttrListPtr::get(AWI, 2),
                                       B.getInt32Ty(),
                                       B.getInt8PtrTy(),
                                       NULL);
  llvm::CallInst *CI = B.CreateCall(LogFun, llvm::CastToCStr(Str, B), Name);
  if (const llvm::Function *F = llvm::dyn_cast<llvm::Function>(LogFun->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());
  return CI;
}
#endif

void llvmAnnotateFunctionEntersAndExits(llvm::Module *module, const std::string &funcToCall_)
{
    std::string Error;
    std::string hostTriple = llvm::sys::getDefaultTargetTriple();
    llvm::StringRef funcToCall(funcToCall_);

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

        if (F->getName() == funcToCall)
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
        llvm::CallInst *CI = EmitLogFunc(funcToCall, textValue, Builder, dataLayout);
#else
        llvm::CallInst *CI = EmitLogFunc(funcToCall, textValue, Builder, targetData);
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
                llvm::CallInst *CI = EmitLogFunc(funcToCall, textValue, Builder, dataLayout);
#else
                llvm::CallInst *CI = EmitLogFunc(funcToCall, textValue, Builder, targetData);
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
                llvm::CallInst *CI = EmitLogFunc(funcToCall, textValue, Builder, dataLayout);
#else
                llvm::CallInst *CI = EmitLogFunc(funcToCall, textValue, Builder, targetData);
#endif
                CI->setName("print_annotation");
            }
        }
    }
}

KIARA_API bool llvmReplaceFunctionCalls(llvm::Module *module, const std::string &oldName, const std::string &newName)
{
    assert(module != 0);
    llvm::Function *newFunc = module->getFunction(newName);
    if (!newFunc)
        return false;
    llvm::Function *oldFunc = module->getFunction(oldName);
    if (!oldFunc)
        return false;

    bool Changed = false;
    for (llvm::Module::iterator F = module->begin(), FE = module->end(); F != FE; ++F)
    {
        for (llvm::Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB)
        {
            for (llvm::BasicBlock::iterator I = BB->begin(); I != BB->end(); )
            {
                // Ignore non-calls.
                llvm::CallInst *CI = llvm::dyn_cast<llvm::CallInst>(I++);
                if (!CI)
                    continue;

                // Ignore indirect calls and calls to non-external functions.
                /* old criteria: Callee == 0 || !Callee->isDeclaration() || !(Callee->hasExternalLinkage() || Callee->hasDLLImportLinkage()))
                 *
                 *  removed to allow replacement of defined functions
                 *
                 */

                llvm::Function *Callee = CI->getCalledFunction();
                if (!Callee)
                    continue;

                // Ignore unknown calls.
                if (Callee != oldFunc)
                    continue;

                // Try to replace this call.

                // Get or emit function with the same signature as CI but with the new name

                llvm::CallSite CS(CI);

                // collect all arguments of the old call
                std::vector<llvm::Value*> args(CS.arg_begin(), CS.arg_end());

                llvm::CallInst *newCI = llvm::CallInst::Create(newFunc, args);

                newCI->setAttributes(CI->getAttributes());
                newCI->setTailCall(CI->isTailCall());
                newCI->setCallingConv(CI->getCallingConv());

                llvm::BasicBlock::iterator ii(CI);
                llvm::ReplaceInstWithInst(CI->getParent()->getInstList(), ii, newCI);
                I = ii;

                // Something changed!
                Changed = true;
            }
        }
    }
    return Changed;
}

bool llvmMarkFunctionsWithAlwaysInline(llvm::Module *module)
{
    bool changed = false;
    for (llvm::Module::iterator F = module->begin(), FE = module->end(); F != FE; ++F)
    {
        if (F->isDeclaration())
              continue;

        if (!F->hasFnAttribute(llvm::Attribute::AlwaysInline) &&
            !F->hasFnAttribute(llvm::Attribute::NoInline))
        {
            F->addFnAttr(llvm::Attribute::AlwaysInline);
            changed = true;
        }
    }
    return changed;
}

static inline void removeFnAttr(llvm::Function *func, llvm::Attribute::AttrKind Attr)
{
    func->setAttributes(
        func->getAttributes().removeAttribute(
            func->getContext(), llvm::AttributeSet::FunctionIndex, Attr));
}

KIARA_API bool llvmMarkFunctionsWithNoInline(llvm::Module *module, bool removeAlwaysInlineAttr)
{
    bool changed = false;
    for (llvm::Module::iterator F = module->begin(), FE = module->end(); F != FE; ++F)
    {
        if (F->isDeclaration())
              continue;

        if (!F->hasFnAttribute(llvm::Attribute::NoInline))
        {
            if (F->hasFnAttribute(llvm::Attribute::AlwaysInline))
            {
                if (removeAlwaysInlineAttr)
                {
                    removeFnAttr(F, llvm::Attribute::AlwaysInline);
                    F->addFnAttr(llvm::Attribute::NoInline);
                    changed = true;
                }
            }
            else
            {
                F->addFnAttr(llvm::Attribute::NoInline);
                changed = true;
            }
        }
    }
    return changed;
}

} // namespace KIARA
