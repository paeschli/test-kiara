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
 * OptimizationConfig.hpp
 *
 *  Created on: Dec 26, 2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMPILER_LLVM_OPTIMIZATIONCONFIG_HPP_INCLUDED
#define KIARA_COMPILER_LLVM_OPTIMIZATIONCONFIG_HPP_INCLUDED

// uncomment if no optimization should be performed
// #define DISABLE_OPTIMIZATION

#ifdef DISABLE_OPTIMIZATION
#define JIT_OPTIMIZATION_LEVEL llvm::CodeGenOpt::Less
#else
//#define JIT_OPTIMIZATION_LEVEL llvm::CodeGenOpt::Default
#define JIT_OPTIMIZATION_LEVEL llvm::CodeGenOpt::Aggressive
// Note: llvm::CodeGenOpt::None produces segfault
#endif

// if CUSTOM_FUNCTION_OPTIMIZATION is defined, a custom sequence of the
// optimization passes will be used for function optimization
#define CUSTOM_FUNCTION_OPTIMIZATION

#endif /* KIARA_COMPILER_OPTIMIZATIONCONFIG_HPP_INCLUDED */
