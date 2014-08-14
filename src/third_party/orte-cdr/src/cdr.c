/*
 *  $Id: cdr.c,v 0.0.0.1                2004/11/26
 *
 *  DEBUG:  section 5                   CDR codeing
 *
 *  -------------------------------------------------------------------
 *                                ORTE
 *                      Open Real-Time Ethernet
 *
 *                      Copyright (C) 2001-2006
 *  Department of Control Engineering FEE CTU Prague, Czech Republic
 *                      http://dce.felk.cvut.cz
 *                      http://www.ocera.org
 *
 *  Author: 		 Petr Smolik	petr@smoliku.cz
 *  Advisor: 		 Pavel Pisa
 *  Project Responsible: Zdenek Hanzalek
 *  --------------------------------------------------------------------
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
 *  This module maintains a hash table of key/value pairs.
 *  Keys can be strings of any size, or numbers up to size
 *  unsigned long (HASHKEYTYPE).
 *  Values should be a pointer to some data you wish to store.
 *
 *  Original of source was from ORBit: A CORBA v2.2 ORB
 *
 */

#include "orte/cdr.h"
#include "orte/rtps_endian.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define _MAX(x, y) ((x) < (y) ? (y) : (x))

CORBA_boolean CDR_buffer_ensure_capacity(CDR_Codec *codec, size_t new_capacity)
{
    if (codec->buf_capacity < new_capacity)
    {
        void *tmp;
        size_t cap2;

        if (!codec->release_buffer)
            return CORBA_FALSE;
        cap2 = codec->buf_capacity * 2;
        new_capacity = _MAX(cap2, new_capacity);
        tmp = REALLOC(codec->buffer, new_capacity);
        if (!tmp)
            return CORBA_FALSE;
        codec->buffer = tmp;
        codec->buf_capacity = new_capacity;
    }
    return CORBA_TRUE;
}

CORBA_boolean CDR_buffer_ensure_capacity_nocopy(CDR_Codec *codec, size_t new_capacity)
{
    if (codec->buf_capacity < new_capacity)
    {
        void *tmp;
        size_t cap2;

        if (!codec->release_buffer)
            return CORBA_FALSE;
        cap2 = codec->buf_capacity * 2;
        new_capacity = _MAX(cap2, new_capacity);
        CDR_codec_release_buffer(codec);
        tmp = MALLOC(new_capacity);
        if (!tmp)
            return CORBA_FALSE;
        codec->buffer = tmp;
        codec->buf_capacity = new_capacity;
    }
    return CORBA_TRUE;
}

static
CORBA_boolean CDR_buffer_resize(CDR_Codec *codec, size_t new_size)
{
    assert(codec);
    if (new_size == codec->buf_size)
        return CORBA_TRUE;
    if (new_size > codec->buf_size)
        if (CDR_buffer_ensure_capacity(codec, new_size) == CORBA_FALSE)
            return CORBA_FALSE;
    codec->buf_size = new_size;
    return CORBA_TRUE;
}

static
CORBA_boolean CDR_buffer_reserve(CDR_Codec *codec, size_t new_capacity)
{
    assert(codec);
    if (codec->buf_capacity < new_capacity)
    {
        void *tmp;
        if (!codec->release_buffer)
            return CORBA_FALSE;

        tmp = REALLOC(codec->buffer, new_capacity);
        if (!tmp)
            return CORBA_FALSE;
        codec->buffer = tmp;
        codec->buf_capacity = new_capacity;
    }
    return CORBA_TRUE;
}

CORBA_boolean
CDR_buffer_init(CDR_Codec *codec, const size_t size)
{

    if(codec->release_buffer) {
        FREE(codec->buffer);
	}

    codec->buffer = size > 0 ? (CORBA_octet *)MALLOC(size) : NULL;
    codec->buf_capacity = size;
    codec->buf_size = 0;
    codec->pos = 0;
    codec->release_buffer=CORBA_TRUE;

    return CORBA_TRUE;
}

