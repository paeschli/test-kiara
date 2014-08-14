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
#ifndef KIARA_PP_ANNOTATION_H_INCLUDED
#define KIARA_PP_ANNOTATION_H_INCLUDED

#ifdef __kiara_parse__

#if 0
#include <stddef.h>
#include <stdint.h>

typedef ssize_t _kiara_ssize_t __attribute__((ext_vector_type(1)));
#define ssize_t _kiara_ssize_t

typedef size_t _kiara_size_t __attribute__((ext_vector_type(1)));
#define size_t _kiara_size_t

typedef int8_t _kiara_int8_t __attribute__((ext_vector_type(1)));
#define int8_t _kiara_int8_t

typedef uint8_t _kiara_uint8_t __attribute__((ext_vector_type(1)));
#define uint8_t _kiara_uint8_t

typedef int16_t _kiara_int16_t __attribute__((ext_vector_type(1)));
#define int16_t _kiara_int16_t

typedef uint16_t _kiara_uint16_t __attribute__((ext_vector_type(1)));
#define uint16_t _kiara_uint16_t

typedef int32_t _kiara_int32_t __attribute__((ext_vector_type(1)));
#define int32_t _kiara_int32_t

typedef uint32_t _kiara_uint32_t __attribute__((ext_vector_type(1)));
#define uint32_t _kiara_uint32_t

typedef int64_t _kiara_int64_t __attribute__((ext_vector_type(1)));
#define int64_t _kiara_int64_t

typedef uint64_t _kiara_uint64_t __attribute__((ext_vector_type(1)));
#define uint64_t _kiara_uint64_t
#endif

#endif

#include <KIARA/kiara.h>

#define KIARA_UNIQUE(x) KIARA_JOIN(x, __COUNTER__)

