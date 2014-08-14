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
 * KDirectiveProcessor.hpp
 *
 *  Created on: Dec 30, 2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_LLVM_KDIRECTIVEPROCESSOR_HPP_INCLUDED
#define KIARA_LLVM_KDIRECTIVEPROCESSOR_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include "KDirective.hpp"
#include <map>

namespace llvm
{
class Module;
class Function;
class Type;
}

namespace KIARA
{

class KIARA_API KDirectiveProcessor
{
public:

    typedef std::map<std::string, llvm::Type *> TypeMap;
    typedef std::vector<std::string> StringList;

    KDirectiveProcessor(llvm::Module &module);
    ~KDirectiveProcessor();

    const TypeMap & getTypeMap() const { return typeMap_; }
    TypeMap & getTypeMap() { return typeMap_; }

    const StringList & getCommands() const { return commands_; }
    StringList & getCommands() { return commands_; }

    void applyCommands(llvm::Module &module);

    void dump() const;

private:
    llvm::Module &module_;
    TypeMap typeMap_;
    StringList commands_;

    void process();
    void addExportTypeMapping(const std::string &typeName, llvm::Type *type);
    void analyzeInfoFunc(const llvm::Function &infoFunc);
    void processDirective(const KDirective &directive);
    bool processExportTypeDirective(const KDirective &directive, std::string *errorMsg = 0);
    bool processCommandDirective(const KDirective &directive, std::string *errorMsg = 0);
};

} // namespace KIARA

#endif /* KIARA_LLVM_KDIRECTIVEPROCESSOR_HPP_INCLUDED */
