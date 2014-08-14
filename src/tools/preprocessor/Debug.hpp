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
/*
 * Debug.hpp
 *
 *  Created on: Nov 25, 2013
 *      Author: rubinste
 */

#ifndef KIARA_PREPROCESSOR_DEBUG_HPP_INCLUDED
#define KIARA_PREPROCESSOR_DEBUG_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <llvm/Support/raw_ostream.h>

#endif /* KIARA_PREPROCESSOR_DEBUG_HPP_INCLUDED */

#ifdef KIARA_DEBUG
#undef KIARA_DEBUG_MODE
#undef KIARA_DEBUG
#undef KIARA_PING
#undef KIARA_IFDEBUG
#endif

//#define KIARA_DO_DEBUG
#if defined(KIARA_DO_DEBUG) && !defined(NDEBUG)
#define KIARA_DEBUG_MODE
#define KIARA_PING() KIARA_DEBUG("CALLED: " << KIARA_FUNCID())
#define KIARA_IFDEBUG(code) code
#define KIARA_DEBUG(msg) llvm::errs() << msg << "\n"
#undef KIARA_DO_DEBUG
#else
#define KIARA_PING()
#define KIARA_IFDEBUG(code)
#define KIARA_DEBUG(msg)
#endif
