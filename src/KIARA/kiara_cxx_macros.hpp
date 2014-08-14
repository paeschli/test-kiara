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
/*
 * kiara_cxx_macros.hpp
 *
 *  Created on: 11.12.2012
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_CXX_MACROS_HPP_INCLUDED
#define KIARA_CXX_MACROS_HPP_INCLUDED

#include <KIARA/kiara.h>
#include <KIARA/Common/TypeTraits.hpp>
#include <KIARA/Utils/Error.hpp>
#include <string>

#include <boost/typeof/typeof.hpp>
#include <boost/static_assert.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/transform.hpp>
#include <boost/assert.hpp>

#if defined(_MSC_VER) && !defined(__clang__)
#define typeofmember(s,m) BOOST_TYPEOF(((s *)0)->m)
#else
#define typeofmember(s,m) BOOST_TYPEOF(s::m)
#endif

/*
 * Following macro prefixes are used by KIARA:
 *
 * KIARA_    - public macro API
 * _KIARA_   - internal macro API
 * _KR_      - internal macro API, short form
 */

/* Reserved name generators */
#define _KR_tdef(suffix) _KIARA_DTYPE(tdef, suffix)
#define _KR_tdef_generated(suffix) _KIARA_DTYPE(tdef_generated, suffix)
#define _KR_get_type(suffix) _KIARA_DTYPE(get_type, suffix)
#define _KR_type(suffix) _KIARA_DTYPE(type, suffix)
#define _KR_service(suffix) _KIARA_DTYPE(xservice, suffix)
#define _KR_service_args(suffix) _KIARA_DTYPE(xservice_args, suffix)
#define _KR_service_wrapper(suffix) _KIARA_DTYPE(xservice_wrapper, suffix)
#define _KR_service_func_conv(suffix) _KIARA_DTYPE(xservice_func_conv, suffix)
#define _KR_func_wrapper(suffix) _KIARA_DTYPE(xfunc_wrapper, suffix)
#define _KR_service_func(suffix) _KIARA_DTYPE(xservice_func, suffix)
#define _KR_service_func_type(suffix) _KIARA_DTYPE(xservice_func_type, suffix)
#define _KR_ptr(suffix) _KIARA_DTYPE(ptr, suffix)
#define _KR_members(suffix) _KIARA_DTYPE(members, suffix)
#define _KR_funcobj(suffix) _KIARA_DTYPE(funcobj, suffix)
#define _KR_funcobj_ptr(suffix) _KIARA_DTYPE(funcobj_ptr, suffix)
#define _KR_func_wrapper_type(suffix) _KIARA_DTYPE(func_wrapper_type, suffix)
#define _KR_struct(suffix) _KIARA_DTYPE(struct, suffix)
#define _KR_type_mismatch_check(suffix) _KIARA_DTYPE(type_mismatch_check, suffix)
#define _KR_args(suffix) _KIARA_DTYPE(args, suffix)
#define _KR_func(suffix) _KIARA_DTYPE(func, suffix)
#define _KR_array(suffix) _KIARA_DTYPE(array, suffix)
#define _KR_fixedArray(suffix) _KIARA_DTYPE(fixedArray, suffix)
#define _KR_fixedArray2D(suffix) _KIARA_DTYPE(fixedArray2D, suffix)
#define _KR_apiFuncs(suffix) _KIARA_DTYPE(apiFuncs, suffix)
#define _KR_opaque(suffix) _KIARA_DTYPE(opaque, suffix)
#define _KR_api(suffix) _KIARA_DTYPE(xapi, suffix)

/* Common macros */

#define _KIARA_CXX_DTYPE_GETTER(kiara_type_name)                        \
    ::KIARA::TypeTraits<kiara_type_name>::getDeclType

#define KIARA_CXX_TYPE(kiara_type_name) _KIARA_CXX_DTYPE_GETTER(kiara_type_name)

#define KIARA_TYPE(kiara_type_name) KIARA_CXX_TYPE(kiara_type_name)

/* Enum declaration */

#define _KIARA_CXX_ENUM_CONSTANT_EXPAND(elem)                           \
    {(int64_t)BOOST_PP_TUPLE_ELEM(1, 0, elem),                          \
     BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(1, 0, elem))}

#define _KIARA_CXX_ENUM_CONSTANT_SEQELEM(z, i, data)                    \
        _KIARA_CXX_ENUM_CONSTANT_EXPAND(BOOST_PP_SEQ_ELEM(i, data))

#define KIARA_CXX_DECL_ENUM(cxx_enum_name, enum_constants)              \
    namespace KIARA {                                                   \
    template <> class TypeNameOf<cxx_enum_name>                         \
    {                                                                   \
    public:                                                             \
        static const char *get()                                        \
        { return BOOST_PP_STRINGIZE(cxx_enum_name); }                   \
    };                                                                  \
    template <> class TypeTraits<cxx_enum_name>                         \
    {                                                                   \
    public:                                                             \
        typedef cxx_enum_name Type;                                     \
        KIARA_DEFAULT_GET_TYPE_NAME(Type)                               \
        static KIARA_DeclType * getDeclType()                           \
        {                                                               \
            static KIARA_DeclEnumConstant kr_constants[] = {            \
                    BOOST_PP_ENUM(BOOST_PP_SEQ_SIZE(enum_constants),    \
                            _KIARA_CXX_ENUM_CONSTANT_SEQELEM,           \
                            enum_constants)                             \
            };                                                          \
            static KIARA_DeclEnum kr_enum = {                           \
                kr_constants,                                           \
                sizeof(kr_constants) / sizeof(kr_constants[0]),         \
                sizeof(cxx_enum_name)                                   \
            };                                                          \
            static KIARA_DeclType kr_type = {                           \
                BOOST_PP_STRINGIZE(cxx_enum_name),                      \
                KIARA_TYPE_ENUM,                                        \
                KIARA_DTF_NONE,                                         \
                {&kr_enum}                                              \
            };                                                          \
            return &kr_type;                                            \
        }                                                               \
    };                                                                  \
    }

#define KIARA_CXX_ENUM_CONST(const_name)                                \
    ((const_name))

/* Struct declaration */

/* STTY - struct type */
#define _KR_CXX_STTY_TUPLE_LEN          3

/* Member name */
#define _KR_CXX_STTY_NAME_IDX           0
/* Main member name */
#define _KR_CXX_STTY_MAIN_NAME_STR_IDX  1
/* Member semantics */
#define _KR_CXX_STTY_SEMANTICS_STR_IDX  2


#define _KR_CXX_STTY_NAME(t)                                            \
    BOOST_PP_TUPLE_ELEM(_KR_CXX_STTY_TUPLE_LEN, _KR_CXX_STTY_NAME_IDX, t)
#define _KR_CXX_STTY_MAIN_NAME_STR(t)                                   \
    BOOST_PP_TUPLE_ELEM(_KR_CXX_STTY_TUPLE_LEN, _KR_CXX_STTY_MAIN_NAME_STR_IDX, t)
#define _KR_CXX_STTY_SEMANTICS_STR(t)                                   \
    BOOST_PP_TUPLE_ELEM(_KR_CXX_STTY_TUPLE_LEN, _KR_CXX_STTY_SEMANTICS_STR_IDX, t)

#define KIARA_CXX_STRUCT_MEMBER(member_name)                                                \
    ((member_name, NULL, NULL))

#define KIARA_CXX_STRUCT_MEMBER_WITH_SEMANTICS(member_name, member_semantics)               \
    ((member_name, NULL, BOOST_PP_STRINGIZE(member_semantics)))