void
CDR_buffer_reset(CDR_Codec *codec)
{
    codec->buf_size = 0;
}

void
CDR_buffer_reset_position(CDR_Codec *codec)
{
    codec->pos = 0;
}

CORBA_boolean
CDR_buffer_puts(CDR_Codec *codec, const void *data, const size_t len)
{
    const size_t required_size = codec->pos+len;
	if(required_size > codec->buf_size) {
	    if (CDR_buffer_resize(codec, required_size) == CORBA_FALSE)
	        return CORBA_FALSE;
	}

	memcpy(&codec->buffer[codec->pos], data, len);
	codec->pos+=len;

	return CORBA_TRUE;
}

CORBA_boolean
CDR_buffer_gets(CDR_Codec *codec, void *dest, const size_t len)
{
	if(codec->pos+len > codec->buf_size) {
		return CORBA_FALSE;
	}

	memcpy(dest, &codec->buffer[codec->pos], len);
	codec->pos+=len;

	return CORBA_TRUE;
}

CORBA_boolean
CDR_buffer_put(CDR_Codec *codec, void *datum)
{
    const size_t required_size = codec->pos+1;

	if(required_size > codec->buf_size) {
        if (CDR_buffer_resize(codec, required_size) == CORBA_FALSE)
            return(CORBA_FALSE);
	}

	codec->buffer[codec->pos++]=*(unsigned char *)datum;
	return CORBA_TRUE;
}

CORBA_boolean
CDR_buffer_get(CDR_Codec *codec, void *dest)
{
	if(codec->pos+1 > codec->buf_size) {
		return(CORBA_FALSE);
	}

	*(CORBA_octet *)dest=codec->buffer[codec->pos++];
	return CORBA_TRUE;
}

#define CDR_buffer_put2(codec, datum) CDR_buffer_putn(codec, datum, 2)
#define CDR_buffer_put4(codec, datum) CDR_buffer_putn(codec, datum, 4)
#define CDR_buffer_put8(codec, datum) CDR_buffer_putn(codec, datum, 8)
#define CDR_buffer_put16(codec, datum) CDR_buffer_putn(codec, datum, 16)
#define CDR_buffer_get2(codec, dest) CDR_buffer_getn(codec, dest, 2)
#define CDR_buffer_get4(codec, dest) CDR_buffer_getn(codec, dest, 4)
#define CDR_buffer_get8(codec, dest) CDR_buffer_getn(codec, dest, 8)
#define CDR_buffer_get16(codec, dest) CDR_buffer_getn(codec, dest, 16)

static CORBA_boolean
CDR_buffer_getn(CDR_Codec *codec, void *dest, int bsize)
{
	codec->pos = (unsigned long)ALIGN_ADDRESS(codec->pos, bsize);
	if(codec->host_endian==codec->data_endian)
		memcpy(dest, codec->buffer + codec->pos, bsize);
	else
		rtps_byteswap(dest, codec->buffer + codec->pos, bsize);
	codec->pos += bsize;

	return CORBA_TRUE;
}

static CORBA_boolean
CDR_buffer_putn(CDR_Codec *codec, void *datum, int bsize)
{
    unsigned long forward,i;
    size_t required_size;

	forward = (unsigned long)ALIGN_ADDRESS(codec->pos, bsize);
	required_size = forward+bsize;
	if (required_size > codec->buf_size) {
        if (CDR_buffer_resize(codec, required_size) == CORBA_FALSE)
            return CORBA_FALSE;
	}

	i = codec->pos;
    while(forward > i)
        codec->buffer[i++] = '\0';

	codec->pos = forward;
	if(codec->host_endian==codec->data_endian)
		memcpy(codec->buffer + codec->pos, datum, bsize);
	else
		rtps_byteswap(codec->buffer + codec->pos, datum, bsize);
	codec->pos += bsize;

	return CORBA_TRUE;
}

