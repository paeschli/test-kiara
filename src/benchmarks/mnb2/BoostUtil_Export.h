// Copyright (c) 2009, Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#ifdef _MSC_VER
# pragma once
#endif
#ifndef BOOST_UTIL_EXPORT_H
#define BOOST_UTIL_EXPORT_H

// Compile time controls for library generation.  Define with /D or #define
// To produce or use a static library: #define BOOST_UTIL_HAS_DLL=0
//   Default is to produce/use a DLL
// While building the BOOST_UTIL_ library: #define BOOST_UTIL_BUILD_DLL
//   Default is to export symbols from a pre-built BOOST_UTIL DLL
//
// Within BOOST_UTIL use the BOOST_UTIL_Export macro where a __declspec is needed.

#if defined (_WIN32)

#pragma warning( disable: 4251 )

#  if !defined (BOOST_UTIL_HAS_DLL)
#    define BOOST_UTIL_HAS_DLL 1
#  endif /* ! BOOST_UTIL_HAS_DLL */

#  if defined (BOOST_UTIL_HAS_DLL) && (BOOST_UTIL_HAS_DLL == 1)
#    if defined (BOOST_UTIL_BUILD_DLL)
#      define BoostUtil_Export __declspec(dllexport)
#    else /* BOOST_UTIL_BUILD_DLL */
#      define BoostUtil_Export __declspec(dllimport)
#    endif /* BOOST_UTIL_BUILD_DLL */
#  else /* BOOST_UTIL_HAS_DLL == 1 */
#    define BoostUtil_Export
#  endif /* BOOST_UTIL_HAS_DLL == 1 */

#  else /* !_WIN32 */

#    define BoostUtil_Export
#  endif /* _WIN32 */
#endif /* BOOST_UTIL_EXPORT_H */
