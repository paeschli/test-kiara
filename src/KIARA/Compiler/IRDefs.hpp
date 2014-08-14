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
 * IRDefs.hpp
 *
 *  Created on: 29.01.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef HANDLE_BASETYPE
#define HANDLE_BASETYPE(type) HANDLE_TYPE(type)
#endif

#ifndef HANDLE_SUBTYPE
#define HANDLE_SUBTYPE(type) HANDLE_TYPE(type)
#endif

#ifndef HANDLE_TYPE
#define HANDLE_TYPE(type)
#endif


HANDLE_SUBTYPE(SymbolExpr)
HANDLE_SUBTYPE(MemRef)
HANDLE_SUBTYPE(PrimLiteral)
HANDLE_SUBTYPE(ListLiteral)
HANDLE_SUBTYPE(TypeExpr)
HANDLE_SUBTYPE(DefExpr)
HANDLE_SUBTYPE(CallExpr)
HANDLE_SUBTYPE(IfExpr)
HANDLE_SUBTYPE(LoopExpr)
HANDLE_SUBTYPE(ForExpr)
HANDLE_SUBTYPE(LetExpr)
HANDLE_SUBTYPE(BlockExpr)
HANDLE_SUBTYPE(BreakExpr)
HANDLE_SUBTYPE(Prototype)
HANDLE_SUBTYPE(Function)
HANDLE_SUBTYPE(ExternFunction)
HANDLE_SUBTYPE(Intrinsic)
HANDLE_SUBTYPE(FunctionDeclaration)
HANDLE_SUBTYPE(FunctionDefinition)
HANDLE_SUBTYPE(TypeDefinition)
HANDLE_BASETYPE(IRExpr)
HANDLE_BASETYPE(Type)
HANDLE_BASETYPE(Object)

#undef HANDLE_BASETYPE
#undef HANDLE_SUBTYPE
#undef HANDLE_TYPE
