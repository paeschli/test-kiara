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
 * KDirective.cpp
 *
 *  Created on: Mar 11, 2010
 *      Author: Dmitri Rubinstein
 */

#define KIARA_LIB
#include <KIARA/Common/Config.hpp>

#include "KDirective.hpp"
#include <KIARA/Core/Exception.hpp>
#include <KIARA/LLVM/Utils.hpp>

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Metadata.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Constants.h>
#include <boost/lexical_cast.hpp>

namespace KIARA
{

KDirective::KDirective() :
    name(), arguments()
{
}

KDirective::KDirective(const std::string &name) :
    name(name), arguments()
{
}

void KDirective::addArgumentsFromCall(const llvm::CallInst *call)
{
    assert(call != 0);
    for (unsigned int i = 0; i < call->getNumArgOperands(); i++)
    {
        addArgument(KDArgument(call->getArgOperand(i)));
    }
}

llvm::MDNode * KDirective::toMDNode(llvm::LLVMContext &context, std::string *errorMsg) const
{
    llvm::SmallVector<llvm::Value *, 9> elts;

    llvm::IntegerType *int64Ty = llvm::Type::getInt64Ty(context);
    llvm::Type *doubleTy = llvm::Type::getDoubleTy(context);

    elts.push_back(llvm::MDString::get(context, name));

    for (ArgumentList::const_iterator it = arguments.begin(),
            e = arguments.end(); it != e; ++it)
    {
        switch (it->getType())
        {
            case KDArgument::EMPTY_ARG:
                // nothing to do
                break;
            case KDArgument::INTEGER_ARG:
                elts.push_back(llvm::ConstantInt::get(int64Ty, it->getInteger()));
                break;
            case KDArgument::FLOAT_ARG:
                elts.push_back(llvm::ConstantFP::get(doubleTy, it->getFloat()));
                break;
            case KDArgument::STRING_ARG:
                elts.push_back(llvm::MDString::get(context, it->getString()));
                break;
            case KDArgument::NULL_PTR_ARG:
                elts.push_back(llvm::ConstantPointerNull::get(it->getNullPtr().type));
                break;
            case KDArgument::LLVM_VALUE_ARG:
                /// will probably crash if modules are not same
                elts.push_back(const_cast<llvm::Value*>(it->getLLVMValue()));
                break;
            default:
                if (errorMsg)
                    *errorMsg = "Invalid KDArgument type: "+boost::lexical_cast<std::string>(it->getType());
                return 0;
        }
    }

    return llvm::MDNode::get(context, llvm::ArrayRef<llvm::Value*>(elts));
}

bool KDirective::initFromMDNode(llvm::MDNode *mdnode, std::string *errorMsg)
{
    BOOST_ASSERT(mdnode != 0 && "MDNode cannot be NULL");

    if (mdnode->getNumOperands() == 0)
    {
        if (!errorMsg)
            *errorMsg = "MDNode need to have at least one operand";
        return false;
    }

    // first argument need to be a string

    llvm::Value *value = mdnode->getOperand(0);

    if (!llvmGetConstStringArg(value, this->name))
    {
        if (!errorMsg)
            *errorMsg = "First MDNode operand must be a string";
        return false;
    }

    for (unsigned int i = 1; i < mdnode->getNumOperands(); ++i)
    {
        addArgument(KDArgument(mdnode->getOperand(i)));
    }

    return true;
}

void KDirective::print(std::ostream &out) const
{
    out<<name<<"(";
    for (ArgumentList::const_iterator it = arguments.begin(),
            e = arguments.end(); it != e; ++it)
    {
        switch (it->getType())
        {
            case KDArgument::EMPTY_ARG:
                out<<"<empty>";
                break;
            case KDArgument::INTEGER_ARG:
                out<<it->getInteger();
                break;
            case KDArgument::FLOAT_ARG:
                out<<it->getFloat();
                break;
            case KDArgument::STRING_ARG:
                out<<"\""<<it->getString()<<"\"";
                break;
            default:
                out<<it->toString();
        }
        if (it != e-1)
            out<<", ";
    }
    out<<")";
}

} // namespace AnySL
