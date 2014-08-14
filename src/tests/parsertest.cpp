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
#include <DFC/Base/Core/LibraryInit.hpp>
#include <KIARA/IDL/IDLParserContext.hpp>
#include <KIARA/IDL/IDLWriter.hpp>
#include <KIARA/DB/World.hpp>
#include <KIARA/DB/Module.hpp>

#include <cstdio>
#include <cstdlib>
#include <fstream>

int main (int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr<<"Usage: "<<argv[0]<<" filename"<<std::endl;
        return 1;
    }

    DFC::LibraryInit init;

    KIARA::World world;


    KIARA::Module::Ptr module = new KIARA::Module(world, "module");
    std::ifstream inf(argv[1]);

    KIARA::IDLParserContext ctx(module, inf, argv[1]);

    bool succeed = ctx.parse();

    //world.dump();
    //module->print(std::cout);
    KIARA::IDLWriter writer(module);
    writer.write(std::cout);

    if (succeed)
    {
        std::cerr<<"PARSING SUCCEED"<<std::endl;
        return 0;
    }

    std::cerr<<"PARSING FAILED"<<std::endl;

    return 1;
}
