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

#include <KIARA/CDT/kr_dbuffer.h>
#include <KIARA/CDT/kr_base64.h>
#include <string.h>
#include <stdio.h>

int mu_tests_run;

#define MU_CHECK_MSG(message, test) do { if (!(test)) return message; } while (0)
#define MU_CHECK(test) MU_CHECK_MSG("Test (" #test ") failed at line " KIARA_STRINGIZE(__LINE__), test)
#define MU_RUN_TEST(test) do { const char *message = test(); mu_tests_run++; \
                               if (message) return message; } while (0)

#define MU_TEST(name) static const char * name()


#define MAKE_BINARY_WRITER(T)                               \
    int write_##T(kr_dbuffer_t *buf, const T v)             \
    {                                                       \
        return kr_dbuffer_append_mem(buf, &v, sizeof(T));   \
    }

#define MAKE_BINARY_READER(T)                                   \
    size_t read_##T(kr_dbuffer_t *buf, size_t offset, T *v)     \
    {                                                           \
        if (offset+sizeof(T) > kr_dbuffer_size(buf))            \
            return 0;                                           \
        memcpy(v, kr_dbuffer_data(buf)+offset, sizeof(T));      \
        return offset+sizeof(T);                                \
    }

#define MAKE_BINARY_RW(T) \
    MAKE_BINARY_WRITER(T) \
    MAKE_BINARY_READER(T)

MAKE_BINARY_RW(int)
MAKE_BINARY_RW(float)
MAKE_BINARY_RW(double)

