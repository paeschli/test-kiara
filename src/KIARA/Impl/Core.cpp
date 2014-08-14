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
#define KIARA_LIB
#include <KIARA/Common/Config.hpp>

#include "KIARA/Runtime/RuntimeContext.hpp"
#include "KIARA/Runtime/RuntimeEnvironment.hpp"

#include "KIARA/Compiler/IR.hpp"
#ifdef HAVE_LLVM
#include "KIARA/LLVM/Utils.hpp"
#endif

#include <KIARA/Impl/Core.hpp>
#include <KIARA/Impl/Network.hpp>
#include <KIARA/kiara_security.h>
#include <KIARA/DB/Attributes.hpp>
#include <KIARA/Core/Exception.hpp>
#include <KIARA/DB/DerivedTypes.hpp>
#include <KIARA/DB/TypeUtils.hpp>
#include <KIARA/IDL/IDLParserContext.hpp>
#include <KIARA/Utils/URLLoader.hpp>
#include <KIARA/Utils/ServerConfiguration.hpp>
#include <DFC/Utils/StrUtils.hpp>
#include <boost/assert.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <cstdarg>
#include <memory>
#include <openssl/aes.h>

#include <KIARA/CDT/kr_dstring.h>
#include <KIARA/CDT/kr_dbuffer.h>
#include <KIARA/CDT/kr_dbuffer_kdecl.h>

#include <KIARA/DB/ValueIO.hpp>

//#define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

// KIARA_Context Declaration

// TODO declare only basic types and use aliases ?