#define KIARA_CXX_STRUCT_DEPENDENT_MEMBER(member_name, main_member_name, semantics)         \
    ((member_name, BOOST_PP_STRINGIZE(main_member_name), BOOST_PP_STRINGIZE(semantics)))

#define KIARA_CXX_STRUCT_ARRAY_MEMBER(ptr_member_name, size_member_name)                        \
    KIARA_CXX_STRUCT_MEMBER_WITH_SEMANTICS(ptr_member_name, VI_ARRAY_PTR)                       \
    KIARA_CXX_STRUCT_DEPENDENT_MEMBER(size_member_name, ptr_member_name, DEFAULT)

#define _KIARA_CXX_STRUCT_MEMBER_STRUCT(cxx_struct_name, member, main_member_name, semantics)   \
    {_KIARA_CXX_DTYPE_GETTER(typeofmember(cxx_struct_name, member)),                            \
     BOOST_PP_STRINGIZE(member),                                                                \
     offsetof(Type, member),                                                                    \
     main_member_name,                                                                          \
     semantics}

#define _KIARA_CXX_STRUCT_MEMBER_EXPAND(elem)                           \
        _KIARA_CXX_STRUCT_MEMBER_STRUCT(                                \
            Type,                                                       \
            _KR_CXX_STTY_NAME(elem),                                    \
            _KR_CXX_STTY_MAIN_NAME_STR(elem),                           \
            _KR_CXX_STTY_SEMANTICS_STR(elem)                            \
            )

#define _KIARA_CXX_STRUCT_MEMBER_SEQELEM(z, i, data)                    \
        _KIARA_CXX_STRUCT_MEMBER_EXPAND(BOOST_PP_SEQ_ELEM(i, data))

#define KIARA_CXX_DECL_STRUCT(cxx_struct_name, struct_members)          \
    namespace KIARA {                                                   \
    template <> class TypeNameOf<cxx_struct_name>                       \
    {                                                                   \
    public:                                                             \
        static const char *get()                                        \
        { return BOOST_PP_STRINGIZE(cxx_struct_name); }                 \
    };                                                                  \
    template <> class TypeTraits<cxx_struct_name>                       \
    {                                                                   \
    public:                                                             \
        typedef cxx_struct_name Type;                                   \
        KIARA_DEFAULT_GET_TYPE_NAME(Type)                               \
        static KIARA_DeclType * getDeclType()                           \
        {                                                               \
            static KIARA_DeclStructMember kr_members[] = {              \
                    BOOST_PP_ENUM(BOOST_PP_SEQ_SIZE(struct_members),    \
                            _KIARA_CXX_STRUCT_MEMBER_SEQELEM,           \
                            struct_members)                             \
            };                                                          \
            static KIARA_DeclStruct kr_struct = {                       \
                kr_members,                                             \
                sizeof(kr_members) / sizeof(kr_members[0]),             \
                sizeof(cxx_struct_name),                                \
                NULL                                                    \
            };                                                          \
            static KIARA_DeclType kr_type = {                           \
                BOOST_PP_STRINGIZE(cxx_struct_name),                    \
                KIARA_TYPE_STRUCT,                                      \
                KIARA_DTF_NONE,                                         \
                {&kr_struct}                                            \
            };                                                          \
            return &kr_type;                                            \
        }                                                               \
    };                                                                  \
    }

#define KIARA_CXX_DECL_STRUCT_WITH_API(cxx_struct_name, struct_members, api_members)    \
    namespace KIARA {                                                                   \
    template <> class TypeNameOf<cxx_struct_name>                                       \
    {                                                                                   \
    public:                                                                             \
        static const char *get()                                                        \
        { return BOOST_PP_STRINGIZE(cxx_struct_name); }                                 \
    };                                                                                  \
    template <> class TypeTraits<cxx_struct_name>                                       \
    {                                                                                   \
    public:                                                                             \
        typedef cxx_struct_name Type;                                                   \
        KIARA_DEFAULT_GET_TYPE_NAME(Type)                                               \
        _KIARA_CXX_USER_API_CHECK(cxx_struct_name, api_members)                         \
        static KIARA_DeclType * getDeclType()                                           \
        {                                                                               \
            _KIARA_CXX_DECL_API_FUNCS(api_members)                                      \
            static KIARA_DeclStructMember kr_members[] = {                              \
                    BOOST_PP_ENUM(BOOST_PP_SEQ_SIZE(struct_members),                    \
                            _KIARA_CXX_STRUCT_MEMBER_SEQELEM,                           \
                            struct_members)                                             \
            };                                                                          \
            static KIARA_DeclStruct kr_struct = {                                       \
                kr_members,                                                             \
                sizeof(kr_members) / sizeof(kr_members[0]),                             \
                sizeof(cxx_struct_name),                                                \
                &kr_api                                                                 \
            };                                                                          \
            static KIARA_DeclType kr_type = {                                           \
                BOOST_PP_STRINGIZE(cxx_struct_name),                                    \
                KIARA_TYPE_STRUCT,                                                      \
                KIARA_DTF_NONE,                                                         \
                {&kr_struct}                                                            \
            };                                                                          \
            return &kr_type;                                                            \
        }                                                                               \
    };                                                                                  \
    }

/* Function declaration */

#define _KR_CXX_CFTY_TUPLE_LEN          5

/* Argument type */
#define _KR_CXX_CFTY_TYPE_IDX           0
/* Argument name as symbol */
#define _KR_CXX_CFTY_NAME_IDX           1
/* Argument name as string */
#define _KR_CXX_CFTY_NAME_STR_IDX       2
/* Annotation string */
#define _KR_CXX_CFTY_ANNOTATION_STR_IDX 3
/* Semantics string */
#define _KR_CXX_CFTY_SEMANTICS_STR_IDX  4

#define _KR_CXX_CFTY_TYPE(t)                                                    \
    BOOST_PP_TUPLE_ELEM(_KR_CXX_CFTY_TUPLE_LEN, _KR_CXX_CFTY_TYPE_IDX, t)
#define _KR_CXX_CFTY_NAME(t)                                                    \
    BOOST_PP_TUPLE_ELEM(_KR_CXX_CFTY_TUPLE_LEN, _KR_CXX_CFTY_NAME_IDX, t)
#define _KR_CXX_CFTY_NAME_STR(t)                                                \
    BOOST_PP_TUPLE_ELEM(_KR_CXX_CFTY_TUPLE_LEN, _KR_CXX_CFTY_NAME_STR_IDX, t)
#define _KR_CXX_CFTY_ANNOTATION_STR(t)                                          \
    BOOST_PP_TUPLE_ELEM(_KR_CXX_CFTY_TUPLE_LEN, _KR_CXX_CFTY_ANNOTATION_STR_IDX, t)
#define _KR_CXX_CFTY_SEMANTICS_STR(t)                                           \
    BOOST_PP_TUPLE_ELEM(_KR_CXX_CFTY_TUPLE_LEN, _KR_CXX_CFTY_SEMANTICS_STR_IDX, t)

#define KIARA_CXX_FUNC_ARG(cxx_type_name, member_name)                      \
    ((cxx_type_name, member_name, BOOST_PP_STRINGIZE(member_name), 0, 0))

#define KIARA_CXX_FUNC_ARG_ANNOTATED(cxx_type_name, member_name, annotation)\
    ((cxx_type_name, member_name, BOOST_PP_STRINGIZE(member_name), BOOST_PP_STRINGIZE(annotation), 0))

