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
 * kr_dbuffer.c
 *
 *  Created on: Aug 7, 2013
 *      Author: Dmitri Rubinstein
 */
#define KIARA_LIB
#include "kr_dbuffer.h"
#include <string.h>
#include <assert.h>

#define _MAX(x, y) ((x) < (y) ? (y) : (x))
#define _MIN(x, y) ((x) < (y) ? (x) : (y))

void kr_dbuffer_dont_free(void *ptr)
{
    /* Nothing to do */
}

void kr_dbuffer_init(kr_dbuffer_t *buf)
{
    assert(buf);
    buf->data = NULL;
    buf->capacity = 0;
    buf->size = 0;
    buf->free_fn = NULL;
}

void kr_dbuffer_init_from_data(kr_dbuffer_t *buf, void *data, size_t size, size_t capacity, kr_dbuffer_free_fn free_fn)
{
    assert(buf);
    assert(capacity >= size);
    buf->data = data;
    buf->size = size;
    buf->capacity = capacity;
    buf->free_fn = free_fn;
}

static void kr_dbuffer_free_data(kr_dbuffer_t *buf)
{
    if (buf->free_fn)
        buf->free_fn(buf->data);
    else
        free(buf->data);
}

static int kr_dbuffer_realloc_data(kr_dbuffer_t *buf, size_t new_size, size_t old_size)
{
    void *tmp;
    if (buf->free_fn)
    {
        tmp = malloc(new_size);
        if (tmp)
        {
            memcpy(tmp, buf->data, _MIN(new_size, old_size));
            buf->free_fn(buf->data);
        }
        else
            return 0;
    }
    else
    {
        tmp = realloc(buf->data, new_size);
        if (!tmp)
            return 0;
    }
    buf->data = tmp;
    buf->free_fn = NULL;
    return 1;
}

void kr_dbuffer_destroy(kr_dbuffer_t *buf)
{
    assert(buf);
    kr_dbuffer_free_data(buf);
    buf->data = NULL;
    buf->capacity = 0;
    buf->size = 0;
}

kr_dbuffer_t * kr_dbuffer_new(void)
{
    kr_dbuffer_t *buf = malloc(sizeof(kr_dbuffer_t));
    kr_dbuffer_init(buf);
    return buf;
}

kr_dbuffer_t * kr_dbuffer_new_from_data(void *data, size_t size, size_t capacity, kr_dbuffer_free_fn free_fn)
{
    kr_dbuffer_t *buf = malloc(sizeof(kr_dbuffer_t));
    kr_dbuffer_init_from_data(buf, data, size, capacity, free_fn);
    return buf;
}

void kr_dbuffer_delete(kr_dbuffer_t *buf)
{
    if (buf)
    {
        kr_dbuffer_destroy(buf);
        free(buf);
    }
}

char * kr_dbuffer_release(kr_dbuffer_t *buf)
{
    char *result;
    assert(buf);
    result = buf->data;
    buf->data = NULL;
    buf->capacity = 0;
    buf->size = 0;
    buf->free_fn = NULL;
    return result;
}

void kr_dbuffer_clear(kr_dbuffer_t *buf)
{
    assert(buf);
    buf->size = 0;
}

/** Returns 0 when failed */
int kr_dbuffer_reserve(kr_dbuffer_t *buf, size_t new_capacity)
{
    assert(buf);
    if (buf->capacity < new_capacity)
    {
        if (!kr_dbuffer_realloc_data(buf, new_capacity, buf->capacity))
            return 0;
        buf->capacity = new_capacity;
    }
    return 1;
}

static int kr_dbuffer_ensure_capacity(kr_dbuffer_t *buf, size_t new_capacity)
{
    if (buf->capacity < new_capacity)
    {
        size_t cap2 = buf->capacity * 2;
        new_capacity = _MAX(cap2, new_capacity);
        if (!kr_dbuffer_realloc_data(buf, new_capacity, buf->capacity))
            return 0;
        buf->capacity = new_capacity;
    }
    return 1;
}

static int kr_dbuffer_ensure_capacity_nocopy(kr_dbuffer_t *buf, size_t new_capacity)
{
    if (buf->capacity < new_capacity)
    {
        void *tmp;
        size_t cap2 = buf->capacity * 2;
        new_capacity = _MAX(cap2, new_capacity);
        kr_dbuffer_destroy(buf);
        tmp = malloc(new_capacity);
        if (!tmp)
            return 0;
        buf->data = tmp;
        buf->free_fn = NULL;
        buf->capacity = new_capacity;
    }
    return 1;
}

int kr_dbuffer_resize(kr_dbuffer_t *buf, size_t new_size)
{
    assert(buf);
    if (new_size == buf->size)
        return 1;
    if (new_size > buf->size)
        if (!kr_dbuffer_ensure_capacity(buf, new_size))
            return 0;
    buf->size = new_size;
    return 1;
}

int kr_dbuffer_resize_nocopy(kr_dbuffer_t *buf, size_t new_size)
{
    assert(buf);
    if (new_size == buf->size)
        return 1;
    if (new_size > buf->size)
        if (!kr_dbuffer_ensure_capacity_nocopy(buf, new_size))
            return 0;
    buf->size = new_size;
    return 1;
}

int kr_dbuffer_copy_mem(kr_dbuffer_t *dest, const void *src, size_t n)
{
    assert(dest);
    if (!src || !n)
        return 1;
    if (!kr_dbuffer_ensure_capacity_nocopy(dest, n))
        return 0;
    memcpy(dest->data, src, n);
    dest->size = n;
    return 1;
}

int kr_dbuffer_append_mem(kr_dbuffer_t *dest, const void *src, size_t n)
{
    assert(dest);
    if (!src || !n)
        return 1;
    if (!kr_dbuffer_ensure_capacity(dest, dest->size + n))
        return 0;
    memcpy(dest->data + dest->size, src, n);
    dest->size += n;
    return 1;
}

int kr_dbuffer_append_byte(kr_dbuffer_t *dest, int byte)
{
    assert(dest);
    if (!kr_dbuffer_ensure_capacity(dest, dest->size + 1))
        return 0;
    ((unsigned char*)dest->data)[dest->size] = byte;
    ++dest->size;
    return 1;
}

int kr_dbuffer_assign(kr_dbuffer_t *dest, const kr_dbuffer_t *src)
{
    assert(src);
    if (src->size == 0)
    {
        kr_dbuffer_clear(dest);
        return 1;
    }
    return kr_dbuffer_copy_mem(dest, src->data, src->size);
}

int kr_dbuffer_append(kr_dbuffer_t *dest, const kr_dbuffer_t *src)
{
    return kr_dbuffer_append_mem(dest, src ? src->data : NULL, src ? src->size : 0);
}

int kr_dbuffer_make_cstr(kr_dbuffer_t *buf)
{
    assert(buf);
    if (buf->size > 0 && buf->data[buf->size-1] == '\0')
        return 1;
    return kr_dbuffer_append_byte(buf, '\0');
}

int kr_dbuffer_move(kr_dbuffer_t *dest, kr_dbuffer_t *src)
{
    kr_dbuffer_destroy(dest);
    *dest = *src;
    kr_dbuffer_release(src);
    return 1;
}

void kr_dbuffer_swap(kr_dbuffer_t *a, kr_dbuffer_t *b)
{
    kr_dbuffer_t tmp;
    assert(a != NULL && b != NULL);
    tmp = *a;
    *a = *b;
    *b = tmp;
}
