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
 * SerializerConfig.cpp
 *
 *  Created on: Oct 31, 2013
 *      Author: rubinste
 */

#define KIARA_LIB
#include "KIARA/IRGen/IRGen.hpp"

namespace KIARA
{

IRGen::SerializerConfig IRGen::_defaultSerializerConfig(
    /*serializerNamePrefix*/"writeMessage_",
    /*writeStructBeginName*/"writeStructBegin",
    /*writeStructEndName*/"writeStructEnd",
    /*writeFieldBeginName*/"writeFieldBegin",
    /*writeFieldEndName*/"writeFieldEnd",
    /*writeUserTypeName*/"writeUserType",
    /*writeArrayBeginName*/"writeArrayBegin",
    /*writeArrayEndName*/"writeArrayEnd",
    /*writeArrayTypeName*/"writeArrayType");

IRGen::DeserializerConfig IRGen::_defaultDeserializerConfig(
    /*deserializerNamePrefix*/"readMessage_",
    /*readStructBeginName*/"readStructBegin",
    /*readStructEndName*/"readStructEnd",
    /*readFieldBeginName*/"readFieldBegin",
    /*readFieldEndName*/"readFieldEnd",
    /*readUserTypeName*/"readUserType",
    /*readArrayBeginName*/"readArrayBegin",
    /*readArrayEndName*/"readArrayEnd",
    /*readArrayTypeName*/"readArrayType");

IRGen::SerializerConfig IRGen::_binarySerializerConfig(
    /*serializerNamePrefix*/"writeTypeAsBinary_",
    /*writeStructBeginName*/"writeStructBeginAsBinary",
    /*writeStructEndName*/"writeStructEndAsBinary",
    /*writeFieldBeginName*/"writeFieldBeginAsBinary",
    /*writeFieldEndName*/"writeFieldEndAsBinary",
    /*writeUserTypeName*/"writeUserTypeAsBinary",
    /*writeArrayBeginName*/"writeArrayBeginAsBinary",
    /*writeArrayEndName*/"writeArrayEndAsBinary",
    /*writeArrayTypeName*/"writeArrayTypeAsBinary");

IRGen::DeserializerConfig IRGen::_binaryDeserializerConfig(
    /*deserializerNamePrefix*/"readTypeAsBinary_",
    /*readStructBeginName*/"readStructBeginAsBinary",
    /*readStructEndName*/"readStructEndAsBinary",
    /*readFieldBeginName*/"readFieldBeginAsBinary",
    /*readFieldEndName*/"readFieldEndAsBinary",
    /*readUserTypeName*/"readUserTypeAsBinary",
    /*readArrayBeginName*/"readArrayBeginAsBinary",
    /*readArrayEndName*/"readArrayEndAsBinary",
    /*readArrayTypeName*/"readArrayTypeAsBinary");

} // namespace KIARA
