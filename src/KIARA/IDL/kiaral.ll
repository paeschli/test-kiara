/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/**
 * Kiara scanner.
 *
 * Tokenizes a kiara definition file.
 */

%{

/* This is redundant with some of the flags in Makefile.am, but it works
 * when people override CXXFLAGS without being careful. The pragmas are
 * the 'right' way to do it, but don't work on old-enough GCC (in particular
 * the GCC that ship on Mac OS X 10.6.5, *counter* to what the GNU docs say)
 *
 * We should revert the Makefile.am changes once Apple ships a reasonable
 * GCC.
 */
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-label"
#endif

#include <string>
#include <cerrno>
#include <cstdlib>
#include <cassert>
#include <sstream>

#ifdef _WIN32
#include <io.h>
#define strtoll _strtoi64
#define isatty _isatty
#define fileno _fileno
#define YY_NO_UNISTD_H
#endif

#define KIARA_LIB
#include <KIARA/IDL/IDLParserContext.hpp>
#include "kiaray_tokens.hpp"

#define RETURN_TOK_ID(tokenid)            \
    yyextra->token.id = (tokenid);        \
    return (tokenid)

#define CLEAR_TOKEN() yyextra->token.clear()

#define RETURN_TOK(tokenid)               \
    CLEAR_TOKEN();                        \
    RETURN_TOK_ID(tokenid)

#define RETURN_INT_TOK(value)             \
    CLEAR_TOKEN();                        \
    yyextra->token.val.iconst = (value);  \
    RETURN_TOK_ID(TOK_INT_CONSTANT)

#define RETURN_DUB_TOK(value)             \
    CLEAR_TOKEN();                        \
    yyextra->token.val.dconst = (value);  \
    RETURN_TOK_ID(TOK_DUB_CONSTANT)

#define RETURN_ID_TOK(value)              \
    CLEAR_TOKEN();                        \
    yyextra->token.str = (value);         \
    RETURN_TOK_ID(TOK_IDENTIFIER)

#define RETURN_ST_ID_TOK(value)           \
    CLEAR_TOKEN();                        \
    yyextra->token.str = (value);         \
    RETURN_TOK_ID(TOK_ST_IDENTIFIER)

#define RETURN_LITERAL_TOK(value)         \
    CLEAR_TOKEN();                        \
    yyextra->token.str = (value);         \
    RETURN_TOK_ID(TOK_LITERAL)

#define PARSER_ERROR(message)                                  \
do {                                                           \
   std::ostringstream msgs;                                    \
   msgs << "[ERROR:"<<yyextra->fileName<<":"<<yylineno<<"] "   \
        << message;                                            \
   yyextra->setScannerError(msgs.str());                       \
} while(false)

#define PARSER_WARNING(message)                                \
do {                                                           \
   std::ostringstream msgs;                                    \
   msgs << "[WARNING:"<<yyextra->fileName<<":"<<yylineno<<"] " \
        << message;                                            \
   yyextra->setScannerError(msgs.str());                       \
} while(false)

#define KIARA_RESERVED_KEYWORD(keyword) \
  { \
  PARSER_ERROR("Cannot use reserved language keyword: \""<<keyword<<"\"");\
  RETURN_TOK(-1);\
  }

#define KIARA_INTEGER_OVERFLOW(text) \
  { \
  PARSER_ERROR("This integer is too big: \""<<text<<"\"");\
  RETURN_TOK(-1);\
  }

#define KIARA_UNEXPECTED_TOKEN(text) \
  { \
  PARSER_ERROR("Unexpected token in input: \""<<text<<"\"");\
  RETURN_TOK(-1);\
  }

#define YY_USER_ACTION yyextra->lineNum = yylineno;

#define YY_INPUT(buf,result,max_size)                     \
{                                                         \
    char c = '*';                                         \
    size_t n;                                             \
    for (n = 0; n < max_size &&                           \
            yyextra->is->good() &&                        \
            !(yyextra->is->get(c).eof()) &&               \
                c != '\n'; ++n )                          \
        buf[n] = (char) c;                                \
    if ( c == '\n' )                                      \
        buf[n++] = (char) c;                              \
    if ( yyextra->is->eof() && ferror( yyin ) )           \
        YY_FATAL_ERROR( "input in flex scanner failed" ); \
    result = n;                                           \
}

#define yyterminate() RETURN_TOK(YY_NULL)

%}

