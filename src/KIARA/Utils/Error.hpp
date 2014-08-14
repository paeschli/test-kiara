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
 * Error.hpp
 *
 *  Created on: 06.08.2012
 *      Author: Dmitri Rubinstein
 */
/*
 *  Includes Code from LLVM distributed under the University of Illinois Open Source
 *  License.
 */

#ifndef KIARA_UTILS_ERROR_HPP_INCLUDED
#define KIARA_UTILS_ERROR_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <KIARA/Common/Compiler.hpp>

namespace KIARA
{

/// This function calls abort(), and prints the optional message to stderr.
/// Use the llvm_unreachable macro (that adds location info), instead of
/// calling this function directly.
KIARA_API KIARA_ATTRIBUTE_NORETURN void kiara_unreachable_internal(const char *msg = 0,
                                                         const char *file = 0,
                                                         unsigned line = 0);

/// Marks that the current location is not supposed to be reachable.
/// In !NDEBUG builds, prints the message and location info to stderr.
/// In NDEBUG builds, becomes an optimizer hint that the current location
/// is not supposed to be reachable.  On compilers that don't support
/// such hints, prints a reduced message instead.
///
/// Use this instead of assert(0).  It conveys intent more clearly and
/// allows compilers to omit some unnecessary code.
#ifndef NDEBUG
#define KIARA_UNREACHABLE(msg) \
  ::KIARA::kiara_unreachable_internal(msg, __FILE__, __LINE__)
#elif defined(KIARA_BUILTIN_UNREACHABLE)
#define KIARA_UNREACHABLE(msg) KIARA_BUILTIN_UNREACHABLE
#else
#define KIARA_UNREACHABLE(msg) ::KIARA::kiara_unreachable_internal()
#endif

} // namespace KIARA

#endif /* KIARA_UTILS_ERROR_HPP_INCLUDED */
