// Copyright (c) 2010, Object Computing, Inc.
// All rights reserved.

#ifdef _MSC_VER
# pragma once
#endif
#ifndef COMMON_TYPES_EXPORT_H
#define COMMON_TYPES_EXPORT_H

// Compile time controls for library generation.  Define with /D or #define
// To produce or use a static library: #define COMMON_TYPES_HAS_DLL=0
//   Default is to produce/use a DLL
// While building the COMMON_TYPES_ library: #define COMMON_TYPES_BUILD_DLL
//   Default is to export symbols from a pre-built COMMON_TYPES DLL
//
// Within COMMON_TYPES use the COMMON_TYPES_Export macro where a __declspec is needed.

#if defined (_WIN32)

#pragma warning( disable: 4251 )

#  if !defined (COMMON_TYPES_HAS_DLL)
#    define COMMON_TYPES_HAS_DLL 1
#  endif /* ! COMMON_TYPES_HAS_DLL */

#  if defined (COMMON_TYPES_HAS_DLL) && (COMMON_TYPES_HAS_DLL == 1)
#    if defined (COMMON_TYPES_BUILD_DLL)
#      define CommonTypes_Export __declspec(dllexport)
#    else /* COMMON_TYPES_BUILD_DLL */
#      define CommonTypes_Export __declspec(dllimport)
#    endif /* COMMON_TYPES_BUILD_DLL */
#  else /* COMMON_TYPES_HAS_DLL == 1 */
#    define CommonTypes_Export
#  endif /* COMMON_TYPES_HAS_DLL == 1 */

#  else /* !_WIN32 */

#    define CommonTypes_Export
#  endif /* _WIN32 */
#endif /* COMMON_TYPES_EXPORT_H */
