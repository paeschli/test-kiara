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
 * LibraryInit.cpp
 *
 *  Created on: 13.08.2012 14:06 +0200
 *   Author(s): Dmitri Rubinstein
 */

#define KIARA_LIB
#include <KIARA/Common/Config.hpp>

#include <KIARA/Core/LibraryInit.hpp>

#include <DFC/Base/Utils/StaticInit.hpp>
#include <DFC/Base/Core/ObjectFactory.hpp>
#include <DFC/Base/Core/ObjectMacros.hpp>
#include <KIARA/Core/GCObject.hpp>
#include <KIARA/DB/DerivedTypes.hpp>
#include <KIARA/DB/Annotation.hpp>
#include <iostream>

namespace KIARA
{

using namespace std;

DFC_STATIC_INIT_FUNC
{
    // Register base types
    DFC_REGISTER_TYPE(GCObject);

    // Types
    DFC_REGISTER_TYPE(TypeType);
    DFC_REGISTER_TYPE(VoidType);
    DFC_REGISTER_TYPE(UnresolvedSymbolType);
    DFC_REGISTER_TYPE(AnyType);
    DFC_REGISTER_TYPE(TypedefType);
    DFC_REGISTER_TYPE(PtrType);
    DFC_REGISTER_TYPE(RefType);
    DFC_REGISTER_TYPE(ArrayType);
    DFC_REGISTER_TYPE(FixedArrayType);
    DFC_REGISTER_TYPE(SymbolType);
    DFC_REGISTER_TYPE(CompositeType);
    DFC_REGISTER_TYPE(StructType);
    DFC_REGISTER_TYPE(FunctionType);
    DFC_REGISTER_TYPE(ServiceType);
    DFC_REGISTER_TYPE(PrimType);
    DFC_REGISTER_TYPE(PrimValueType);
    DFC_REGISTER_TYPE(EnumType);

    // Misc
    DFC_REGISTER_TYPE(Namespace);
    DFC_REGISTER_TYPE(Annotation);
}

// DFC_STATIC_SHUTDOWN_FUNC
// {
// }

} // namespace KIARA
