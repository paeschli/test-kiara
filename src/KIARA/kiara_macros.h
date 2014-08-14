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
 * kiara_macros.h
 *
 *  Created on: 04.09.2012
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_MACROS_H_INCLUDED
#define KIARA_MACROS_H_INCLUDED

#include <KIARA/kiara.h>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/transform.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>

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
#define _KR_enc(suffix) _KIARA_DTYPE(enc, suffix)
#define _KR_enctd(suffix) _KIARA_DTYPE(enctd, suffix)
#define _KR_anno(suffix) _KIARA_DTYPE(anno, suffix)

/* Common macros */

#define _KIARA_DTYPE_REF(kiara_type_name)       \
    (&_KR_type(kiara_type_name))

#define _KIARA_DTYPE_GETTER(kiara_type_name)    \
    _KR_get_type(kiara_type_name)

#define KIARA_TYPE(kiara_type_name) _KIARA_DTYPE_GETTER(kiara_type_name)

/* Modifiers */
#undef _KR_TYPE
#undef _KR_DTF
#define KIARA_NO_MOD(what, x) x
#define KIARA_CONST_MOD(what, x) KIARA_JOIN(KIARA_CONST_MOD,what)(x)
#define KIARA_CONST_MOD_KR_TYPE(x) const x
#define KIARA_CONST_MOD_KR_DTF(x) KIARA_DTF_CONST_TYPE

/* Opaque type declaration */

/* OPTY - opaque type */
#define _KR_OPTY_TUPLE_LEN              5

#define _KR_OPTY_METHOD_SUFFIX_STR_IDX  0
#define _KR_OPTY_API_SUFFIX_IDX         1
#define _KR_OPTY_API_SUFFIX_STR_IDX     2
#define _KR_OPTY_API_FUNC_IDX           3
#define _KR_OPTY_MOD_IDX                4

#define _KR_OPTY_METHOD_SUFFIX_STR(t)                                       \
    BOOST_PP_TUPLE_ELEM(_KR_OPTY_TUPLE_LEN, _KR_OPTY_METHOD_SUFFIX_STR_IDX, t)
#define _KR_OPTY_API_SUFFIX(t)                                              \
    BOOST_PP_TUPLE_ELEM(_KR_OPTY_TUPLE_LEN, _KR_OPTY_API_SUFFIX_IDX, t)
#define _KR_OPTY_API_SUFFIX_STR(t)                                          \
    BOOST_PP_TUPLE_ELEM(_KR_OPTY_TUPLE_LEN, _KR_OPTY_API_SUFFIX_STR_IDX, t)
#define _KR_OPTY_API_FUNC(t)                                                \
    BOOST_PP_TUPLE_ELEM(_KR_OPTY_TUPLE_LEN, _KR_OPTY_API_FUNC_IDX, t)
#define _KR_OPTY_MOD(t)                                                     \
    BOOST_PP_TUPLE_ELEM(_KR_OPTY_TUPLE_LEN, _KR_OPTY_MOD_IDX, t)

#define KIARA_MEMBER_USER_API(member_name, kiara_api_name, value)                               \
    ((BOOST_PP_STRINGIZE(member_name), kiara_api_name, BOOST_PP_STRINGIZE(kiara_api_name), value, KIARA_NO_MOD))

#define KIARA_USER_API(kiara_api_name, value)                                                   \
    (("$this", kiara_api_name, BOOST_PP_STRINGIZE(kiara_api_name), value, KIARA_NO_MOD))

#define _KIARA_USER_API_CHECK(r, data, elem)                                                    \
{                                                                                               \
    KIARA_JOIN(KIARA_,_KR_OPTY_API_SUFFIX(elem))                                                \
      KIARA_JOIN(ftmp,__LINE__) =                                                               \
      &(_KR_OPTY_API_FUNC(elem));                                                               \
    (void)KIARA_JOIN(ftmp,__LINE__);                                                            \
}

#define _KR_OPUSER_API(user_api_tuple)                                      \
    {_KR_OPTY_METHOD_SUFFIX_STR(user_api_tuple),                            \
     _KR_OPTY_API_SUFFIX_STR(user_api_tuple),                               \
     BOOST_PP_STRINGIZE(_KR_OPTY_API_FUNC(user_api_tuple)),                 \
     (KIARA_GenericFunc)&_KR_OPTY_API_FUNC(user_api_tuple)}

#define _KR_OPUSER_APIS_EXPAND(z, n, c_type_name_and_args_tuple)            \
    BOOST_PP_COMMA_IF(n)                                                    \
    _KR_OPUSER_API(                                                         \
                 BOOST_PP_SEQ_ELEM(n,                                       \
                    BOOST_PP_TUPLE_ELEM(2, 1,                               \
                    c_type_name_and_args_tuple)))

/* This macro can only be used in following context:
 *  KIARA_BEGIN_EXTERN_C
 *  typedef c_type_name _KR_tdef(kiara_type_name);
 *  ...
 *  _KIARA_DECL_USER_API_TYPE_NAME(...)
 *  ...
 *  KIARA_END_EXTERN_C
 */
#define _KIARA_DECL_USER_API_TYPE_NAME(kiara_type_name, c_type_name, func_args) \
    static void KIARA_UNUSED _KR_type_mismatch_check(kiara_type_name)(void)                     \
    {                                                                                           \
        BOOST_PP_SEQ_FOR_EACH(_KIARA_USER_API_CHECK, c_type_name, func_args)                    \
    }                                                                                           \
    static KIARA_DeclAPIFunc _KR_apiFuncs(kiara_type_name)[] = {                                \
        BOOST_PP_REPEAT(BOOST_PP_SEQ_SIZE(func_args), _KR_OPUSER_APIS_EXPAND,                   \
                (c_type_name, func_args))                                                       \
    };                                                                                          \
    static KIARA_DeclAPI _KR_api(kiara_type_name) = {                                           \
        _KR_apiFuncs(kiara_type_name),                                                          \
        sizeof(_KR_apiFuncs(kiara_type_name)) /                                                 \
        sizeof(_KR_apiFuncs(kiara_type_name)[0])};

#define KIARA_DECL_OPAQUE_TYPE_NAME(kiara_type_name, c_type_name, func_args)                    \
    KIARA_BEGIN_EXTERN_C                                                                        \
    typedef c_type_name _KR_tdef(kiara_type_name);                                              \
    _KIARA_DECL_USER_API_TYPE_NAME(kiara_type_name, c_type_name, func_args)                     \
    static KIARA_DeclOpaqueType _KR_opaque(kiara_type_name) = {                                 \
        _KR_apiFuncs(kiara_type_name),                                                          \
        sizeof(_KR_apiFuncs(kiara_type_name)) /                                                 \
        sizeof(_KR_apiFuncs(kiara_type_name)[0])};                                              \
    static KIARA_DeclType _KR_type(kiara_type_name) = {                                         \
        BOOST_PP_STRINGIZE(c_type_name),                                                        \
        KIARA_TYPE_OPAQUE,                                                                      \
        KIARA_DTF_NONE,                                                                         \
        {&_KR_api(kiara_type_name)}};                                                           \
    _KIARA_GEN_DTYPE_GETTER(kiara_type_name)                                                    \
    KIARA_END_EXTERN_C