#define CDR_swap2(d,s) rtps_byteswap((d), (s), 2)
#define CDR_swap4(d,s) rtps_byteswap((d), (s), 4)
#define CDR_swap8(d,s) rtps_byteswap((d), (s), 8)
#define CDR_swap16(d,s) rtps_byteswap((d), (s), 16)

INLINE CORBA_boolean
CDR_put_short(CDR_Codec *codec, CORBA_short s)
{
	return CDR_buffer_put2(codec, &s);
}

INLINE CORBA_boolean
CDR_get_short(CDR_Codec *codec, CORBA_short *s)
{
	return CDR_buffer_get2(codec, s);
}

INLINE CORBA_boolean
CDR_put_ushort(CDR_Codec *codec, CORBA_unsigned_short us)
{
	return CDR_buffer_put2(codec, &us);
}

INLINE CORBA_boolean
CDR_get_ushort(CDR_Codec *codec, CORBA_unsigned_short *us)
{
	return CDR_buffer_get2(codec, us);
}

INLINE CORBA_boolean
CDR_put_long(CDR_Codec *codec, CORBA_long l)
{
	return CDR_buffer_put4(codec, &l);
}

INLINE CORBA_boolean
CDR_get_long(CDR_Codec *codec, CORBA_long *l)
{
	return CDR_buffer_get4(codec, l);
}

INLINE CORBA_boolean
CDR_put_ulong(CDR_Codec *codec, CORBA_unsigned_long ul)
{
	return CDR_buffer_put4(codec, &ul);
}

INLINE CORBA_boolean
CDR_get_ulong(CDR_Codec *codec, CORBA_unsigned_long *ul)
{
	return CDR_buffer_get4(codec, ul);
}

INLINE CORBA_boolean
CDR_get_long_long(CDR_Codec *codec, CORBA_long_long *ul)
{
	return CDR_buffer_get8(codec, ul);
}

INLINE CORBA_boolean
CDR_put_long_long(CDR_Codec *codec, CORBA_long_long ll)
{
	return CDR_buffer_put8(codec, &ll);
}

INLINE CORBA_boolean
CDR_put_ulong_long(CDR_Codec *codec, CORBA_unsigned_long_long ll)
{
	return CDR_buffer_put8(codec, &ll);
}

INLINE CORBA_boolean
CDR_get_ulong_long(CDR_Codec *codec, CORBA_unsigned_long_long *ull)
{
	return CDR_buffer_get8(codec, ull);
}

INLINE CORBA_boolean
CDR_put_float(CDR_Codec *codec, CORBA_float f)
{
	return CDR_buffer_put4(codec, &f);
}

INLINE CORBA_boolean
CDR_get_float(CDR_Codec *codec, CORBA_float *f)
{
	return CDR_buffer_get4(codec, f);
}

INLINE CORBA_boolean
CDR_put_double(CDR_Codec *codec, CORBA_double d)
{
	return CDR_buffer_put8(codec, &d);
}

INLINE CORBA_boolean
CDR_get_double(CDR_Codec *codec, CORBA_double *d)
{
	return CDR_buffer_get8(codec, d);
}

INLINE CORBA_boolean
CDR_put_long_double(CDR_Codec *codec, CORBA_long_double ld)
{
	return CDR_buffer_put16(codec, &ld);
}

INLINE CORBA_boolean
CDR_put_octet(CDR_Codec *codec, CORBA_octet datum)
{
	return CDR_buffer_put(codec, &datum);
}

INLINE CORBA_boolean
CDR_get_octet(CDR_Codec *codec, CORBA_octet *datum)
{
	return(CDR_buffer_get(codec, datum));
}

INLINE CORBA_boolean
CDR_put_octets(CDR_Codec *codec, void *data, unsigned long len)
{
	return CDR_buffer_puts(codec, data, len);
}