#define KIARA_CXX_FUNC_RESULT(cxx_type_name, member_name)                   \
    ((cxx_type_name, member_name, "$result", 0, 0))

#define KIARA_CXX_FUNC_EXCEPTION(cxx_type_name, member_name)                \
    ((cxx_type_name, member_name, "$exception", 0, 0))

#define _KIARA_CXX_FUNC_ARG_EXPAND(elem)                                \
        {   _KIARA_CXX_DTYPE_GETTER(_KR_CXX_CFTY_TYPE(elem)),           \
            _KR_CXX_CFTY_NAME_STR(elem),                                \
            _KR_CXX_CFTY_ANNOTATION_STR(elem),                          \
            _KR_CXX_CFTY_SEMANTICS_STR(elem)                            \
        }

#define _KIARA_CXX_FUNC_ARG_SEQELEM(z, i, data)                         \
        _KIARA_CXX_FUNC_ARG_EXPAND(BOOST_PP_SEQ_ELEM(i, data))

#define _KIARA_CXX_FUNC_ARG_TYPE(s, data, elem)                         \
        _KR_CXX_CFTY_TYPE(elem)

#define _KIARA_CXX_FUNC_ARG_NAME(s, data, elem)                         \
        _KR_CXX_CFTY_NAME(elem)

#define _KIARA_CXX_FUNC_ARG_VOID_PTR(s, data, elem)                     \
        ((void*)&_KIARA_CXX_FUNC_ARG_NAME(s, data, elem))

#define _KIARA_CXX_FUNC_ARG_TYPE_AND_NAME(s, data, elem)                \
        _KR_CXX_CFTY_TYPE(elem)                                         \
        _KR_CXX_CFTY_NAME(elem)

#define _KIARA_GEN_FUNC_TYPEDEF(kiara_type_name, func_name,             \
                                kiara_return_type_name, func_args)      \
    typedef int                                                         \
            (*func_name)(KIARA_FUNC_OBJ(kiara_type_name),               \
                        BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(       \
                        _KIARA_CXX_FUNC_ARG_TYPE_AND_NAME, 0, func_args)));

#define KIARA_FUNC_OBJ_TYPEDEF(kiara_type_name)                         \
        _KIARA_DTYPE(funcobj_ptr, kiara_type_name)

#define _KIARA_CXX_GEN_FUNC_FWD_DECL(kiara_type_name, func_args)            \
    typedef struct _KIARA_DTYPE(funcobj, kiara_type_name)                   \
        _KIARA_DTYPE(funcobj, kiara_type_name),                             \
        * KIARA_FUNC_OBJ_TYPEDEF(kiara_type_name);                          \
    typedef int (*_KIARA_DTYPE(func_wrapper_type, kiara_type_name))         \
                (KIARA_FUNC_OBJ(kiara_type_name) kiara_funcobj_,            \
                        BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(           \
                       _KIARA_CXX_FUNC_ARG_TYPE_AND_NAME, 0, func_args)));  \
    static int _KIARA_DTYPE(func_wrapper, kiara_type_name)                  \
                (KIARA_FUNC_OBJ(kiara_type_name) kiara_funcobj_,            \
                   BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(                \
               _KIARA_CXX_FUNC_ARG_TYPE_AND_NAME, 0, func_args)));

#define _KIARA_CXX_GEN_FUNC_FWD_DECL_NO_ARGS(kiara_type_name)               \
    typedef struct _KIARA_DTYPE(funcobj, kiara_type_name)                   \
        _KIARA_DTYPE(funcobj, kiara_type_name),                             \
        * KIARA_FUNC_OBJ_TYPEDEF(kiara_type_name);                          \
    typedef int (*_KIARA_DTYPE(func_wrapper_type, kiara_type_name))         \
                (KIARA_FUNC_OBJ(kiara_type_name) kiara_funcobj_);           \
    static int _KIARA_DTYPE(func_wrapper, kiara_type_name)                  \
                (KIARA_FUNC_OBJ(kiara_type_name) kiara_funcobj_);

#define _KIARA_CXX_GEN_FUNC_WRAPPER(kiara_type_name, func_args)             \
    static int _KIARA_DTYPE(func_wrapper, kiara_type_name)                  \
                (KIARA_FUNC_OBJ(kiara_type_name) kiara_funcobj_,            \
                   BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(                \
               _KIARA_CXX_FUNC_ARG_TYPE_AND_NAME, 0, func_args)))           \
    {                                                                       \
        void *args[] = {                                                    \
            BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(                       \
                                _KIARA_CXX_FUNC_ARG_VOID_PTR, 0, func_args))\
        };                                                                  \
        return kiara_funcobj_->base.vafunc((KIARA_FuncObj*)kiara_funcobj_,  \
                args,                                                       \
                BOOST_PP_SEQ_SIZE(func_args));                              \
    }

#define _KIARA_CXX_GEN_FUNC_WRAPPER_NO_ARGS(kiara_type_name)                \
    static int _KIARA_DTYPE(func_wrapper, kiara_type_name)                  \
                (KIARA_FUNC_OBJ(kiara_type_name) kiara_funcobj_)            \
    {                                                                       \
        return kiara_funcobj_->base.vafunc((KIARA_FuncObj*)kiara_funcobj_,  \
                0, /*args*/                                                 \
                0  /*number of args*/);                                     \
    }

#define _KIARA_CXX_GEN_FUNC_OBJ_TYPE(kiara_type_name)                       \
    struct _KIARA_DTYPE(funcobj, kiara_type_name) {                         \
        KIARA_FuncObjBase base;                                             \
        _KIARA_DTYPE(func_wrapper_type, kiara_type_name) func;              \
    };

#define KIARA_CXX_DECL_FUNC(func_obj_name, func_args)                       \
    KIARA_BEGIN_EXTERN_C                                                    \
    _KIARA_CXX_GEN_FUNC_FWD_DECL(func_obj_name, func_args)                  \
    _KIARA_CXX_GEN_FUNC_OBJ_TYPE(func_obj_name)                             \
    _KIARA_CXX_GEN_FUNC_WRAPPER(func_obj_name, func_args)                   \
    KIARA_END_EXTERN_C                                                      \
    class func_obj_name {                                                   \
    public:                                                                 \
        func_obj_name() : funcobj_(0) { }                                   \
        func_obj_name(const func_obj_name &f) : funcobj_(f.funcobj_) { }    \
        func_obj_name(KIARA_FUNC_OBJ(func_obj_name) f) : funcobj_(f) { }    \
        const func_obj_name & operator=(const func_obj_name &f)             \
        {                                                                   \
            funcobj_ = f.funcobj_;                                          \
            return *this;                                                   \
        }                                                                   \
        const func_obj_name & operator=(KIARA_FUNC_OBJ(func_obj_name) f)    \
        {                                                                   \
            funcobj_ = f;                                                   \
            return *this;                                                   \
        }                                                                   \
        int operator()(BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(            \
           _KIARA_CXX_FUNC_ARG_TYPE_AND_NAME, 0, func_args)))               \
        {                                                                   \
            BOOST_ASSERT(funcobj_ != 0);                                    \
            return funcobj_->func(funcobj_,                                 \
                    BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(               \
                                  _KIARA_CXX_FUNC_ARG_NAME, 0, func_args)));\
        }                                                                   \
        typedef KIARA_FUNC_OBJ(func_obj_name)                               \
                func_obj_name::*UnspecifiedBoolType;                        \
        operator UnspecifiedBoolType () const                               \
        {                                                                   \
            return funcobj_ == 0 ? 0 : &func_obj_name::funcobj_;            \
        }                                                                   \
    private:                                                                \
        KIARA_FUNC_OBJ(func_obj_name) funcobj_;                             \
    };                                                                      \
    namespace KIARA {                                                       \
    template <> class TypeNameOf<func_obj_name>                             \
    {                                                                       \
    public:                                                                 \
        static const char *get()                                            \
        { return BOOST_PP_STRINGIZE(func_obj_name); }                       \
    };                                                                      \
    template <> class TypeTraits<func_obj_name>                             \
    {                                                                       \
    public:                                                                 \
        typedef ::func_obj_name Type;                                       \
        KIARA_DEFAULT_GET_TYPE_NAME(Type)                                   \
        static KIARA_DeclType * getDeclType()                               \
        {                                                                   \
            static KIARA_DeclFuncArgument kr_args[] = {                     \
                    BOOST_PP_ENUM(BOOST_PP_SEQ_SIZE(func_args),             \
                                    _KIARA_CXX_FUNC_ARG_SEQELEM, func_args) \
            };                                                              \
            static KIARA_DeclFunc kr_func = {                               \
                    (KIARA_Func)_KIARA_DTYPE(func_wrapper, func_obj_name),  \
                    _KIARA_CXX_DTYPE_GETTER(int),                           \
                    kr_args,                                                \
                    sizeof(kr_args) / sizeof(kr_args[0])                    \
            };                                                              \
            static KIARA_DeclType kr_type = {                               \
                BOOST_PP_STRINGIZE(func_obj_name),                          \
                KIARA_TYPE_FUNC,                                            \
                KIARA_DTF_NONE,                                             \
                {&kr_func}                                                  \
            };                                                              \
            return &kr_type;                                                \
        }                                                                   \
    };                                                                      \
    }

