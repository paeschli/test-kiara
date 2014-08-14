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
#include <KIARA/kiara_security.h>
#include <string.h>
#include <stdio.h>

int mu_tests_run;

#define MU_CHECK_MSG(message, test) do { if (!(test)) return message; } while (0)
#define MU_CHECK(test) MU_CHECK_MSG("Test (" #test ") failed at line " KIARA_STRINGIZE(__LINE__), test)
#define MU_RUN_TEST(test) do { const char *message = test(); mu_tests_run++; \
                               if (message) return message; } while (0)

#define MU_TEST(name) static const char * name()

static void dump(const char *text, FILE *stream, void *rawptr, size_t size, char nohex)
{
    unsigned char *ptr = (unsigned char*)rawptr;
    unsigned int width = 0x10;
    size_t i;
    size_t c;

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


MU_TEST(test_encrypt)
{
    KIARA_CipherContext *ec, *dc;
    KIARA_SymmetricKey *key;
    kr_dbuffer_t *plaintext, *ciphertext;
    int i;
    KIARA_Result result;

    const char *input[] = {
        "a", "abcd", "this is a test", "this is a bigger test",
        "\nWho are you ?\nI am the 'Doctor'.\n'Doctor' who ?\nPrecisely!",
        "üöüäödß"
    };
    const size_t num_inputs = sizeof(input)/sizeof(input[0]);

    key = kiaraNewSymmetricKey(kiaraGetCipher(NULL));
    kiaraInitSymmetricKeyFromText(key, "password");

    ec = kiaraNewCipherContext();
    dc = kiaraNewCipherContext();

    kiaraInitEncryption(ec, key);
    kiaraInitDecryption(dc, key);


    plaintext = kr_dbuffer_new();
    ciphertext = kr_dbuffer_new();

    MU_CHECK(plaintext != NULL);
    MU_CHECK(kr_dbuffer_size(plaintext) == 0);
    MU_CHECK(ciphertext != NULL);
    MU_CHECK(kr_dbuffer_size(ciphertext) == 0);

    for (i = 0; i < num_inputs; ++i)
    {
        /* Copy input with trailing '\0' */
        kr_dbuffer_copy_mem(plaintext, input[i], strlen(input[i])+1);
        kr_dbuffer_resize(plaintext, kr_dbuffer_size(plaintext)-1);
        printf("Plaintext: '%s'\n", kr_dbuffer_data(plaintext));

        result = kiaraEncrypt(ec, ciphertext, plaintext);
        MU_CHECK(result == KIARA_SUCCESS);

        dump("Ciphertext", stdout, kr_dbuffer_data(ciphertext), kr_dbuffer_size(ciphertext), 0);

        memset(kr_dbuffer_data(plaintext), 0, kr_dbuffer_size(plaintext));

        result = kiaraDecrypt(dc, plaintext, ciphertext);
        MU_CHECK(result == KIARA_SUCCESS);
        MU_CHECK(kr_dbuffer_size(plaintext) == strlen(input[i]));
        MU_CHECK(memcmp(input[i], kr_dbuffer_data(plaintext), kr_dbuffer_size(plaintext)) == 0);

        dump("Decrypted plaintext", stdout, kr_dbuffer_data(plaintext), kr_dbuffer_size(plaintext), 0);
    }

    kr_dbuffer_delete(ciphertext);
    kr_dbuffer_delete(plaintext);

    kiaraFreeCipherContext(ec);
    kiaraFreeCipherContext(dc);
    kiaraFreeSymmetricKey(key);

    return NULL;
}

MU_TEST(all_tests)
{
    kiaraInit(NULL, NULL);

    MU_RUN_TEST(test_encrypt);

    kiaraFinalize();
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
