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
 * dumpdata.c
 *
 *  Created on: Aug 22, 2013
 *      Author: Dmitri Rubinstein
 */
#define KIARA_LIB
#include "kr_dumpdata.h"

void kr_dump_data(const char *text, FILE *stream, unsigned char *ptr, size_t size, char nohex)
{
    size_t i;
    size_t c;

    unsigned int width = 0x10;

    if (nohex)
        /* without the hex output, we can fit more on screen */
        width = 0x40;

    fprintf(stream, "%s, %10.10ld bytes (0x%8.8lx)\n", text, (long) size, (long) size);

    for (i = 0; i < size; i += width)
    {

        fprintf(stream, "%4.4lx: ", (long) i);

        if (!nohex)
        {
            /* hex not disabled, show it */
            for (c = 0; c < width; c++)
                if (i + c < size)
                    fprintf(stream, "%02x ", ptr[i + c]);
                else
                    fputs("   ", stream);
        }

        for (c = 0; (c < width) && (i + c < size); c++)
        {
            /* check for 0D0A; if found, skip past and start a new line of output */
            if (nohex && (i + c + 1 < size) && ptr[i + c] == 0x0D && ptr[i + c + 1] == 0x0A)
            {
                i += (c + 2 - width);
                break;
            }
            fprintf(stream, "%c", (ptr[i + c] >= 0x20) && (ptr[i + c] < 0x80) ? ptr[i + c] : '.');
            /* check again for 0D0A, to avoid an extra \n if it's at width */
            if (nohex && (i + c + 2 < size) && ptr[i + c + 1] == 0x0D && ptr[i + c + 2] == 0x0A)
            {
                i += (c + 3 - width);
                break;
            }
        }
        fputc('\n', stream); /* newline */
    }
    fflush(stream);
}
