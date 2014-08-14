#include "orte/cdr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CDR_Codec * create_cdr_codec(void)
{
    CDR_Codec *codec;
    codec = CDR_codec_init();
    CDR_buffer_init(codec, 1);
    codec->data_endian = codec->host_endian;
    return codec;
}

void destroy_cdr_codec(CDR_Codec * codec)
{
    CDR_codec_free(codec);
}

CORBA_boolean write_data(CDR_Codec *codec)
{
    CORBA_short a = 12;
    CORBA_boolean b = CORBA_FALSE;
    CORBA_string s = "CORBA String";
    if (CDR_put_short(codec, a) == CORBA_FALSE)
        return CORBA_FALSE;
    if (CDR_put_boolean(codec, b) == CORBA_FALSE)
        return CORBA_FALSE;
    if (CDR_put_string(codec, s) == CORBA_FALSE)
        return CORBA_FALSE;
    return CORBA_TRUE;
}

CORBA_boolean read_data(CDR_Codec *codec)
{
    CORBA_short a;
    CORBA_boolean b;
    CORBA_string s;

    if (CDR_get_short(codec, &a) == CORBA_FALSE)
        return CORBA_FALSE;

    if (a != 12)
        return CORBA_FALSE;

    if (CDR_get_boolean(codec, &b) == CORBA_FALSE)
        return CORBA_FALSE;

    if (b != CORBA_FALSE)
        return CORBA_FALSE;

    if (CDR_get_string(codec, &s) == CORBA_FALSE)
        return CORBA_FALSE;

    if (strcmp(s, "CORBA String") != 0)
        return CORBA_FALSE;

    FREE(s);

    return CORBA_TRUE;
}

int main(int argc, char **argv)
{
    CDR_Codec *codec;
    codec = create_cdr_codec();
    if (!codec)
    {
        fprintf(stderr, "Could not create CDR codec\n");
        exit(1);
    }
    if (write_data(codec) == CORBA_FALSE)
    {
        fprintf(stderr, "Could not write data to CDR buffer\n");
        destroy_cdr_codec(codec);
        exit(1);
    }

    CDR_buffer_reset_position(codec);

    if (read_data(codec) == CORBA_FALSE)
    {
        fprintf(stderr, "Could not read data from CDR buffer\n");
        destroy_cdr_codec(codec);
        exit(1);
    }
    destroy_cdr_codec(codec);

    fprintf(stdout, "All tests passed\n");

    return 0;
}
