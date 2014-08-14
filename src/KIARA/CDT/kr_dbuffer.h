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
 * kr_dbuffer.h
 *
 *  Created on: Aug 7, 2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_CDT_KR_DBUFFER_H_INCLUDED
#define KIARA_CDT_KR_DBUFFER_H_INCLUDED

#include <KIARA/Common/Config.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*kr_dbuffer_free_fn)(void * ptr);

typedef struct kr_dbuffer {
    char *data;
    size_t size;
    size_t capacity;
    kr_dbuffer_free_fn free_fn; /* if free_fn is NULL it was allocated by kr_dbuffer */
} kr_dbuffer_t;

/** Use kr_dbuffer_dont_free for specifying memory that should not be destroyed */
KIARA_API void kr_dbuffer_dont_free(void *ptr);

KIARA_API void kr_dbuffer_init(kr_dbuffer_t *buf);
/*
 * Pass kr_dbuffer_dont_free to free_fn parameter when data should be not freed.
 */
KIARA_API void kr_dbuffer_init_from_data(kr_dbuffer_t *buf, void *data, size_t size, size_t capacity, kr_dbuffer_free_fn free_fn);

KIARA_API void kr_dbuffer_destroy(kr_dbuffer_t *buf);
/** Allocates kr_dbuffer structure with malloc, then calls kr_dbuffer_init */
KIARA_API kr_dbuffer_t * kr_dbuffer_new(void);

/** Allocates kr_dbuffer structure with malloc, then calls kr_dbuffer_init_from_data */
KIARA_API kr_dbuffer_t * kr_dbuffer_new_from_data(void *data, size_t size, size_t capacity, kr_dbuffer_free_fn free_fn);

/** Calls kr_dbuffer_destroy, then frees kr_dbuffer structure with free. */
KIARA_API void kr_dbuffer_delete(kr_dbuffer_t *buf);
KIARA_API char * kr_dbuffer_release(kr_dbuffer_t *buf);
KIARA_API void kr_dbuffer_clear(kr_dbuffer_t *buf);
KIARA_API int kr_dbuffer_reserve(kr_dbuffer_t *buf, size_t new_capacity);
KIARA_API int kr_dbuffer_resize(kr_dbuffer_t *buf, size_t new_size);
/** Resize buffer but don't copy contents when memory needs to be reallocated */
KIARA_API int kr_dbuffer_resize_nocopy(kr_dbuffer_t *buf, size_t new_size);
KIARA_API int kr_dbuffer_copy_mem(kr_dbuffer_t *dest, const void *src, size_t n);
KIARA_API int kr_dbuffer_append_mem(kr_dbuffer_t *dest, const void *src, size_t n);
KIARA_API int kr_dbuffer_append_byte(kr_dbuffer_t *dest, int byte);
KIARA_API int kr_dbuffer_assign(kr_dbuffer_t *dest, const kr_dbuffer_t *src);

KIARA_API int kr_dbuffer_append(kr_dbuffer_t *dest, const kr_dbuffer_t *src);
/** Checks if buffer ends with '\0' and adds it if not */
KIARA_API int kr_dbuffer_make_cstr(kr_dbuffer_t *buf);

KIARA_API int kr_dbuffer_move(kr_dbuffer_t *dest, kr_dbuffer_t *src);

KIARA_API void kr_dbuffer_swap(kr_dbuffer_t *a, kr_dbuffer_t *b);

#define kr_dbuffer_data(buf) (buf)->data
#define kr_dbuffer_size(buf) (buf)->size
#define kr_dbuffer_capacity(buf) (buf)->capacity
#define kr_dbuffer_free_fn(buf) (buf)->free_fn

#ifdef __cplusplus
}
#endif

#endif /* KIARA_CDT_KR_DBUFFER_H_INCLUDED */
