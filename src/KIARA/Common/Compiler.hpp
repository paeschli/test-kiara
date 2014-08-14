/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2012  German Research Center for Artificial Intelligence (DFKI)
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
 * Compiler.hpp
 *
 *  Created on: 06.08.2012
 *      Author: Dmitri Rubinstein
 */
/*
 *  Includes code from LLVM distributed under the University of Illinois Open Source
 *  License.
 *
 *  Includes code from Boost.
 *
 *  Includes code from libffi.
 */

#ifndef KIARA_COMMON_COMPILER_HPP_INCLUDED
#define KIARA_COMMON_COMPILER_HPP_INCLUDED

#ifdef __GNUC__
#define KIARA_ATTRIBUTE_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#define KIARA_ATTRIBUTE_NORETURN __declspec(noreturn)
#else
#define KIARA_ATTRIBUTE_NORETURN
#endif

// KIARA_BUILTIN_UNREACHABLE - On compilers which support it, expands
// to an expression which states that it is undefined behavior for the
// compiler to reach this point.  Otherwise is not defined.
#if defined(__clang__) || (__GNUC__ > 4) \
 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5)
# define KIARA_BUILTIN_UNREACHABLE __builtin_unreachable()
#endif

/* LONG_LONG_MAX is not always defined (not if STRICT_ANSI, for example).
   But we can find it either under the correct ANSI name, or under GNU
   C's internal name.  */

#define KIARA_64_BIT_MAX 9223372036854775807

#ifdef LONG_LONG_MAX
# define KIARA_LONG_LONG_MAX LONG_LONG_MAX
#else
# ifdef LLONG_MAX
#  define KIARA_LONG_LONG_MAX LLONG_MAX
#  ifdef _AIX52 /* or newer has C99 LLONG_MAX */
#   undef KIARA_64_BIT_MAX
#   define KIARA_64_BIT_MAX 9223372036854775807LL
#  endif /* _AIX52 or newer */
# else
#  ifdef __GNUC__
#   define KIARA_LONG_LONG_MAX __LONG_LONG_MAX__
#  endif
#  ifdef _AIX /* AIX 5.1 and earlier have LONGLONG_MAX */
#   ifndef __PPC64__
#    if defined (__IBMC__) || defined (__IBMCPP__)
#     define KIARA_LONG_LONG_MAX LONGLONG_MAX
#    endif
#   endif /* __PPC64__ */
#   undef  KIARA_64_BIT_MAX
#   define KIARA_64_BIT_MAX 9223372036854775807LL
#  endif
# endif
#endif

#endif /* KIARA_COMMON_COMPILER_HPP_INCLUDED */
