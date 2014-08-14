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
#define KIARA_LIB
#include "IDLToken.hpp"
#include <KIARA/IDL/kiaray_tokens.hpp>

namespace KIARA
{

std::ostream & operator<<(std::ostream &out, const IDLToken &token)
{
    switch (token.id)
    {
        case TOK_NAMESPACE: out<<"'namespace'"; break;
        case TOK_IDENTIFIER: out<<"identifier:'"<<token.str<<"'"; break;
        case TOK_STAR: out<<"'*'"; break;
        case TOK_INCLUDE: out<<"'include'"; break;
        case TOK_LITERAL: out<<"literal:'"<<token.str<<"'"; break;
        case TOK_TYPEDEF: out<<"'typedef'"; break;
        case TOK_COMMA: out<<"','"; break;
        case TOK_SEMI: out<<"';'"; break;
        case TOK_ENUM: out<<"'enum'"; break;
        case TOK_LBRACE: out<<"'{'"; break;
        case TOK_RBRACE: out<<"'}'"; break;
        case TOK_EQ: out<<"'='"; break;
        case TOK_INT_CONSTANT: out<<"int_const:"<<token.val.iconst; break;
        case TOK_SENUM: out<<"'senum'"; break;
        case TOK_CONST: out<<"'const'"; break;
        case TOK_DUB_CONSTANT: out<<"dub_const:"<<token.val.dconst; break;
        case TOK_LBRACKET: out<<"'['"; break;
        case TOK_RBRACKET: out<<"']'"; break;
        case TOK_COLON: out<<"':'"; break;
        case TOK_STRUCT: out<<"'struct'"; break;
        case TOK_UNION: out<<"'union'"; break;
        case TOK_XSD_ALL: out<<"'xsd_all'"; break;
        case TOK_XSD_OPTIONAL: out<<"'xsd_optional'"; break;
        case TOK_XSD_NILLABLE: out<<"'xsd_nillable'"; break;
        case TOK_XSD_ATTRS: out<<"'xsd_attrs'"; break;
        case TOK_XCEPTION: out<<"'xception'"; break;
        case TOK_ANNOTATION: out<<"'annotation'"; break;
        case TOK_SERVICE: out<<"'service'"; break;
        case TOK_EXTENDS: out<<"'extends'"; break;
        case TOK_LPAREN: out<<"'('"; break;
        case TOK_RPAREN: out<<"')'"; break;
        case TOK_THROWS: out<<"'throws'"; break;
        case TOK_REQUIRED: out<<"'required'"; break;
        case TOK_OPTIONAL: out<<"'optional'"; break;
        case TOK_VOID: out<<"'void'"; break;
        case TOK_STRING: out<<"'string'"; break;
        case TOK_BINARY: out<<"'binary'"; break;
        case TOK_SLIST: out<<"'slist'"; break;
        case TOK_BOOLEAN: out<<"'boolean'"; break;
        case TOK_I8: out<<"'i8'"; break;
        case TOK_U8: out<<"'u8'"; break;
        case TOK_I16: out<<"'i16'"; break;
        case TOK_U16: out<<"'u16'"; break;
        case TOK_I32: out<<"'i32'"; break;
        case TOK_U32: out<<"'u32'"; break;
        case TOK_I64: out<<"'i64'"; break;
        case TOK_U64: out<<"'u64'"; break;
        case TOK_FLOAT: out<<"'float'"; break;
        case TOK_DOUBLE: out<<"'double'"; break;
        //case TOK_MAP: out<<"'map'"; break;
        case TOK_LT: out<<"'<'"; break;
        case TOK_GT: out<<"'>'"; break;
        //case TOK_SET: out<<"'set'"; break;
        //case TOK_LIST: out<<"'list'"; break;
        //case TOK_CPP_TYPE: out<<"'cpp_type'"; break;
        //case TOK_AT: out<<"'@'"; break;
        default: out<<"unknown:"<<token.id; break;
    }
    return out;
}

} // namespace KIARA
