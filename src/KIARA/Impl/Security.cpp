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
 * kiara_security_impl.cpp
 *
 *  Created on: Aug 8, 2013
 *      Author: Dmitri Rubinstein
 */
#define KIARA_LIB

#include <KIARA/Common/Config.hpp>
#include <KIARA/kiara_security.h>
#include <cstring>
#include <cassert>
#include <openssl/evp.h>
//#include <cstdio>

// This code is based on example from http://saju.net.in/blog/?p=36

struct KIARA_SymmetricKey
{
    unsigned char key[EVP_MAX_KEY_LENGTH];
    unsigned char iv[EVP_MAX_IV_LENGTH];
    const EVP_CIPHER *evpcipher;

    KIARA_SymmetricKey(const EVP_CIPHER *evpcipher = 0) : evpcipher(evpcipher) { }
    KIARA_SymmetricKey(const KIARA_Cipher *cipher = 0) : evpcipher((const EVP_CIPHER*)cipher) { }
    ~KIARA_SymmetricKey() { }

    KIARA_Result initFromText(const char *text)
    {
        const int nrounds = 5;
        /*
         * The salt paramter is used as a salt in the derivation:
         * it should point to an 8 byte buffer or NULL if no salt is used.
         */
        unsigned char salt[] = {0x51,0x21,0x00,0x23,0x76,0xfa,0xd7,0xac};

        /*
         * Gen key & IV for AES 256 CBC mode. A SHA1 digest is used to hash the supplied key material.
         * nrounds is the number of times the we hash the material. More rounds are more secure but
         * slower.
         */
        const int keySize = EVP_BytesToKey(evpcipher, EVP_sha1(), salt,
            reinterpret_cast<const unsigned char*>(text), strlen(text),
            nrounds, key, iv);

        if (keySize == EVP_CIPHER_key_length(evpcipher)) // FIXME Is this assertion ?
            return KIARA_SUCCESS;
        else
            return KIARA_SYMMETRIC_KEY_INIT_FAILED;
    }
};

struct KIARA_CipherContext
{
    EVP_CIPHER_CTX evpctx;

    KIARA_CipherContext()
    {
        EVP_CIPHER_CTX_init(&evpctx);
    }

    KIARA_Result initEncryption(KIARA_SymmetricKey *key)
    {
        EVP_EncryptInit_ex(&evpctx, key->evpcipher, NULL, key->key, key->iv);
        return KIARA_SUCCESS;
    }

    KIARA_Result initDecryption(KIARA_SymmetricKey *key)
    {
        EVP_DecryptInit_ex(&evpctx, key->evpcipher, NULL, key->key, key->iv);
        return KIARA_SUCCESS;
    }

    KIARA_Result encrypt(kr_dbuffer_t *ciphertext, const kr_dbuffer_t *plaintext)
    {
        assert(ciphertext != 0 && plaintext != 0);
        //EVP_CIPHER_CTX_key_length(&evpctx)
        //EVP_CIPHER_CTX_iv_length(&evpctx)

        /* max ciphertext len for a n bytes of plaintext is n + cipher_block_size - 1 bytes */
        const size_t maxCipherTextLen = kr_dbuffer_size(plaintext) + EVP_CIPHER_CTX_block_size(&evpctx);

        kr_dbuffer_resize_nocopy(ciphertext, maxCipherTextLen);

        /* allows reusing of 'e' for multiple encryption cycles */
        if (!EVP_EncryptInit_ex(&evpctx, NULL, NULL, NULL, NULL))
        {
            //printf("ERROR in EVP_EncryptInit_ex \n"); // FIXME
            return KIARA_ENCRYPTION_FAILED;
        }

        /* update ciphertext, coutl is filled with the length of ciphertext generated */
        int coutl; // number of bytes written to ciphertext
        if (!EVP_EncryptUpdate(&evpctx,
            reinterpret_cast<unsigned char *>(kr_dbuffer_data(ciphertext)), &coutl,
            reinterpret_cast<const unsigned char*>(kr_dbuffer_data(plaintext)),
            kr_dbuffer_size(plaintext)))
        {
            return KIARA_ENCRYPTION_FAILED;
        }

        assert(static_cast<size_t>(coutl) <= maxCipherTextLen);

        /* update ciphertext with the final remaining bytes */
        int foutl; // number of final bytes written to ciphertext
        if (!EVP_EncryptFinal_ex(&evpctx, reinterpret_cast<unsigned char *>(kr_dbuffer_data(ciphertext)+coutl), &foutl))
        {
            //printf("ERROR in EVP_EncryptFinal_ex \n");
            return KIARA_ENCRYPTION_FAILED;
        }
        assert(kr_dbuffer_size(ciphertext) >= static_cast<size_t>(coutl+foutl));
        kr_dbuffer_resize(ciphertext, coutl+foutl);

        //fprintf(stderr, "encrypt: Ciphertext size = %i\n", (int)kr_dbuffer_size(ciphertext));

        return KIARA_SUCCESS;
    }