#define KIARA_DECL_OPAQUE_TYPE(c_type_name, args)                                               \
        KIARA_DECL_OPAQUE_TYPE_NAME(c_type_name, c_type_name, args)

/* Struct declaration */

/* STTY - struct type */
#define _KR_STTY_TUPLE_LEN          4

/* Member type */
#define _KR_STTY_TYPE_IDX           0
/* Member name as symbol */
#define _KR_STTY_NAME_IDX           1
/* Main member name as string */
#define _KR_STTY_MAIN_NAME_STR_IDX  2
/* Member semantics */
#define _KR_STTY_SEMANTICS_STR_IDX  3

#define _KR_STTY_TYPE(t)                                                \
    BOOST_PP_TUPLE_ELEM(_KR_STTY_TUPLE_LEN, _KR_STTY_TYPE_IDX, t)
#define _KR_STTY_NAME(t)                                                \
    BOOST_PP_TUPLE_ELEM(_KR_STTY_TUPLE_LEN, _KR_STTY_NAME_IDX, t)
#define _KR_STTY_MAIN_NAME_STR(t)                                       \
    BOOST_PP_TUPLE_ELEM(_KR_STTY_TUPLE_LEN, _KR_STTY_MAIN_NAME_STR_IDX, t)
#define _KR_STTY_SEMANTICS_STR(t)                                       \
    BOOST_PP_TUPLE_ELEM(_KR_STTY_TUPLE_LEN, _KR_STTY_SEMANTICS_STR_IDX, t)


#define KIARA_STRUCT_MEMBER(kiara_type_name, member_name)                                            \
    ((kiara_type_name, member_name, NULL, NULL))

#define KIARA_STRUCT_MEMBER_WITH_SEMANTICS(kiara_type_name, member_name, semantics)                  \
    ((kiara_type_name, member_name, NULL, BOOST_PP_STRINGIZE(semantics)))

#define KIARA_STRUCT_DEPENDENT_MEMBER(kiara_type_name, member_name, main_member_name, semantics)     \
    ((kiara_type_name, member_name, BOOST_PP_STRINGIZE(main_member_name), BOOST_PP_STRINGIZE(semantics)))

#define KIARA_STRUCT_ARRAY_MEMBER(ptr_type_name, ptr_member_name, size_type_name, size_member_name)  \
    KIARA_STRUCT_MEMBER_WITH_SEMANTICS(ptr_type_name, ptr_member_name, VI_ARRAY_PTR)                                      \
    KIARA_STRUCT_DEPENDENT_MEMBER(size_type_name, size_member_name, ptr_member_name, DEFAULT)

#define KIARA_FORWARD_DECL(kiara_type_name)                                 \
    KIARA_BEGIN_EXTERN_C                                                    \
    static KIARA_DeclType * _KR_get_type(kiara_type_name)(void);            \
    KIARA_END_EXTERN_C

#define _KIARA_GEN_DTYPE_GETTER(kiara_type_name)                            \
    static KIARA_DeclType * _KR_get_type(kiara_type_name)(void)             \
    {                                                                       \
        return _KIARA_DTYPE_REF(kiara_type_name);                           \
    }

#define _KIARA_GEN_DTYPE_UNUSED_GETTER(kiara_type_name)                     \
    static KIARA_DeclType * _KR_get_type(kiara_type_name)(void)             \
                                                   KIARA_UNUSED;            \
    static KIARA_DeclType * _KR_get_type(kiara_type_name)(void)             \
    {                                                                       \
        return _KIARA_DTYPE_REF(kiara_type_name);                           \
    }

#define KIARA_DECL_PTR_EX(kiara_ptr_type_name, kiara_element_type_name, modifier)   \
    KIARA_BEGIN_EXTERN_C                                                            \
    typedef modifier(_KR_TYPE, kiara_element_type_name) *                           \
            _KR_tdef(kiara_ptr_type_name);                                          \
    static KIARA_DeclPtr _KR_ptr(kiara_ptr_type_name) = {                           \
        _KIARA_DTYPE_GETTER(kiara_element_type_name)                                \
    };                                                                              \
    static KIARA_DeclType _KR_type(kiara_ptr_type_name) =                           \
    {BOOST_PP_STRINGIZE(kiara_ptr_type_name),                                       \
        KIARA_TYPE_POINTER,                                                         \
        modifier(_KR_DTF, KIARA_DTF_NONE),                                          \
        {&_KR_ptr(kiara_ptr_type_name)}};                                           \
    _KIARA_GEN_DTYPE_GETTER(kiara_ptr_type_name)                                    \
    KIARA_END_EXTERN_C

#define KIARA_DECL_PTR(kiara_ptr_type_name, kiara_element_type_name)                \
    KIARA_DECL_PTR_EX(kiara_ptr_type_name, kiara_element_type_name, KIARA_NO_MOD)

#define KIARA_DECL_CONST_PTR(kiara_ptr_type_name, kiara_element_type_name)          \
    KIARA_DECL_PTR_EX(kiara_ptr_type_name, kiara_element_type_name, KIARA_CONST_MOD)

#define _KR_STMEMBER(c_struct_name, member_tuple)               \
    {_KIARA_DTYPE_GETTER(_KR_STTY_TYPE(member_tuple)),          \
      BOOST_PP_STRINGIZE(_KR_STTY_NAME(member_tuple)),          \
      offsetof(c_struct_name, _KR_STTY_NAME(member_tuple)),     \
      _KR_STTY_MAIN_NAME_STR(member_tuple),                     \
      _KR_STTY_SEMANTICS_STR(member_tuple)                      \
    }

#define _KR_STMEMBERS_EXPAND(z, n, c_struct_name_and_member_tuple)          \
    BOOST_PP_COMMA_IF(n)                                                    \
    _KR_STMEMBER(BOOST_PP_TUPLE_ELEM(2, 0, c_struct_name_and_member_tuple), \
                 BOOST_PP_SEQ_ELEM(n,                                       \
                    BOOST_PP_TUPLE_ELEM(2, 1,                               \
                    c_struct_name_and_member_tuple)))

