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
#include "IDLParserContext.hpp"
#include "kiaray_tokens.h"

#include <cstdio>
#include <fstream>

int main (int argc, char **argv)
{
    std::ifstream inf(argv[1]);
    KIARA::IDLParserContext ctx(inf);
    int tok;

    while ((tok = ctx.lex()) > 0)
    {
        if (ctx.isError())
            std::cerr<<"Last error: "<<ctx.getLastError()<<std::endl;
        printf("tok=%d  yytext=%s", tok, ctx.getText().c_str());
        if (tok == TOK_IDENTIFIER || tok == TOK_STRING || tok == TOK_ST_IDENTIFIER || tok == TOK_LITERAL)
            std::cout<<" ID = "<<ctx.token.str<<std::endl;
        std::cout<<std::endl;
    }

    if (ctx.isError())
        std::cerr<<"Last error: "<<ctx.getLastError()<<std::endl;

    return 0;
}
