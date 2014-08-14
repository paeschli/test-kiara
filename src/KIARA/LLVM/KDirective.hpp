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
 * KDirective.hpp
 *
 *  Created on: Mar 11, 2010
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_LLVM_KDIRECTIVE_HPP_INCLUDED
#define KIARA_LLVM_KDIRECTIVE_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include "KDArgument.hpp"
#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant/bad_visit.hpp>
#include <vector>
#include <ostream>

namespace llvm
{
class CallInst;
class LLVMContext;
class MDNode;
}

namespace KIARA
{

// Representation of the KIARA directive
class KIARA_API KDirective
{
public:

    typedef std::vector<KDArgument> ArgumentList;

    KDirective();

    KDirective(const std::string &name);

    const std::string & getName() const { return name; }

    void setName(const std::string &_name) { name = _name; }

    unsigned int getNumArguments() const { return arguments.size(); }

    const ArgumentList & getArguments() const { return arguments; }

    const KDArgument &getArgument(unsigned int i) const
    {
        return arguments[i];
    }

    bool hasArgumentOfType(KDArgument::Type argType) const
    {
        for (ArgumentList::const_iterator it = arguments.begin(), end = arguments.end();
                it != end; ++it)
        {
            if (it->getType() == argType)
                return true;
        }
        return false;
    }

    void addArgument(const KDArgument &arg)
    {
        arguments.push_back(arg);
    }

    /// add all arguments from specified call
    void addArgumentsFromCall(const llvm::CallInst *call);

    /// convert directive to LLVM metadata node
    llvm::MDNode *toMDNode(llvm::LLVMContext &context, std::string *errorMsg = 0) const;

    bool initFromMDNode(llvm::MDNode *mdnode, std::string *errorMsg = 0);

    void print(std::ostream &out) const;

    friend std::ostream & operator<<(std::ostream &out, const KDirective &directive)
    {
        directive.print(out);
        return out;
    }

private:
    std::string name;
    ArgumentList arguments;
};

} // namespace KIARA

#endif