namespace KIARA
{

namespace Impl
{


Context::Context()
    : KIARA::World()
    , securityConfiguration_(Global::getSecurityConfiguration())
    , module_()
    , runtimeContext_(0)
{
    runtimeContext_ = KIARA::RuntimeContext::create(world());
    runtimeContext_->setSearchPaths(Global::getModuleSearchPath().c_str());

    module_ = new KIARA::Module(world(), "context");
}

KIARA::PtrType::Ptr Context::getContextPtrType() const { return runtimeContext_->getContextPtrType(); }
KIARA::PtrType::Ptr Context::getConnectionPtrType() const { return runtimeContext_->getConnectionPtrType(); }
KIARA::PtrType::Ptr Context::getMessagePtrType() const { return runtimeContext_->getMessagePtrType(); }
KIARA::PtrType::Ptr Context::getFuncObjPtrType() const { return runtimeContext_->getFuncObjPtrType(); }
KIARA::PtrType::Ptr Context::getServiceFuncObjPtrType() const { return runtimeContext_->getServiceFuncObjPtrType(); }
KIARA::PtrType::Ptr Context::getDBufferPtrType() const { return runtimeContext_->getDBufferPtrType(); }
KIARA::PtrType::Ptr Context::getBinaryStreamPtrType() const { return runtimeContext_->getBinaryStreamPtrType(); }


Context::~Context()
{
    delete runtimeContext_;
}

KIARA_Type * Context::declareStructType(const char *name, int numMembers, KIARA_StructDecl members[])
{
    if (numMembers < 0)
    {
        error_.set(KIARA_INVALID_VALUE, "DeclareStruct: number of struct members is negative");
        return 0;
    }

    KIARA::StructType::Ptr ty = KIARA::StructType::create(world(), name, numMembers);
    for (unsigned int i = 0; i < static_cast<unsigned int>(numMembers); ++i)
    {
        KIARA_StructDecl &member = members[i];
        KIARA::Type::Ptr mty = unwrapType(member.type);
        if (!mty)
        {
            error_.set(KIARA_INVALID_VALUE, "DeclareStruct: struct member type is NULL");
            return 0;
        }
        ty->setElementAt(i, mty);
        KIARA::ElementData &data = ty->getElementDataAt(i);
        data.setName(member.name ? member.name : "");
        data.setAttributeValue<KIARA::NativeOffsetAttr>(member.offset);
    }

    if (name)
    {
        try
        {
            module_->bindType(name, ty);
        }
        catch (KIARA::Exception &e)
        {
            error_.set(KIARA_INVALID_OPERATION, e.what());
            return 0;
        }
    }

    return wrapType(ty);
}

KIARA_Type * Context::getTypeByName(const char *name)
{
	KIARA::Type::Ptr ty = module_->lookupType(name);
	return wrapType(ty);
}

KIARA::TypePtr Context::getTypeFromDeclTypeGetter(KIARA_GetDeclType declTypeGetter, Error &error)
{
    assert(declTypeGetter != 0);

    // FIXME For C++ we need to use global lock around declTypeGetter
    //       because declarations use static function variables
    const KIARA_DeclType * declType = declTypeGetter();

    return getTypeFromDeclType(declType, error);
}


namespace
{

// returns true if non-default semantics was determined
bool setupDefaultSemantics(const KIARA::TypePtr &type, KIARA::MemberSemantics &memberSemantics, unsigned int numDeref = 0)
{
    if (!type)
    {
        memberSemantics.setToDefaults();
        return false;
    }

    KIARA::World &world = type->getWorld();

    if (KIARA::isa<KIARA::PtrType>(type) || KIARA::isa<KIARA::RefType>(type))
    {
        if (setupDefaultSemantics(KIARA::TypeUtils::getElementType(type), memberSemantics, numDeref+1))
            return true;
    }

    if (KIARA::canonicallyEqual(type, KIARA::PtrType::get(world.type_c_char()))) // char*
    {
        memberSemantics.valueInterp = KIARA::MemberSemantics::VI_CSTRING_PTR;
        memberSemantics.allocationMode = KIARA::MemberSemantics::AM_MALLOC_FREE;
        memberSemantics.numDeref = numDeref;
    }
    else if (type->getAs<KIARA::PtrType>())
    {
        memberSemantics.valueInterp = KIARA::MemberSemantics::VI_ARRAY_PTR;
        memberSemantics.allocationMode = KIARA::MemberSemantics::AM_MALLOC_FREE;
        memberSemantics.numDeref = numDeref;
    }
    else
    {
        memberSemantics.setToDefaults();
        return false;
    }

    return true;
}

// returns optionally modified type
KIARA::TypePtr parseSemanticsField(const char *semantics, const KIARA::TypePtr &type, KIARA::MemberSemantics &memberSemantics)
{
    KIARA::TypePtr result = type;

    std::vector<std::string> flags;
    if (semantics)
    {
        boost::algorithm::split(flags, semantics,
                                boost::algorithm::is_any_of(" ,"));
    }

    if (flags.empty())
        setupDefaultSemantics(type, memberSemantics);
    else for (std::vector<std::string>::const_iterator it = flags.begin(), end = flags.end(); it != end; ++it)
    {
        if (boost::algorithm::iequals("DEFAULT", *it))
            setupDefaultSemantics(type, memberSemantics);
        else if (boost::algorithm::iequals("AM_NONE", *it))
            memberSemantics.allocationMode = KIARA::MemberSemantics::AM_NONE;
        else if (boost::algorithm::iequals("AM_MALLOC_FREE", *it))
            memberSemantics.allocationMode = KIARA::MemberSemantics::AM_MALLOC_FREE;
        else if (boost::algorithm::iequals("AM_NEW_DELETE", *it))
            memberSemantics.allocationMode = KIARA::MemberSemantics::AM_NEW_DELETE;
        else if (boost::algorithm::iequals("AM_CUSTOM", *it))
            memberSemantics.allocationMode = KIARA::MemberSemantics::AM_CUSTOM;
        else if (boost::algorithm::iequals("VI_NONE", *it))
            memberSemantics.valueInterp = KIARA::MemberSemantics::VI_NONE;
        else if (boost::algorithm::iequals("VI_ARRAY_PTR", *it))
            memberSemantics.valueInterp = KIARA::MemberSemantics::VI_ARRAY_PTR;
        else if (boost::algorithm::iequals("VI_VALUE_PTR", *it))
            memberSemantics.valueInterp = KIARA::MemberSemantics::VI_VALUE_PTR;
        else if (boost::algorithm::iequals("VI_CSTRING_PTR", *it))
            memberSemantics.valueInterp = KIARA::MemberSemantics::VI_CSTRING_PTR;
    }

    if (memberSemantics.numDeref == 0 &&
        ((memberSemantics.allocationMode != KIARA::MemberSemantics::AM_NONE) ||
        (memberSemantics.valueInterp != KIARA::MemberSemantics::VI_NONE)))
    {
        // annotate type with member informations
        KIARA::TypedefType::Ptr tdef = KIARA::TypedefType::create(type->getFullTypeName()+"_ANNOTATED", type);
        KIARA::TypeSemantics &tsem = tdef->getOrCreateAttributeValueRef<KIARA::TypeSemanticsAttr>();
        tsem.allocationMode = memberSemantics.allocationMode;
        tsem.valueInterp = memberSemantics.valueInterp;
        result = tdef;
    }

    return result;
}

void parseSemanticsField(const char *semantics, KIARA::TypeSemantics &typeSemantics)
{
    std::vector<std::string> flags;
    if (semantics)
    {
        boost::algorithm::split(flags, semantics,
                                boost::algorithm::is_any_of(" ,"));
    }

    typeSemantics.setToDefaults();

    for (std::vector<std::string>::const_iterator it = flags.begin(), end = flags.end(); it != end; ++it)
    {
        if (boost::algorithm::iequals("AM_NONE", *it))
            typeSemantics.allocationMode = KIARA::TypeSemantics::AM_NONE;
        else if (boost::algorithm::iequals("AM_MALLOC_FREE", *it))
            typeSemantics.allocationMode = KIARA::TypeSemantics::AM_MALLOC_FREE;
        else if (boost::algorithm::iequals("AM_NEW_DELETE", *it))
            typeSemantics.allocationMode = KIARA::TypeSemantics::AM_NEW_DELETE;
        else if (boost::algorithm::iequals("AM_CUSTOM", *it))
            typeSemantics.allocationMode = KIARA::TypeSemantics::AM_CUSTOM;
        else if (boost::algorithm::iequals("VI_NONE", *it))
            typeSemantics.valueInterp = KIARA::TypeSemantics::VI_NONE;
        else if (boost::algorithm::iequals("VI_ARRAY_PTR", *it))
            typeSemantics.valueInterp = KIARA::TypeSemantics::VI_ARRAY_PTR;
        else if (boost::algorithm::iequals("VI_VALUE_PTR", *it))
            typeSemantics.valueInterp = KIARA::TypeSemantics::VI_VALUE_PTR;
        else if (boost::algorithm::iequals("VI_CSTRING_PTR", *it))
            typeSemantics.valueInterp = KIARA::TypeSemantics::VI_CSTRING_PTR;
    }
}

} // namespace

KIARA::TypePtr Context::getTypeFromDeclType(const KIARA_DeclType *declType, Error &error)
{
    assert(declType != 0);
    error.clear();

    DeclCacheMap::iterator it = declCacheMap_.find(declType);
    if (it != declCacheMap_.end())
        return it->second;

    KIARA::TypePtr type;
    std::string typeName = declType->name;

    switch (declType->typeKind)
    {
        case KIARA_TYPE_BUILTIN: {
#define _HANDLE_TYPE(T)                                     \
            if (typeName == "KIARA_" KIARA_STRINGIZE(T) ||  \
                typeName == KIARA_STRINGIZE(T))             \
            {                                               \
                type = world().KIARA_JOIN(type_c_,T)();     \
            } else

#define _HANDLE_TYPE2(T, T1)                                \
            if (typeName == "KIARA_" KIARA_STRINGIZE(T) ||  \
                typeName == KIARA_STRINGIZE(T) ||           \
                typeName == KIARA_STRINGIZE(T1))            \
            {                                               \
                type = world().KIARA_JOIN(type_c_,T)();     \
            } else

            _HANDLE_TYPE(char)
            _HANDLE_TYPE(wchar_t)
            _HANDLE_TYPE2(schar, signed char)
            _HANDLE_TYPE2(uchar, unsigned char)
            _HANDLE_TYPE(short)
            _HANDLE_TYPE2(ushort, unsigned short)
            _HANDLE_TYPE(int)
            _HANDLE_TYPE2(uint, unsigned int)
            _HANDLE_TYPE(long)
            _HANDLE_TYPE2(ulong, unsigned long)
            _HANDLE_TYPE2(longlong, long long)
            _HANDLE_TYPE2(ulonglong, unsigned long long)
            _HANDLE_TYPE(size_t)
            _HANDLE_TYPE(ssize_t)
            _HANDLE_TYPE(float)
            _HANDLE_TYPE(double)
            _HANDLE_TYPE2(longdouble, long double)

            _HANDLE_TYPE(int8_t)
            _HANDLE_TYPE(uint8_t)
            _HANDLE_TYPE(int16_t)
            _HANDLE_TYPE(uint16_t)
            _HANDLE_TYPE(int32_t)
            _HANDLE_TYPE(uint32_t)
            _HANDLE_TYPE(int64_t)
            _HANDLE_TYPE(uint64_t)

#undef _HANDLE_TYPE
#undef _HANDLE_TYPE2

            if (typeName == "void")
                type = world().type_void();
            else if ((typeName == "KIARA_raw_char_ptr" ||
                      typeName == "KIARA_const_raw_char_ptr" ||
                      typeName == "raw_char_ptr" ||
                      typeName == "const raw_char_ptr" ||
                      typeName == "raw_char *" ||
                      typeName == "const raw_char *"))
                type = world().type_c_raw_char_ptr();
            else if ((typeName == "KIARA_char_ptr" ||
                      typeName == "KIARA_const_char_ptr" ||
                      typeName == "char_ptr" ||
                      typeName == "const char_ptr" ||
                      typeName == "char *" ||
                      typeName == "const char *"))
                type = world().type_c_char_ptr();
            else if ((typeName == "KIARA_void_ptr" ||
                      typeName == "KIARA_const_void_ptr" ||
                      typeName == "void_ptr" ||
                      typeName == "const_void_ptr" ||
                      typeName == "void *" ||
                      typeName == "const void *"))
                type = world().type_c_void_ptr();
            else
            {
                error.set(KIARA_INVALID_TYPE, std::string("Unknown builtin type '")+typeName+"'");
                return 0;
            }
        }
        break;
        case KIARA_TYPE_ANNOTATED:
        {
            KIARA_DeclAnnotated *annotatedDecl = declType->typeDecl.annotatedDecl;

            KIARA::TypePtr parentType = getTypeFromDeclTypeGetter(annotatedDecl->type, error);
            if (error.isError())
                return 0;

            KIARA::TypedefType::Ptr typedefType = KIARA::TypedefType::create(declType->name, parentType);

            KIARA::TypeSemantics &typeSemantics = typedefType->getOrCreateAttributeValueRef<KIARA::TypeSemanticsAttr>();
            parseSemanticsField(annotatedDecl->semantics, typeSemantics);

            if (annotatedDecl->annotation)
            {
                typedefType->setAttributeValue<KIARA::AnnotationAttr>(annotatedDecl->annotation);
            }

            type = typedefType;
        }
        break;
        case KIARA_TYPE_ENUM:
        {
            KIARA_DeclEnum *enumDecl = declType->typeDecl.enumDecl;
            KIARA_DeclEnumConstant *constant;
            size_t i;

            KIARA::EnumType::Ptr enumType = KIARA::EnumType::create(world(), declType->name);

            for (i = 0; i < enumDecl->numConstants; ++i)
            {
                constant = &enumDecl->constants[i];
                enumType->addConstant(constant->name, new KIARA::IR::PrimLiteral(constant->value, world()));
            }

            type = enumType;
        }
        break;
        case KIARA_TYPE_STRUCT:
        {
            KIARA_DeclStruct *structDecl = declType->typeDecl.structDecl;
            KIARA_DeclStructMember *member;

            KIARA::StructType::Ptr structType = KIARA::StructType::create(world(), declType->name, structDecl->numMembers);
            structType->setAttributeValue<KIARA::NativeSizeAttr>(structDecl->size);

            // prevent infinite loop when we will lookup again for this structType
            declCacheMap_[declType] = structType;

            // Dependent members list contains name pairs (dependent member index, main member name)
            std::vector<std::pair<size_t, const char *> > dependentStructMembers;
            dependentStructMembers.reserve(structDecl->numMembers);

            for (size_t i = 0; i < structDecl->numMembers; ++i)
            {
                member = &structDecl->members[i];

                KIARA::TypePtr ty = getTypeFromDeclTypeGetter(member->type, error);
                if (error.isError())
                    return 0;
                if (!ty)
                {
                    error.set(KIARA_INVALID_TYPE, "Could not create type");
                    return 0;
                }
                structType->setElementNameAt(i, member->name);

                KIARA::ElementData &elementData = structType->getElementDataAt(i);

                elementData.setAttributeValue<KIARA::NativeOffsetAttr>(member->offset);

                if (member->mainName)
                {
                    structType->getElementDataAt(i).setAttributeValue<KIARA::MainMemberAttr>(member->mainName);
                    dependentStructMembers.push_back(std::make_pair(i, member->mainName));
                }

                KIARA::MemberSemantics &memberSemantics = elementData.getOrCreateAttributeValueRef<KIARA::MemberSemanticsAttr>();
                ty = parseSemanticsField(member->semantics, ty, memberSemantics);

                structType->setElementAt(i, ty);
            }

            for (std::vector<std::pair<size_t, const char *> >::iterator it = dependentStructMembers.begin(),
                end = dependentStructMembers.end(); it != end; ++it)
            {
                structType->getElementDataAt(it->first);
                size_t mainMemberIndex = structType->getElementIndexByName(it->second);
                if (mainMemberIndex == KIARA::StructType::npos)
                {
                    error.set(KIARA_INVALID_ARGUMENT, "No member '"+std::string(it->second)+"' in struct '"+
                        std::string(declType->name)+"' specified");
                    return 0;
                }
                KIARA::ElementData &elementData = structType->getElementDataAt(mainMemberIndex);
                std::vector<std::string> &dependentMembers = elementData.getOrCreateAttributeValueRef<KIARA::DependentMembersAttr>();
                dependentMembers.push_back(structType->getElementNameAt(it->first));
            }

            if (KIARA_DeclAPI *apiDecl = structDecl->apiDecl)
            {
                for (size_t i = 0; i < apiDecl->numAPIFuncs; ++i)
                {
                    KIARA_DeclAPIFunc &apiFuncDecl = apiDecl->apiFuncs[i];
                    std::string attrKey = KIARA_ATTR_PREFIX_API;
                    attrKey += apiFuncDecl.apiName;
                    KIARA::NamedGenericFunc &genFunc = structType->getOrCreateAttributeValueRef<KIARA::GenericFuncAttr>(attrKey);
                    genFunc.func = apiFuncDecl.apiFunc;
                    genFunc.funcName = apiFuncDecl.apiFuncName;
                }
            }

            type = structType;
        }
        break;
        case KIARA_TYPE_FUNC:
        {
            KIARA_DeclFunc *funcDecl = declType->typeDecl.funcDecl;
            KIARA_DeclFuncArgument *arg;
            size_t i;

            KIARA::TypePtr returnType = getTypeFromDeclTypeGetter(funcDecl->returnType, error);
            if (error.isError())
                return 0;
            if (!returnType)
            {
                error.set(KIARA_INVALID_TYPE, "Could not create return function type");
                return 0;
            }

            KIARA::FunctionType::ParamTypes paramTypes(funcDecl->numArgs);

            for (i = 0; i < funcDecl->numArgs; ++i)
            {
                arg = &funcDecl->args[i];
                KIARA::TypePtr ty = getTypeFromDeclTypeGetter(arg->type, error);
                if (error.isError())
                    return 0;
                if (!ty)
                {
                    error.set(KIARA_INVALID_TYPE, "Could not create type");
                    return 0;
                }

                paramTypes[i].second = ty;
                paramTypes[i].first = arg->name;
            }
            KIARA::FunctionType::Ptr fty = KIARA::FunctionType::create(declType->name, returnType, paramTypes);
            for (i = 0; i < funcDecl->numArgs; ++i)
            {
                arg = &funcDecl->args[i];
                KIARA::Type::Ptr argTy = fty->getParamType(i);
                KIARA::ElementData &data = fty->getParamElementDataAt(i);

                if (arg->annotation)
                {
                    data.setAttributeValue<KIARA::AnnotationAttr>(arg->annotation);
                }

                KIARA::MemberSemantics &memberSemantics = data.getOrCreateAttributeValueRef<KIARA::MemberSemanticsAttr>();
                parseSemanticsField(arg->semantics, argTy, memberSemantics);
            }

            fty->setAttributeValue<KIARA::WrapperFuncAttr>(declType->typeDecl.funcDecl->wrapperFunc);
            type = fty;
        }
        break;
        case KIARA_TYPE_SERVICE:
        {
            KIARA_DeclService *serviceDecl = declType->typeDecl.serviceDecl;
            KIARA_DeclServiceArgument *arg;
            size_t i;

            KIARA::TypePtr returnType = getTypeFromDeclTypeGetter(serviceDecl->returnType, error);
            if (error.isError())
                return 0;
            if (!returnType)
            {
                error.set(KIARA_INVALID_TYPE, "Could not create return type of service function");
                return 0;
            }

            KIARA::FunctionType::ParamTypes paramTypes(serviceDecl->numArgs);

            for (i = 0; i < serviceDecl->numArgs; ++i)
            {
                arg = &serviceDecl->args[i];
                KIARA::TypePtr ty = getTypeFromDeclTypeGetter(arg->type, error);
                if (error.isError())
                    return 0;
                if (!ty)
                {
                    error.set(KIARA_INVALID_TYPE, "Could not create type");
                    return 0;
                }

                paramTypes[i].second = ty;
                paramTypes[i].first = arg->name;
            }
            KIARA::FunctionType::Ptr fty = KIARA::FunctionType::create(declType->name, returnType, paramTypes);
            for (i = 0; i < serviceDecl->numArgs; ++i)
            {
                arg = &serviceDecl->args[i];
                KIARA::Type::Ptr argTy = fty->getParamType(i);
                KIARA::ElementData &data = fty->getParamElementDataAt(i);

                if (arg->annotation)
                {
                    data.setAttributeValue<KIARA::AnnotationAttr>(arg->annotation);
                }

                KIARA::MemberSemantics &memberSemantics = data.getOrCreateAttributeValueRef<KIARA::MemberSemanticsAttr>();
                parseSemanticsField(arg->semantics, argTy, memberSemantics);
            }

            fty->setAttributeValue<KIARA::ServiceFuncAttr>(serviceDecl->serviceFunc);
            fty->setAttributeValue<KIARA::ServiceWrapperFuncAttr>(serviceDecl->serviceWrapperFunc);
            type = fty;
        }
        break;
        case KIARA_TYPE_POINTER:
        {
            KIARA_DeclPtr *ptrDecl = declType->typeDecl.ptrDecl;
            KIARA::TypePtr elementType = getTypeFromDeclTypeGetter(ptrDecl->elementType, error);
            if (error.isError())
                return 0;
            if (!elementType)
            {
                error.set(KIARA_INVALID_TYPE, "Could not create type");
                return 0;
            }

            type = world().type_c_ptr(elementType);
        }
        break;
        case KIARA_TYPE_REFERENCE:
        {
            KIARA_DeclPtr *ptrDecl = declType->typeDecl.ptrDecl;
            KIARA::TypePtr elementType = getTypeFromDeclTypeGetter(ptrDecl->elementType, error);
            if (error.isError())
                return 0;
            if (!elementType)
            {
                error.set(KIARA_INVALID_TYPE, "Could not create type");
                return 0;
            }

            type = world().type_c_ref(elementType);
        }
        break;
        case KIARA_TYPE_ARRAY:
        {
            KIARA_DeclPtr *arrayDecl = declType->typeDecl.ptrDecl;
            KIARA::TypePtr elementType = getTypeFromDeclTypeGetter(arrayDecl->elementType, error);
            if (error.isError())
                return 0;
            if (!elementType)
            {
                error.set(KIARA_INVALID_TYPE, "Could not create type");
                return 0;
            }

            type = KIARA::ArrayType::get(elementType);
        }
        break;
        case KIARA_TYPE_FIXED_ARRAY:
        {
            KIARA_DeclFixedArray *arrayDecl = declType->typeDecl.arrayDecl;
            KIARA::TypePtr elementType = getTypeFromDeclTypeGetter(arrayDecl->elementType, error);
            if (error.isError())
                return 0;
            if (!elementType)
            {
                error.set(KIARA_INVALID_TYPE, "Could not create type");
                return 0;
            }

            type = KIARA::FixedArrayType::get(elementType, static_cast<int64_t>(arrayDecl->size));
        }
        break;
        case KIARA_TYPE_FIXED_ARRAY_2D:
        {
            KIARA_DeclFixedArray2D *array2DDecl = declType->typeDecl.array2DDecl;
            KIARA::TypePtr elementType = getTypeFromDeclTypeGetter(array2DDecl->elementType, error);
            if (error.isError())
                return 0;
            if (!elementType)
            {
                error.set(KIARA_INVALID_TYPE, "Could not create type");
                return 0;
            }

            type =  KIARA::FixedArrayType::get(
                KIARA::FixedArrayType::get(elementType, static_cast<int64_t>(array2DDecl->numCols)),
                static_cast<int64_t>(array2DDecl->numRows));
        }
        break;
        case KIARA_TYPE_OPAQUE:
        {
            KIARA_DeclOpaqueType *opaqueTypeDecl = declType->typeDecl.opaqueTypeDecl;

            KIARA::StructType::Ptr structType = KIARA::StructType::create(world(), declType->name);

            for (size_t i = 0; i < opaqueTypeDecl->numAPIFuncs; ++i)
            {
                KIARA_DeclAPIFunc &apiFuncDecl = opaqueTypeDecl->apiFuncs[i];
                std::string attrKey = KIARA_ATTR_PREFIX_API;
                attrKey += apiFuncDecl.apiName;

                KIARA::NamedGenericFunc &genFunc = structType->getOrCreateAttributeValueRef<KIARA::GenericFuncAttr>(attrKey);
                genFunc.func = apiFuncDecl.apiFunc;
                genFunc.funcName = apiFuncDecl.apiFuncName;
            }

            DFC_DEBUG("OPAQUE TYPE: "<<structType->toString());

            type = structType;
        }
        break;
        case KIARA_TYPE_ENCRYPTED:
        {
            /*
             * Encrypted type T is represented as a pointer to the struct containing kr_dbuffer_t type.
             * The struct is annotated with the 'nativeEncrypted' annotation containing type T.
             */

            KIARA_DeclEncrypted *encryptedDecl = declType->typeDecl.encryptedDecl;

            KIARA::TypePtr elementType = getTypeFromDeclTypeGetter(encryptedDecl->elementType, error);
            if (error.isError())
                return 0;
            if (!elementType)
            {
                error.set(KIARA_INVALID_TYPE, "Could not create type");
                return 0;
            }

            KIARA::StructType::Ptr structType = KIARA::StructType::create(world(), declType->name, 1);

            structType->setElementAt(0, getDBufferPtrType()->getElementType());
            structType->setElementNameAt(0, "data");

            structType->setAttributeValue<KIARA::NativeEncryptedTypeAttr>(elementType);
            structType->setAttributeValue<KIARA::AllocateTypeAPIAttr>(KIARA::NamedGenericFunc((KIARA_GenericFunc)&kr_dbuffer_allocate, ""));
            structType->setAttributeValue<KIARA::DeallocateTypeAPIAttr>(KIARA::NamedGenericFunc((KIARA_GenericFunc)&kr_dbuffer_deallocate, ""));

            DFC_DEBUG("ENCRYPTED TYPE: "<<structType->toString());

            type = KIARA::PtrType::get(structType);
        }
        break;
    }

    if (type)
        declCacheMap_[declType] = type;
    return type;
}

KIARA_Type * Context::getTypeFromDecl(KIARA_GetDeclType declTypeGetter, Error &error)
{
    assert(declTypeGetter != 0);

    return wrapType(getTypeFromDeclTypeGetter(declTypeGetter, error));
}

bool Context::loadIDL(std::istream &in, const std::string &fileName)
{
    KIARA::IDLParserContext ctx(module_, in, fileName);
    return ctx.parse();
}

bool Context::loadIDLFromURL(const std::string &url)
{
    std::string idlContents;
    bool result = KIARA::URLLoader::load(url, idlContents);
    if (!result)
        return false;
    std::istringstream iss(idlContents);
    return loadIDL(iss, url);
}

bool Context::loadIDLFromURL(KIARA::URLLoader::Connection * handle, const std::string &url)
{
    std::string idlContents;
    bool result = KIARA::URLLoader::load(handle, url, idlContents);
    if (!result)
        return false;
    std::istringstream iss(idlContents);
    return loadIDL(iss, url);
}

bool Context::loadIDL(const std::string &fileName)
{
    std::ifstream inf(fileName.c_str());
    return loadIDL(inf, fileName);
}

KIARA_Result Context::loadLLVMModule(const std::string &fileName)
{
    llvmModuleNames_.push_back(fileName);
    return KIARA_SUCCESS;
}

// KIARA_Base

Base::Base(Context *context) :
    context_(context)
{
}

Base::~Base()
{
}

// KIARA_Global

bool Global::initialized_ = false;
KIARA::LibraryConfiguration Global::libraryConfiguration_;

int Global::initialize(int *argc, char **argv)
{
    if (initialized_)
        return KIARA_INIT_ERROR;

    initialized_ = true;

    // Library
    KIARA::LibraryInit::initialize();
    libraryConfiguration_.setToDefaults();
    libraryConfiguration_.parseCommandLine(argc, argv);
    std::string errorMsg;
    if (!libraryConfiguration_.load(&errorMsg))
    {
        // FIXME use logging for this
        std::cerr<<"KIARA: Configuration loading error: "<<errorMsg<<std::endl;
    }

    if (libraryConfiguration_.moduleSearchPath.empty())
    {
        // FIXME use logging for this
        std::cerr<<"KIARA: Warning: KIARA_MODULE_PATH was not set !"<<std::endl;
    }

    DFC_DEBUG("Loaded configuration:\n"<<KIARA::ValueIO::toJSON(libraryConfiguration_.config));

#ifdef HAVE_LLVM
    KIARA::llvmInitialize();
    KIARA::llvmInitializeJIT();
#endif

    return KIARA_SUCCESS;
}

int Global::finalize()
{
    if (!initialized_)
        return KIARA_FINI_ERROR;

    // Library
    KIARA::LibraryInit::shutdown();
#ifdef HAVE_LLVM
    KIARA::llvmShutdown();
#endif

    initialized_ = false;

    return KIARA_SUCCESS;
}

} // namespace Impl

} // namespace KIARA

