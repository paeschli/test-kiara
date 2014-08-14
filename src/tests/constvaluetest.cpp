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
 * constvaluetest.cpp
 *
 *  Created on: 07.08.2012
 *      Author: Dmitri Rubinstein
 */
#include <DFC/Base/Core/LibraryInit.hpp>
#include <KIARA/Compiler/IR.hpp>
#include <KIARA/DB/World.hpp>
#include <KIARA/DB/Module.hpp>
#include <cstdio>
#include <cstdlib>
#include <fstream>

template <class T> KIARA::IR::PrimLiteral::Ptr check(T value, KIARA::World &world)
{
    KIARA::IR::PrimLiteral::Ptr v = new KIARA::IR::PrimLiteral(
            KIARA::normalize_value(value), world);

    std::cout << "type: ";
    v->getValueType()->getCanonicalType()->print(std::cout);
    std::cout << " value: "<< *v;
    std::cout << "\n";

    return v;
}

int main (int argc, char **argv)
{
    DFC::LibraryInit init;

    KIARA::World world;
    world.dump();

    check(12, world);
    check('x', world);
    check<signed char>(-11, world);
    check<unsigned char>(13, world);
    check<long long>(22, world);
    check<std::string>("TEST", world);
    check("TEST1", world);
    check<short>(33, world);
    check(true, world);
    check(0.2, world);
    check(2.2f, world);

    KIARA::IR::PrimLiteral::Ptr data[] = {
            new KIARA::IR::PrimLiteral((int32_t)12, world),
            new KIARA::IR::PrimLiteral("test", world),
            new KIARA::IR::PrimLiteral(0.2, world)
    };
    size_t dataLen = sizeof(data)/sizeof(data[0]);

    KIARA::IR::ListLiteral::Ptr l = new KIARA::IR::ListLiteral(data, data+dataLen, world.type_any());
    std::cout << *l << std::endl;
}
