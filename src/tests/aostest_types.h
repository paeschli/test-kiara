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
 * aostest_types.h
 *
 *  Created on: Dec 2, 2013
 *      Author: Dmitri Rubinstein
 */

/*
 * This file contains application code and data structures independent of KIARA framework
 */

#ifndef AOSTEST_TYPES_H_INCLUDED
#define AOSTEST_TYPES_H_INCLUDED

#include <KIARA/kiara.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Location */

typedef struct Vec3f {
    float x;
    float y;
    float z;
} Vec3f;

typedef struct Quatf {
    float r; /* real part */
    Vec3f v; /* imaginary vector */
} Quatf;

typedef struct Location {
    Vec3f position;
    Quatf rotation;
} Location;

/* Data */

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
        loclist->locations = (Location *)malloc(sizeof(loclist->locations[0])*size);
    }
}

static void destroyLocationList(LocationList *loclist)
{
    loclist->num_locations = 0;
    free(loclist->locations);
    loclist->locations = NULL;
}

#ifdef __cplusplus
}
#endif

#endif
