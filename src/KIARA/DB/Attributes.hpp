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
 * Attributes.hpp
 *
 *  Created on: 29.05.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_DB_ATTRIBUTES_HPP_INCLUDED
#define KIARA_DB_ATTRIBUTES_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <KIARA/kiara.h>
#include <KIARA/DB/Value.hpp>
#include <KIARA/DB/Type.hpp>
#include <KIARA/DB/Annotation.hpp>
#include <KIARA/DB/MemberSemantics.hpp>
#include <KIARA/DB/TypeSemantics.hpp>

#define KIARA_ATTR_PREFIX_API "api."

#define KIARA_ATTR_SYM_SETCSTRING_API SetCString
#define KIARA_ATTR_NAME_SETCSTRING_API KIARA_STRINGIZE(KIARA_ATTR_SYM_SETCSTRING_API)
#define KIARA_ATTR_SETCSTRING_API KIARA_ATTR_PREFIX_API KIARA_ATTR_NAME_SETCSTRING_API

#define KIARA_ATTR_SYM_GETCSTRING_API GetCString
#define KIARA_ATTR_NAME_GETCSTRING_API KIARA_STRINGIZE(KIARA_ATTR_SYM_GETCSTRING_API)
#define KIARA_ATTR_GETCSTRING_API KIARA_ATTR_PREFIX_API KIARA_ATTR_NAME_GETCSTRING_API

#define KIARA_ATTR_SYM_SETGENERICERROR_API SetGenericError
#define KIARA_ATTR_NAME_SETGENERICERROR_API KIARA_STRINGIZE(KIARA_ATTR_SYM_SETGENERICERROR_API)
#define KIARA_ATTR_SETGENERICERROR_API KIARA_ATTR_PREFIX_API KIARA_ATTR_NAME_SETGENERICERROR_API

#define KIARA_ATTR_SYM_GETGENERICERROR_API GetGenericError
#define KIARA_ATTR_NAME_GETGENERICERROR_API KIARA_STRINGIZE(KIARA_ATTR_SYM_GETGENERICERROR_API)
#define KIARA_ATTR_GETGENERICERROR_API KIARA_ATTR_PREFIX_API KIARA_ATTR_NAME_GETGENERICERROR_API

#define KIARA_ATTR_SYM_ALLOCATETYPE_API AllocateType
#define KIARA_ATTR_NAME_ALLOCATETYPE_API KIARA_STRINGIZE(KIARA_ATTR_SYM_ALLOCATETYPE_API)
#define KIARA_ATTR_ALLOCATETYPE_API KIARA_ATTR_PREFIX_API KIARA_ATTR_NAME_ALLOCATETYPE_API

#define KIARA_ATTR_SYM_DEALLOCATETYPE_API DeallocateType
#define KIARA_ATTR_NAME_DEALLOCATETYPE_API KIARA_STRINGIZE(KIARA_ATTR_SYM_DEALLOCATETYPE_API)
#define KIARA_ATTR_DEALLOCATETYPE_API KIARA_ATTR_PREFIX_API KIARA_ATTR_NAME_DEALLOCATETYPE_API

#define KIARA_ATTR_EXCEPTION_TYPE "exceptionType"
#define KIARA_ATTR_ANNOTATION_TYPE "annotationType"
#define KIARA_ATTR_ANNOTATION_LIST "annotationList"
#define KIARA_ATTR_NATIVE_SIZE "nativeSize"
#define KIARA_ATTR_NATIVE_OFFSET "nativeOffset"
#define KIARA_ATTR_NATIVE_ALIGNMENT "nativeAlignment"
#define KIARA_ATTR_ANNOTATION "annotation"
#define KIARA_ATTR_WRAPPER_FUNC "wrapperFunc"
#define KIARA_ATTR_SERVICE_FUNC "serviceFunc"
#define KIARA_ATTR_SERVICE_WRAPPER_FUNC "serviceWrapperFunc"
#define KIARA_ATTR_DEFAULT_FIELD_VALUE "defaultFieldValue"
#define KIARA_ATTR_MAIN_MEMBER "mainMember"
#define KIARA_ATTR_DEPENDENT_MEMBERS "dependentMembers"
#define KIARA_ATTR_MEMBER_SEMANTICS "memberSemantics"
#define KIARA_ATTR_TYPE_SEMANTICS "typeSemantics"

#define KIARA_ATTR_NATIVE_ENCRYPTED_TYPE "nativeEncrypted"

namespace KIARA
{

// Attribute tags

template <typename T>
struct AttributeTag
{
    typedef T type;
    typedef const T & arg_type;