/**
 * Our scanner is reentrant
 */
%option reentrant

%option prefix="kiaral_"

/**
 * Provides the yylineno global, useful for debugging output
 */
%option yylineno

/**
 * Our inputs are all single files, so no need for yywrap
 */
%option noyywrap

/**
 * We don't use it, and it fires up warnings at -Wall
 */
%option nounput

%option extra-type="::KIARA::IDLParserContext *"

/**
 * Helper definitions, comments, constants, and whatnot
 */

intconstant   ([+-]?[0-9]+)
hexconstant   ("0x"[0-9A-Fa-f]+)
dubconstant   ([+-]?[0-9]*(\.[0-9]+)?([eE][+-]?[0-9]+)?)
identifier    ([a-zA-Z_][\.a-zA-Z_0-9]*)
whitespace    ([ \t\r\n]*)
sillycomm     ("/*""*"*"*/")
multicomm     ("/*"[^*]"/"*([^*/]|[^*]"/"|"*"[^/])*"*"*"*/")
doctext       ("/**"([^*/]|[^*]"/"|"*"[^/])*"*"*"*/")
comment       ("//"[^\n]*)
unixcomment   ("#"[^\n]*)
symbol        ([:;\,\{\}\(\)\=<>\[\]])

st_identifier ([a-zA-Z-][\.a-zA-Z_0-9-]*)
literal_begin (['\"])


%%

{whitespace}         { /* do nothing */                 }
{sillycomm}          { /* do nothing */                 }
{multicomm}          { /* do nothing */                 }
{comment}            { /* do nothing */                 }
{unixcomment}        { /* do nothing */                 }

":"                  { RETURN_TOK(TOK_COLON);           }
";"                  { RETURN_TOK(TOK_SEMI);            }
","                  { RETURN_TOK(TOK_COMMA);           }
"{"                  { RETURN_TOK(TOK_LBRACE);          }
"}"                  { RETURN_TOK(TOK_RBRACE);          }
"("                  { RETURN_TOK(TOK_LPAREN);          }
")"                  { RETURN_TOK(TOK_RPAREN);          }
"="                  { RETURN_TOK(TOK_EQ);              }
"<"                  { RETURN_TOK(TOK_LT);              }
">"                  { RETURN_TOK(TOK_GT);              }
"["                  { RETURN_TOK(TOK_LBRACKET);        }
"]"                  { RETURN_TOK(TOK_RBRACKET);        }
"*"                  { RETURN_TOK(TOK_STAR);            }
  /* "@"                  { RETURN_TOK(TOK_AT);              } */

"false"              { RETURN_INT_TOK(0); }
"true"               { RETURN_INT_TOK(1); }

"namespace"          { RETURN_TOK(TOK_NAMESPACE);            }

  /* "cpp_namespace"      { RETURN_TOK(TOK_CPP_NAMESPACE);        } */
  /* "cpp_include"        { RETURN_TOK(TOK_CPP_INCLUDE);          } */
  /* "cpp_type"           { RETURN_TOK(TOK_CPP_TYPE);             } */
  /* "java_package"       { RETURN_TOK(TOK_JAVA_PACKAGE);         } */
  /* "cocoa_prefix"       { RETURN_TOK(TOK_COCOA_PREFIX);         } */
  /* "csharp_namespace"   { RETURN_TOK(TOK_CSHARP_NAMESPACE);     } */
  /* "delphi_namespace"   { RETURN_TOK(TOK_DELPHI_NAMESPACE);     } */
  /* "php_namespace"      { RETURN_TOK(TOK_PHP_NAMESPACE);        } */
  /* "py_module"          { RETURN_TOK(TOK_PY_MODULE);            } */
  /* "perl_package"       { RETURN_TOK(TOK_PERL_PACKAGE);         } */
  /* "ruby_namespace"     { RETURN_TOK(TOK_RUBY_NAMESPACE);       } */
  /* "smalltalk_category" { RETURN_TOK(TOK_SMALLTALK_CATEGORY);   } */
  /* "smalltalk_prefix"   { RETURN_TOK(TOK_SMALLTALK_PREFIX);     } */
"xsd_all"            { RETURN_TOK(TOK_XSD_ALL);              }
"xsd_optional"       { RETURN_TOK(TOK_XSD_OPTIONAL);         }
"xsd_nillable"       { RETURN_TOK(TOK_XSD_NILLABLE);         }
  /* "xsd_namespace"      { RETURN_TOK(TOK_XSD_NAMESPACE);        } */
"xsd_attrs"          { RETURN_TOK(TOK_XSD_ATTRS);            }
"include"            { RETURN_TOK(TOK_INCLUDE);              }
"void"               { RETURN_TOK(TOK_VOID);                 }
"boolean"            { RETURN_TOK(TOK_BOOLEAN);              }
"i8"                 { RETURN_TOK(TOK_I8);                   }
"u8"                 { RETURN_TOK(TOK_U8);                   }
"i16"                { RETURN_TOK(TOK_I16);                  }
"u16"                { RETURN_TOK(TOK_U16);                  }
"i32"                { RETURN_TOK(TOK_I32);                  }
"u32"                { RETURN_TOK(TOK_U32);                  }
"i64"                { RETURN_TOK(TOK_I64);                  }
"u64"                { RETURN_TOK(TOK_U64);                  }
"float"              { RETURN_TOK(TOK_FLOAT);                }
"double"             { RETURN_TOK(TOK_DOUBLE);               }
"any"                { RETURN_TOK(TOK_ANY);                  }

"string"             { RETURN_TOK(TOK_STRING);               }
"binary"             { RETURN_TOK(TOK_BINARY);               }
"slist"              { RETURN_TOK(TOK_SLIST);                }
"senum"              { RETURN_TOK(TOK_SENUM);                }
"typedef"            { RETURN_TOK(TOK_TYPEDEF);              }
"struct"             { RETURN_TOK(TOK_STRUCT);               }
"union"              { RETURN_TOK(TOK_UNION);                }
"exception"          { RETURN_TOK(TOK_XCEPTION);             }
"extends"            { RETURN_TOK(TOK_EXTENDS);              }
"throws"             { RETURN_TOK(TOK_THROWS);               }
"service"            { RETURN_TOK(TOK_SERVICE);              }
"enum"               { RETURN_TOK(TOK_ENUM);                 }
"const"              { RETURN_TOK(TOK_CONST);                }
"required"           { RETURN_TOK(TOK_REQUIRED);             }
"optional"           { RETURN_TOK(TOK_OPTIONAL);             }
"annotation"         { RETURN_TOK(TOK_ANNOTATION);           }

"BEGIN"              { KIARA_RESERVED_KEYWORD(yytext); }
"END"                { KIARA_RESERVED_KEYWORD(yytext); }
"__CLASS__"          { KIARA_RESERVED_KEYWORD(yytext); }
"__DIR__"            { KIARA_RESERVED_KEYWORD(yytext); }
"__FILE__"           { KIARA_RESERVED_KEYWORD(yytext); }
"__FUNCTION__"       { KIARA_RESERVED_KEYWORD(yytext); }
"__LINE__"           { KIARA_RESERVED_KEYWORD(yytext); }
"__METHOD__"         { KIARA_RESERVED_KEYWORD(yytext); }
"__NAMESPACE__"      { KIARA_RESERVED_KEYWORD(yytext); }
"abstract"           { KIARA_RESERVED_KEYWORD(yytext); }
"alias"              { KIARA_RESERVED_KEYWORD(yytext); }
"and"                { KIARA_RESERVED_KEYWORD(yytext); }
"args"               { KIARA_RESERVED_KEYWORD(yytext); }
"as"                 { KIARA_RESERVED_KEYWORD(yytext); }
"assert"             { KIARA_RESERVED_KEYWORD(yytext); }
"begin"              { KIARA_RESERVED_KEYWORD(yytext); }
"break"              { KIARA_RESERVED_KEYWORD(yytext); }
"case"               { KIARA_RESERVED_KEYWORD(yytext); }
"catch"              { KIARA_RESERVED_KEYWORD(yytext); }
"class"              { KIARA_RESERVED_KEYWORD(yytext); }
"clone"              { KIARA_RESERVED_KEYWORD(yytext); }
"continue"           { KIARA_RESERVED_KEYWORD(yytext); }
"declare"            { KIARA_RESERVED_KEYWORD(yytext); }
"def"                { KIARA_RESERVED_KEYWORD(yytext); }
"default"            { KIARA_RESERVED_KEYWORD(yytext); }
"del"                { KIARA_RESERVED_KEYWORD(yytext); }
"delete"             { KIARA_RESERVED_KEYWORD(yytext); }
"do"                 { KIARA_RESERVED_KEYWORD(yytext); }
"dynamic"            { KIARA_RESERVED_KEYWORD(yytext); }
"elif"               { KIARA_RESERVED_KEYWORD(yytext); }
"else"               { KIARA_RESERVED_KEYWORD(yytext); }
"elseif"             { KIARA_RESERVED_KEYWORD(yytext); }
"elsif"              { KIARA_RESERVED_KEYWORD(yytext); }
"end"                { KIARA_RESERVED_KEYWORD(yytext); }
"enddeclare"         { KIARA_RESERVED_KEYWORD(yytext); }
"endfor"             { KIARA_RESERVED_KEYWORD(yytext); }
"endforeach"         { KIARA_RESERVED_KEYWORD(yytext); }
"endif"              { KIARA_RESERVED_KEYWORD(yytext); }
"endswitch"          { KIARA_RESERVED_KEYWORD(yytext); }
"endwhile"           { KIARA_RESERVED_KEYWORD(yytext); }
"ensure"             { KIARA_RESERVED_KEYWORD(yytext); }
"except"             { KIARA_RESERVED_KEYWORD(yytext); }
"exec"               { KIARA_RESERVED_KEYWORD(yytext); }
"finally"            { KIARA_RESERVED_KEYWORD(yytext); }
"for"                { KIARA_RESERVED_KEYWORD(yytext); }
"foreach"            { KIARA_RESERVED_KEYWORD(yytext); }
"function"           { KIARA_RESERVED_KEYWORD(yytext); }
"global"             { KIARA_RESERVED_KEYWORD(yytext); }
"goto"               { KIARA_RESERVED_KEYWORD(yytext); }
"if"                 { KIARA_RESERVED_KEYWORD(yytext); }
"implements"         { KIARA_RESERVED_KEYWORD(yytext); }
"import"             { KIARA_RESERVED_KEYWORD(yytext); }
"in"                 { KIARA_RESERVED_KEYWORD(yytext); }
"inline"             { KIARA_RESERVED_KEYWORD(yytext); }
"instanceof"         { KIARA_RESERVED_KEYWORD(yytext); }
"interface"          { KIARA_RESERVED_KEYWORD(yytext); }
"is"                 { KIARA_RESERVED_KEYWORD(yytext); }
"lambda"             { KIARA_RESERVED_KEYWORD(yytext); }
"module"             { KIARA_RESERVED_KEYWORD(yytext); }
"native"             { KIARA_RESERVED_KEYWORD(yytext); }
"new"                { KIARA_RESERVED_KEYWORD(yytext); }
"next"               { KIARA_RESERVED_KEYWORD(yytext); }
"nil"                { KIARA_RESERVED_KEYWORD(yytext); }
"not"                { KIARA_RESERVED_KEYWORD(yytext); }
"or"                 { KIARA_RESERVED_KEYWORD(yytext); }
"pass"               { KIARA_RESERVED_KEYWORD(yytext); }
"print"              { KIARA_RESERVED_KEYWORD(yytext); }
"private"            { KIARA_RESERVED_KEYWORD(yytext); }
"protected"          { KIARA_RESERVED_KEYWORD(yytext); }
"public"             { KIARA_RESERVED_KEYWORD(yytext); }
"raise"              { KIARA_RESERVED_KEYWORD(yytext); }
"redo"               { KIARA_RESERVED_KEYWORD(yytext); }
"rescue"             { KIARA_RESERVED_KEYWORD(yytext); }
"retry"              { KIARA_RESERVED_KEYWORD(yytext); }
"register"           { KIARA_RESERVED_KEYWORD(yytext); }
"return"             { KIARA_RESERVED_KEYWORD(yytext); }
"self"               { KIARA_RESERVED_KEYWORD(yytext); }
"sizeof"             { KIARA_RESERVED_KEYWORD(yytext); }
"static"             { KIARA_RESERVED_KEYWORD(yytext); }
"super"              { KIARA_RESERVED_KEYWORD(yytext); }
"switch"             { KIARA_RESERVED_KEYWORD(yytext); }
"synchronized"       { KIARA_RESERVED_KEYWORD(yytext); }
"then"               { KIARA_RESERVED_KEYWORD(yytext); }
"this"               { KIARA_RESERVED_KEYWORD(yytext); }
"throw"              { KIARA_RESERVED_KEYWORD(yytext); }
"transient"          { KIARA_RESERVED_KEYWORD(yytext); }
"try"                { KIARA_RESERVED_KEYWORD(yytext); }
"undef"              { KIARA_RESERVED_KEYWORD(yytext); }
"unless"             { KIARA_RESERVED_KEYWORD(yytext); }
"unsigned"           { KIARA_RESERVED_KEYWORD(yytext); }
"until"              { KIARA_RESERVED_KEYWORD(yytext); }
"use"                { KIARA_RESERVED_KEYWORD(yytext); }
"var"                { KIARA_RESERVED_KEYWORD(yytext); }
"virtual"            { KIARA_RESERVED_KEYWORD(yytext); }
"volatile"           { KIARA_RESERVED_KEYWORD(yytext); }
"when"               { KIARA_RESERVED_KEYWORD(yytext); }
"while"              { KIARA_RESERVED_KEYWORD(yytext); }
"with"               { KIARA_RESERVED_KEYWORD(yytext); }
"xor"                { KIARA_RESERVED_KEYWORD(yytext); }
"yield"              { KIARA_RESERVED_KEYWORD(yytext); }

{intconstant} {
  errno = 0;
  int64_t tmp = strtoll(yytext, NULL, 10);
  if (errno == ERANGE) {
    KIARA_INTEGER_OVERFLOW(yytext);
  }
  RETURN_INT_TOK(tmp);
}

{hexconstant} {
  errno = 0;
  int64_t tmp = strtoll(yytext+2, NULL, 16);
  if (errno == ERANGE) {
    KIARA_INTEGER_OVERFLOW(yytext);
  }
  RETURN_INT_TOK(tmp);
}

{dubconstant} {
  double tmp = atof(yytext);
  RETURN_DUB_TOK(tmp);
}

{identifier} {
  RETURN_ID_TOK(yytext);
}

  /*
{st_identifier} {
  RETURN_ST_ID_TOK(yytext);
}
  */

{literal_begin} {
  char mark = yytext[0];
  std::string result;
  for(;;)
  {
    int ch = yyinput(yyscanner);
    switch (ch) {
      case EOF:
        PARSER_ERROR("End of file while read string at "<<yylineno);
        RETURN_TOK(-1);
      case '\n':
        PARSER_ERROR("End of line while read string at "<<(yylineno - 1));
        RETURN_TOK(-1);
      case '\\':
        ch = yyinput(yyscanner);
        switch (ch) {
          case 'r':
            result.push_back('\r');
            continue;
          case 'n':
            result.push_back('\n');
            continue;
          case 't':
            result.push_back('\t');
            continue;
          case '"':
            result.push_back('"');
            continue;
          case '\'':
            result.push_back('\'');
            continue;
          case '\\':
            result.push_back('\\');
            continue;
          default:
            PARSER_ERROR("Bad escape character");
            RETURN_TOK(-1);
        }
        break;
      default:
        if (ch == mark) {
          RETURN_LITERAL_TOK(result);
        } else {
          result.push_back(ch);
        }
    }
  }
}


{doctext} {
 /* This does not show up in the parse tree. */
 /* Rather, the parser will grab it out of the global. */
#if 0
  if (g_parse_mode == PROGRAM) {
    clear_doctext();
    g_doctext = strdup(yytext + 3);
    g_doctext[strlen(g_doctext) - 2] = '\0';
    g_doctext = clean_up_doctext(g_doctext);
    g_doctext_lineno = yylineno;
  }
#endif
}

. {
  KIARA_UNEXPECTED_TOKEN(yytext);
}

%%

namespace KIARA
{

int IDLParserContext::lex()
{
    int tok = kiaral_lex(scanner);
    assert(tok == token.id);
    token.lineNum = lineNum;
    return tok;
}

std::string IDLParserContext::getText()
{
    return kiaral_get_text(scanner);
}

void IDLParserContext::initScanner()
{
    kiaral_lex_init_extra(this, &scanner);
}

void IDLParserContext::destroyScanner()
{
    kiaral_lex_destroy(scanner);
}

} // namespace KIARA

/* vim: filetype=lex
*/
