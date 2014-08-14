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
 * Config.h
 *
 *  Created on: 27.07.2011
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMMON_CONFIG_H_INCLUDED
#define KIARA_COMMON_CONFIG_H_INCLUDED

// Determine Operating System
#ifdef _WIN32
#   define KIARA_WINDOWS
#elif defined(__CYGWIN__) ||                                                   \
      defined(__linux__) ||                                                    \
      defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) ||   \
      defined(__hpux) ||                                                       \
      defined(__sgi) ||                                                        \
      defined(__sun) ||                                                        \
      defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE)
#   define KIARA_POSIX
#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
#   define KIARA_OS_MACOSX
#else
#   pragma message("Warning: Cannot determine OS. Using default: POSIX")
#   define KIARA_POSIX
#endif

#if defined(KIARA_WINDOWS) && !defined(KIARA_STATIC_LIB) && !defined(KIARA_LLVM_BITCODE)
#  if defined(KIARA_LIB)
#    define KIARA_API __declspec(dllexport)
#  else
#    define KIARA_API __declspec(dllimport)
#  endif
#else
#  define KIARA_API
#endif

// Macros from Boost C++ Libraries

//
// Helper macro KIARA_STRINGIZE:
// Converts the parameter X to a string after macro replacement
// on X has been performed.
//
#define KIARA_STRINGIZE(X) KIARA_DO_STRINGIZE(X)
#define KIARA_DO_STRINGIZE(X) #X

//
// Helper macro KIARA_JOIN:
// The following piece of macro magic joins the two
// arguments together, even when one of the arguments is
// itself a macro (see 16.3.1 in C++ standard).  The key
// is that macro expansion of macro arguments does not
// occur in KIARA_DO_JOIN2 but does in KIARA_DO_JOIN.
//
#define KIARA_JOIN( X, Y ) KIARA_DO_JOIN( X, Y )
#define KIARA_DO_JOIN( X, Y ) KIARA_DO_JOIN2(X,Y)
#define KIARA_DO_JOIN2( X, Y ) X##Y

#if defined(__GNUC__) || defined(__clang__)
#define KIARA_FUNCSIG()  __PRETTY_FUNCTION__
#define KIARA_FUNCNAME() __FUNCTION__
#define KIARA_FUNCID()   KIARA_FUNCSIG()
#elif defined(_MSC_VER)
#define KIARA_FUNCSIG()  __FUNCSIG__
#define KIARA_FUNCNAME() __FUNCTION__
#define KIARA_FUNCID()   KIARA_FUNCSIG()
#else
#define KIARA_FUNCSIG()  "unknown"
#define KIARA_FUNCNAME() "unknown"
#define KIARA_FUNCID() __FILE__ " : "  KIARA_STRINGIZE(__LINE__)
#endif

#if (__GNUC__ > 2) || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#  define KIARA_UNUSED __attribute__((__unused__))
#else
#  define KIARA_UNUSED
#endif

#ifdef _MSC_VER
#define KIARA_INLINE __inline
#else
#define KIARA_INLINE inline
#endif

/* KIARA_ALWAYS_INLINE force inlining */
#ifndef KIARA_ALWAYS_INLINE
#  if defined(__clang__)
#    if __has_attribute(always_inline)
#      define KIARA_ALWAYS_INLINE __attribute__((always_inline))
#    else
#      define KIARA_ALWAYS_INLINE
#    endif
#  elif defined(__GNUC__) && (__GNUC__ > 3 ||(__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
#    define KIARA_ALWAYS_INLINE __attribute__((always_inline))
#  else
#    define KIARA_ALWAYS_INLINE
#  endif
#endif

#ifdef KIARA_LLVM_BITCODE
#define KIARA_LLVM_BITCODE_INLINE KIARA_ALWAYS_INLINE
#else
#define KIARA_LLVM_BITCODE_INLINE
#endif

/* KIARA_NOINLINE disables inlining */
#ifndef KIARA_NOINLINE
#  if defined(__clang__)
#    if __has_attribute(noinline)
#      define KIARA_NOINLINE __attribute__((noinline))
#    else
#      define KIARA_NOINLINE
#    endif
#  elif defined(__GNUC__) && (__GNUC__ > 3 ||(__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
#    define KIARA_NOINLINE __attribute__((noinline))
#  else
#    define KIARA_NOINLINE
#  endif
#endif

#ifndef KIARA_LIKELY
#  if defined(__GNUC__)
#    define KIARA_LIKELY(x) (__builtin_expect(!!(x), 1))
#  else
#    define KIARA_LIKELY
#  endif
#endif

#endif /* KIARA_COMMON_CONFIG_H_INCLUDED */