#define KIARA_CXX_DECL_FUNC_NO_ARGS(func_obj_name)                          \
    KIARA_BEGIN_EXTERN_C                                                    \
    _KIARA_CXX_GEN_FUNC_FWD_DECL_NO_ARGS(func_obj_name)                     \
    _KIARA_CXX_GEN_FUNC_OBJ_TYPE(func_obj_name)                             \
    _KIARA_CXX_GEN_FUNC_WRAPPER_NO_ARGS(func_obj_name)                      \
    KIARA_END_EXTERN_C                                                      \
    class func_obj_name {                                                   \
    public:                                                                 \
        func_obj_name() : funcobj_(0) { }                                   \
        func_obj_name(const func_obj_name &f) : funcobj_(f.funcobj_) { }    \
        func_obj_name(KIARA_FUNC_OBJ(func_obj_name) f) : funcobj_(f) { }    \
        const func_obj_name & operator=(const func_obj_name &f)             \
        {                                                                   \
            funcobj_ = f.funcobj_;                                          \
            return *this;                                                   \
        }                                                                   \
        const func_obj_name & operator=(KIARA_FUNC_OBJ(func_obj_name) f)    \
        {                                                                   \
            funcobj_ = f;                                                   \
            return *this;                                                   \
        }                                                                   \
        int operator()(void)                                                \
        {                                                                   \
            BOOST_ASSERT(funcobj_ != 0);                                    \
            return funcobj_->func(funcobj_);                                \
        }                                                                   \
        typedef KIARA_FUNC_OBJ(func_obj_name)                               \
                func_obj_name::*UnspecifiedBoolType;                        \
        operator UnspecifiedBoolType () const                               \
        {                                                                   \
            return funcobj_ == 0 ? 0 : &func_obj_name::funcobj_;            \
        }                                                                   \
    private:                                                                \
        KIARA_FUNC_OBJ(func_obj_name) funcobj_;                             \
    };                                                                      \
    namespace KIARA {                                                       \
    template <> class TypeNameOf<func_obj_name>                             \
    {                                                                       \
    public:                                                                 \
        static const char *get()                                            \
        { return BOOST_PP_STRINGIZE(func_obj_name); }                       \
    };                                                                      \
    template <> class TypeTraits<func_obj_name>                             \
    {                                                                       \
    public:                                                                 \
        typedef ::func_obj_name Type;                                       \
        KIARA_DEFAULT_GET_TYPE_NAME(Type)                                   \
        static KIARA_DeclType * getDeclType()                               \
        {                                                                   \
            static KIARA_DeclFunc kr_func = {                               \
                    (KIARA_Func)_KIARA_DTYPE(func_wrapper, func_obj_name),  \
                    _KIARA_CXX_DTYPE_GETTER(int),                           \
                    0, /* kr_args */                                        \
                    0  /* number of args */                                 \
            };                                                              \
            static KIARA_DeclType kr_type = {                               \
                BOOST_PP_STRINGIZE(func_obj_name),                          \
                KIARA_TYPE_FUNC,                                            \
                KIARA_DTF_NONE,                                             \
                {&kr_func}                                                  \
            };                                                              \
            return &kr_type;                                                \
        }                                                                   \
    };                                                                      \
    }

#define KIARA_CALL(funcobj, ...)                                            \
    ((funcobj)->func((funcobj), __VA_ARGS__))

#define KIARA_GENERATE_CLIENT_FUNC(connection, idl_method_name, kiara_type_name, mapping)   \
        (KIARA_FUNC_OBJ(kiara_type_name))kiaraGenerateClientFuncObj(                        \
            (connection),                                                                   \
            (idl_method_name),                                                              \
            KIARA_TYPE(kiara_type_name), (mapping))

#define KIARA_REGISTER_SERVICE_IMPL(service, idl_method_name, kiara_type_name, mapping)     \
    kiaraRegisterServiceFunc((service), idl_method_name, KIARA_TYPE(kiara_type_name),       \
                             (mapping), 0)

#define KIARA_REGISTER_SERVICE_FUNC(service, idl_method_name, kiara_type_name, mapping, service_func)   \
    kiaraRegisterServiceFunc((service), idl_method_name, KIARA_TYPE(kiara_type_name),                   \
            (mapping), _KR_service_func_conv(kiara_type_name)(service_func))

/* Service type declaration */

/* SRTY - service type */
#define _KR_CXX_SRTY_TUPLE_LEN              5

#define _KR_CXX_SRTY_TYPE_IDX               0
#define _KR_CXX_SRTY_NAME_IDX               1
#define _KR_CXX_SRTY_NAME_STR_IDX           2
#define _KR_CXX_SRTY_ANNOTATION_STR_IDX     3
#define _KR_CXX_SRTY_SEMANTICS_STR_IDX      4

#define _KR_CXX_SRTY_TYPE(t)                                                    \
    BOOST_PP_TUPLE_ELEM(_KR_CXX_SRTY_TUPLE_LEN, _KR_CXX_SRTY_TYPE_IDX, t)
#define _KR_CXX_SRTY_NAME(t)                                                    \
    BOOST_PP_TUPLE_ELEM(_KR_CXX_SRTY_TUPLE_LEN, _KR_CXX_SRTY_NAME_IDX, t)
#define _KR_CXX_SRTY_NAME_STR(t)                                                \
    BOOST_PP_TUPLE_ELEM(_KR_CXX_SRTY_TUPLE_LEN, _KR_CXX_SRTY_NAME_STR_IDX, t)
#define _KR_CXX_SRTY_ANNOTATION_STR(t)                                          \
    BOOST_PP_TUPLE_ELEM(_KR_CXX_SRTY_TUPLE_LEN, _KR_CXX_SRTY_ANNOTATION_STR_IDX, t)