// API Wrapper Code

int kiaraInit(int *argc, char **argv)
{
    return KIARA::Impl::Global::initialize(argc, argv);
}

int kiaraFinalize(void)
{
    return KIARA::Impl::Global::finalize();
}

KIARA_Context * kiaraNewContext()
{
    if (!KIARA::Impl::Global::isInitialized())
        return 0;
    return KIARA::Impl::wrap(new KIARA::Impl::Context);
}

int kiaraFreeContext(KIARA_Context *context)
{
    delete KIARA::Impl::unwrap(context);
    return KIARA_SUCCESS;
}

const char * kiaraGetContextError(KIARA_Context *context)
{
    assert(context != 0);
    return KIARA::Impl::unwrap(context)->getErrorMessage();
}

KIARA_Result kiaraGetContextErrorCode(KIARA_Context *context)
{
    assert(context != 0);
    return KIARA::Impl::unwrap(context)->getErrorCode();
}

void kiaraClearContextError(KIARA_Context *context)
{
    assert(context != 0);
    KIARA::Impl::unwrap(context)->clearError();
}


#define BUILTIN_TYPE(T)                                         \
    KIARA_Type * KIARA_JOIN(kiaraType_,T)(KIARA_Context *ctx)   \
    {                                                           \
        assert(ctx != 0);                                       \
        return KIARA::Impl::unwrap(ctx)->KIARA_JOIN(type_,T)(); \
    }