INLINE CORBA_boolean
CDR_put_char(CDR_Codec *codec, CORBA_char c)
{
	return CDR_buffer_put(codec, &c);
}

INLINE CORBA_boolean
CDR_get_char(CDR_Codec *codec, CORBA_char *c)
{
	return CDR_buffer_get(codec, c);
}

INLINE CORBA_boolean
CDR_put_boolean(CDR_Codec *codec, CORBA_boolean datum)
{
	datum = datum&&1;
	return CDR_buffer_put(codec, &datum);
}

INLINE CORBA_boolean
CDR_get_boolean(CDR_Codec *codec, CORBA_boolean *b)
{
	return CDR_buffer_get(codec, b);
}

CORBA_boolean
CDR_put_string(CDR_Codec *codec, const char *str)
{
	unsigned int len;

	len=strlen(str)+1;

	if (CDR_put_ulong(codec, len)==CORBA_FALSE) return CORBA_FALSE;
	return CDR_buffer_puts(codec, str, len);
}

CORBA_boolean
CDR_put_seq_begin(CDR_Codec *codec, CORBA_unsigned_long ul)
{
    return(CDR_put_ulong(codec, ul));
}

CORBA_boolean
CDR_get_string_static(CDR_Codec *codec,CORBA_char **str)
{
	CORBA_unsigned_long len;

	if(CDR_get_ulong(codec, (CORBA_unsigned_long *)&len)==CORBA_FALSE)
		return CORBA_FALSE;

	if((codec->pos + len) > codec->buf_size)
		return CORBA_FALSE;

	*str = ((CORBA_char *)codec->buffer) + codec->pos;

	codec->pos += len;

	return CORBA_TRUE;
}

CORBA_boolean
CDR_get_string(CDR_Codec *codec, CORBA_char **str)
{
	CORBA_unsigned_long len;

	if(CDR_get_ulong(codec, (CORBA_unsigned_long *)&len)==CORBA_FALSE)
		return(CORBA_FALSE);

	if(len==0)
		return(CORBA_FALSE);

	*str=MALLOC(len);

	if(CDR_buffer_gets(codec, *str, len)==CORBA_FALSE) {
		FREE(*str);
		return(CORBA_FALSE);
	}

	if((*str)[len-1]!='\0') {
		(*str)[len-1]='\0';
	}

	return(CORBA_TRUE);
}

CORBA_boolean
CDR_get_string_buff(CDR_Codec *codec, CORBA_char *str)
{
	CORBA_unsigned_long len;

	if(CDR_get_ulong(codec, (CORBA_unsigned_long *)&len)==CORBA_FALSE)
		return(CORBA_FALSE);

	if(len==0)
		return(CORBA_FALSE);

	if(CDR_buffer_gets(codec, str, len)==CORBA_FALSE) {
		return(CORBA_FALSE);
	}

	if(str[len-1]!='\0') {
		str[len-1]='\0';
	}

	return(CORBA_TRUE);
}

CORBA_boolean
CDR_get_seq_begin(CDR_Codec *codec, CORBA_unsigned_long *ul)
{
	return(CDR_get_ulong(codec, (CORBA_unsigned_long *)ul));
}

CDR_Codec *
CDR_codec_init_static(CDR_Codec *codec)
{
	memset(codec, 0, sizeof(CDR_Codec));

	codec->host_endian = FLAG_ENDIANNESS;

	return codec;
}

CDR_Codec *
CDR_codec_init(void)
{
	CDR_Codec *c;

	c=MALLOC(sizeof(CDR_Codec));
	CDR_codec_init_static(c);

	return(c);
}

void
CDR_codec_release_buffer(CDR_Codec *codec)
{
	if(codec->release_buffer)
		FREE(codec->buffer);
}


void
CDR_codec_free(CDR_Codec *codec)
{
	CDR_codec_release_buffer(codec);
	FREE(codec);
}