#define _KR_CXX_SRTY_SEMANTICS_STR(t)                                           \
    BOOST_PP_TUPLE_ELEM(_KR_CXX_SRTY_TUPLE_LEN, _KR_CXX_SRTY_SEMANTICS_STR_IDX, t)

#define KIARA_CXX_SERVICE_ARG(kiara_type_name, member_name)                                         \
    ((kiara_type_name, member_name, BOOST_PP_STRINGIZE(member_name), NULL, NULL))

#define KIARA_CXX_SERVICE_CONST_ARG(kiara_type_name, member_name)                                   \
    ((kiara_type_name, member_name, BOOST_PP_STRINGIZE(member_name), NULL, NULL))

#define KIARA_CXX_SERVICE_RESULT(kiara_type_name, member_name)                                      \
    ((kiara_type_name, member_name, "$result", NULL, NULL))

#define KIARA_CXX_SERVICE_EXCEPTION(kiara_type_name, member_name)                                   \
    ((kiara_type_name, member_name, "$exception", NULL, NULL))

/* Expand service wrapper arguments */
#define _KR_SRWARGS_EXPAND(z, n, arg_tuple)                                     \
    BOOST_PP_COMMA_IF(n)                                                        \
    ::KIARA::ConvertVoidPtrToType<_KR_CXX_SRTY_TYPE(BOOST_PP_SEQ_ELEM(n, arg_tuple))>::convert(args[n])

/* Expand service function arguments
 */
#define _KR_SRFARGS_EXPAND(z, n, arg_tuple)                                     \
    BOOST_PP_COMMA_IF(n)                                                        \
    _KR_CXX_SRTY_TYPE(BOOST_PP_SEQ_ELEM(n, arg_tuple))                          \
    _KR_CXX_SRTY_NAME(BOOST_PP_SEQ_ELEM(n, arg_tuple))

/* Expand service declaration */
#define _KR_SRDECL_EXPAND(z, n, arg_tuple)                                          \
    BOOST_PP_COMMA_IF(n)                                                            \
    {_KIARA_CXX_DTYPE_GETTER(_KR_CXX_SRTY_TYPE(BOOST_PP_SEQ_ELEM(n, arg_tuple))),   \
     _KR_CXX_SRTY_NAME_STR(BOOST_PP_SEQ_ELEM(n, arg_tuple)),                        \
     _KR_CXX_SRTY_ANNOTATION_STR(BOOST_PP_SEQ_ELEM(n, arg_tuple)), /*annotation*/   \
     _KR_CXX_SRTY_SEMANTICS_STR(BOOST_PP_SEQ_ELEM(n, arg_tuple)), /*semantics*/     \
    }

/*
 * Code used before for declaration of the function type:
 *
 *   typedef c_func_name _KR_tdef(kiara_type_name);
 *
 */

#define KIARA_CXX_DECL_SERVICE(kiara_type_name, service_args)                                       \
    struct kiara_type_name { };                                                                     \
    KIARA_BEGIN_EXTERN_C                                                                            \
    typedef KIARA_Result (*_KR_service_func_type(kiara_type_name))(                                 \
            KIARA_ServiceFuncObj *kiara_funcobj_,                                                   \
            BOOST_PP_REPEAT(BOOST_PP_SEQ_SIZE(service_args), _KR_SRFARGS_EXPAND, service_args));    \
                                                                                                    \
    inline KIARA_ServiceFunc _KR_service_func_conv(kiara_type_name)(                                \
        _KR_service_func_type(kiara_type_name) arg)                                                 \
    {                                                                                               \
        return (KIARA_ServiceFunc)arg;                                                              \
    }                                                                                               \
                                                                                                    \
    static KIARA_Result _KR_service_wrapper(kiara_type_name)(                                       \
            KIARA_ServiceFuncObj * kiara_funcobj_, void *args[], size_t num_args)                   \
    {                                                                                               \
        return ((_KR_service_func_type(kiara_type_name))(kiara_funcobj_->func))(                    \
            kiara_funcobj_,                                                                         \
            BOOST_PP_REPEAT(BOOST_PP_SEQ_SIZE(service_args), _KR_SRWARGS_EXPAND, service_args)      \
        );                                                                                          \
    }                                                                                               \
    KIARA_END_EXTERN_C                                                                              \
    namespace KIARA {                                                                               \
    template <> class TypeNameOf<kiara_type_name>                                                   \
    {                                                                                               \
    public:                                                                                         \
        static const char *get()                                                                    \
        { return BOOST_PP_STRINGIZE(kiara_type_name); }                                             \
    };                                                                                              \
    template <> class TypeTraits<kiara_type_name>                                                   \
    {                                                                                               \
    public:                                                                                         \
        typedef ::kiara_type_name Type;                                                             \
        KIARA_DEFAULT_GET_TYPE_NAME(Type)                                                           \
        static KIARA_DeclType * getDeclType()                                                       \
        {                                                                                           \
            static KIARA_DeclServiceArgument kr_args[] = {                                          \
                BOOST_PP_REPEAT(BOOST_PP_SEQ_SIZE(service_args), _KR_SRDECL_EXPAND, service_args)   \
            };                                                                                      \
            static KIARA_DeclService kr_service = {                                                 \
                    0,                                                                              \
                    _KR_service_wrapper(kiara_type_name),                                           \
                    _KIARA_CXX_DTYPE_GETTER(int),                                                   \
                    kr_args,                                                                        \
                    sizeof(kr_args) / sizeof(kr_args[0])                                            \
            };                                                                                      \
            static KIARA_DeclType kr_type = {                                                       \
                BOOST_PP_STRINGIZE(kiara_type_name),                                                \
                KIARA_TYPE_SERVICE,                                                                 \
                KIARA_DTF_NONE,                                                                     \
                {&kr_service}                                                                       \
            };                                                                                      \
            return &kr_type;                                                                        \
        }                                                                                           \
    };                                                                                              \
    }


