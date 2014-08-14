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
 * user_types_dds.hpp
 *
 *  Created on: 16.01.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef USER_TYPES_DDS_HPP_INCLUDED
#define USER_TYPES_DDS_HPP_INCLUDED

#include <KIARA/kiara.h>
#include <KIARA/kiara_dds_cxx_macros.hpp>
#include <KIARA/kiara_cxx_macros.hpp>

namespace User
{

struct Sequences {
    DDS_LongSeq ls;
    DDS_FloatSeq fs;
};

class gps_position
{
public:
    DDS_Long degrees;
    DDS_Long minutes;
    DDS_Float seconds;

    gps_position() { }
    gps_position(DDS_Long _d, DDS_Long _m, DDS_Float _s) :
        degrees(_d), minutes(_m), seconds(_s)
    { }
};

class gps_direction
{
public:
    gps_position pos;
    DDS_Double distance;

    gps_direction() { }
    gps_direction(const gps_position &_pos, DDS_Double _distance) :
        pos(_pos), distance(_distance)
    { }
};

typedef struct {
   DDS_Char   flavor;
   DDS_Char   color;
   DDS_Long    charge;
   DDS_Double  mass;
   DDS_Double  x, y, z;
   DDS_Double px, py, pz;
} Quark;

typedef struct {
    DDS_Float x;
    DDS_Float y;
} Vec2f;

typedef struct Linef {
    Vec2f a;
    Vec2f b;
} Linef;

/* IntList and IntList* */

typedef struct IntList {
    struct IntList *next;
    DDS_Long data;
} IntList;

/* float mat[4][4] and Matrix44 */

typedef struct Matrix44 {
    DDS_Float mat[4][4];
} Matrix44;

/* IntArray and FloatArray4 */

typedef DDS_Long IntArray[];
typedef DDS_Float FloatArray4[4];

/* Data */

typedef struct Data {
    DDS_Long i;
    DDS_Float f;
} Data;

/* MatrixPos */

typedef struct MatrixPos {
    DDS_Long row;
    DDS_Long column;
} MatrixPos;

/* Linef mat1k[1000][1000] and Matrix1k */

typedef struct Matrix1k {
    Linef mat[1000][1000];
    MatrixPos pos;
} Matrix1k;

} // namespace User

#endif /* USER_TYPES_DDS_HPP_INCLUDED */
