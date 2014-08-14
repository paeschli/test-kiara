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
#ifndef calctest_defs_h_included
#define calctest_defs_h_included

/* For tagging */
#include <KIARA/kiara_pp_annotation.h>

/* For kr_dstring_t type */
#include <KIARA/CDT/kr_dstring.h>

/* For KIARA_DECL_FUNC_OBJ and KIARA_FUNC_OBJ macros */
#include <KIARA/kiara.h>

#include <string.h>

static size_t addi(size_t value)
{
    size_t x = sizeof(value);
    return x+1;
}


static int8_t add8(int8_t value)
{
    int8_t x = sizeof(value);
    return x+1;
}

/* Access functions for kr_dstring_t type */

static int dstring_SetCString(KIARA_UserType *ustr, const char *cstr)
{
    int result = kr_dstring_assign_str((kr_dstring_t*)ustr, cstr);
    return result ? 0 : 1;
}

static int dstring_GetCString(KIARA_UserType *ustr, const char **cstr)
{
    *cstr = kr_dstring_str((kr_dstring_t*)ustr);
    return 0;
}

/* Location */

typedef struct Vec3f {
    float x;
    float y;
    float z;
} Vec3f;

typedef struct Quatf {
    float r; // real part
    Vec3f v; // imaginary vector
} Quatf;

typedef struct Location {
    Vec3f position;
    Quatf rotation;
} Location;


typedef struct LocationList
{
    int num_locations;
    Location *locations;
} LocationList;

static void initLocationList(LocationList *loclist, size_t size)
{
    loclist->num_locations = size;
    if (size == 0)
    {
        loclist->locations = NULL;
    }
    else
    {
        loclist->locations = malloc(sizeof(loclist->locations[0])*size);
    }
}

static void destroyLocationList(LocationList *loclist)
{
    loclist->num_locations = 0;
    free(loclist->locations);
    loclist->locations = NULL;
}

static void copyLocationList(LocationList *dest, const LocationList *src)
{
    if (dest->num_locations != src->num_locations)
    {
        destroyLocationList(dest);
        initLocationList(dest, src->num_locations);
    }
    memcpy(dest->locations, src->locations, sizeof(src->locations[0]) * src->num_locations);
}

static KIARA_UserType * LocationList_Allocate(void)
{
    LocationList *loclist = malloc(sizeof(LocationList));
    initLocationList(loclist, 0);
    return (KIARA_UserType *)loclist;
}

static void LocationList_Deallocate(KIARA_UserType *value)
{
    if (value)
    {
        destroyLocationList((LocationList*)value);
        free(value);
    }
}

typedef int64_t Integer;

typedef struct Test {
    int8_t a;
    uint8_t b;
    int16_t c;
    uint16_t d;
    int32_t e;
    uint32_t f;
    int64_t g;
    uint64_t h;
    ssize_t i;
    Integer xi;
    int j;
    long k;
    short l;
    unsigned short m;
    const char *s;
} Test;

/* Registration */

#if 1

kiara_declare_func(Calc_Add, int * result_value result, int a, int b)
kiara_declare_service(On_Calc_Add, int * result_value result, int a, int b)

kiara_declare_func(Calc_Add_Float, float * result_value result, float a, float b)
kiara_declare_func(Calc_String_To_Int32, int * result_value result, const char *s)
kiara_declare_func(Calc_Int32_To_String, char ** result_value result, int i)
kiara_declare_func(Calc_DString_To_Int32, int * result_value result, const kr_dstring_t *s)
kiara_declare_func(Calc_Int32_To_DString, kr_dstring_t * result_value result, int i)

kiara_declare_func(GetLocation, Location * result_value result)
kiara_declare_func(SetLocation, const Location * location)

kiara_declare_service(OnGetLocation, Location * result_value result)
kiara_declare_service(OnSetLocation, const Location * location)
#endif

#if 1
kiara_declare_opaque_object(kr_dstring_t,
    kiara_user_api(SetCString, dstring_SetCString),
    kiara_user_api(GetCString, dstring_GetCString))
#endif

kiara_declare_struct_with_api(LocationList,
    kiara_struct_array_member(locations, num_locations),
    kiara_user_api(AllocateType, LocationList_Allocate),
    kiara_user_api(DeallocateType, LocationList_Deallocate))

kiara_declare_func(AOSTest_GetLocations, LocationList * result_value locations)
kiara_declare_func(AOSTest_SetLocations, const LocationList *locations)

#if 1
kiara_declare_object(Test)
#endif
#endif
