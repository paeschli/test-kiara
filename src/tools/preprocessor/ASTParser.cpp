/*
 * ASTParser.cpp
 *
 *  Created on: Nov 20, 2013
 *      Author: Dmitri Rubinstein
 */

//
// Based on clReflect tool:
//
// ===============================================================================
// clReflect
// -------------------------------------------------------------------------------
// Copyright (c) 2011-2012 Don Williamson & clReflect Authors (see AUTHORS file)
// Released under MIT License (see LICENSE file)
// ===============================================================================
//

#include "ASTParser.hpp"

#include "llvm/Support/Host.h"
#include "llvm/Support/Path.h"

#include "clang/AST/ASTConsumer.h"
#include "clang/Basic/Version.h"
#include "clang/Basic/FileManager.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/LangStandard.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Sema/Sema.h"

#include "TypeDatabase.hpp"

namespace
{
    //
    // Empty consumer that gets called during parsing of the AST.
    // I'm not entirely sure how to build an AST without having a callback called
    // for each top-level declaration so this will do for now.
    //
    struct EmptyASTConsumer : public clang::ASTConsumer
    {
        virtual ~EmptyASTConsumer() { }
        virtual bool HandleTopLevelDecl(clang::DeclGroupRef) { return true; }
    };
}

namespace KIARA
{

ASTParser::ASTParser(const ASTParser::Options &options, llvm::raw_ostream &errorStream)
    : errorStream_(errorStream)
{
    compilerInvocation_.reset(new clang::CompilerInvocation);

    diagnosticOptions_ = &compilerInvocation_->getDiagnosticOpts();

    // we add a customized macro here to distinguish a clreflect parsing process from a compling using clang
    clang::PreprocessorOptions& preprocessor_options = compilerInvocation_->getPreprocessorOpts();
    preprocessor_options.addMacroDef("__kiara_parse__");

    // Add define/undefine macros to the pre-processor
    typedef std::vector<std::string> SVec;
    for (SVec::const_iterator it = options.defs.begin(), end = options.defs.end(); it != end; ++it)
    {
        preprocessor_options.addMacroDef(it->c_str());
    }

    for (SVec::const_iterator it = options.undefs.begin(), end = options.undefs.end(); it != end; ++it)
    {
        preprocessor_options.addMacroUndef(it->c_str());
    }

    // Setup the language parsing options for C++
    clang::LangOptions& lang_options = *compilerInvocation_->getLangOpts();

    if (options.cxxLang)
    {
        compilerInvocation_->setLangDefaults(lang_options, clang::IK_CXX, clang::LangStandard::lang_cxx03);
        lang_options.CPlusPlus = 1;
        lang_options.Bool = 1;
        lang_options.RTTI = 0;
    }
    else
    {
        compilerInvocation_->setLangDefaults(lang_options, clang::IK_C, clang::LangStandard::lang_c99);
        lang_options.C99 = 1;
        lang_options.CPlusPlus = 0;
        lang_options.Bool = 1;
        lang_options.RTTI = 0;
    }

    if (options.msvcCompiler)
    {
        lang_options.MicrosoftExt = 1;
        lang_options.MicrosoftMode = 1;
        lang_options.MSBitfields = 1;

        //
        // This is MSVC specific to get STL compiling with clang. MSVC doesn't do semantic analysis
        // of templates until instantiation, whereas clang will try to resolve non type-based function
        // calls. In MSVC STL land, this causes hundreds of errors referencing '_invalid_parameter_noinfo'.
        //
        // The problem in a nutshell:
        //
        //    template <typename TYPE> void A()
        //    {
        //       // Causes an error in clang because B() is not defined yet, MSVC is fine
        //       B();
        //    }
        //    void B() { }
        //
        lang_options.DelayedTemplateParsing = 1;
    }
    // Gather C++ header searches from the command-line
    clang::HeaderSearchOptions& header_search_options = compilerInvocation_->getHeaderSearchOpts();
    for (SVec::const_iterator it = options.includeDirs.begin(), end = options.includeDirs.end(); it != end; ++it)
    {
        header_search_options.AddPath(it->c_str(), clang::frontend::Angled, false, false);
    }

    for (SVec::const_iterator it = options.sysIncludeDirs.begin(), end = options.sysIncludeDirs.end(); it != end; ++it)
    {
        header_search_options.AddPath(it->c_str(), clang::frontend::System, false, false);
    }

    // Setup diagnostics output; MSVC line-clicking and suppress warnings from system headers
    if (options.msvcCompiler)
        diagnosticOptions_->setFormat(diagnosticOptions_->Msvc);
    else
        diagnosticOptions_->setFormat(diagnosticOptions_->Clang);

#if 0
    // This code is not safe, since tool can be moved after installation
    if (compilerInstance_.getHeaderSearchOpts().UseBuiltinIncludes)
    {
#ifdef CLANG_LIB_DIR
        llvm::errs() << "CLANG_LIB_DIR = "<<CLANG_LIB_DIR << " VER "<< CLANG_VERSION_STRING <<"\n";
        // ResourceDir might be wrongly set, so fix it in any case
        llvm::sys::Path P(CLANG_LIB_DIR);
        P.appendComponent(CLANG_VERSION_STRING);
        compilerInstance_.getHeaderSearchOpts().ResourceDir = P.str();
#endif
    }
#endif

    clang::TextDiagnosticPrinter *client = new clang::TextDiagnosticPrinter(errorStream_, diagnosticOptions_.getPtr());
    compilerInstance_.createDiagnostics(client);
    compilerInstance_.getDiagnostics().setSuppressSystemWarnings(true);

    // Setup target options - ensure record layout calculations use the MSVC C++ ABI
    clang::TargetOptions& target_options = compilerInvocation_->getTargetOpts();
    target_options.Triple = llvm::sys::getDefaultTargetTriple();
    if (options.msvcCompiler)
        target_options.CXXABI = "microsoft";
    else
        target_options.CXXABI = "itanium";

    targetInfo_.reset(clang::TargetInfo::CreateTargetInfo(compilerInstance_.getDiagnostics(), &target_options));
    compilerInstance_.setTarget(targetInfo_.take());

    // Set the invocation on the instance
    compilerInstance_.createFileManager();
    compilerInstance_.createSourceManager(compilerInstance_.getFileManager());
    compilerInstance_.setInvocation(compilerInvocation_.take());
}

bool ASTParser::parseAST(const char *fileName)
{
    // Recreate preprocessor and AST context
    compilerInstance_.createPreprocessor();
    compilerInstance_.createASTContext();

    // Initialize builtins
    if (compilerInstance_.hasPreprocessor())
    {
        clang::Preprocessor& preprocessor = compilerInstance_.getPreprocessor();
        preprocessor.getBuiltinInfo().InitializeBuiltins(preprocessor.getIdentifierTable(),
            preprocessor.getLangOpts());
    }

    // Get the file  from the file system
    const clang::FileEntry* file = compilerInstance_.getFileManager().getFile(fileName);
    if (!file)
        return false;
    compilerInstance_.getSourceManager().createMainFileID(file);

    // Parse the AST
    EmptyASTConsumer ast_consumer;
    clang::DiagnosticConsumer* client = compilerInstance_.getDiagnostics().getClient();
    client->BeginSourceFile(compilerInstance_.getLangOpts(), &compilerInstance_.getPreprocessor());
    clang::ParseAST(compilerInstance_.getPreprocessor(), &ast_consumer, compilerInstance_.getASTContext());
    client->EndSourceFile();

    return client->getNumErrors() == 0;
}

void ASTParser::getIncludedFiles(HeaderFileList & files) const
{
    const clang::HeaderSearch& header_search = compilerInstance_.getPreprocessor().getHeaderSearchInfo();

    // Get all files loaded during the scan
    llvm::SmallVector<const clang::FileEntry*, 16> file_uids;
    header_search.getFileMgr().GetUniqueIDMapping(file_uids);

    for (unsigned uid = 0, last_uid = file_uids.size(); uid != last_uid; ++uid)
    {
        const clang::FileEntry* fe = file_uids[uid];
        if (fe == 0)
            continue;

        // Only interested in header files
        const clang::HeaderFileInfo& hfi = header_search.getFileInfo(fe);
        if (!hfi.isNonDefault())
            continue;

        // Classify the kind of include
        clang::SrcMgr::CharacteristicKind kind = (clang::SrcMgr::CharacteristicKind)hfi.DirInfo;
        HeaderType headerType;
        if (kind == clang::SrcMgr::C_User)
            headerType = USER_HEADER;
        else if (kind == clang::SrcMgr::C_System)
            headerType = SYSTEM_HEADER;
        else
            headerType = EXTERN_C_HEADER;

        files.push_back(std::pair<HeaderType,std::string>(headerType, fe->getName()));
    }
}

} // namespace KIARA
