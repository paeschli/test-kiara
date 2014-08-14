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
 * binaryio.h
 *
 *  Created on: Aug 21, 2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMPONENTS_BINARYIO_H_INCLUDED
#define KIARA_COMPONENTS_BINARYIO_H_INCLUDED

#include <KIARA/kiara.h>
#include <KIARA/kiara_security.h>
#include <stdlib.h>
#include <stdint.h>
#include <KIARA/CDT/kr_dbuffer.h>

#ifdef __cplusplus
extern "C" {
#endif

KIARA_BinaryStream * createOutputStream(KIARA_Connection *conn) KIARA_ALWAYS_INLINE;
KIARA_BinaryStream * createInputStream(KIARA_Connection *conn) KIARA_ALWAYS_INLINE;
void freeStream(KIARA_BinaryStream *stream) KIARA_ALWAYS_INLINE;

size_t getStreamSize(KIARA_BinaryStream *stream) KIARA_ALWAYS_INLINE;
void * getStreamData(KIARA_BinaryStream *stream) KIARA_ALWAYS_INLINE;
void setStreamData(KIARA_BinaryStream *stream, void *data, size_t dataSize) KIARA_ALWAYS_INLINE;

kr_dbuffer_t * getStreamBuffer(KIARA_BinaryStream *stream);

/* Note that after setStreamBuffer, stream takes ownership over the passed buffer */
void setStreamBuffer(KIARA_BinaryStream *stream, kr_dbuffer_t *buf) KIARA_ALWAYS_INLINE;
void copyStreamToBuffer(kr_dbuffer_t *dest, KIARA_BinaryStream *src) KIARA_ALWAYS_INLINE;
void copyBufferToStream(KIARA_BinaryStream *dest, kr_dbuffer_t *src) KIARA_ALWAYS_INLINE;

KIARA_Result writeStructBeginAsBinary(KIARA_BinaryStream *out, const char *name) KIARA_ALWAYS_INLINE;
KIARA_Result writeStructEndAsBinary(KIARA_BinaryStream *out) KIARA_ALWAYS_INLINE;
KIARA_Result writeFieldBeginAsBinary(KIARA_BinaryStream *out, const char *name) KIARA_ALWAYS_INLINE;
KIARA_Result writeFieldEndAsBinary(KIARA_BinaryStream *out) KIARA_ALWAYS_INLINE;

KIARA_Result readStructBeginAsBinary(KIARA_BinaryStream *in) KIARA_ALWAYS_INLINE;
KIARA_Result readStructEndAsBinary(KIARA_BinaryStream *in) KIARA_ALWAYS_INLINE;
KIARA_Result readFieldBeginAsBinary(KIARA_BinaryStream *in, const char *name) KIARA_ALWAYS_INLINE;
KIARA_Result readFieldEndAsBinary(KIARA_BinaryStream *in) KIARA_ALWAYS_INLINE;

KIARA_Result writeArrayBeginAsBinary(KIARA_BinaryStream *out, size_t size) KIARA_ALWAYS_INLINE;
KIARA_Result writeArrayEndAsBinary(KIARA_BinaryStream *out) KIARA_ALWAYS_INLINE;

KIARA_Result readArrayBeginAsBinary(KIARA_BinaryStream *in, size_t *size) KIARA_ALWAYS_INLINE;
KIARA_Result readArrayEndAsBinary(KIARA_BinaryStream *in) KIARA_ALWAYS_INLINE;

/* Write boolean type to message */
KIARA_Result writeTypeAsBinary_boolean(KIARA_BinaryStream *out, int value) KIARA_ALWAYS_INLINE;
/* Read boolean type from message */
KIARA_Result readTypeAsBinary_boolean(KIARA_BinaryStream *in, int *value) KIARA_ALWAYS_INLINE;

/* Write i8 type to message */
KIARA_Result writeTypeAsBinary_i8(KIARA_BinaryStream *out, int8_t value) KIARA_ALWAYS_INLINE;
/* Read i8 type from message */
KIARA_Result readTypeAsBinary_i8(KIARA_BinaryStream *in, int8_t *value) KIARA_ALWAYS_INLINE;

/* Write u8 type to message */
KIARA_Result writeTypeAsBinary_u8(KIARA_BinaryStream *out, uint8_t value) KIARA_ALWAYS_INLINE;
/* Read u8 type from message */
KIARA_Result readTypeAsBinary_u8(KIARA_BinaryStream *in, uint8_t *value) KIARA_ALWAYS_INLINE;

/* Write i16 type to message */
KIARA_Result writeTypeAsBinary_i16(KIARA_BinaryStream *out, int16_t value) KIARA_ALWAYS_INLINE;
/* Read i16 type from message */
KIARA_Result readTypeAsBinary_i16(KIARA_BinaryStream *in, int16_t *value) KIARA_ALWAYS_INLINE;

/* Write u16 type to message */
KIARA_Result writeTypeAsBinary_u16(KIARA_BinaryStream *out, uint16_t value) KIARA_ALWAYS_INLINE;
/* Read u16 type from message */
KIARA_Result readTypeAsBinary_u16(KIARA_BinaryStream *in, uint16_t *value) KIARA_ALWAYS_INLINE;

/* Write i32 type to message */
KIARA_Result writeTypeAsBinary_i32(KIARA_BinaryStream *out, int32_t value) KIARA_ALWAYS_INLINE;
/* Read i32 type from message */
KIARA_Result readTypeAsBinary_i32(KIARA_BinaryStream *in, int32_t *value) KIARA_ALWAYS_INLINE;

/* Write i32 type to message */
KIARA_Result writeTypeAsBinary_u32(KIARA_BinaryStream *out, uint32_t value) KIARA_ALWAYS_INLINE;
/* Read i32 type from message */
KIARA_Result readTypeAsBinary_u32(KIARA_BinaryStream *in, uint32_t *value) KIARA_ALWAYS_INLINE;

/* Write i32 type to message */
KIARA_Result writeTypeAsBinary_i64(KIARA_BinaryStream *out, int64_t value) KIARA_ALWAYS_INLINE;
/* Read i32 type from message */
KIARA_Result readTypeAsBinary_i64(KIARA_BinaryStream *in, int64_t *value) KIARA_ALWAYS_INLINE;

/* Write i32 type to message */
KIARA_Result writeTypeAsBinary_u64(KIARA_BinaryStream *out, uint64_t value) KIARA_ALWAYS_INLINE;
/* Read i32 type from message */
KIARA_Result readTypeAsBinary_u64(KIARA_BinaryStream *in, uint64_t *value) KIARA_ALWAYS_INLINE;

/* Write float type to message */
KIARA_Result writeTypeAsBinary_float(KIARA_BinaryStream *out, float value) KIARA_ALWAYS_INLINE;
/* Read float type from message */
KIARA_Result readTypeAsBinary_float(KIARA_BinaryStream *in, float *value) KIARA_ALWAYS_INLINE;

/* Write float type to message */
KIARA_Result writeTypeAsBinary_double(KIARA_BinaryStream *out, double value) KIARA_ALWAYS_INLINE;
/* Read float type from message */
KIARA_Result readTypeAsBinary_double(KIARA_BinaryStream *in, double *value) KIARA_ALWAYS_INLINE;

/* Write zero terminated string to message */
KIARA_Result writeTypeAsBinary_string(KIARA_BinaryStream *out, const char * value) KIARA_ALWAYS_INLINE;

/* Read zero terminated string from message */
KIARA_Result readTypeAsBinary_string(KIARA_BinaryStream *in, char ** value) KIARA_ALWAYS_INLINE;

/* Write zero terminated string to message with user-defined getter method */
KIARA_Result writeTypeAsBinary_user_string(KIARA_BinaryStream *out, KIARA_UserType *value, KIARA_GetCString getCStringFunc) KIARA_ALWAYS_INLINE;

/* Read zero terminated string from message with user-defined setter method */
KIARA_Result readTypeAsBinary_user_string(KIARA_BinaryStream *in, KIARA_UserType *value, KIARA_SetCString setCStringFunc) KIARA_ALWAYS_INLINE;


KIARA_Result encryptStream(KIARA_Connection *conn, KIARA_BinaryStream *stream, const char *keyName) KIARA_ALWAYS_INLINE;
KIARA_Result decryptStream(KIARA_Connection *conn, KIARA_BinaryStream *stream, const char *keyName) KIARA_ALWAYS_INLINE;

/*
KIARA_Bool isErrorResponse(KIARA_BinaryStream *dest) KIARA_ALWAYS_INLINE;

KIARA_Result readGenericError(KIARA_BinaryStream *dest, KIARA_UserType *userException, KIARA_SetGenericError setGenericErrorFunc) KIARA_ALWAYS_INLINE;
KIARA_Result writeGenericError(KIARA_BinaryStream *dest, KIARA_UserType *userException, KIARA_GetGenericError getGenericErrorFunc) KIARA_ALWAYS_INLINE;
*/

#ifdef __cplusplus
}
#endif

#endif /* KIARA_COMPONENTS_BINARYIO_H_INCLUDED */