BUILTIN_TYPE(c_char_p)
BUILTIN_TYPE(c_void_p)
#include <KIARA/DB/Type.def>

KIARA_Type * kiaraType_c_ptr(KIARA_Type *elementType)
{
    assert(elementType != 0);
    KIARA_Context *ctx = reinterpret_cast<KIARA_Context*>(&((KIARA::Type::RawPtr)elementType)->getWorld());
    return KIARA::Impl::unwrap(ctx)->type_c_ptr(elementType);
}

KIARA_Type * kiaraType_c_int_of_size(KIARA_Context *ctx, size_t size)
{
    switch (size)
    {
        case 1: return kiaraType_c_int8_t(ctx);
        case 2: return kiaraType_c_int16_t(ctx);
        case 4: return kiaraType_c_int32_t(ctx);
        case 8: return kiaraType_c_int64_t(ctx);
        default:
            break;
    }
    return 0;
}

KIARA_Type * kiaraType_c_uint_of_size(KIARA_Context *ctx, size_t size)
{
    switch (size)
    {
        case 1: return kiaraType_c_uint8_t(ctx);
        case 2: return kiaraType_c_uint16_t(ctx);
        case 4: return kiaraType_c_uint32_t(ctx);
        case 8: return kiaraType_c_uint64_t(ctx);
        default:
            break;
    }
    return 0;
}