#define _KIARA_DECL_STRUCT_NAME_WITH_API(kiara_type_name, c_struct_name, struct_members, api_ptr)   \
    static KIARA_DeclStructMember _KR_members(kiara_type_name)[] = {                                \
        BOOST_PP_REPEAT(BOOST_PP_SEQ_SIZE(struct_members), _KR_STMEMBERS_EXPAND,                    \
                (c_struct_name, struct_members))                                                    \
    };                                                                                              \
    static KIARA_DeclStruct _KR_struct(kiara_type_name) = {                                         \
        _KR_members(kiara_type_name),                                                               \
        sizeof(_KR_members(kiara_type_name)) /                                                      \
        sizeof(_KR_members(kiara_type_name)[0]),                                                    \
        sizeof(c_struct_name),                                                                      \
        (api_ptr)                                                                                   \
    };                                                                                              \
    static KIARA_DeclType _KR_type(kiara_type_name) =                                               \
        {BOOST_PP_STRINGIZE(c_struct_name),                                                         \
            KIARA_TYPE_STRUCT,                                                                      \
            KIARA_DTF_NONE,                                                                         \
            {&_KR_struct(kiara_type_name)}};                                                        \
    _KIARA_GEN_DTYPE_GETTER(kiara_type_name)

#define KIARA_DECL_STRUCT_NAME(kiara_type_name, c_struct_name, struct_members)                      \
        KIARA_BEGIN_EXTERN_C                                                                        \
        typedef c_struct_name _KR_tdef(kiara_type_name);                                            \
        _KIARA_DECL_STRUCT_NAME_WITH_API(kiara_type_name, c_struct_name, struct_members, NULL)      \
        KIARA_END_EXTERN_C

#define KIARA_DECL_STRUCT(c_struct_name, members)                                   \
        KIARA_DECL_STRUCT_NAME(c_struct_name, c_struct_name, members)

#define KIARA_DECL_STRUCT_NAME_WITH_API(kiara_type_name, c_struct_name, struct_members, api_members)                    \
        KIARA_BEGIN_EXTERN_C                                                                                            \
        typedef c_struct_name _KR_tdef(kiara_type_name);                                                                \
        _KIARA_DECL_USER_API_TYPE_NAME(kiara_type_name, c_struct_name, api_members)                                     \
        _KIARA_DECL_STRUCT_NAME_WITH_API(kiara_type_name, c_struct_name, struct_members, &_KR_api(kiara_type_name))     \
        KIARA_END_EXTERN_C

#define KIARA_DECL_STRUCT_WITH_API(c_struct_name, members, api_members)                                     \
        KIARA_DECL_STRUCT_NAME_WITH_API(c_struct_name, c_struct_name, members, api_members)

/* Function declaration */

/* CFTY - client function type */
#define _KR_CFTY_TUPLE_LEN          6

/* Argument type */
#define _KR_CFTY_TYPE_IDX           0
/* Argument name as symbol */
#define _KR_CFTY_NAME_IDX           1
/* Argument name as string */
#define _KR_CFTY_NAME_STR_IDX       2
/* Annotation string */
#define _KR_CFTY_ANNOTATION_STR_IDX 3
/* Argument semantics string */
#define _KR_CFTY_SEMANTICS_STR_IDX  4
/* Argument type modifier */
#define _KR_CFTY_MOD_IDX            5

#define _KR_CFTY_TYPE(t)                                                \
    BOOST_PP_TUPLE_ELEM(_KR_CFTY_TUPLE_LEN, _KR_CFTY_TYPE_IDX, t)
#define _KR_CFTY_NAME(t)                                                \
    BOOST_PP_TUPLE_ELEM(_KR_CFTY_TUPLE_LEN, _KR_CFTY_NAME_IDX, t)
#define _KR_CFTY_NAME_STR(t)                                            \
    BOOST_PP_TUPLE_ELEM(_KR_CFTY_TUPLE_LEN, _KR_CFTY_NAME_STR_IDX, t)
#define _KR_CFTY_ANNOTATION_STR(t)                                      \
    BOOST_PP_TUPLE_ELEM(_KR_CFTY_TUPLE_LEN, _KR_CFTY_ANNOTATION_STR_IDX, t)
#define _KR_CFTY_SEMANTICS_STR(t)                                       \
    BOOST_PP_TUPLE_ELEM(_KR_CFTY_TUPLE_LEN, _KR_CFTY_SEMANTICS_STR_IDX, t)
#define _KR_CFTY_MOD(t)                                                 \
    BOOST_PP_TUPLE_ELEM(_KR_CFTY_TUPLE_LEN, _KR_CFTY_MOD_IDX, t)

#define KIARA_FUNC_ARG(kiara_type_name, member_name)                            \
    ((kiara_type_name, member_name, BOOST_PP_STRINGIZE(member_name), NULL, NULL, KIARA_NO_MOD))

#define KIARA_FUNC_ARG_ANNOTATED(kiara_type_name, member_name, annotation)      \
    ((kiara_type_name, member_name, BOOST_PP_STRINGIZE(member_name), BOOST_PP_STRINGIZE(annotation), NULL, KIARA_NO_MOD))

#define KIARA_FUNC_ARG_WITH_SEMANTICS(kiara_type_name, member_name, semantics)  \
    ((kiara_type_name, member_name, BOOST_PP_STRINGIZE(member_name), NULL, BOOST_PP_STRINGIZE(semantics), KIARA_NO_MOD))

#define KIARA_FUNC_CONST_ARG(kiara_type_name, member_name)                      \
    ((kiara_type_name, member_name, BOOST_PP_STRINGIZE(member_name), NULL, NULL, KIARA_CONST_MOD))

#define KIARA_FUNC_CONST_ARG_ANNOTATED(kiara_type_name, member_name, annotation)        \
    ((kiara_type_name, member_name, BOOST_PP_STRINGIZE(member_name), BOOST_PP_STRINGIZE(annotation), NULL, KIARA_CONST_MOD))

#define KIARA_FUNC_CONST_ARG_WITH_SEMANTICS(kiara_type_name, member_name, semantics)    \
    ((kiara_type_name, member_name, BOOST_PP_STRINGIZE(member_name), NULL, BOOST_PP_STRINGIZE(semantics), KIARA_CONST_MOD))

#define KIARA_FUNC_RESULT(kiara_type_name, member_name)                         \
    ((kiara_type_name, member_name, "$result", NULL, NULL, KIARA_NO_MOD))

#define KIARA_FUNC_EXCEPTION(kiara_type_name, member_name)                      \
    ((kiara_type_name, member_name, "$exception", NULL, NULL, KIARA_NO_MOD))

#define _KIARA_FUNC_ARG_EXPAND(kiara_type_name, c_func_name, arg_name_str)  \
    {_KIARA_DTYPE_GETTER(kiara_type_name), arg_name_str},