MU_TEST(test_buffer)
{
    kr_dbuffer_t *buf;
    int i;
    const int buf_size = 10;

    buf = kr_dbuffer_new();

    MU_CHECK(buf != NULL);
    MU_CHECK(kr_dbuffer_size(buf) == 0);

    kr_dbuffer_resize(buf, buf_size);
    MU_CHECK(kr_dbuffer_size(buf) == buf_size);
    for (i = 0; i < buf_size; ++i)
    {
        kr_dbuffer_data(buf)[i] = i;
    }

    {
        kr_dbuffer_t *buf1 = kr_dbuffer_new();
        kr_dbuffer_assign(buf1, buf);
        MU_CHECK(kr_dbuffer_size(buf) == kr_dbuffer_size(buf1));

        for (i = 0; i < buf_size; ++i)
        {
            MU_CHECK(kr_dbuffer_data(buf1)[i] == kr_dbuffer_data(buf)[i]);
        }

        kr_dbuffer_delete(buf1);
    }

    {
        char data1[] = "THIS IS TEST";
        const char data2[] = " 1234";
        kr_dbuffer_t *buf1 = kr_dbuffer_new_from_data(data1, sizeof(data1), sizeof(data1), kr_dbuffer_dont_free);
        kr_dbuffer_resize(buf1, kr_dbuffer_size(buf1)-1); /* overwrite trailing '\0' */
        kr_dbuffer_append_mem(buf1, data2, sizeof(data2));
        MU_CHECK(kr_dbuffer_size(buf1) == sizeof(data1)+sizeof(data2)-1);
        MU_CHECK(kr_dbuffer_data(buf1) != data1 && kr_dbuffer_data(buf1) != data2);
        MU_CHECK(strcmp(kr_dbuffer_data(buf1), "THIS IS TEST 1234") == 0);
        kr_dbuffer_delete(buf1);
    }

    {
        const char data[] = "THIS IS TEST 1234 FOO bar";
        kr_dbuffer_copy_mem(buf, data, sizeof(data));
        MU_CHECK(kr_dbuffer_size(buf) == sizeof(data));
        for (i = 0; i < sizeof(data); ++i)
        {
            MU_CHECK(kr_dbuffer_data(buf)[i] == data[i]);
        }
        MU_CHECK(kr_dbuffer_data(buf)[kr_dbuffer_size(buf)-1] == 0);
    }

    {
        const char data[] = "TEST 222 TEST";
        size_t start_pos = kr_dbuffer_size(buf)-1;
        kr_dbuffer_resize(buf, start_pos);
        kr_dbuffer_append_mem(buf, data, sizeof(data));
        for (i = 0; i < sizeof(data); ++i)
        {
            MU_CHECK(kr_dbuffer_data(buf)[start_pos+i] == data[i]);
        }
    }

    {
        const char data[] = "THIS IS TEST 1234 FOO barTEST 222 TEST";
        MU_CHECK(kr_dbuffer_size(buf) == sizeof(data));
        for (i = 0; i < sizeof(data); ++i)
        {
            MU_CHECK(kr_dbuffer_data(buf)[i] == data[i]);
        }
        MU_CHECK(kr_dbuffer_data(buf)[kr_dbuffer_size(buf)-1] == 0);
    }

    {
        size_t offset = 0;
        const int seq_size = 10;
        int iv;
        float fv;
        double dv;

        kr_dbuffer_clear(buf);
        MU_CHECK(kr_dbuffer_size(buf) == 0);

        for (i = 0; i < seq_size; ++i)
        {
            write_int(buf, i);
            write_float(buf, i+0.5);
            write_double(buf, i-0.5);
        }

        MU_CHECK(kr_dbuffer_size(buf) = seq_size * (sizeof(int)+sizeof(float)+sizeof(double)));

        for (i = 0; i < seq_size; ++i)
        {
            printf("offset %i\n", (int)offset);
            offset = read_int(buf, offset, &iv);
            MU_CHECK(offset != 0 && iv == i);

            printf("offset %i\n", (int)offset);
            offset = read_float(buf, offset, &fv);
            MU_CHECK(offset != 0);

            printf("offset %i\n", (int)offset);
            offset = read_double(buf, offset, &dv);
            MU_CHECK(offset != 0);

            printf("iv = %i, fv = %f, dv = %f\n", iv, fv, dv);
        }
    }

    {
        kr_dbuffer_clear(buf);
        MU_CHECK(kr_dbuffer_size(buf) == 0);
        kr_dbuffer_append_byte(buf, 'A');
        kr_dbuffer_append_byte(buf, 'B');
        kr_dbuffer_append_byte(buf, 'C');
        kr_dbuffer_append_byte(buf, 'D');
        kr_dbuffer_make_cstr(buf);

        MU_CHECK(kr_dbuffer_size(buf) == 5);
        MU_CHECK(strlen(kr_dbuffer_data(buf)) == 4);
        MU_CHECK(strcmp(kr_dbuffer_data(buf), "ABCD") == 0);
    }

    {
        const char *data[] = {
            "",
            "A",
            "AB",
            "ABC",
            "ABCD"
        };
        const char *test[] = {
            "",
            "QQ==",
            "QUI=",
            "QUJD",
            "QUJDRA=="
        };
		#define DATA2SIZE (0xff+1)
        char data2[DATA2SIZE];
        const char *test2 =
            "AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIj"
            "JCUmJygpKissLS4vMDEyMzQ1Njc4OTo7PD0+P0BBQkNERUZH"
            "SElKS0xNTk9QUVJTVFVWV1hZWltcXV5fYGFiY2RlZmdoaWpr"
            "bG1ub3BxcnN0dXZ3eHl6e3x9fn+AgYKDhIWGh4iJiouMjY6P"
            "kJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6ytrq+wsbKz"
            "tLW2t7i5uru8vb6/wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX"
            "2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7"
            "/P3+/w==";
        kr_dbuffer_t *buf1 = kr_dbuffer_new();

        for (i = 0; i < DATA2SIZE; ++i)
        {
            data2[i] = (char)((unsigned char)i);
        }

        for (i = 3; i < sizeof(data)/sizeof(data[0]);++i)
        {
            kr_dbuffer_clear(buf);
            MU_CHECK(kr_dbuffer_size(buf) == 0);

            kr_base64_encode(data[i], strlen(data[i]), buf, 0);
            kr_dbuffer_make_cstr(buf);
            printf("\"%s\" -base64-> \"%s\"\n", data[i], kr_dbuffer_data(buf));

            MU_CHECK(strcmp(kr_dbuffer_data(buf), test[i]) == 0);

            kr_dbuffer_clear(buf1);
            kr_base64_decode(kr_dbuffer_data(buf), strlen(kr_dbuffer_data(buf)), buf1);
            MU_CHECK(kr_dbuffer_size(buf1) == strlen(data[i]));
            MU_CHECK(memcmp(kr_dbuffer_data(buf1), data[i], kr_dbuffer_size(buf1)) == 0);
        }

        {
            kr_dbuffer_clear(buf);
            MU_CHECK(kr_dbuffer_size(buf) == 0);

            kr_base64_encode(data2, DATA2SIZE, buf, 0);
            kr_dbuffer_make_cstr(buf);
            printf("0x00..0xff -base64-> \"%s\"\n", kr_dbuffer_data(buf));

            MU_CHECK(strcmp(kr_dbuffer_data(buf), test2) == 0);

            kr_dbuffer_clear(buf1);
            kr_base64_decode(kr_dbuffer_data(buf), strlen(kr_dbuffer_data(buf)), buf1);
            MU_CHECK(kr_dbuffer_size(buf1) == DATA2SIZE);
            MU_CHECK(memcmp(kr_dbuffer_data(buf1), data2, kr_dbuffer_size(buf1)) == 0);
        }

        kr_dbuffer_delete(buf1);
    }

    kr_dbuffer_delete(buf);
    return NULL;
}

MU_TEST(all_tests)
{
    MU_RUN_TEST(test_buffer);
    return NULL;
}

int main (int argc, char **argv)
{
    const char *result = all_tests();
    if (result)
    {
        printf("Test Error: %s\n", result);
    }
    else
    {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", mu_tests_run);

    return result != NULL;
}
