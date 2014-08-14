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
 * dstring.h
 *
 *  Created on: 02.07.2011
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_CDT_KR_DSTRING_H_INCLUDED
#define KIARA_CDT_KR_DSTRING_H_INCLUDED

#include <KIARA/Common/Config.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kr_dstring {
    char *data;
    size_t size;
    size_t capacity;
} kr_dstring_t;

KIARA_API void KIARA_LLVM_BITCODE_INLINE kr_dstring_init(kr_dstring_t *str);
KIARA_API void KIARA_LLVM_BITCODE_INLINE kr_dstring_destroy(kr_dstring_t *str);
/** Allocates kr_dstring structure with malloc, then calls kr_dstring_init */
KIARA_API kr_dstring_t * KIARA_LLVM_BITCODE_INLINE kr_dstring_new(void);
/** Calls kr_dstring_destroy, then frees kr_dstring structure with free. */
KIARA_API void KIARA_LLVM_BITCODE_INLINE kr_dstring_delete(kr_dstring_t *str);
KIARA_API char * KIARA_LLVM_BITCODE_INLINE kr_dstring_release(kr_dstring_t *str);
KIARA_API void KIARA_LLVM_BITCODE_INLINE kr_dstring_clear(kr_dstring_t *str);
KIARA_API int KIARA_LLVM_BITCODE_INLINE kr_dstring_reserve(kr_dstring_t *str, size_t new_capacity);
KIARA_API int KIARA_LLVM_BITCODE_INLINE kr_dstring_assign_str(kr_dstring_t *str, const char *value);
KIARA_API int KIARA_LLVM_BITCODE_INLINE kr_dstring_append_strn(kr_dstring_t *str, const char *value, size_t n);
KIARA_API int KIARA_LLVM_BITCODE_INLINE kr_dstring_append_str(kr_dstring_t *str, const char *value);
KIARA_API int KIARA_LLVM_BITCODE_INLINE kr_dstring_append_char(kr_dstring_t *str, int value);
KIARA_API int KIARA_LLVM_BITCODE_INLINE kr_dstring_append(kr_dstring_t *str1, kr_dstring_t *str2);

#define kr_dstring_str(str) (str)->data
#define kr_dstring_length(str) (str)->size
#define kr_dstring_capacity(str) (str)->capacity

#ifdef __cplusplus
}
#endif

#endif /* KIARA_CDT_KR_DSTRING_H_INCLUDED */