#define _KIARA_FUNC_ARG(r, data, elem)                                      \
    _KIARA_FUNC_ARG_EXPAND(                                                 \
            _KR_CFTY_TYPE(elem),                                            \
            data,                                                           \
            _KR_CFTY_NAME_STR(elem))

#define _KIARA_FUNC_ARG_TYPE(s, data, elem)                                 \
    _KR_tdef(_KR_CFTY_TYPE(elem))

#define _KIARA_FUNC_ARG_NAME(s, data, elem)                                 \
    _KR_CFTY_NAME(elem)

#define _KIARA_FUNC_ARG_VOID_PTR(s, data, elem)                             \
    ((void*)&_KIARA_FUNC_ARG_NAME(s, data, elem))

/**
 * Apply type modifier to argument type, retrieve argument name
 */
#define _KIARA_FUNC_ARG_TYPE_AND_NAME(s, data, elem)                        \
    _KR_CFTY_MOD(elem)(_KR_TYPE, _KR_tdef(_KR_CFTY_TYPE(elem)))             \
    _KR_CFTY_NAME(elem)

/* Original implementation */
/*
#define _KIARA_GEN_FUNC_TYPEDEF(func_name, kiara_return_type_name, func_args)   \
    typedef _KR_tdef(kiara_return_type_name)                                    \
            (*func_name)(BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(              \
                       _KIARA_FUNC_ARG_TYPE_AND_NAME, 0, func_args)));
*/

#define _KIARA_GEN_FUNC_TYPEDEF(kiara_type_name, func_name,                 \
                                kiara_return_type_name, func_args)          \
    typedef int                                                             \
            (*func_name)(KIARA_FUNC_OBJ(kiara_type_name),                   \
                        BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(           \
                        _KIARA_FUNC_ARG_TYPE_AND_NAME, 0, func_args)));

#define KIARA_FUNC_OBJ_TYPEDEF(kiara_type_name)                             \
        _KR_funcobj_ptr(kiara_type_name)

#define _KIARA_GEN_FUNC_FWD_DECL(kiara_type_name, func_args)                \
    typedef struct _KR_funcobj(kiara_type_name)                             \
        _KR_funcobj(kiara_type_name),                                       \
        * KIARA_FUNC_OBJ_TYPEDEF(kiara_type_name);                          \
    typedef int (*_KR_func_wrapper_type(kiara_type_name))                   \
                (KIARA_FUNC_OBJ(kiara_type_name) kiara_funcobj_,            \
                        BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(           \
                       _KIARA_FUNC_ARG_TYPE_AND_NAME, 0, func_args)));      \
    static int _KR_func_wrapper(kiara_type_name)                            \
                (KIARA_FUNC_OBJ(kiara_type_name) kiara_funcobj_,            \
                   BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(                \
               _KIARA_FUNC_ARG_TYPE_AND_NAME, 0, func_args)));

#define _KIARA_GEN_FUNC_WRAPPER(kiara_type_name, func_args)                 \
    static int _KR_func_wrapper(kiara_type_name)                            \
                (KIARA_FUNC_OBJ(kiara_type_name) kiara_funcobj_,            \
                   BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(                \
               _KIARA_FUNC_ARG_TYPE_AND_NAME, 0, func_args)))               \
    {                                                                       \
        void *args[] = {                                                    \
            BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(                       \
                                _KIARA_FUNC_ARG_VOID_PTR, 0, func_args))    \
        };                                                                  \
        return kiara_funcobj_->base.vafunc((KIARA_FuncObj*)kiara_funcobj_,  \
                args,                                                       \
                BOOST_PP_SEQ_SIZE(func_args));                              \
    }

#define _KIARA_GEN_FUNC_OBJ_TYPE(kiara_type_name)                           \
    struct _KR_funcobj(kiara_type_name) {                                   \
        KIARA_FuncObjBase base;                                             \
        _KR_func_wrapper_type(kiara_type_name) func;                        \
    };

#define _KIARA_GEN_FUNC_TYPECHECK(kiara_type_name, func_args)               \
    static void KIARA_UNUSED _KR_type_mismatch_check(kiara_type_name)(void) \
    {                                                                       \
        _KIARA_GEN_FUNC_TYPEDEF(kiara_type_name,                            \
                _KR_tdef_generated(kiara_type_name),                        \
                kr_tdef_int, func_args)                                     \
                                                                            \
        _KR_tdef_generated(kiara_type_name) generated;                      \
        _KR_tdef(kiara_type_name) user_specified;                           \
        generated = NULL;                                                   \
        user_specified = generated;                                         \
        (void)user_specified;                                               \
    }

#define _KR_CFARG(c_func_name, args_tuple)                                  \
    {_KIARA_DTYPE_GETTER(_KR_CFTY_TYPE(args_tuple)),                        \
     _KR_CFTY_NAME_STR(args_tuple),                                         \
     _KR_CFTY_ANNOTATION_STR(args_tuple),                                   \
     _KR_CFTY_SEMANTICS_STR(args_tuple)                                     \
    }

#define _KR_CFARGS_EXPAND(z, n, c_func_name_and_args_tuple)                 \
    BOOST_PP_COMMA_IF(n)                                                    \
    _KR_CFARG(BOOST_PP_TUPLE_ELEM(2, 0, c_func_name_and_args_tuple),        \
                 BOOST_PP_SEQ_ELEM(n,                                       \
                    BOOST_PP_TUPLE_ELEM(2, 1,                               \
                    c_func_name_and_args_tuple)))


/*
 * Code used before for declaration of the function type:
 *
 *   typedef c_func_name _KR_tdef(kiara_type_name);
 *
 */

#define KIARA_DECL_FUNC_NAME(kiara_type_name, c_func_name, func_args)       \
    KIARA_BEGIN_EXTERN_C                                                    \
    KIARA_DECL_FUNC_OBJ(kiara_type_name)                                    \
    _KIARA_GEN_FUNC_TYPEDEF(kiara_type_name,                                \
        _KR_tdef(kiara_type_name),                                          \
        kr_tdef_int, func_args)                                             \
    typedef _KR_tdef(kiara_type_name) c_func_name;                          \
    _KIARA_GEN_FUNC_FWD_DECL(kiara_type_name, func_args)                    \
    _KIARA_GEN_FUNC_TYPECHECK(kiara_type_name, func_args)                   \
    _KIARA_GEN_FUNC_OBJ_TYPE(kiara_type_name)                               \
    _KIARA_GEN_FUNC_WRAPPER(kiara_type_name, func_args)                     \
    static KIARA_DeclFuncArgument _KR_args(kiara_type_name)[] = {           \
        BOOST_PP_REPEAT(BOOST_PP_SEQ_SIZE(func_args), _KR_CFARGS_EXPAND,    \
            (c_func_name, func_args))                                       \
    };                                                                      \
    static KIARA_DeclFunc _KR_func(kiara_type_name) = {                     \
        (KIARA_Func)_KR_func_wrapper(kiara_type_name),                      \
        _KIARA_DTYPE_GETTER(KIARA_INT),                                     \
        _KR_args(kiara_type_name),                                          \
        sizeof(_KR_args(kiara_type_name)) /                                 \
        sizeof(_KR_args(kiara_type_name)[0])};                              \
    static KIARA_DeclType _KR_type(kiara_type_name) = {                     \
        BOOST_PP_STRINGIZE(c_func_name),                                    \
        KIARA_TYPE_FUNC,                                                    \
        KIARA_DTF_NONE,                                                     \
        {&_KR_func(kiara_type_name)}};                                      \
    _KIARA_GEN_DTYPE_GETTER(kiara_type_name)                                \
    KIARA_END_EXTERN_C