KIARA_API KIARA_Type * kiaraType_array(KIARA_Type *elementType)
{
    // TODO implement
    return 0;
}

KIARA_API KIARA_Type * kiaraType_fixed_array(KIARA_Type *elementType, size_t size)
{
    // TODO implement
    return 0;
}

KIARA_Type * kiaraDeclareStructType(KIARA_Context *ctx, const char *name, int numMembers, KIARA_StructDecl members[])
{
    assert(ctx != 0);
    return KIARA::Impl::unwrap(ctx)->declareStructType(name, numMembers, members);
}

KIARA_API KIARA_Type * kiaraDeclareFuncType(KIARA_Context *ctx, const char *name, KIARA_Type *returnType, int numArgs, KIARA_FuncDecl args[])
{
    assert(ctx != 0);
    return 0; // FIXME
}

// unnamed namespace

KIARA_Connection * kiaraOpenConnection(KIARA_Context *context, const char *uri)
{
    assert(context != 0);
    if (KIARA::Impl::unwrap(context)->isError())
        return 0;

    // FIXME This is just a temporary dummy connection for testing
    std::auto_ptr<KIARA::Impl::ClientConnection> conn(
        new KIARA::Impl::ClientConnection(KIARA::Impl::unwrap(context), uri));
    if (conn->isError())
    {
        KIARA::Impl::unwrap(context)->setError(conn->getErrorCode(), conn->getErrorMessage());
        conn.reset();
    }

    return KIARA::Impl::wrap(conn.release());
}

