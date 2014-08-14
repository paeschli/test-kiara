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
#include <cstdio>
#include <cstdarg>

/**
 * Report an error to the user. This is called yyerror for historical
 * reasons (lex and yacc expect the error reporting routine to be called
 * this). Call this function to report any errors to the user.
 * yyerror takes printf style arguments.
 *
 * @param fmt C format string followed by additional arguments
 */
void yyerror(const char* fmt, ...) {
  va_list args;
  fprintf(stderr,
          "[ERROR:%s:%d] (last token was '%s')\n",
          "<CURPATH>",//TODO: g_curpath.c_str(),
          0, //TODO: yylineno,
          "<yytext>");  //TODO: yytext);

  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);

  fprintf(stderr, "\n");
}

/**
 * Prints a warning message
 *
 * @param fmt C format string followed by additional arguments
 */
void pwarning(int level, const char* fmt, ...) {
  // if (g_warn < level) {
  //   return;
  // }
  va_list args;
  printf("[WARNING:%s:%d] ", "<PATH>"/*TODO g_curpath.c_str()*/, 0/*TODO:yylineno*/);
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  printf("\n");
}

/**
 * Prints a debug message from the parser.
 *
 * @param fmt C format string followed by additional arguments
 */
void pdebug(const char* fmt, ...) {
  // if (g_debug == 0) {
  //   return;
  // }
  va_list args;
  printf("[PARSE:%d] ", 0 /*TODO:yylineno*/);
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  printf("\n");
}