#define KIARA_DECL_FUNC(c_func_name, args) \
        KIARA_DECL_FUNC_NAME(c_func_name, c_func_name, args)

/* Array declaration */

#define KIARA_DECL_ARRAY(kiara_array_type_name, kiara_element_type_name)        \
    KIARA_BEGIN_EXTERN_C                                                        \
    typedef kiara_element_type_name                                             \
            _KR_tdef(kiara_array_type_name)[];                                  \
    static KIARA_DeclPtr _KR_array(kiara_array_type_name) = {                   \
        _KIARA_DTYPE_GETTER(kiara_element_type_name)                            \
    };                                                                          \
    static KIARA_DeclType _KR_type(kiara_array_type_name) = {                   \
        BOOST_PP_STRINGIZE(kiara_array_type_name),                              \
        KIARA_TYPE_ARRAY,                                                       \
        KIARA_DTF_NONE,                                                         \
        {&_KR_array(kiara_array_type_name)}};                                   \
    _KIARA_GEN_DTYPE_GETTER(kiara_array_type_name)                              \
    KIARA_END_EXTERN_C

#define KIARA_DECL_FIXED_ARRAY(kiara_type_name, kiara_element_type_name, size)  \
    KIARA_BEGIN_EXTERN_C                                                        \
    typedef kiara_element_type_name                                             \
            _KR_tdef(kiara_type_name)[size];                                    \
    static KIARA_DeclFixedArray _KR_fixedArray(kiara_type_name) = {             \
        _KIARA_DTYPE_GETTER(kiara_element_type_name), size                      \
    };                                                                          \
    static KIARA_DeclType _KR_type(kiara_type_name) = {                         \
        BOOST_PP_STRINGIZE(kiara_type_name),                                    \
        KIARA_TYPE_FIXED_ARRAY,                                                 \
        KIARA_DTF_NONE,                                                         \
        {&_KR_fixedArray(kiara_type_name)}};                                    \
    _KIARA_GEN_DTYPE_GETTER(kiara_type_name)                                    \
    KIARA_END_EXTERN_C

#define KIARA_DECL_FIXED_ARRAY_2D(kiara_type_name, kiara_element_type_name, num_rows, num_cols) \
    KIARA_BEGIN_EXTERN_C                                                                        \
    typedef kiara_element_type_name                                                             \
            _KR_tdef(kiara_type_name)[num_rows][num_cols];                                      \
    static KIARA_DeclFixedArray2D _KR_fixedArray2D(kiara_type_name) = {                         \
        _KIARA_DTYPE_GETTER(kiara_element_type_name), num_rows, num_cols                        \
    };                                                                                          \
    static KIARA_DeclType _KR_type(kiara_type_name) = {                                         \
        BOOST_PP_STRINGIZE(kiara_type_name),                                                    \
        KIARA_TYPE_FIXED_ARRAY_2D,                                                              \
        KIARA_DTF_NONE,                                                                         \
        {&_KR_fixedArray2D(kiara_type_name)}};                                                  \
    _KIARA_GEN_DTYPE_GETTER(kiara_type_name)                                                    \
    KIARA_END_EXTERN_C

#define KIARA_GET_TYPE(ctx, kiara_type_name) \
    kiaraGetContextTypeFromDecl(ctx, _KIARA_DTYPE_GETTER(kiara_type_name))

#define KIARA_REGISTER_TYPE(ctx, kiara_type_name) \
        (void)kiaraGetContextTypeFromDecl(ctx, _KIARA_DTYPE_GETTER(kiara_type_name))

#define KIARA_CALL(funcobj, ...)                \
    ((funcobj)->func((funcobj), __VA_ARGS__))

#define KIARA_GENERATE_CLIENT_FUNC(connection, idl_method_name, kiara_type_name, mapping)   \
        (KIARA_FUNC_OBJ(kiara_type_name))kiaraGenerateClientFuncObj((connection),           \
            (idl_method_name), KIARA_TYPE(kiara_type_name), (mapping))

#define KIARA_REGISTER_SERVICE_IMPL(service, idl_method_name, kiara_type_name, mapping)     \
    kiaraRegisterServiceFunc((service), idl_method_name, KIARA_TYPE(kiara_type_name),       \
            (mapping), NULL)

#define KIARA_REGISTER_SERVICE_FUNC(service, idl_method_name, kiara_type_name, mapping, service_func)   \
    kiaraRegisterServiceFunc((service), idl_method_name, KIARA_TYPE(kiara_type_name),                   \
            (mapping), _KR_service_func_conv(kiara_type_name)(service_func))

/* Service type declaration */

/* SRTY - service type */
#define _KR_SRTY_TUPLE_LEN          6

/* Argument type */
#define _KR_SRTY_TYPE_IDX           0
/* Argument name as symbol */
#define _KR_SRTY_NAME_IDX           1
/* Argument name as string */
#define _KR_SRTY_NAME_STR_IDX       2
/* Argument annotation string */
#define _KR_SRTY_ANNOTATION_STR_IDX 3
/* Argument semantics string */
#define _KR_SRTY_SEMANTICS_STR_IDX  4
/* Argument type modifier */
#define _KR_SRTY_NAME_MOD_IDX       5

#define _KR_SRTY_TYPE(t)                                                \
    BOOST_PP_TUPLE_ELEM(_KR_SRTY_TUPLE_LEN, _KR_SRTY_TYPE_IDX, t)
#define _KR_SRTY_NAME(t)                                                \
    BOOST_PP_TUPLE_ELEM(_KR_SRTY_TUPLE_LEN, _KR_SRTY_NAME_IDX, t)
#define _KR_SRTY_NAME_STR(t)                                            \
    BOOST_PP_TUPLE_ELEM(_KR_SRTY_TUPLE_LEN, _KR_SRTY_NAME_STR_IDX, t)
#define _KR_SRTY_ANNOTATION_STR(t)                                      \
    BOOST_PP_TUPLE_ELEM(_KR_SRTY_TUPLE_LEN, _KR_SRTY_ANNOTATION_STR_IDX, t)