/** Closes connection.
 *  @return KIARA_SUCCESS if connection was successfully closed.
 */
int kiaraCloseConnection(KIARA_Connection *connection)
{
    delete KIARA::Impl::unwrap(connection);
    return KIARA_SUCCESS;
}

const char * kiaraGetConnectionError(KIARA_Connection *connection)
{
    assert(connection != 0);
    return KIARA::Impl::unwrap(connection)->getErrorMessage();
}

void kiaraClearConnectionError(KIARA_Connection *connection)
{
    assert(connection != 0);
    return KIARA::Impl::unwrap(connection)->clearError();
}

KIARA_Type * kiaraGetTypeByName(KIARA_Connection *connection, const char *name)
{
    assert(connection != 0 && name != 0);
    assert(KIARA::Impl::unwrap(connection)->getContext() != 0);
    return KIARA::Impl::unwrap(connection)->getContext()->getTypeByName(name);
}

KIARA_API KIARA_Type * kiaraGetTypeFromContext(KIARA_Context *ctx, KIARA_GetDeclType declTypeGetter)
{
    assert(ctx != 0 && declTypeGetter != 0);
    return kiaraGetContextTypeFromDecl(ctx, declTypeGetter);
}

KIARA_Type * kiaraGetTypeFromDecl(KIARA_Connection *connection, KIARA_GetDeclType declTypeGetter)
{
    assert(connection != 0 && declTypeGetter != 0);
    KIARA_Context *ctx = KIARA::Impl::wrap(KIARA::Impl::unwrap(connection)->getContext());
    return kiaraGetContextTypeFromDecl(ctx, declTypeGetter);
}

