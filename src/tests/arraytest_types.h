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
 * arraytest_types.h
 *
 *  Created on: Nov 29, 2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_ARRAYTEST_TYPES_H_INCLUDED
#define KIARA_ARRAYTEST_TYPES_H_INCLUDED

#include <KIARA/kiara.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Application data structures */

typedef struct Data
{
    int size_boolean;
    int *array_boolean;

    int size_i8;
    int8_t *array_i8;

    int size_u8;
    uint8_t *array_u8;

    int size_i16;
    int16_t *array_i16;

    int size_u16;
    uint16_t *array_u16;

    int size_i32;
    int32_t *array_i32;

    int size_u32;
    uint32_t *array_u32;

    int size_i64;
    int64_t *array_i64;

    int size_u64;
    uint64_t *array_u64;

    int size_float;
    float *array_float;

    int size_double;
    double *array_double;
} Data;

static void initData(Data *data, size_t size)
{
    data->size_boolean = size;
    data->size_i8 = size;
    data->size_u8 = size;
    data->size_i16 = size;
    data->size_u16 = size;
    data->size_i32 = size;
    data->size_u32 = size;
    data->size_i64 = size;
    data->size_u64 = size;
    data->size_float = size;
    data->size_double = size;

    if (size == 0)
    {
        data->array_boolean = NULL;
        data->array_i8 = NULL;
        data->array_u8 = NULL;
        data->array_i16 = NULL;
        data->array_u16 = NULL;
        data->array_i32 = NULL;
        data->array_u32 = NULL;
        data->array_i64 = NULL;
        data->array_u64 = NULL;
        data->array_float = NULL;
        data->array_double = NULL;
    }
    else
    {
        data->array_boolean = malloc(sizeof(data->array_boolean[0])*size);
        data->array_i8 = malloc(sizeof(data->array_i8[0])*size);
        data->array_u8 = malloc(sizeof(data->array_u8[0])*size);
        data->array_i16 = malloc(sizeof(data->array_i16[0])*size);
        data->array_u16 = malloc(sizeof(data->array_u16[0])*size);
        data->array_i32 = malloc(sizeof(data->array_i32[0])*size);
        data->array_u32 = malloc(sizeof(data->array_u32[0])*size);
        data->array_i64 = malloc(sizeof(data->array_i64[0])*size);
        data->array_u64 = malloc(sizeof(data->array_u64[0])*size);
        data->array_float = malloc(sizeof(data->array_float[0])*size);
        data->array_double = malloc(sizeof(data->array_double[0])*size);
    }
}

static void destroyData(Data *data)
{
    free(data->array_boolean);
    free(data->array_i8);
    free(data->array_u8);
    free(data->array_i16);
    free(data->array_u16);
    free(data->array_i32);
    free(data->array_u32);
    free(data->array_i64);
    free(data->array_u64);
    free(data->array_float);
    free(data->array_double);
}

static KIARA_UserType * Data_Allocate(void)
{
    Data *data = malloc(sizeof(Data));
    initData(data, 0);
    return (KIARA_UserType *)data;
}

static void Data_Deallocate(KIARA_UserType *value)
{
    if (value)
    {
        destroyData((Data*)value);
        free(value);
    }
}

#ifdef __cplusplus
}
#endif


#endif /* KIARA_ARRAYTEST_TYPES_H_INCLUDED */