#define KIARA_CXX_DECL_SERVICE_IMPL(kiara_type_name, service_args)                                  \
    struct kiara_type_name { };                                                                     \
    KIARA_BEGIN_EXTERN_C                                                                            \
    typedef KIARA_Result (*_KR_service_func_type(kiara_type_name))(                                 \
            KIARA_ServiceFuncObj *kiara_funcobj_,                                                   \
            BOOST_PP_REPEAT(BOOST_PP_SEQ_SIZE(service_args), _KR_SRFARGS_EXPAND, service_args));    \
                                                                                                    \
    inline KIARA_ServiceFunc _KR_service_func_conv(kiara_type_name)(                                \
        _KR_service_func_type(kiara_type_name) arg)                                                 \
    {                                                                                               \
        return (KIARA_ServiceFunc)arg;                                                              \
    }                                                                                               \
                                                                                                    \
    static KIARA_Result _KR_service_func(kiara_type_name)(                                          \
            KIARA_ServiceFuncObj *kiara_funcobj,                                                    \
            BOOST_PP_REPEAT(BOOST_PP_SEQ_SIZE(service_args), _KR_SRFARGS_EXPAND, service_args));    \
                                                                                                    \
    static KIARA_Result _KR_service_wrapper(kiara_type_name)(                                       \
            KIARA_ServiceFuncObj * kiara_funcobj_, void *args[], size_t num_args)                   \
    {                                                                                               \
        return ((_KR_service_func_type(kiara_type_name))(kiara_funcobj_->func))(                    \
            kiara_funcobj_,                                                                         \
            BOOST_PP_REPEAT(BOOST_PP_SEQ_SIZE(service_args), _KR_SRWARGS_EXPAND, service_args)      \
        );                                                                                          \
    }                                                                                               \
    KIARA_END_EXTERN_C                                                                              \
    namespace KIARA {                                                                               \
    template <> class TypeNameOf<kiara_type_name>                           \
    {                                                                       \
    public:                                                                 \
        static const char *get()                                            \
        { return BOOST_PP_STRINGIZE(kiara_type_name); }                     \
    };                                                                      \
    template <> class TypeTraits<kiara_type_name>                           \
    {                                                                       \
    public:                                                                 \
        typedef ::kiara_type_name Type;                                     \
        KIARA_DEFAULT_GET_TYPE_NAME(Type)                                   \
        static KIARA_DeclType * getDeclType()                               \
        {                                                                   \
            static KIARA_DeclServiceArgument kr_args[] = {                  \
                BOOST_PP_REPEAT(BOOST_PP_SEQ_SIZE(service_args), _KR_SRDECL_EXPAND, service_args)   \
            };                                                              \
            static KIARA_DeclService kr_service = {                         \
                    (KIARA_ServiceFunc)_KR_service_func(kiara_type_name),   \
                    _KR_service_wrapper(kiara_type_name),                   \
                    _KIARA_CXX_DTYPE_GETTER(int),                           \
                    kr_args,                                                \
                    sizeof(kr_args) / sizeof(kr_args[0])                    \
            };                                                              \
            static KIARA_DeclType kr_type = {                               \
                BOOST_PP_STRINGIZE(kiara_type_name),                        \
                KIARA_TYPE_SERVICE,                                         \
                KIARA_DTF_NONE,                                             \
                {&kr_service}                                               \
            };                                                              \
            return &kr_type;                                                \
        }                                                                   \
    };                                                                      \
    }                                                                       \
    static KIARA_Result _KR_service_func(kiara_type_name)(                                          \
            KIARA_ServiceFuncObj *kiara_funcobj,                                                    \
            BOOST_PP_REPEAT(BOOST_PP_SEQ_SIZE(service_args), _KR_SRFARGS_EXPAND, service_args))

/* Encrypted data types */


#define KIARA_CXX_DECL_FUNC_NO_ARGS(func_obj_name)                          \
    KIARA_BEGIN_EXTERN_C                                                    \
    _KIARA_CXX_GEN_FUNC_FWD_DECL_NO_ARGS(func_obj_name)                     \
    _KIARA_CXX_GEN_FUNC_OBJ_TYPE(func_obj_name)                             \
    _KIARA_CXX_GEN_FUNC_WRAPPER_NO_ARGS(func_obj_name)                      \
    KIARA_END_EXTERN_C                                                      \
    class func_obj_name {                                                   \
    public:                                                                 \
        func_obj_name() : funcobj_(0) { }                                   \
        func_obj_name(const func_obj_name &f) : funcobj_(f.funcobj_) { }    \
        func_obj_name(KIARA_FUNC_OBJ(func_obj_name) f) : funcobj_(f) { }    \
        const func_obj_name & operator=(const func_obj_name &f)             \
        {                                                                   \
            funcobj_ = f.funcobj_;                                          \
            return *this;                                                   \
        }                                                                   \
        const func_obj_name & operator=(KIARA_FUNC_OBJ(func_obj_name) f)    \
        {                                                                   \
            funcobj_ = f;                                                   \
            return *this;                                                   \
        }                                                                   \
        int operator()(void)                                                \
        {                                                                   \
            BOOST_ASSERT(funcobj_ != 0);                                    \
            return funcobj_->func(funcobj_);                                \
        }                                                                   \
        typedef KIARA_FUNC_OBJ(func_obj_name)                               \
                func_obj_name::*UnspecifiedBoolType;                        \
        operator UnspecifiedBoolType () const                               \
        {                                                                   \
            return funcobj_ == 0 ? 0 : &func_obj_name::funcobj_;            \
        }                                                                   \
    private:                                                                \
        KIARA_FUNC_OBJ(func_obj_name) funcobj_;                             \
    };                                                                      \
    namespace KIARA {                                                       \
    template <> class TypeNameOf<func_obj_name>                             \
    {                                                                       \
    public:                                                                 \
        static const char *get()                                            \
        { return BOOST_PP_STRINGIZE(func_obj_name); }                       \
    };                                                                      \
    template <> class TypeTraits<func_obj_name>                             \
    {                                                                       \
    public:                                                                 \
        typedef ::func_obj_name Type;                                       \
        KIARA_DEFAULT_GET_TYPE_NAME(Type)                                   \
        static KIARA_DeclType * getDeclType()                               \
        {                                                                   \
            static KIARA_DeclFunc kr_func = {                               \
                    (KIARA_Func)_KIARA_DTYPE(func_wrapper, func_obj_name),  \
                    _KIARA_CXX_DTYPE_GETTER(int),                           \
                    0, /* kr_args */                                        \
                    0  /* number of args */                                 \
            };                                                              \
            static KIARA_DeclType kr_type = {                               \
                BOOST_PP_STRINGIZE(func_obj_name),                          \
                KIARA_TYPE_FUNC,                                            \
                KIARA_DTF_NONE,                                             \
                {&kr_func}                                                  \
            };                                                              \
            return &kr_type;                                                \
        }                                                                   \
    };                                                                      \
    }


#define KIARA_CXX_DECL_ENCRYPTED(kiara_type_name, kiara_element_type_name)  \
    KIARA_FWD_DECL_ENCRYPTED(kiara_type_name)                               \
    KIARA_BEGIN_EXTERN_C                                                    \
    struct _KIARA_DTYPE(enc, kiara_type_name) { kr_dbuffer_t data; };       \
    typedef KIARA_ENCRYPTED(kiara_type_name) kiara_type_name;               \
    KIARA_END_EXTERN_C                                                      \
                                                                            \
    inline kiara_type_name KIARA_JOIN(new_, kiara_type_name)(void)          \
    {                                                                       \
        kiara_type_name enc = (kiara_type_name)malloc(                      \
            sizeof(_KIARA_DTYPE(enc, kiara_type_name)));                    \
        kr_dbuffer_init(&enc->data);                                        \
        return enc;                                                         \
    }                                                                       \
                                                                            \
    inline void KIARA_JOIN(delete_, kiara_type_name)(kiara_type_name enc)   \
    {                                                                       \
        if (enc)                                                            \
            kr_dbuffer_destroy(&enc->data);                                 \
        free(enc);                                                          \
    }                                                                       \
                                                                            \
    namespace KIARA {                                                       \
    template <> class TypeNameOf<kiara_type_name>                           \
    {                                                                       \
    public:                                                                 \
        static const char *get()                                            \
        { return BOOST_PP_STRINGIZE(kiara_type_name); }                     \
    };                                                                      \
    template <> class TypeTraits<kiara_type_name>                           \
    {                                                                       \
    public:                                                                 \
        typedef ::kiara_type_name Type;                                     \
        KIARA_DEFAULT_GET_TYPE_NAME(Type)                                   \
        static KIARA_DeclType * getDeclType()                               \
        {                                                                   \
            static KIARA_DeclEncrypted kr_enctd = {                         \
                _KIARA_CXX_DTYPE_GETTER(kiara_type_name)                    \
            };                                                              \
            static KIARA_DeclType kr_type = {                               \
                BOOST_PP_STRINGIZE(kiara_type_name),                        \
                KIARA_TYPE_ENCRYPTED,                                       \
                KIARA_DTF_NONE,                                             \
                {&kr_enctd}                                                 \
            };                                                              \
            return &kr_type;                                                \
        }                                                                   \
    };                                                                      \
    }