/** Get type handle from static declaration
 */
KIARA_Type * kiaraGetContextTypeFromDecl(KIARA_Context *context, KIARA_GetDeclType declTypeGetter)
{
    assert(context != 0 && declTypeGetter != 0);
    return KIARA::Impl::unwrap(context)->getTypeFromDecl(declTypeGetter, KIARA::Impl::unwrap(context)->getError());
}

/** Get context from connection */
KIARA_Context * kiaraGetContext(KIARA_Connection *connection)
{
    assert(connection != 0);
    return KIARA::Impl::wrap(KIARA::Impl::unwrap(connection)->getContext());
}

KIARA_FuncObj * kiaraGenerateClientFuncObj(KIARA_Connection *connection, const char *idlMethodName, KIARA_GetDeclType declTypeGetter, const char *mapping)
{
    assert(connection != 0 && declTypeGetter != 0 && idlMethodName != 0);
    return KIARA::Impl::unwrap(connection)->generateClientFuncObj(idlMethodName, declTypeGetter, mapping);
}

KIARA_Type * kiaraDeclareOpaqueType(KIARA_Context *ctx, const char *name)
{
    // TODO implement
    return 0;
}

KIARA_Type * kiaraGetContextTypeByName(KIARA_Context *ctx, const char *name)
{
	assert(ctx != 0);
	return KIARA::Impl::unwrap(ctx)->getTypeByName(name);
}

KIARA_Type * kiaraGetOrDeclareContextTypeByName(KIARA_Context *ctx, const char *name, KIARA_TypeDeclarator declarator)
{
    assert(ctx != 0);
    assert(declarator != 0);

    KIARA_Type *type = kiaraGetContextTypeByName(ctx, name);
    if (type)
        return type;
    return declarator(ctx);
}

KIARA_Result kiaraLoadLLVMModule(KIARA_Context *ctx, const char *name)
{
    assert(ctx != 0);
    return KIARA::Impl::unwrap(ctx)->loadLLVMModule(name);
}

int kiaraMapType(KIARA_Type *abstractType, KIARA_Type *nativeType)
{
    // TODO implement
    return 0;
}

int kiaraMapName(KIARA_Connection *connection, const char *abstractTypePath, const char *nativeTypePath)
{
    // TODO implement
    return 0;
}

int kiaraTypedef(const char *newTypeName, KIARA_Type *type)
{
    // TODO implement
    return 0;
}

// Service

KIARA_Service * kiaraNewService(KIARA_Context *context)
{
    assert(context != 0);
    if (KIARA::Impl::unwrap(context)->isError())
        return 0;

    std::auto_ptr<KIARA::Impl::Service> service(
        new KIARA::Impl::Service(KIARA::Impl::unwrap(context)));
    if (service->isError())
    {
        KIARA::Impl::unwrap(context)->setError(service->getErrorCode(), service->getErrorMessage());
        service.reset();
    }

    return KIARA::Impl::wrap(service.release());
}