    static type getDefaultValue()
    {
        return T();
    }
};

template <typename T>
struct AnyAttrTag : public AttributeTag<T>
{
    static bool isValidValue(const Value &value)
    {
        if (!value.isAny())
            return false;
        const boost::any & anyval = value.getAny();
        return boost::any_cast<T>(&anyval) != 0;
    }

    static T get(const Value &value)
    {
        return boost::any_cast<T>(value.getAny());
    }

    static T* get_ptr(Value &value)
    {
        if (!value.isAny())
            return 0;
        boost::any & anyval = value.getAny();
        return boost::any_cast<T>(&anyval);
    }

    static const T * get_const_ptr(const Value &value)
    {
        if (!value.isAny())
            return 0;
        const boost::any & anyval = value.getAny();
        return boost::any_cast<T>(&anyval);
    }

    static void set(Value &value, const T &newValue)
    {
        value.setAny(newValue);
    }
};

template <typename T>
struct PointerAttrTag : public AnyAttrTag<T>
{
    typedef T arg_type;

    static typename AnyAttrTag<T>::type getDefaultValue()
    {
        return 0;
    }
};

template <typename T>
struct NumberAttrTag : public AttributeTag<T>
{
    typedef T arg_type;

    static bool isValidValue(const Value &value)
    {
        return value.isNumber();
    }

    static T get(const Value &value)
    {
        return value.getNumber().get<T>();
    }

    static void set(Value &value, const T newValue)
    {
        value.setNumber(normalize_value(newValue));
    }

    static typename AttributeTag<T>::type getDefaultValue()
    {
        return 0;
    }
};

struct BoolAttrTag : public AttributeTag<bool>
{
    typedef bool arg_type;

    static bool isValidValue(const Value &value)
    {
        return value.isBool();
    }

    static bool get(const Value &value)
    {
        return value.getBool();
    }

    static void set(Value &value, const bool newValue)
    {
        value.setBool(newValue);
    }

    static bool getDefaultValue()
    {
        return false;
    }
};

struct StringAttrTag : public AttributeTag<std::string>
{
    typedef const std::string & arg_type;

    static bool isValidValue(const Value &value)
    {
        return value.isString();
    }

    static std::string get(const Value &value)
    {
        return value.getString();
    }

    static void set(Value &value, const std::string &newValue)
    {
        value.setString(newValue);
    }
};

struct ObjectAttrTag : public AttributeTag<Object::Ptr>
{
    typedef const Object::Ptr & arg_type;

    static bool isValidValue(const Value &value)
    {
        return value.isObject();
    }

    static Object::Ptr get(const Value &value)
    {
        return value.getObject();
    }

    static void set(Value &value, const Object::Ptr &newValue)
    {
        value.setObject(newValue);
    }
};

struct TypeAttrTag : public AttributeTag<Type::Ptr>
{
    typedef const Type::Ptr & arg_type;

    static bool isValidValue(const Value &value)
    {
        return value.isObject() && isa<Type>(value.getObject());
    }

    static Type::Ptr get(const Value &value)
    {
        return dyn_cast<Type>(value.getObject());
    }

    static void set(Value &value, const Type::Ptr &newValue)
    {
        value.setObject(newValue);
    }
};

// Attributes

struct NamedGenericFunc
{
    KIARA_GenericFunc func;
    std::string funcName;

    NamedGenericFunc()
        : func(0)
        , funcName()
    { }

    NamedGenericFunc(KIARA_GenericFunc func, const std::string &funcName)
        : func(func)
        , funcName(funcName)
    { }

    NamedGenericFunc(const NamedGenericFunc &other)
        : func(other.func)
        , funcName(other.funcName)
    { }

    NamedGenericFunc & operator=(const NamedGenericFunc &other)
    {
        func = other.func;
        funcName = other.funcName;
        return *this;
    }

