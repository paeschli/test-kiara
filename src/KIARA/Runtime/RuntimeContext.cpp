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
 * RuntimeContext.cpp
 *
 *  Created on: Dec 29, 2013
 *      Author: Dmitri Rubinstein
 */
#define KIARA_LIB

#ifdef HAVE_LLVM
// ssize_t is defined by LLVM's DataTypes.h
#define _SSIZE_T_DEFINED
#endif

#include "RuntimeContext.hpp"
#include "RuntimeEnvironment.hpp"

#ifdef HAVE_LLVM
#include "llvm/Config/llvm-config.h"
#if (LLVM_VERSION_MAJOR >= 3 && LLVM_VERSION_MINOR >= 3)
#include "llvm/IR/LLVMContext.h"
#else
#include "llvm/LLVMContext.h"
#endif
#include "llvm/Support/DataTypes.h"
#endif

#ifdef HAVE_LLVM
#include "KIARA/Compiler/LLVM/Evaluator.hpp"
#endif

namespace KIARA
{

// RuntimeContext

RuntimeContext::RuntimeContext(World &world)
    : world_(world)
    , pathFinder_()
{
    KIARA::StructType::Ptr contextType = KIARA::StructType::create(getWorld(), "KIARA_Context");
    KIARA::StructType::Ptr connectionType = KIARA::StructType::create(getWorld(), "KIARA_Connection");
    KIARA::StructType::Ptr messageType = KIARA::StructType::create(getWorld(), "KIARA_Message");
    KIARA::StructType::Ptr funcObjType = KIARA::StructType::create(getWorld(), "KIARA_FuncObj");
    KIARA::StructType::Ptr serviceFuncObjType = KIARA::StructType::create(getWorld(), "KIARA_ServiceFuncObj");
    KIARA::StructType::Ptr userTypeType = KIARA::StructType::create(getWorld(), "KIARA_UserType");
    KIARA::StructType::Ptr dbufferType = KIARA::StructType::create(getWorld(), "kr_dbuffer_t");
    KIARA::StructType::Ptr binaryStreamType = KIARA::StructType::create(getWorld(), "KIARA_BinaryStream");

    contextPtrType_ = KIARA::PtrType::get(contextType);
    connectionPtrType_ = KIARA::PtrType::get(connectionType);
    messagePtrType_ = KIARA::PtrType::get(messageType);
    funcObjPtrType_ = KIARA::PtrType::get(funcObjType);
    serviceFuncObjPtrType_ = KIARA::PtrType::get(serviceFuncObjType);
    userTypePtrType_ = KIARA::PtrType::get(userTypeType);
    dbufferPtrType_ = KIARA::PtrType::get(dbufferType);
    binaryStreamPtrType_ = KIARA::PtrType::get(binaryStreamType);
}

RuntimeContext::~RuntimeContext()
{ }

void RuntimeContext::setSearchPaths(const char *pathList)
{
    pathFinder_.setSearchPathsFromPathList(pathList);
}

std::string RuntimeContext::findPath(const std::string &fileName, std::string *errorMsg) const
{
    return pathFinder_.findPath(fileName, errorMsg);
}

RuntimeContext * RuntimeContext::create(World & world)
{
#ifdef HAVE_LLVM
    return new LLVMRuntimeContext(world);
#else
    return new InterpreterRuntimeContext(world);
#endif
}

// InterpreterRuntimeContext

InterpreterRuntimeContext::InterpreterRuntimeContext(World &world)
    : RuntimeContext(world)
{

}

InterpreterRuntimeContext::~InterpreterRuntimeContext()
{

}

RuntimeEnvironment * InterpreterRuntimeContext::createEnvironment()
{
    return new InterpreterRuntimeEnvironment(*this);
}

#ifdef HAVE_LLVM

// LLVMRuntimeContext

LLVMRuntimeContext::LLVMRuntimeContext(World &world)
    : RuntimeContext(world)
    , llvmContext_(0)
{
    llvmContext_ = new llvm::LLVMContext;
}

LLVMRuntimeContext::~LLVMRuntimeContext()
{
    delete llvmContext_;
}

RuntimeEnvironment * LLVMRuntimeContext::createEnvironment()
{
    return new LLVMRuntimeEnvironment(*this);
}

#endif

} // namespace KIARA