#define _KR_SRTY_SEMANTICS_STR(t)                                       \
    BOOST_PP_TUPLE_ELEM(_KR_SRTY_TUPLE_LEN, _KR_SRTY_SEMANTICS_STR_IDX, t)
#define _KR_SRTY_MOD(t)                                                 \
    BOOST_PP_TUPLE_ELEM(_KR_SRTY_TUPLE_LEN, _KR_SRTY_NAME_MOD_IDX, t)

#define KIARA_SERVICE_ARG(kiara_type_name, member_name)                                             \
    ((kiara_type_name, member_name, BOOST_PP_STRINGIZE(member_name), NULL, NULL, KIARA_NO_MOD))

#define KIARA_SERVICE_CONST_ARG(kiara_type_name, member_name)                                       \
    ((kiara_type_name, member_name, BOOST_PP_STRINGIZE(member_name), NULL, NULL, KIARA_CONST_MOD))

#define KIARA_SERVICE_RESULT(kiara_type_name, member_name)                                          \
    ((kiara_type_name, member_name, "$result", NULL, NULL, KIARA_NO_MOD))

#define KIARA_SERVICE_RESULT_WITH_SEMANTICS(kiara_type_name, member_name, semantics)                \
    ((kiara_type_name, member_name, "$result", NULL, BOOST_PP_STRINGIZE(semantics), KIARA_NO_MOD))

#define KIARA_SERVICE_EXCEPTION(kiara_type_name, member_name)                                       \
    ((kiara_type_name, member_name, "$exception", NULL, NULL, KIARA_NO_MOD))


/* Expand service wrapper arguments */
#define _KR_SRWARGS_EXPAND(z, n, arg_tuple)                                     \
    BOOST_PP_COMMA_IF(n)                                                        \
    *(_KR_tdef(_KR_SRTY_TYPE(BOOST_PP_SEQ_ELEM(n, arg_tuple))) *)args[n]

/* Expand service function arguments
 *
 * Apply type modifier (_KR_SRTY_MOD_IDX-th element of the tuple) to
 * argument type (_KR_SRTY_TYPE_IDX-nd element of the tuple),
 * retrieve argument name (_KR_SRTY_NAME_IDX-st element of the tuple)
 */
#define _KR_SRFARGS_EXPAND(z, n, arg_tuple)                                 \
    BOOST_PP_COMMA_IF(n)                                                    \
    _KR_SRTY_MOD(BOOST_PP_SEQ_ELEM(n, arg_tuple))(_KR_TYPE,                 \
                _KR_tdef(_KR_SRTY_TYPE(BOOST_PP_SEQ_ELEM(n, arg_tuple))))   \
    _KR_SRTY_NAME(BOOST_PP_SEQ_ELEM(n, arg_tuple))

/* Expand service declaration */
#define _KR_SRDECL_EXPAND(z, n, arg_tuple)                                      \
    BOOST_PP_COMMA_IF(n)                                                        \
    {_KIARA_DTYPE_GETTER(_KR_SRTY_TYPE(BOOST_PP_SEQ_ELEM(n, arg_tuple))),       \
     _KR_SRTY_NAME_STR(BOOST_PP_SEQ_ELEM(n, arg_tuple)),                        \
     _KR_SRTY_ANNOTATION_STR(BOOST_PP_SEQ_ELEM(n, arg_tuple)), /*annotation*/   \
     _KR_SRTY_SEMANTICS_STR(BOOST_PP_SEQ_ELEM(n, arg_tuple))   /*semantics*/    \
    }

/*
 * Code used before for declaration of the function type:
 *
 *   typedef c_func_name _KR_tdef(kiara_type_name);
 *
 */

