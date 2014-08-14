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
 * kiara_interpreter_impl.cpp
 *
 *  Created on: 11.07.2013
 *      Author: Dmitri Rubinstein
 */
#define KIARA_LIB
#include <KIARA/Common/Config.hpp>
#include "KIARA/Impl/Network.hpp"
#include "KIARA/Impl/Interpreter.hpp"
#include "KIARA/DB/DerivedTypes.hpp"
#include "KIARA/Impl/Core.hpp"

extern "C" {

int KIARA_Interpreter(KIARA_FuncObj * closure, void *args[], size_t num_args)
{
    KIARA_Connection *connection = closure->base.connection;
    KIARA::Impl::Context *context = KIARA::Impl::unwrap(connection)->getContext();
    //KIARA::World &world = context->world();
    KIARA::FunctionType::Ptr fty =
            KIARA::dyn_cast<KIARA::FunctionType>(
                    context->unwrapType(closure->base.funcType));

    KIARA::Type::Ptr returnType = fty->getReturnType();
    //KIARA::StructType::Ptr argsType = fty->getArgsType();

    //kiaraDbgDumpType(closure->base.funcType); //???DEBUG

    const size_t numArgs = fty->getNumParams(); //argsType->getNumElements();
    assert(numArgs == num_args); // FIXME add proper error handling

    for (size_t i = 0; i < numArgs; ++i)
    {
        KIARA::Type::Ptr argTy = fty->getParamType(i); //argsType->getAs<KIARA::Type>(i);
        if (KIARA::dyn_cast<KIARA::PtrType>(argTy))
        {
            // Pointer (T*) is stored as a pointer to pointer (T**)
            void *value = *(void**)args[i];
            printf("pointer %p\n", value);
            continue;
        }

        if (KIARA::dyn_cast<KIARA::RefType>(argTy))
        {
            // Reference (T&) is stored as a pointer (T*)
            void *value = (void*)args[i];
            printf("reference %p\n", value);
            continue;
        }

        KIARA::PrimType::Ptr primTy = DFC::safe_object_cast<KIARA::PrimType>(argTy);
        const KIARA::PrimTypeKind primTyKind = primTy->primtype_kind();
        if (primTy->isFloatingPoint())
        {
            switch (primTyKind)
            {
                case KIARA::PRIMTYPE_c_float:
                case KIARA::PRIMTYPE_float:
                {
                    float value = *(float*)args[i];
                    printf("float = %f\n", value);
                }
                break;
                case KIARA::PRIMTYPE_c_double:
                case KIARA::PRIMTYPE_double:
                {
                    double value = *(double*)args[i];
                    printf("double = %f\n", value);
                }
                break;
                case KIARA::PRIMTYPE_c_longdouble:
                {
                    long double value = *(long double*)args[i];
                    printf("long double = %f\n", (double)value);
                }
                break;
                default:
                    assert(false);
            }
            continue;
        }

        if (primTy && primTy->isInteger())
        {
            switch (primTyKind)
            {
                case KIARA::PRIMTYPE_i8:
                case KIARA::PRIMTYPE_c_int8_t:
                {
                    int8_t value = *(int8_t*)args[i];
                    printf("int8_t = %i\n", (int)value);
                }
                break;
                case KIARA::PRIMTYPE_u8:
                case KIARA::PRIMTYPE_c_uint8_t:
                {
                    uint8_t value = *(uint8_t*)args[i];
                    printf("uint8_t = %i\n", (int)value);
                }
                break;
                case KIARA::PRIMTYPE_i16:
                case KIARA::PRIMTYPE_c_int16_t:
                {
                    int16_t value = *(int16_t*)args[i];
                    printf("int16_t = %i\n", (int)value);
                }
                break;
                case KIARA::PRIMTYPE_u16:
                case KIARA::PRIMTYPE_c_uint16_t:
                {
                    uint16_t value = *(uint16_t*)args[i];
                    printf("uint16_t = %i\n", (int)value);
                }
                break;
                case KIARA::PRIMTYPE_i32:
                case KIARA::PRIMTYPE_c_int32_t:
                {
                    int32_t value = *(int32_t*)args[i];
                    printf("int32_t = %i\n", (int)value);
                }
                break;
                case KIARA::PRIMTYPE_u32:
                case KIARA::PRIMTYPE_c_uint32_t:
                {
                    uint32_t value = *(uint32_t*)args[i];
                    printf("uint32_t = %i\n", (int)value);
                }
                break;
                case KIARA::PRIMTYPE_i64:
                case KIARA::PRIMTYPE_c_int64_t:
                {
                    int64_t value = *(int64_t*)args[i];
                    printf("int64_t = %i\n", (int)value);
                }
                break;
                case KIARA::PRIMTYPE_u64:
                case KIARA::PRIMTYPE_c_uint64_t:
                {
                    uint64_t value = *(uint64_t*)args[i];
                    printf("uint64_t = %i\n", (int)value);
                }
                default:
                    assert(false);
            }
            continue;
        }
    }

    printf("Called interpreter with %p\n", closure);
    printf("INTERPRETER IS NOT IMPLEMENTED YET !\n");
    return 0;
}

} // extern "C"