    KIARA_Result decrypt(kr_dbuffer_t *plaintext, const kr_dbuffer_t *ciphertext)
    {
        //fprintf(stderr, "decrypt: Ciphertext size = %i\n", (int)kr_dbuffer_size(ciphertext));

        /* plaintext will always be equal to or lesser than length of ciphertext*/
        kr_dbuffer_resize_nocopy(plaintext, kr_dbuffer_size(ciphertext));

        if (!EVP_DecryptInit_ex(&evpctx, NULL, NULL, NULL, NULL))
        {
            //fprintf(stderr, "ERROR in EVP_DecryptInit_ex\n");
            return KIARA_DECRYPTION_FAILED;
        }

        int poutl; // number of bytes written to plaintext
        if (!EVP_DecryptUpdate(&evpctx,
            reinterpret_cast<unsigned char*>(kr_dbuffer_data(plaintext)),
            &poutl,
            reinterpret_cast<const unsigned char*>(kr_dbuffer_data(ciphertext)),
            kr_dbuffer_size(ciphertext)))
        {
            //fprintf(stderr, "ERROR in EVP_DecryptUpdate\n");
            return KIARA_DECRYPTION_FAILED;
        }

        int foutl; // number of final bytes written to plaintext
        if (!EVP_DecryptFinal_ex(&evpctx, reinterpret_cast<unsigned char*>(kr_dbuffer_data(plaintext)+poutl), &foutl))
        {
            //fprintf(stderr, "ERROR in EVP_DecryptFinal_ex\n");
            return KIARA_DECRYPTION_FAILED;
        }

        assert(kr_dbuffer_size(plaintext) >= static_cast<size_t>(poutl+foutl));
        kr_dbuffer_resize(plaintext, poutl+foutl);
        return KIARA_SUCCESS;
    }

    ~KIARA_CipherContext()
    {
        EVP_CIPHER_CTX_cleanup(&evpctx);
    }
};


const KIARA_Cipher * kiaraGetCipher(const char *name)
{
    return (const KIARA_Cipher*)EVP_aes_256_cbc();
}

KIARA_SymmetricKey * kiaraNewSymmetricKey(const KIARA_Cipher *cipher)
{
    assert(cipher != 0);
    return new KIARA_SymmetricKey(cipher);
}

KIARA_Result kiaraFreeSymmetricKey(KIARA_SymmetricKey *key)
{
    delete key;
    return KIARA_SUCCESS;
}

KIARA_Result kiaraInitSymmetricKeyFromText(KIARA_SymmetricKey *key, const char *text)
{
    assert(key != 0);
    assert(text != 0);
    return key->initFromText(text);
}

KIARA_CipherContext * kiaraNewCipherContext()
{
    return new KIARA_CipherContext();
}

KIARA_Result kiaraFreeCipherContext(KIARA_CipherContext *cipherContext)
{
    delete cipherContext;
    return KIARA_SUCCESS;
}

KIARA_Result kiaraInitEncryption(KIARA_CipherContext *cipherContext, KIARA_SymmetricKey *key)
{
    assert(cipherContext != 0);
    assert(key != 0);
    return cipherContext->initEncryption(key);
}

KIARA_Result kiaraInitDecryption(KIARA_CipherContext *cipherContext, KIARA_SymmetricKey *key)
{
    assert(cipherContext != 0);
    assert(key != 0);
    return cipherContext->initDecryption(key);
}

KIARA_Result kiaraEncrypt(KIARA_CipherContext *cipherContext, kr_dbuffer_t *ciphertext, const kr_dbuffer_t *plaintext)
{
    assert(cipherContext != 0);
    return cipherContext->encrypt(ciphertext, plaintext);
}

KIARA_Result kiaraDecrypt(KIARA_CipherContext *cipherContext, kr_dbuffer_t *plaintext, const kr_dbuffer_t *ciphertext)
{
    assert(cipherContext != 0);
    return cipherContext->decrypt(plaintext, ciphertext);
}
