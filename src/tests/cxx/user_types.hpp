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
/*
 * user_types.hpp
 *
 *  Created on: 11.12.2012
 *      Author: Dmitri Rubinstein
 */

#ifndef USER_TYPES_HPP_INCLUDED
#define USER_TYPES_HPP_INCLUDED

#include <KIARA/kiara.h>
#include <boost/preprocessor/cat.hpp>

namespace User
{

class gps_position
{
public:
    int degrees;
    int minutes;
    float seconds;

    gps_position() { }
    gps_position(int _d, int _m, float _s) :
        degrees(_d), minutes(_m), seconds(_s)
    { }
};

class gps_direction
{
public:
    gps_position pos;
    double distance;

    gps_direction() { }
    gps_direction(const gps_position &_pos, double _distance) :
        pos(_pos), distance(_distance)
    { }
};

typedef struct {
   char   flavor;
   char   color;
   int    charge;
   double mass;
   double x, y, z;
   double px, py, pz;
} Quark;

typedef struct {
    float x;
    float y;
} Vec2f;

typedef struct Linef {
    Vec2f a;
    Vec2f b;
} Linef;

/* IntList and IntList* */

typedef struct IntList {
    struct IntList *next;
    int data;
} IntList;

/* float mat[4][4] and Matrix44 */

typedef struct Matrix44 {
    float mat[4][4];
} Matrix44;

/* IntArray and FloatArray4 */

typedef int IntArray[];
typedef float FloatArray4[4];

/* Data */

typedef struct Data {
    int i;
    float f;
} Data;

/* MatrixPos */

typedef struct MatrixPos {
    int row;
    int column;
} MatrixPos;

/* Linef mat1k[1000][1000] and Matrix1k */

typedef struct Matrix1k {
    Linef mat[1000][1000];
    MatrixPos pos;
} Matrix1k;

}

#endif /* USER_TYPES_HPP_INCLUDED */