KIARA_Result kiaraFreeService(KIARA_Service *service)
{
    delete KIARA::Impl::unwrap(service);
    return KIARA_SUCCESS;
}

const char * kiaraGetServiceError(KIARA_Service *service)
{
    assert(service != 0);
    return KIARA::Impl::unwrap(service)->getErrorMessage();
}

void kiaraClearServiceError(KIARA_Service *service)
{
    assert(service != 0);
    return KIARA::Impl::unwrap(service)->clearError();
}

KIARA_Type * kiaraGetServiceTypeByName(KIARA_Service *service, const char *name)
{
    assert(service != 0 && name != 0);
    assert(KIARA::Impl::unwrap(service)->getContext() != 0);
    return KIARA::Impl::unwrap(service)->getContext()->getTypeByName(name);
}

KIARA_Type * kiaraGetServiceTypeFromDecl(KIARA_Service *service, KIARA_GetDeclType declTypeGetter)
{
    assert(service != 0 && declTypeGetter != 0);
    KIARA_Context *ctx = KIARA::Impl::wrap(KIARA::Impl::unwrap(service)->getContext());
    return kiaraGetContextTypeFromDecl(ctx, declTypeGetter);
}

KIARA_Context * kiaraGetServiceContext(KIARA_Service *service)
{
    assert(service != 0);
    return KIARA::Impl::wrap(KIARA::Impl::unwrap(service)->getContext());
}

KIARA_Result kiaraLoadServiceIDL(KIARA_Service *service, const char *fileName)
{
    assert(service != 0);
    return KIARA::Impl::unwrap(service)->loadIDL(fileName);
}

KIARA_Result kiaraLoadServiceIDLFromString(KIARA_Service *service, const char *idlLanguage, const char *idlContents)
{
    assert(service != 0);
    return KIARA::Impl::unwrap(service)->loadIDLFromString(idlLanguage, idlContents);
}

KIARA_Result kiaraRegisterServiceFunc(KIARA_Service *service, const char *idlMethodName, KIARA_GetDeclType declTypeGetter, const char *mapping, KIARA_ServiceFunc func)
{
    assert(service != 0 && idlMethodName != 0 && declTypeGetter != 0);
    return KIARA::Impl::unwrap(service)->registerServiceFunc(idlMethodName, declTypeGetter, mapping, func);
}

// Server


KIARA_Server * kiaraNewServer(KIARA_Context *context, const char *host, unsigned int port, const char *configPath)
{
    assert(context != 0);
    if (KIARA::Impl::unwrap(context)->isError())
        return 0;

    std::auto_ptr<KIARA::Impl::Server> server(
        new KIARA::Impl::Server(KIARA::Impl::unwrap(context), host, port, configPath));
    if (server->isError())
    {
        KIARA::Impl::unwrap(context)->setError(server->getErrorCode(), server->getErrorMessage());
        server.reset();
    }

    return KIARA::Impl::wrap(server.release());
}

KIARA_Result kiaraAddService(KIARA_Server *server, const char *path, const char *protocol, KIARA_Service *service)
{
    assert(server != 0 && path != 0);
    return KIARA::Impl::unwrap(server)->addService(path, protocol, KIARA::Impl::unwrap(service));
}

KIARA_Result kiaraRemoveService(KIARA_Server *server, KIARA_Service *service)
{
    assert(server != 0);
    return KIARA::Impl::unwrap(server)->removeService(KIARA::Impl::unwrap(service));
}

/** Returns description of the error or NULL if no error occurred. */
const char * kiaraGetServerError(KIARA_Server *server)
{
    assert(server != 0);
    return KIARA::Impl::unwrap(server)->getErrorMessage();
}

/** Reset error state of a connection */
void kiaraClearServerError(KIARA_Server *server)
{
    assert(server != 0);
    return KIARA::Impl::unwrap(server)->clearError();
}

/** Get parent context of the server. */
KIARA_Context * kiaraGetServerContext(KIARA_Server *server)
{
    assert(server != 0);
    return KIARA::Impl::wrap(KIARA::Impl::unwrap(server)->getContext());
}

KIARA_Result kiaraFreeServer(KIARA_Server * server)
{
    delete KIARA::Impl::unwrap(server);
    return KIARA_SUCCESS;
}

KIARA_Result kiaraRunServer(KIARA_Server * server)
{
    assert(server != 0);
    return KIARA::Impl::unwrap(server)->run();
}

const char * kiaraGetSecretKeyText(KIARA_Connection *connection, const char *keyName)
{
    assert(connection != 0);
    return KIARA::Impl::unwrap(connection)->getContext()->getSecurityConfiguraton().encryptionPassword.c_str();
}

// Version

int kiaraGetVersionMajor(void)
{
    return KIARA_VERSION_MAJOR;
}

int kiaraGetVersionMinor(void)
{
    return KIARA_VERSION_MINOR;
}

int kiaraGetVersionPatch(void)
{
    return KIARA_VERSION_PATCH;
}

// Debug

void kiaraDbgDumpType(KIARA_Type *type)
{
    KIARA::Type::RawPtr p =
            reinterpret_cast<KIARA::Type::RawPtr>(type);
    std::cout<<"KIARA Type: ";
    if (p)
    {
        p->printRepr(std::cout);
        std::cout<<", Canonical Type: ";
        p->getCanonicalType()->printRepr(std::cout);
    }
    else
        std::cout<<"NULL";
    std::cout<<std::endl;
}

void kiaraDbgSimulateCall(KIARA_Service *service, const char *requestData)
{
    assert(service != 0);
    /* FIXME This feature is not supported anymore !!! */
    /* service->dbgSimulateCall(requestData); */
}