#define KIARA_DECL_SERVICE(kiara_type_name, service_args)                                           \
    KIARA_BEGIN_EXTERN_C                                                                            \
    typedef KIARA_Result (*_KR_service_func_type(kiara_type_name))(                                 \
             KIARA_ServiceFuncObj *kiara_funcobj_,                                                  \
             BOOST_PP_REPEAT(BOOST_PP_SEQ_SIZE(service_args), _KR_SRFARGS_EXPAND, service_args));   \
                                                                                                    \
    static KIARA_INLINE KIARA_ServiceFunc _KR_service_func_conv(kiara_type_name)(                   \
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
                                                                                                    \
    static KIARA_DeclServiceArgument _KR_service_args(kiara_type_name) [] = {                       \
        BOOST_PP_REPEAT(BOOST_PP_SEQ_SIZE(service_args), _KR_SRDECL_EXPAND, service_args)           \
    };                                                                                              \
    static KIARA_DeclService _KR_service(kiara_type_name) = {                                       \
        NULL,                                                                                       \
        _KR_service_wrapper(kiara_type_name),                                                       \
        _KR_get_type(KIARA_INT),                                                                    \
        _KR_service_args(kiara_type_name),                                                          \
        sizeof(_KR_service_args(kiara_type_name)) /                                                 \
        sizeof(_KR_service_args(kiara_type_name)[0])};                                              \
    static KIARA_DeclType _KR_type(kiara_type_name) = {                                             \
        BOOST_PP_STRINGIZE(kiara_type_name),                                                        \
        KIARA_TYPE_SERVICE,                                                                         \
        KIARA_DTF_NONE,                                                                             \
        {&_KR_service(kiara_type_name)}};                                                           \
    _KIARA_GEN_DTYPE_GETTER(kiara_type_name)                                                        \
    KIARA_END_EXTERN_C

#define KIARA_DECL_SERVICE_IMPL(kiara_type_name, service_args)                                      \
    KIARA_BEGIN_EXTERN_C                                                                            \
    typedef KIARA_Result (*_KR_service_func_type(kiara_type_name))(                                 \
             KIARA_ServiceFuncObj *kiara_funcobj_,                                                  \
             BOOST_PP_REPEAT(BOOST_PP_SEQ_SIZE(service_args), _KR_SRFARGS_EXPAND, service_args));   \
                                                                                                    \
    static KIARA_INLINE KIARA_ServiceFunc _KR_service_func_conv(kiara_type_name)(                   \
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
                                                                                                    \
    static KIARA_DeclServiceArgument _KR_service_args(kiara_type_name) [] = {                       \
        BOOST_PP_REPEAT(BOOST_PP_SEQ_SIZE(service_args), _KR_SRDECL_EXPAND, service_args)           \
    };                                                                                              \
    static KIARA_DeclService _KR_service(kiara_type_name) = {                                       \
        (KIARA_ServiceFunc)_KR_service_func(kiara_type_name),                                       \
        _KR_service_wrapper(kiara_type_name),                                                       \
        _KR_get_type(KIARA_INT),                                                                    \
        _KR_service_args(kiara_type_name),                                                          \
        sizeof(_KR_service_args(kiara_type_name)) /                                                 \
        sizeof(_KR_service_args(kiara_type_name)[0])};                                              \
    static KIARA_DeclType _KR_type(kiara_type_name) = {                                             \
        BOOST_PP_STRINGIZE(kiara_type_name),                                                        \
        KIARA_TYPE_SERVICE,                                                                         \
        KIARA_DTF_NONE,                                                                             \
        {&_KR_service(kiara_type_name)}};                                                           \
    _KIARA_GEN_DTYPE_GETTER(kiara_type_name)                                                        \
    KIARA_END_EXTERN_C                                                                              \
    static KIARA_Result _KR_service_func(kiara_type_name)(                                          \
            KIARA_ServiceFuncObj *kiara_funcobj,                                                    \
            BOOST_PP_REPEAT(BOOST_PP_SEQ_SIZE(service_args), _KR_SRFARGS_EXPAND, service_args))

/* Encrypted data types */

#define KIARA_DECL_ENCRYPTED(kiara_type_name, kiara_element_type_name)      \
    KIARA_FWD_DECL_ENCRYPTED(kiara_type_name)                               \
    typedef struct _KR_enc(kiara_type_name) { kr_dbuffer_t data; }          \
        _KR_enc(kiara_type_name);                                           \
    typedef KIARA_ENCRYPTED(kiara_type_name) kiara_type_name;               \
    typedef kiara_type_name _KR_tdef(kiara_type_name);                      \
                                                                            \
    static kiara_type_name KIARA_JOIN(new_, kiara_type_name)(void)          \
    {                                                                       \
        kiara_type_name enc = (kiara_type_name)malloc(                      \
            sizeof(_KR_enc(kiara_type_name)));                              \
        kr_dbuffer_init(&enc->data);                                        \
        return enc;                                                         \
    }                                                                       \
                                                                            \
    static void KIARA_JOIN(delete_, kiara_type_name)(kiara_type_name enc)   \
    {                                                                       \
        if (enc)                                                            \
            kr_dbuffer_destroy(&enc->data);                                 \
        free(enc);                                                          \
    }                                                                       \
                                                                            \
    static void KIARA_JOIN(assign_, kiara_type_name)(kiara_type_name dest,  \
                                                     kiara_type_name src)   \
    {                                                                       \
        kr_dbuffer_assign(&dest->data, &src->data);                         \
    }                                                                       \
                                                                            \
    static KIARA_DeclEncrypted _KR_enctd(kiara_type_name) = {               \
        _KR_get_type(kiara_element_type_name)                               \
    };                                                                      \
    static KIARA_DeclType _KR_type(kiara_type_name) = {                     \
        BOOST_PP_STRINGIZE(kiara_type_name),                                \
        KIARA_TYPE_ENCRYPTED,                                               \
        0,                                                                  \
        {&_KR_enctd(kiara_type_name)}};                                     \
    static KIARA_DeclType * _KR_get_type(kiara_type_name)(void)             \
    {                                                                       \
        return (&_KR_type(kiara_type_name));                                \
    }

/* Annotated type */

#define KIARA_DECL_ANNOTATED(kiara_anno_type_name, kiara_type_name, annotation, semantics)  \
    KIARA_BEGIN_EXTERN_C                                                                    \
    typedef kiara_type_name _KR_tdef(kiara_anno_type_name);                                 \
    static KIARA_DeclAnnotated _KR_anno(kiara_anno_type_name) = {                           \
        _KIARA_DTYPE_GETTER(kiara_type_name),                                               \
        BOOST_PP_STRINGIZE(annotation),                                                     \
        BOOST_PP_STRINGIZE(semantics),                                                      \
    };                                                                                      \
    static KIARA_DeclType _KR_type(kiara_anno_type_name) = {                                \
        BOOST_PP_STRINGIZE(kiara_anno_type_name),                                           \
        KIARA_TYPE_ANNOTATED,                                                               \
        KIARA_DTF_NONE,                                                                     \
        {&_KR_anno(kiara_anno_type_name)}};                                                 \
    _KIARA_GEN_DTYPE_GETTER(kiara_anno_type_name)                                           \
    KIARA_END_EXTERN_C

/* Builtin types */

/* We can't use types with spaces in their names so we define aliases */
typedef char * KIARA_char_ptr;      /* This type is interpreted as pointer to null terminated C-string */
typedef char * KIARA_raw_char_ptr;  /* This type is interpreted as a pointer to char array */
typedef void * KIARA_void_ptr;
typedef const char * KIARA_const_char_ptr;      /* This type is interpreted as pointer to null terminated C-string */
typedef const char * KIARA_const_raw_char_ptr;  /* This type is interpreted as a pointer to char array */
typedef const void * KIARA_const_void_ptr;

typedef signed char KIARA_schar;
typedef unsigned char KIARA_uchar;
typedef unsigned short KIARA_ushort;
typedef unsigned int KIARA_uint;
typedef unsigned long KIARA_ulong;
typedef long long KIARA_longlong;
typedef unsigned long long KIARA_ulonglong;
typedef long double KIARA_longdouble;

#define KIARA_CHAR char
#define KIARA_WCHAR_T wchar_t
#define KIARA_SCHAR KIARA_schar
#define KIARA_UCHAR KIARA_uchar
#define KIARA_SHORT short
#define KIARA_USHORT KIARA_ushort
#define KIARA_INT int
#define KIARA_UINT KIARA_uint
#define KIARA_LONG long
#define KIARA_ULONG KIARA_ulong
#define KIARA_LONGLONG KIARA_longlong
#define KIARA_ULONGLONG KIARA_ulonglong
#define KIARA_SIZE_T size_t
#define KIARA_SSIZE_T ssize_t
#define KIARA_VOID void
#define KIARA_FLOAT float
#define KIARA_DOUBLE double
#define KIARA_LONGDOUBLE KIARA_longdouble

#define KIARA_RAW_CHAR_PTR KIARA_raw_char_ptr
#define KIARA_CHAR_PTR KIARA_char_ptr
#define KIARA_VOID_PTR KIARA_void_ptr
#define KIARA_INT8_T int8_t
#define KIARA_UINT8_T uint8_t
#define KIARA_INT16_T int16_t
#define KIARA_UINT16_T uint16_t
#define KIARA_INT32_T int32_t
#define KIARA_UINT32_T uint32_t
#define KIARA_INT64_T int64_t
#define KIARA_UINT64_T uint64_t

#if 0
#define KIARA_DECL_BUILTIN(kiara_type_name) \
    extern KIARA_API KIARA_DeclType KIARA_JOIN(kr_type_, kiara_type_name);

#define KIARA_DEF_BUILTIN2(kiara_type_name, c_type)             \
    KIARA_BEGIN_EXTERN_C                                    \
    KIARA_DeclType _KR_type(kiara_type_name) =    \
    {BOOST_PP_STRINGIZE(c_type), KIARA_TYPE_BUILTIN, {NULL}};  \
    KIARA_END_EXTERN_C

#define KIARA_DEF_BUILTIN(c_type) \
    KIARA_DEF_BUILTIN2(c_type, c_type)
#else
#define KIARA_DECL_BUILTIN(kiara_type_name)

#define KIARA_DEF_BUILTIN_EX(kiara_type_name, c_type, flags)                \
    KIARA_BEGIN_EXTERN_C                                                    \
    typedef c_type _KR_tdef(kiara_type_name);                               \
    static KIARA_DeclType _KR_type(kiara_type_name)                         \
        KIARA_UNUSED = {                                                    \
            BOOST_PP_STRINGIZE(c_type),                                     \
            KIARA_TYPE_BUILTIN,                                             \
            flags,                                                          \
            {NULL}};                                                        \
    _KIARA_GEN_DTYPE_UNUSED_GETTER(kiara_type_name)                         \
    KIARA_END_EXTERN_C

#define KIARA_DEF_BUILTIN2(kiara_type_name, c_type)                         \
    KIARA_DEF_BUILTIN_EX(kiara_type_name, c_type, KIARA_DTF_NONE)

#define KIARA_DEF_BUILTIN(c_type)                                           \
    KIARA_DEF_BUILTIN2(c_type, c_type)
#endif

#define KIARA_DECL_DERIVED_BUILTIN(kiara_type_name, kiara_builtin_type_name)                \
    KIARA_BEGIN_EXTERN_C                                                                    \
    typedef _KR_tdef(kiara_builtin_type_name) _KR_tdef(kiara_type_name);                    \
    static KIARA_DeclType * _KR_get_type(kiara_type_name)(void) KIARA_UNUSED;               \
    static KIARA_DeclType * _KR_get_type(kiara_type_name)(void)                             \
    {                                                                                       \
        return _KR_get_type(kiara_builtin_type_name)();                                     \
    }                                                                                       \
    KIARA_END_EXTERN_C

KIARA_DECL_BUILTIN(KIARA_CHAR)
KIARA_DECL_BUILTIN(KIARA_WCHAR)
KIARA_DECL_BUILTIN(KIARA_SCHAR)
KIARA_DECL_BUILTIN(KIARA_UCHAR)
KIARA_DECL_BUILTIN(KIARA_SHORT)
KIARA_DECL_BUILTIN(KIARA_USHORT)
KIARA_DECL_BUILTIN(KIARA_INT)
KIARA_DECL_BUILTIN(KIARA_UINT)
KIARA_DECL_BUILTIN(KIARA_LONG)
KIARA_DECL_BUILTIN(KIARA_ULONG)
KIARA_DECL_BUILTIN(KIARA_LONGLONG)
KIARA_DECL_BUILTIN(KIARA_ULONGLONG)
KIARA_DECL_BUILTIN(KIARA_SIZE_T)
KIARA_DECL_BUILTIN(KIARA_SSIZE_T)
KIARA_DECL_BUILTIN(KIARA_VOID)
KIARA_DECL_BUILTIN(KIARA_FLOAT)
KIARA_DECL_BUILTIN(KIARA_DOUBLE)
KIARA_DECL_BUILTIN(KIARA_LONGDOUBLE)

KIARA_DECL_BUILTIN(KIARA_CHAR_PTR)
KIARA_DECL_BUILTIN(KIARA_VOID_PTR)
KIARA_DECL_BUILTIN(KIARA_RAW_CHAR_PTR)
KIARA_DECL_BUILTIN(KIARA_INT8_T)
KIARA_DECL_BUILTIN(KIARA_UINT8_T)
KIARA_DECL_BUILTIN(KIARA_INT16_T)
KIARA_DECL_BUILTIN(KIARA_UINT16_T)
KIARA_DECL_BUILTIN(KIARA_INT32_T)
KIARA_DECL_BUILTIN(KIARA_UINT32_T)
KIARA_DECL_BUILTIN(KIARA_INT64_T)
KIARA_DECL_BUILTIN(KIARA_UINT64_T)

KIARA_DEF_BUILTIN(char)
KIARA_DEF_BUILTIN(wchar_t)
KIARA_DEF_BUILTIN2(KIARA_schar, signed char)
KIARA_DEF_BUILTIN2(KIARA_uchar, unsigned char)
KIARA_DEF_BUILTIN(short)
KIARA_DEF_BUILTIN2(KIARA_ushort, unsigned short)
KIARA_DEF_BUILTIN(int)
KIARA_DEF_BUILTIN2(KIARA_uint, unsigned int)
KIARA_DEF_BUILTIN(long)
KIARA_DEF_BUILTIN2(KIARA_ulong, unsigned long)
KIARA_DEF_BUILTIN2(KIARA_longlong, long long)
KIARA_DEF_BUILTIN2(KIARA_ulonglong, unsigned long long)
KIARA_DEF_BUILTIN(size_t)
KIARA_DEF_BUILTIN(ssize_t)
KIARA_DEF_BUILTIN(void)
KIARA_DEF_BUILTIN(float)
KIARA_DEF_BUILTIN(double)
KIARA_DEF_BUILTIN2(KIARA_longdouble, KIARA_longdouble)
KIARA_DEF_BUILTIN2(KIARA_char_ptr, KIARA_char_ptr)
KIARA_DEF_BUILTIN2(KIARA_raw_char_ptr, KIARA_raw_char_ptr)
KIARA_DEF_BUILTIN2(KIARA_void_ptr, KIARA_void_ptr)
KIARA_DEF_BUILTIN_EX(KIARA_const_void_ptr, KIARA_const_void_ptr, KIARA_DTF_CONST_TYPE)
KIARA_DEF_BUILTIN_EX(KIARA_const_char_ptr, KIARA_const_char_ptr, KIARA_DTF_CONST_TYPE)
KIARA_DEF_BUILTIN_EX(KIARA_const_raw_char_ptr, KIARA_const_raw_char_ptr, KIARA_DTF_CONST_TYPE)

KIARA_DEF_BUILTIN(int8_t)
KIARA_DEF_BUILTIN(uint8_t)
KIARA_DEF_BUILTIN(int16_t)
KIARA_DEF_BUILTIN(uint16_t)
KIARA_DEF_BUILTIN(int32_t)
KIARA_DEF_BUILTIN(uint32_t)
KIARA_DEF_BUILTIN(int64_t)
KIARA_DEF_BUILTIN(uint64_t)

#endif /* KIARA_MACROS_H_INCLUDED */