/* Custom typename declaration */

#define KIARA_CXX_DECL_TYPENAME(cxx_type_name)                          \
    namespace KIARA {                                                   \
    template <> class TypeNameOf<cxx_type_name>                         \
    {                                                                   \
    public:                                                             \
        static const char *get()                                        \
        { return BOOST_PP_STRINGIZE(cxx_type_name); }                   \
    };                                                                  \
    }

/* Opaque type declaration */

#define _KRX_OPTY_TUPLE_LEN             4

#define _KRX_OPTY_METHOD_SUFFIX_STR_IDX 0
#define _KRX_OPTY_API_SUFFIX_IDX        1
#define _KRX_OPTY_API_SUFFIX_STR_IDX    2
#define _KRX_OPTY_API_FUNC_IDX          3

#define KIARA_CXX_DECL_OPAQUE_TYPE(cxx_type_name, args)                     \
        KIARA_CXX_DECL_OPAQUE_TYPE_NAME(cxx_type_name, cxx_type_name, args)

#define KIARA_CXX_USER_API(kiara_api_name, value)                           \
    (("$this", kiara_api_name, BOOST_PP_STRINGIZE(kiara_api_name), value))

#define KIARA_CXX_METHOD_USER_API(method_name, kiara_api_name, value)       \
    ((BOOST_PP_STRINGIZE(method_name), kiara_api_name, BOOST_PP_STRINGIZE(kiara_api_name), value))


#define _KIARA_CXX_USER_API(method_suffix_str, kiara_api_suffix, kiara_api_suffix_str, api_func)    \
    {method_suffix_str, kiara_api_suffix_str,                                                       \
     BOOST_PP_STRINGIZE(api_func),                                                                  \
     (KIARA_GenericFunc)&(api_func)}

#define _KIARA_CXX_USER_API_EXPAND(elem)                                                            \
    _KIARA_CXX_USER_API(                                                                            \
            BOOST_PP_TUPLE_ELEM(_KRX_OPTY_TUPLE_LEN, _KRX_OPTY_METHOD_SUFFIX_STR_IDX, elem),        \
            BOOST_PP_TUPLE_ELEM(_KRX_OPTY_TUPLE_LEN, _KRX_OPTY_API_SUFFIX_IDX, elem),               \
            BOOST_PP_TUPLE_ELEM(_KRX_OPTY_TUPLE_LEN, _KRX_OPTY_API_SUFFIX_STR_IDX, elem),           \
            BOOST_PP_TUPLE_ELEM(_KRX_OPTY_TUPLE_LEN, _KRX_OPTY_API_FUNC_IDX, elem))

#define _KIARA_CXX_USER_API_SEQELEM(z, i, data)                                                     \
        _KIARA_CXX_USER_API_EXPAND(BOOST_PP_SEQ_ELEM(i, data))

#define _KIARA_CXX_USER_API_CHECK_ELEM(r, data, elem)                                               \
{                                                                                                   \
    KIARA_JOIN(KIARA_,BOOST_PP_TUPLE_ELEM(_KRX_OPTY_TUPLE_LEN, _KRX_OPTY_API_SUFFIX_IDX, elem))     \
      KIARA_JOIN(ftmp,__LINE__) =                                                                   \
      &(BOOST_PP_TUPLE_ELEM(_KRX_OPTY_TUPLE_LEN, _KRX_OPTY_API_FUNC_IDX, elem));                    \
    (void)KIARA_JOIN(ftmp,__LINE__);                                                                \
}

#define _KIARA_CXX_USER_API_CHECK(cxx_type_name, api_methods)                                       \
        static void type_mismatch_check()                                                           \
        {                                                                                           \
            BOOST_PP_SEQ_FOR_EACH(_KIARA_CXX_USER_API_CHECK_ELEM, cxx_type_name, api_methods)       \
        }                                                                                           \

#define _KIARA_CXX_DECL_API_FUNCS(api_methods)                                                      \
        static KIARA_DeclAPIFunc kr_apiFuncs[] = {                                                  \
                BOOST_PP_ENUM(BOOST_PP_SEQ_SIZE(api_methods),                                       \
                                _KIARA_CXX_USER_API_SEQELEM, api_methods)                           \
        };                                                                                          \
        static KIARA_DeclOpaqueType kr_api = {                                                      \
                kr_apiFuncs,                                                                        \
                sizeof(kr_apiFuncs) /                                                               \
                sizeof(kr_apiFuncs[0])                                                              \
        };

#define KIARA_CXX_DECL_OPAQUE_TYPE_NAME(kiara_type_name, cxx_type_name, func_args)                  \
    KIARA_CXX_DECL_TYPENAME(cxx_type_name)                                                          \
    namespace KIARA                                                                                 \
    {                                                                                               \
    template <> class TypeTraits<cxx_type_name>                                                     \
    {                                                                                               \
    public:                                                                                         \
        typedef ::cxx_type_name Type;                                                               \
        KIARA_DEFAULT_GET_TYPE_NAME(Type)                                                           \
        _KIARA_CXX_USER_API_CHECK(cxx_type_name, func_args)                                         \
        static KIARA_DeclType * getDeclType()                                                       \
        {                                                                                           \
            _KIARA_CXX_DECL_API_FUNCS(func_args)                                                    \
            static KIARA_DeclType kr_type = {                                                       \
                BOOST_PP_STRINGIZE(cxx_type_name),                                                  \
                KIARA_TYPE_OPAQUE,                                                                  \
                KIARA_DTF_NONE,                                                                     \
                {&kr_api}                                                                           \
            };                                                                                      \
            return &kr_type;                                                                        \
        }                                                                                           \
    };                                                                                              \
    template <> class TypeNameOf<const cxx_type_name> : public TypeNameOf<cxx_type_name>            \
    {                                                                                               \
    };                                                                                              \
    template <> class TypeTraits<const cxx_type_name> : public TypeTraits<cxx_type_name>            \
    {                                                                                               \
    };                                                                                              \
    }

#define KIARA_CXX_DECL_ANNOTATED(kiara_anno_type_name, kiara_type_name, annotation, semantics)  \
    typedef kiara_type_name _KR_tdef(kiara_anno_type_name);                                     \
    namespace KIARA                                                                             \
    {                                                                                           \
    template <> class TypeTraits<::_KR_tdef(kiara_anno_type_name)>                                          \
    {                                                                                           \
    public:                                                                                     \
        typedef ::_KR_tdef(kiara_anno_type_name) Type;                                                    \
        KIARA_DEFAULT_GET_TYPE_NAME(Type)                                                       \
        static KIARA_DeclType * getDeclType()                                                   \
        {                                                                                       \
            static KIARA_DeclAnnotated kr_annotd = {                                            \
                _KIARA_CXX_DTYPE_GETTER(kiara_type_name),                                       \
                BOOST_PP_STRINGIZE(annotation),                                                 \
                BOOST_PP_STRINGIZE(semantics),                                                  \
            };                                                                                  \
            static KIARA_DeclType kr_type = {                                                   \
                BOOST_PP_STRINGIZE(kiara_anno_type_name),                                       \
                KIARA_TYPE_ANNOTATED,                                                           \
                KIARA_DTF_NONE,                                                                 \
                {&kr_annotd}                                                                    \
            };                                                                                  \
            return &kr_type;                                                                    \
        }                                                                                       \
    };                                                                                          \
    }


/* Builtin types */

namespace KIARA
{

    template <typename T> class TypeNameOf
    {
    };

