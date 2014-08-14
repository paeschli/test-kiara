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
 * RuntimeContext.hpp
 *
 *  Created on: Dec 29, 2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_RUNTIME_RUNTIMECONTEXT_HPP_INCLUDED
#define KIARA_RUNTIME_RUNTIMECONTEXT_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <KIARA/DB/World.hpp>
#include <KIARA/DB/DerivedTypes.hpp>
#include <KIARA/Utils/PathFinder.hpp>

#ifdef HAVE_LLVM
namespace llvm
{
class LLVMContext;
}
#endif

namespace KIARA
{

class RuntimeEnvironment;

class KIARA_API RuntimeContext
{
public:

    virtual ~RuntimeContext();

    virtual RuntimeEnvironment * createEnvironment() = 0;

    World & getWorld() const { return world_; }

    void setSearchPaths(const char *pathList);

    const KIARA::PathFinder & getPathFinder() const { return pathFinder_; }

    std::string findPath(const std::string &fileName, std::string *errorMsg = 0) const;

    KIARA::Type::Ptr getContextType() const { return contextPtrType_->getElementType(); }
    KIARA::PtrType::Ptr getContextPtrType() const { return contextPtrType_; }

    KIARA::Type::Ptr getConnectionType() const { return connectionPtrType_->getElementType(); }
    KIARA::PtrType::Ptr getConnectionPtrType() const { return connectionPtrType_; }

    KIARA::Type::Ptr getMessageType() const { return messagePtrType_->getElementType(); }
    KIARA::PtrType::Ptr getMessagePtrType() const { return messagePtrType_; }

    KIARA::Type::Ptr getFuncObjType() const { return funcObjPtrType_->getElementType(); }
    KIARA::PtrType::Ptr getFuncObjPtrType() const { return funcObjPtrType_; }

    KIARA::Type::Ptr getServiceFuncObjType() const { return serviceFuncObjPtrType_->getElementType(); }
    KIARA::PtrType::Ptr getServiceFuncObjPtrType() const { return serviceFuncObjPtrType_; }

    KIARA::Type::Ptr getUserTypeType() const { return userTypePtrType_->getElementType(); }
    KIARA::PtrType::Ptr getUserTypePtrType() const { return userTypePtrType_; }

    KIARA::Type::Ptr getDBufferType() const { return dbufferPtrType_->getElementType(); }
    KIARA::PtrType::Ptr getDBufferPtrType() const { return dbufferPtrType_; }

    KIARA::Type::Ptr getBinaryStreamType() const { return binaryStreamPtrType_->getElementType(); }
    KIARA::PtrType::Ptr getBinaryStreamPtrType() const { return binaryStreamPtrType_; }

    static RuntimeContext * create(World & world);

protected:
    RuntimeContext(World &world);
private:
    World &world_;
    KIARA::PathFinder pathFinder_;
    KIARA::PtrType::Ptr contextPtrType_;
    KIARA::PtrType::Ptr connectionPtrType_;
    KIARA::PtrType::Ptr messagePtrType_;
    KIARA::PtrType::Ptr funcObjPtrType_;
    KIARA::PtrType::Ptr serviceFuncObjPtrType_;
    KIARA::PtrType::Ptr userTypePtrType_;
    KIARA::PtrType::Ptr dbufferPtrType_;
    KIARA::PtrType::Ptr binaryStreamPtrType_;
};

class KIARA_API InterpreterRuntimeContext : public RuntimeContext
{
    friend class RuntimeContext;
public:

    virtual ~InterpreterRuntimeContext();

    virtual RuntimeEnvironment * createEnvironment();

private:
    InterpreterRuntimeContext(World &world);
};

#ifdef HAVE_LLVM

class KIARA_API LLVMRuntimeContext : public RuntimeContext
{
    friend class RuntimeContext;
public:

    virtual ~LLVMRuntimeContext();

    virtual RuntimeEnvironment * createEnvironment();

    llvm::LLVMContext & getLLVMContext() const { return *llvmContext_; }

private:
    llvm::LLVMContext * llvmContext_;

    LLVMRuntimeContext(World &world);
};

#endif

} // namespace KIARA

#endif /* KIARA_RUNTIME_RUNTIMECONTEXT_HPP_INCLUDED */
