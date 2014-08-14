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
 * dstring.c
 *
 *  Created on: 02.07.2011
 *      Author: Dmitri Rubinstein
 */
#define KIARA_LIB
#include "kr_dstring.h"
#include <string.h>
#include <assert.h>

#define _MAX(x, y) ((x) < (y) ? (y) : (x))

void kr_dstring_init(kr_dstring_t *str)
{
    assert(str);
    str->data = NULL;
    str->capacity = 0;
    str->size = 0;
}

void kr_dstring_destroy(kr_dstring_t *str)
{
    assert(str);
    free(str->data);
    str->data = NULL;
    str->capacity = 0;
    str->size = 0;
}

kr_dstring_t * kr_dstring_new(void)
{
    kr_dstring_t *str = malloc(sizeof(kr_dstring_t));
    kr_dstring_init(str);
    return str;
}

void kr_dstring_delete(kr_dstring_t *str)
{
    if (str)
    {
        kr_dstring_destroy(str);
        free(str);
    }
}

char * kr_dstring_release(kr_dstring_t *str)
{
    char *result;
    assert(str);
    result = str->data;
    str->data = NULL;
    str->capacity = 0;
    str->size = 0;
    return result;
}

void kr_dstring_clear(kr_dstring_t *str)
{
    assert(str);
    str->size = 0;
    if (str->data)
        *str->data = '\0';
}

/** Returns 0 when failed */
int kr_dstring_reserve(kr_dstring_t *str, size_t new_capacity)
{
    assert(str);
    if (str->capacity != new_capacity && new_capacity >= str->size)
    {
        void *tmp = realloc(str->data, new_capacity+1);
        if (!tmp)
            return 0;
        str->data = tmp;
        str->capacity = new_capacity;
    }
    return 1;
}

static int kr_dstring_ensure_capacity(kr_dstring_t *str, size_t new_capacity)
{
    if (str->capacity < new_capacity)
    {
        void *tmp;
        size_t cap2 = str->capacity * 2;
        new_capacity = _MAX(cap2, new_capacity);
        tmp = realloc(str->data, new_capacity+1);
        if (!tmp)
            return 0;
        str->data = tmp;
        str->capacity = new_capacity;
    }
    return 1;
}

static int kr_dstring_ensure_capacity_nocopy(kr_dstring_t *str, size_t new_capacity)
{
    if (str->capacity < new_capacity)
    {
        void *tmp;
        size_t cap2 = str->capacity * 2;
        new_capacity = _MAX(cap2, new_capacity);
        kr_dstring_destroy(str);
        tmp = malloc(new_capacity+1);
        if (!tmp)
            return 0;
        str->data = tmp;
        str->capacity = new_capacity;
    }
    return 1;
}

int kr_dstring_assign_str(kr_dstring_t *str, const char *value)
{
    size_t len;
    assert(str);
    if (value == 0)
    {
        kr_dstring_clear(str);
        return 1;
    }
    len = strlen(value);
    if (!kr_dstring_ensure_capacity_nocopy(str, len))
        return 0;
    strcpy(str->data, value);
    str->size = len;
    return 1;
}

int kr_dstring_append_strn(kr_dstring_t *str, const char *value, size_t n)
{
    assert(str);
    if (!value || !n)
        return 1;
    if (!kr_dstring_ensure_capacity(str, str->size + n))
        return 0;
    memcpy(str->data + str->size, value, n);
    str->size += n;
    str->data[str->size] = '\0';
    return 1;
}

int kr_dstring_append_str(kr_dstring_t *str, const char *value)
{
    assert(str);
    if (!value)
        return 1;
    return kr_dstring_append_strn(str, value, strlen(value));
}

int kr_dstring_append_char(kr_dstring_t *str, int value)
{
    assert(str);
    if (!kr_dstring_ensure_capacity(str, str->size + 1))
        return 0;
    str->data[str->size] = value;
    str->size += 1;
    str->data[str->size] = '\0';
    return 1;
}

int kr_dstring_append(kr_dstring_t *str1, kr_dstring_t *str2)
{
    assert(str1 && str2);
    return kr_dstring_append_strn(str1, str2->data, str2->size);
}