    template <typename T> class TypeTraits
    {
    };

    template <typename T> class TypeNameOf<const T> : public TypeNameOf<T>
    {
    };

    template <typename T> class TypeTraits<const T> : public TypeTraits<T>
    {
    };

    template <typename T> class ConvertVoidPtrToType
    {
    public:
        static T convert(void *value)
        {
            return *reinterpret_cast<T*>(value);
        }
    };

#if 0
    template <typename T> class ConvertVoidPtrToType<T *>
    {
    public:
        static T * convert(void *value)
        {
            return reinterpret_cast<T*>(value);
        }
    };

#endif

    template <typename T> class ConvertVoidPtrToType<T &>
    {
    public:
        typedef T & Type;
        typedef T * PtrType;
        static Type convert(void *value)
        {
            return *(*reinterpret_cast<PtrType*>(value));
        }
    };

    template <typename T> class ConvertVoidPtrToType<const T &>
    {
    public:
        typedef const T & Type;
        typedef const T * PtrType;
        static Type convert(void *value)
        {
            return *(*reinterpret_cast<PtrType*>(value));
        }
    };

#define KIARA_DEFAULT_GET_TYPE_NAME(T)              \
        static const char *getTypeName()            \
        {                                           \
            return ::KIARA::TypeNameOf<T>::get();   \
        }

    template <typename T> class TypeNameOf<T *>
    {
    public:
        static const char * get()
        {
            static const char * elemTypeName = ::KIARA::TypeNameOf<T>::get();
            static std::string typeName(std::string(elemTypeName) + "_ptr");
            return typeName.c_str();
        }
    };

    template <typename T> class TypeTraits<T *>
    {
    public:
        typedef T ElementType;
        typedef T * Type;

        KIARA_DEFAULT_GET_TYPE_NAME(Type)

        static KIARA_DeclType * getDeclType()
        {
            static KIARA_DeclPtr kr_ptr =
                {_KIARA_CXX_DTYPE_GETTER(T)};
            static KIARA_DeclType kr_type =
                {getTypeName(), KIARA_TYPE_POINTER, KIARA_DTF_NONE, {&kr_ptr}};
            return &kr_type;
        }
    };

    template <typename T> class TypeNameOf<T &>
    {
    public:
        static const char * get()
        {
            static const char * elemTypeName = ::KIARA::TypeNameOf<T>::get();
            static std::string typeName(std::string(elemTypeName) + "_ref");
            return typeName.c_str();
        }
    };

    template <typename T> class TypeTraits<T &>
    {
    public:
        typedef T ElementType;
        typedef T & Type;

        KIARA_DEFAULT_GET_TYPE_NAME(Type)

        static KIARA_DeclType * getDeclType()
        {
            static KIARA_DeclPtr kr_ref =
                {_KIARA_CXX_DTYPE_GETTER(T)};
            static KIARA_DeclType kr_type =
                {getTypeName(), KIARA_TYPE_REFERENCE, KIARA_DTF_NONE, {&kr_ref}};
            return &kr_type;
        }
    };

    template <typename T> class TypeNameOf<T[]>
    {
    public:
        static const char * get()
        {
            static const char * elemTypeName = ::KIARA::TypeNameOf<T>::get();
            static std::string typeName(std::string(elemTypeName) + "_array");
            return typeName.c_str();
        }
    };

    template <typename T> class TypeTraits<T[]>
    {
    public:
        typedef T ElementType;
        typedef T Type[];

        KIARA_DEFAULT_GET_TYPE_NAME(Type)

        static KIARA_DeclType * getDeclType()
        {
            static KIARA_DeclPtr kr_array =
                {_KIARA_CXX_DTYPE_GETTER(T)};
            static KIARA_DeclType kr_type =
                {getTypeName(), KIARA_TYPE_ARRAY, KIARA_DTF_NONE, {&kr_array}};
            return &kr_type;
        }
    };

    template <typename T, int n> class TypeNameOf<T[n]>
    {
    public:
        static const char * get()
        {
            static const char * elemTypeName = ::KIARA::TypeNameOf<T>::get();
            static std::string typeName(std::string(elemTypeName) + "_array_" + boost::lexical_cast<std::string>(n));
            return typeName.c_str();
        }
    };

    template <typename T, int n> class TypeTraits<T[n]>
    {
    public:
        typedef T ElementType;
        BOOST_STATIC_CONSTANT(int, numElements = n);
        typedef T Type[n];

        KIARA_DEFAULT_GET_TYPE_NAME(Type)

        static KIARA_DeclType * getDeclType()
        {
            static KIARA_DeclFixedArray kr_fixedArray =
                {_KIARA_CXX_DTYPE_GETTER(T), n};
            static KIARA_DeclType kr_type =
                {getTypeName(), KIARA_TYPE_FIXED_ARRAY, KIARA_DTF_NONE, {&kr_fixedArray}};
            return &kr_type;
        }
    };

}

#define _KI_TYPE_MAP(T1, T2)                                        \
namespace KIARA                                                     \
{                                                                   \
    template <> class TypeNameOf<T1>                                \
    {                                                               \
    public:                                                         \
        static const char * get()                                   \
        {                                                           \
            return BOOST_PP_STRINGIZE(T2);                          \
        }                                                           \
    };                                                              \
    template <> class TypeTraits<T1>                                \
    {                                                               \
    public:                                                         \
        typedef int Type;                                           \
        static const char * getTypeName()                           \
        {                                                           \
            return BOOST_PP_STRINGIZE(T2);                          \
        }                                                           \
        static KIARA_DeclType * getDeclType()                       \
        {                                                           \
            static KIARA_DeclType kr_type =                         \
                {BOOST_PP_STRINGIZE(T2),                            \
                 KIARA_TYPE_BUILTIN,                                \
                 KIARA_DTF_NONE, {0}};                              \
            return &kr_type;                                        \
        }                                                           \
    };                                                              \
    template <> class TypeNameOf<const T1> : public TypeNameOf<T1>  \
    {                                                               \
    };                                                              \
    template <> class TypeTraits<const T1> : public TypeTraits<T1>  \
    {                                                               \
    };                                                              \
}

_KI_TYPE_MAP(bool, bool)
_KI_TYPE_MAP(char, char)
_KI_TYPE_MAP(wchar_t, wchar_t)
_KI_TYPE_MAP(signed char, KIARA_schar)
_KI_TYPE_MAP(unsigned char, KIARA_uchar)
_KI_TYPE_MAP(short, short)
_KI_TYPE_MAP(unsigned short, KIARA_ushort)
_KI_TYPE_MAP(int, int)
_KI_TYPE_MAP(unsigned int, KIARA_uint)
_KI_TYPE_MAP(long, long)
_KI_TYPE_MAP(unsigned long, KIARA_ulong)
_KI_TYPE_MAP(long long, KIARA_longlong)
_KI_TYPE_MAP(unsigned long long, KIARA_ulonglong)
_KI_TYPE_MAP(float, float)
_KI_TYPE_MAP(double, double)
_KI_TYPE_MAP(long double, KIARA_longdouble)
//_KI_TYPE_MAP(const char *, KIARA_char_ptr)
_KI_TYPE_MAP(char *, KIARA_char_ptr)
//_KI_TYPE_MAP(const void *, KIARA_void_ptr)
_KI_TYPE_MAP(void *, KIARA_void_ptr)
_KI_TYPE_MAP(void, void)

#undef _KI_TYPE_MAP

#endif /* KIARA_CXX_MACROS_HPP_INCLUDED */
