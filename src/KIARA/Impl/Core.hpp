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
#ifndef KIARA_IMPL_CORE_HPP_INCLUDED
#define KIARA_IMPL_CORE_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <KIARA/kiara.h>
#include <KIARA/kiara_macros.h>
#include <KIARA/Core/LibraryInit.hpp>
#include <KIARA/DB/Module.hpp>
#include <KIARA/Transport/Server.hpp>
#include <KIARA/DB/LibraryConfiguration.hpp>
#include <KIARA/Utils/ServerConfiguration.hpp>
#include <KIARA/Utils/PathFinder.hpp>
#include <KIARA/Utils/URLLoader.hpp>
#include <boost/asio/io_service.hpp>
#include <set>
#include "API.h"

namespace KIARA
{

class RuntimeEnvironment;
class RuntimeContext;

namespace Impl
{

class Global
{
public:

    static int initialize(int *argc, char **argv);

    static int finalize();

    static bool isInitialized() { return initialized_; }

    static const std::string & getModuleSearchPath() { return libraryConfiguration_.moduleSearchPath; }

    static KIARA::SecurityConfiguration getSecurityConfiguration() { return libraryConfiguration_.getSecurityConfiguration(); }

    static KIARA::JITConfiguration getJITConfiguration() { return libraryConfiguration_.getJITConfiguration(); }

private:
    static bool initialized_;
    static LibraryConfiguration libraryConfiguration_;
};

class Error
{
public:

    Error() :
        errorCode_(KIARA_NO_ERROR),
        message_()
    { }

    bool isError() const { return errorCode_ != KIARA_NO_ERROR; }

    void set(KIARA_Result errorCode, const std::string &message)
    {
        errorCode_ = errorCode;
        message_ = message;
    }

    void set(KIARA_Result errorCode)
    {
        errorCode_ = errorCode;
        message_ = "Error";
    }

    KIARA_Result getErrorCode() const
    {
        return errorCode_;
    }

    const char * getMessage() const
    {
        if (!isError())
            return 0;
        return message_.c_str();
    }

    void clear()
    {
        errorCode_ = KIARA_NO_ERROR;
        message_.clear();
    }

private:
    KIARA_Result errorCode_;
    std::string message_;
};

class Context : public KIARA::World
{
public:

    typedef std::map<const KIARA_DeclType *, KIARA::TypePtr> DeclCacheMap;

    Context();

    virtual ~Context();

//    boost::asio::io_service & getIOService() { return ioService_; }
    KIARA::Transport::AsioNetworkContext::Ptr & getIOService() { return ioService_; }

    KIARA::World & world() { return *this; }

    const KIARA::SecurityConfiguration & getSecurityConfiguraton() const { return securityConfiguration_; }

    const KIARA::Module::Ptr & getModule() const { return module_; }

    Error & getError() { return error_; }

    const Error & getError() const { return error_; }

    bool isError() const { return error_.isError(); }

    const char * getErrorMessage() const { return error_.getMessage(); }

    KIARA_Result getErrorCode() const { return error_.getErrorCode(); }

    void setError(KIARA_Result errorCode, const std::string &errorMessage)
    {
        error_.set(errorCode, errorMessage);
    }

    void clearError() { error_.clear(); }

    KIARA_Type * wrapType(const KIARA::Type::Ptr &type)
    {
        if (type)
            types_.insert(type);
        return reinterpret_cast<KIARA_Type*>(type.get());
    }

    KIARA::Type::Ptr unwrapType(KIARA_Type *type) const
    {
        KIARA::Type::RawPtr p =
                reinterpret_cast<KIARA::Type::RawPtr>(type);
        return p;
    }

    KIARA_Type * declareStructType(const char *name, int numMembers, KIARA_StructDecl members[]);

#define BUILTIN_TYPE(T) KIARA_Type * KIARA_JOIN(type_,T)()  \
{                                                           \
    return wrapType(world().KIARA_JOIN(type_,T)());	        \
}
#include <KIARA/DB/Type.def>

    KIARA_Type * type_c_ptr(KIARA_Type *elementType)
    {
        return wrapType(world().type_c_ptr(unwrapType(elementType)));
    }

    KIARA_Type * type_c_char_p() { return type_c_ptr(type_c_char()); }
    KIARA_Type * type_c_void_p() { return type_c_ptr(type_void()); }

#undef _KI_TYPE

    KIARA_Type * getTypeByName(const char *name);

    KIARA_Type * getTypeFromDecl(KIARA_GetDeclType declTypeGetter, Error &error);

    KIARA::TypePtr getTypeFromDeclType(const KIARA_DeclType *declType, Error &error);

    KIARA::TypePtr getTypeFromDeclTypeGetter(KIARA_GetDeclType declTypeGetter, Error &error);

    KIARA::PtrType::Ptr getContextPtrType() const;
    KIARA::PtrType::Ptr getConnectionPtrType() const;
    KIARA::PtrType::Ptr getMessagePtrType() const;
    KIARA::PtrType::Ptr getFuncObjPtrType() const;
    KIARA::PtrType::Ptr getServiceFuncObjPtrType() const;
    KIARA::PtrType::Ptr getDBufferPtrType() const;
    KIARA::PtrType::Ptr getBinaryStreamPtrType() const;

    bool loadIDL(const std::string &fileName);
    bool loadIDL(std::istream &in, const std::string &fileName);
    bool loadIDLFromURL(const std::string &url);
    bool loadIDLFromURL(KIARA::URLLoader::Connection * handle, const std::string &url);

    KIARA_Result loadLLVMModule(const std::string &fileName);

    const std::vector<std::string> & getLLVMModuleNames() const { return llvmModuleNames_; }

    KIARA::RuntimeContext & getRuntimeContext()
    {
        return *runtimeContext_;
    }

private:
    KIARA::SecurityConfiguration securityConfiguration_;
    KIARA::Module::Ptr module_;
    Error error_;
    KIARA::TypeSet types_;
    DeclCacheMap declCacheMap_;

    KIARA::RuntimeContext *runtimeContext_;
//    boost::asio::io_service ioService_;
    KIARA::Transport::AsioNetworkContext::Ptr ioService_;
    std::vector<std::string> llvmModuleNames_;
};

class Base
{
public:
    Base(Context *context);

    virtual ~Base();

    Context * getContext() const { return context_; }

    KIARA::World & getWorld() const
    {
        assert(getContext() != 0);
        return *getContext();
    }

    Error & getError() { return error_; }

    const Error & getError() const { return error_; }

    bool isError() const { return error_.isError(); }

    const char * getErrorMessage() const { return error_.getMessage(); }

    KIARA_Result getErrorCode() const { return error_.getErrorCode(); }

    void setError(KIARA_Result errorCode, const std::string &errorMessage)
    {
        error_.set(errorCode, errorMessage);
    }

    void setError(const Error &error)
    {
        error_ = error;
    }

    void clearError() { error_.clear(); }

protected:

    Error error_;

private:
    Context *context_;
};

#define DEFINE_WRAPPER_FUNCTIONS(CXXType, CType) \
    inline CType * wrap(CXXType *p)              \
    {                                            \
        return reinterpret_cast<CType*>(p);      \
    }                                            \
                                                 \
    inline CXXType * unwrap(CType *p)            \
    {                                            \
        return reinterpret_cast<CXXType *>(p);   \
    }

DEFINE_WRAPPER_FUNCTIONS(::KIARA::Impl::Context, ::KIARA_Context)

} // namespace Impl

} // namespace KIARA

#endif
