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
 * KDArgument.cpp
 *
 *  Created on: Mar 11, 2010
 *      Author: Dmitri Rubinstein
 */

#define KIARA_LIB
#include <KIARA/Common/Config.hpp>

#include "KDArgument.hpp"
#include <KIARA/LLVM/Utils.hpp>
#include <llvm/IR/Value.h>
#include <llvm/IR/Constants.h>

namespace KIARA
{

KDArgument::KDArgument(const llvm::Value *llvmValue) :
    value(Empty())
{
    setLLVMValue(llvmValue);
}


/// Removes trailing '\0'.
/// Useful for LLVM string constants which contain trailing zero.
static inline void removeTrailingZero(std::string &s)
{
    if (s.length() && s[s.length()-1] == '\0')
        s.resize(s.length()-1);
}

void KDArgument::setLLVMValue(const llvm::Value *v)
{
    std::string s;
    int64_t i;
    double d;
    if (llvmGetConstStringArg(v, s))
    {
        // constants from LLVM contain trailing '\0',
        // so remove it first

        removeTrailingZero(s);
        setString(s);
    }
    else if (llvmGetConstIntArg(v, i))
        setInteger(i);
    else if (llvmGetConstFloatArg(v, d))
        setFloat(d);
    else if (llvm::isa<llvm::ConstantPointerNull>(v))
    {
        const llvm::ConstantPointerNull *constNull = llvm::cast<llvm::ConstantPointerNull>(v);
        setNullPtr(constNull->getType());
    }
    else
        value = v;
}

} // namespace AnySL