// This is a customized macro added into kiara-preprocessor, we use this to distinguish a KIARA preprocessor scanning
// from a normal compiling using clang as compiler.
// This can help cut normal compiling time since starting from XCode 4.3, clang has become the official
// compiler on Mac OS X
#ifdef __kiara_parse__


    //
    // Injects a unique structure within the kiara_internal namespace that only the Clang frontend
    // can see so that it can register the specified symbol for reflection.
    // Can only be called from the global namespace and results in the primitive and any children
    // being fully reflected.
    //
    #define kiara_reflect(name)                     \
                                                    \
        namespace kiara_internal                    \
        {                                           \
            struct                                  \
            __attribute__((annotate("full-"#name))) \
            KIARA_UNIQUE(cldb_reflect) { };         \
        }


    //
    // Similar to kiara_reflect with the only difference being that the primitive being specified
    // is being partially reflected. Anything that is a child of that primitive has to be
    // explicitly reflected as a result.
    //
    #define kiara_reflect_part(name)                \
                                                    \
        namespace kiara_internal                    \
        {                                           \
            struct                                  \
            __attribute__((annotate("part-"#name))) \
            KIARA_UNIQUE(cldb_reflect) { }; \
        }


    //
    // A container must have iterators if you want to use reflection to inspect it. Call this from
    // the global namespace in the neighbourhood of any iterator implementations and it will
    // partially reflect the iterators and allow the parent container to be used with reflection.
    //
    #define kiara_container_iterators(container, read_iterator, write_iterator, keyinfo)                            \
        kiara_reflect_part(read_iterator)                                                                           \
        kiara_reflect_part(write_iterator)                                                                          \
        namespace kiara_internal                                                                                    \
        {                                                                                                           \
            struct                                                                                                  \
            __attribute__((annotate("container-" #container "-" #read_iterator "-" #write_iterator "-" #keyinfo)))  \
            KIARA_UNIQUE(container_info) { };                                                               \
        }


    #define kiara_attr(...) __attribute__((annotate("attr:" #__VA_ARGS__)))
    #define kiara_push_attr(...) struct KIARA_UNIQUE(push_attr) { } __attribute__((annotate(#__VA_ARGS__)));
    #define kiara_pop_attr(...) struct KIARA_UNIQUE(pop_attr) { } __attribute__((annotate(#__VA_ARGS__)));


    //
    // Clang does not need to see these
    //
    #define kiara_impl_class(scoped_type)

#ifdef __cplusplus
extern "C"
{
#endif

void * kiara_reflect_get_type_ptr(unsigned long);
void   kiara_reflect_object(unsigned long);
void   kiara_reflect_object_ext(unsigned long, ...);

#ifdef __cplusplus
}
#endif


#define kiara_get_reflected_type(T, R)  \
    (R*)kiara_reflect_get_type_ptr(sizeof(T))

#define kiara_reflect_object(T)                \
    kiara_reflect_object(sizeof(T))

#define kiara_reflect_object_ext(T, ...)                \
    kiara_reflect_object_ext(sizeof(T), __VA_ARGS__)

#define kiara_reflect_type(T)                        \
    struct                                           \
    __attribute__((type_tag_for_datatype(kiara, T))) \
    KIARA_UNIQUE(kiara_reflect) { };

#else


    //
    // The main compiler does not need to see these
    //
    #define kiara_reflect(name)
    #define kiara_reflect_part(name)
    #define kiara_container_iterators(container, read_iterator, write_iterator, keyinfo)
    #define kiara_attr(...)
    #define kiara_push_attr(...)
    #define kiara_pop_attr(...)


    //
    // Introduces overloaded construction and destruction functions into the KIARA::internal
    // namespace for the type you specify. These functions end up in the list of methods
    // in the specified type for easy access.
    // This can only be used from global namespace.
    //
    #define kiara_impl_class(type)                              \
                                                                \
        KIARA_API void kiaraConstructObject(type* object)       \
        {                                                       \
            KIARA::internal::CallConstructor(object);           \
        }                                                       \
        KIARA_API void kiaraDestructObject(type* object)        \
        {                                                       \
            KIARA::internal::CallDestructor(object);            \
        }                                                       \


#define kiara_reflect_object(T)
#define kiara_reflect_type(T)

#endif

#endif

typedef struct KiaraType KiaraType;

#ifdef __kiara_parse__
#define kiara_type(T) kiara_get_reflected_type(T, KiaraType)
#else
#define kiara_type(T) (KiaraType*)NULL
#endif

#ifdef __kiara_parse__
#define kiara_declare_type(T) kiara_reflect_type(T)
#else
#define kiara_declare_type(T)
#endif

#ifdef __kiara_parse__

#define result_value __attribute__((annotate("kiara-result")))

#define _kiara_decls(decls) static void KIARA_UNIQUE(kiara_reg)(void) { decls }
#define kiara_declare_object(T) _kiara_decls( kiara_reflect_object(T); )
#define kiara_declare_opaque_object(T, ...) _kiara_decls( kiara_reflect_object_ext(T, "kiara-opaque-object", "kiara-eop", __VA_ARGS__); )
#define kiara_declare_struct(T, ...) _kiara_decls( kiara_reflect_object_ext(T, __VA_ARGS__); )
#define kiara_declare_struct_with_api(T, ...) _kiara_decls( kiara_reflect_object_ext(T, __VA_ARGS__); )
#define kiara_user_api(api_name, func) "kiara-user-api", #api_name, sizeof(func), "kiara-eop"
#define kiara_struct_array_member(ptr_member_name, size_member_name) \
    "kiara-struct-array-member", #ptr_member_name, #size_member_name, "kiara-eop"
#define kiara_declare_func(name, ...)                                                                           \
    KIARA_DECL_FUNC_OBJ(name);                                                                                  \
    KIARA_BEGIN_EXTERN_C                                                                                        \
    static KIARA_Result KIARA_JOIN(_kpp_,name)(KIARA_FUNC_OBJ(name) kiara_func_object, __VA_ARGS__) { return KIARA_SUCCESS; }  \
    KIARA_END_EXTERN_C                                                                                          \
    kiara_declare_object(KIARA_JOIN(_kpp_,name))
#define kiara_declare_service(name, ...)                                                                        \
    KIARA_BEGIN_EXTERN_C                                                                                        \
    static KIARA_Result KIARA_JOIN(_kpp_,name)(KIARA_ServiceFuncObj *kiara_func_object, __VA_ARGS__) { return KIARA_SUCCESS; }    \
    KIARA_END_EXTERN_C                                                                                          \
    kiara_declare_object(KIARA_JOIN(_kpp_,name))
#else
#define _kiara_decls(decls)
#define kiara_declare_object(T)
#define kiara_declare_opaque_object(T, ...)
#define kiara_declare_struct(T, ...)
#define kiara_declare_struct_with_api(T, ...)
#define kiara_user_api(api_name, func)
#define kiara_struct_array_member(ptr_type_name, ptr_member_name, size_type_name, size_member_name)
#define kiara_declare_func(name, ...)
#define kiara_declare_service(name, ...)
#endif
