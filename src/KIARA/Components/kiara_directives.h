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
 * kiara_directives.h
 *
 *  Created on: Dec 30, 2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMPONENTS_DIRECTIVES_H_INCLUDED
#define KIARA_COMPONENTS_DIRECTIVES_H_INCLUDED

#include <KIARA/kiara.h>

/* Macros for declaration of directives */

#define KIARA_DIRECTIVE KIARA_NOINLINE
#define KIARA_BEGIN_DIRECTIVES KIARA_BEGIN_EXTERN_C
#define KIARA_END_DIRECTIVES KIARA_END_EXTERN_C

/* Directives */

#define KIARA_INFO_BEGIN                                            \
KIARA_BEGIN_EXTERN_C                                                \
void KIARA_DIRECTIVE KIARA_JOIN(kiara_info_,__LINE__)(void)         \
{

#define KIARA_INFO_END                                              \
KIARA_END_EXTERN_C                                                  \
}

#define KIARA_INFO_BEGIN_X(X)                                       \
KIARA_BEGIN_EXTERN_C                                                \
static void KIARA_JOIN(KIARA_JOIN(kiara_info_,X),__LINE__)(void) {

KIARA_BEGIN_DIRECTIVES

#define KIARA_EXPORT_TYPE(Type)                                                     \
{                                                                                   \
    extern void KIARA_DIRECTIVE KIARA_JOIN(kiara_export_type_, __LINE__)            \
                                                    (Type*, const char *name);      \
    KIARA_JOIN(kiara_export_type_, __LINE__)(0, KIARA_STRINGIZE(Type));             \
}

extern void KIARA_DIRECTIVE kiara_command(const char *command);

#define KIARA_COMMAND(Command) kiara_command(Command);

KIARA_END_DIRECTIVES

#endif /* KIARA_COMPONENTS_DIRECTIVES_H_INCLUDED */
