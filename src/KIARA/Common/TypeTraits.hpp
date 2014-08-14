/*
 * TypeTraits.hpp
 *
 *  Created on: 07.08.2012
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMMON_TYPETRAITS_HPP_INCLUDED
#define KIARA_COMMON_TYPETRAITS_HPP_INCLUDED

/// Code based on libffi
/// Original copyright:
/* --------------------------------------------------------------------
   libffi @VERSION@ - Copyright (c) 2011 Anthony Green
                    - Copyright (c) 1996-2003, 2007, 2008 Red Hat, Inc.

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the ``Software''), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.

   ----------------------------------------------------------------------- */

#include <KIARA/Common/Types.hpp>
#include <KIARA/Common/Compiler.hpp>
#include <climits>
#include <boost/static_assert.hpp>

namespace KIARA
{

template <class T>
struct normalize_type
{
    typedef T type;
};

#define _KIARA_NORMALIZE_TYPE(Type, NewType)                  \
    template <>                                               \
    struct normalize_type<Type>                               \
    {                                                         \
        BOOST_STATIC_ASSERT(sizeof(Type) == sizeof(NewType)); \
        typedef NewType type;                                 \
    };

#if SCHAR_MAX == 127
_KIARA_NORMALIZE_TYPE(unsigned char, uint8_t)
_KIARA_NORMALIZE_TYPE(signed char, int8_t)
#else
#error "char size not supported"
#endif

#if CHAR_MIN == 0
_KIARA_NORMALIZE_TYPE(char, uint8_t)
#else
_KIARA_NORMALIZE_TYPE(char, int8_t)
#endif

#if SHRT_MAX == 32767
_KIARA_NORMALIZE_TYPE(unsigned short, uint16_t)
_KIARA_NORMALIZE_TYPE(short, int16_t)
#elif SHRT_MAX == 2147483647
_KIARA_NORMALIZE_TYPE(unsigned short, uint32_t)
_KIARA_NORMALIZE_TYPE(short, int32_t)
#else
#error "short size not supported"
#endif

#if INT_MAX == 32767
_KIARA_NORMALIZE_TYPE(unsigned int, uint16_t)
_KIARA_NORMALIZE_TYPE(int, int16_t)
#elif INT_MAX == 2147483647
_KIARA_NORMALIZE_TYPE(unsigned int, uint32_t)
_KIARA_NORMALIZE_TYPE(int, int32_t)
#elif INT_MAX == 9223372036854775807
_KIARA_NORMALIZE_TYPE(unsigned int, uint64_t)
_KIARA_NORMALIZE_TYPE(int, int64_t)
#else
#error "int size not supported"
#endif

#if LONG_MAX == 2147483647
_KIARA_NORMALIZE_TYPE(unsigned long, uint32_t)
_KIARA_NORMALIZE_TYPE(long, int32_t)
#elif LONG_MAX == KIARA_64_BIT_MAX
_KIARA_NORMALIZE_TYPE(unsigned long, uint64_t)
_KIARA_NORMALIZE_TYPE(long, int64_t)
#else
#error "long size not supported"
#endif

#if defined(BOOST_HAS_LONG_LONG)
#if KIARA_LONG_LONG_MAX == KIARA_64_BIT_MAX
_KIARA_NORMALIZE_TYPE(unsigned long long, uint64_t)
_KIARA_NORMALIZE_TYPE(long long, int64_t)
#elif defined(BOOST_HAS_MS_INT64)
_KIARA_NORMALIZE_TYPE(unsigned __int64, uint64_t)
_KIARA_NORMALIZE_TYPE(__int64, int64_t)
#else
#error "long long size not supported"
#endif
#endif

#undef _KIARA_NORMALIZE_TYPE

template <class T>
inline typename normalize_type<T>::type normalize_value(T value)
{
    return static_cast<typename normalize_type<T>::type>(value);
}

} // namespace KIARA

#endif /* KIARA_COMMON_TYPETRAITS_HPP_INCLUDED */