    void swap(NamedGenericFunc &other)
    {
        std::swap(func, other.func);
        funcName.swap(other.funcName);
    }
};

struct GenericFuncAttr : public AnyAttrTag<NamedGenericFunc>
{
};

struct WrapperFuncAttr : public PointerAttrTag<KIARA_Func>
{
    static const char *getAttrName()
    {
        return KIARA_ATTR_WRAPPER_FUNC;
    }
};

struct ServiceFuncAttr : public PointerAttrTag<KIARA_ServiceFunc>
{
    static const char *getAttrName()
    {
        return KIARA_ATTR_SERVICE_FUNC;
    }
};

struct ServiceWrapperFuncAttr : public PointerAttrTag<KIARA_VAServiceFunc>
{
    static const char *getAttrName()
    {
        return KIARA_ATTR_SERVICE_WRAPPER_FUNC;
    }
};


struct GetCStringAPIAttr : public GenericFuncAttr
{
    static const char *getAttrName()
    {
        return KIARA_ATTR_GETCSTRING_API;
    }
};

struct SetCStringAPIAttr : public GenericFuncAttr
{
    static const char *getAttrName()
    {
        return KIARA_ATTR_SETCSTRING_API;
    }
};

struct GetGenericErrorAPIAttr : public GenericFuncAttr
{
    static const char *getAttrName()
    {
        return KIARA_ATTR_GETGENERICERROR_API;
    }
};

struct SetGenericErrorAPIAttr : public GenericFuncAttr
{
    static const char *getAttrName()
    {
        return KIARA_ATTR_SETGENERICERROR_API;
    }
};

struct AllocateTypeAPIAttr : public GenericFuncAttr
{
    static const char *getAttrName()
    {
        return KIARA_ATTR_ALLOCATETYPE_API;
    }
};

struct DeallocateTypeAPIAttr : public GenericFuncAttr
{
    static const char *getAttrName()
    {
        return KIARA_ATTR_DEALLOCATETYPE_API;
    }
};

struct ExceptionTypeAttr : public BoolAttrTag
{
    static const char *getAttrName()
    {
        return KIARA_ATTR_EXCEPTION_TYPE;
    }
};

struct AnnotationTypeAttr : public BoolAttrTag
{
    static const char *getAttrName()
    {
        return KIARA_ATTR_ANNOTATION_TYPE;
    }
};

struct NativeSizeAttr : public NumberAttrTag<size_t>
{
    static const char *getAttrName()
    {
        return KIARA_ATTR_NATIVE_SIZE;
    }
};

struct NativeOffsetAttr : public NumberAttrTag<size_t>
{
    static const char *getAttrName()
    {
        return KIARA_ATTR_NATIVE_OFFSET;
    }
};

struct NativeAlignmentAttr : public NumberAttrTag<size_t>
{
    static const char *getAttrName()
    {
        return KIARA_ATTR_NATIVE_ALIGNMENT;
    }
};

struct AnnotationAttr : public StringAttrTag
{
    static const char *getAttrName()
    {
        return KIARA_ATTR_ANNOTATION;
    }
};

struct NativeEncryptedTypeAttr : public TypeAttrTag
{
    static const char *getAttrName()
    {
        return KIARA_ATTR_NATIVE_ENCRYPTED_TYPE;
    }
};

struct AnnotationListAttr : public AnyAttrTag<AnnotationList>
{
    typedef const AnnotationList & arg_type;

    static AnnotationList getDefaultValue()
    {
        return AnnotationList();
    }

    static const char *getAttrName()
    {
        return KIARA_ATTR_ANNOTATION_LIST;
    }
};

struct DefaultFieldValueAttr : public AttributeTag<Expr::Ptr>
{
    typedef const Expr::Ptr & arg_type;

    static bool isValidValue(const Value &value)
    {
        return value.isObject() && isa<Expr>(value.getObject());
    }

    static Expr::Ptr get(const Value &value)
    {
        return dyn_cast<Expr>(value.getObject());
    }

    static void set(Value &value, const Expr::Ptr &newValue)
    {
        value.setObject(newValue);
    }

    static Expr::Ptr getDefaultValue()
    {
        return 0;
    }

    static const char *getAttrName()
    {
        return KIARA_ATTR_DEFAULT_FIELD_VALUE;
    }
};

struct MainMemberAttr : public StringAttrTag
{
    static const char *getAttrName()
    {
        return KIARA_ATTR_MAIN_MEMBER;
    }
};

struct DependentMembersAttr : public AnyAttrTag<std::vector<std::string> >
{
    typedef const std::vector<std::string> & arg_type;

    static std::vector<std::string> getDefaultValue()
    {
        return std::vector<std::string>();
    }

    static const char *getAttrName()
    {
        return KIARA_ATTR_DEPENDENT_MEMBERS;
    }
};

struct TypeSemanticsAttr : public AnyAttrTag<TypeSemantics>
{
    typedef const TypeSemantics & arg_type;

    static TypeSemantics getDefaultValue()
    {
        return TypeSemantics();
    }

    static const char *getAttrName()
    {
        return KIARA_ATTR_TYPE_SEMANTICS;
    }
};

struct MemberSemanticsAttr : public AnyAttrTag<MemberSemantics>
{
    typedef const MemberSemantics & arg_type;

    static MemberSemantics getDefaultValue()
    {
        return MemberSemantics();
    }

    static const char *getAttrName()
    {
        return KIARA_ATTR_MEMBER_SEMANTICS;
    }
};

} // namespace KIARA

#endif /* KIARA_ATTRIBUTES_HPP_INCLUDED */
