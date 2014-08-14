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
 * EnableLLVMDebug.hpp
 *
 *  Created on: Mar 3, 2010
 *      Author: Dmitri Rubinstein
 */

// Include this file only in *.cpp files !!!

#ifndef KIARA_LLVM_ENABLELLVMDEBUG_HPP_INCLUDED
#define KIARA_LLVM_ENABLELLVMDEBUG_HPP_INCLUDED

#ifdef LLVM_HAS_DEBUGFLAG
#   ifdef NDEBUG
#       define _LLVM_DO_DEFINE_NDEBUG
#       undef NDEBUG
#   endif
#endif

#include "llvm/Support/Debug.h"

#ifdef _LLVM_DO_DEFINE_NDEBUG
#   define NDEBUG
#   undef _LLVM_DO_DEFINE_NDEBUG
#endif

#endif /* ANYSL_LLVM_ENABLELLVMDEBUG_HPP_INCLUDED */
