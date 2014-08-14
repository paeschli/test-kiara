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
 * ASTParser.hpp
 *
 *  Created on: Nov 20, 2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_ASTPARSER_HPP_INCLUDED
#define KIARA_ASTPARSER_HPP_INCLUDED

#include "llvm/Support/raw_ostream.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/CompilerInstance.h"
#include <string>
#include <vector>

namespace KIARA
{

class ASTParser
{
public:

    struct Options
    {
        std::vector<std::string> defs;              // define macros
        std::vector<std::string> undefs;            // undef macros
        std::vector<std::string> includeDirs;       // include directories
        std::vector<std::string> sysIncludeDirs;    // system include directories
        bool cxxLang;                               // preprocess C++ source
        bool msvcCompiler;                          // target compiler is MSVC

        Options()
            : defs()
            , undefs()
            , includeDirs()
            , sysIncludeDirs()
            , cxxLang(true)
            , msvcCompiler(false)
        { }
    };

    ASTParser(const Options &options, llvm::raw_ostream &errorStream);

    bool parseAST(const char *fileName);

    enum HeaderType
    {
        USER_HEADER,
        SYSTEM_HEADER,
        EXTERN_C_HEADER
    };

    typedef std::vector< std::pair<HeaderType, std::string> > HeaderFileList;

    void getIncludedFiles(HeaderFileList & files) const;

    clang::ASTContext& getASTContext() { return compilerInstance_.getASTContext(); }

private:
    llvm::raw_ostream &errorStream_;
    clang::IntrusiveRefCntPtr<clang::DiagnosticOptions> diagnosticOptions_;
    llvm::OwningPtr<clang::CompilerInvocation> compilerInvocation_;
    clang::CompilerInstance compilerInstance_;
    llvm::OwningPtr<clang::TargetInfo> targetInfo_;
};

} // namespace KIARA

#endif /* KIARA_ASTPARSER_HPP_INCLUDED */
