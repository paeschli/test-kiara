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
 * kiara_security.h
 *
 *  Created on: Aug 8, 2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_SECURITY_H_INCLUDED
#define KIARA_SECURITY_H_INCLUDED

#include <KIARA/kiara.h>
#include <KIARA/CDT/kr_dbuffer.h>

KIARA_BEGIN_EXTERN_C

/** Handle for cipher algorithm */
typedef struct KIARA_Cipher KIARA_Cipher;
/** Handle for cipher context */
typedef struct KIARA_CipherContext KIARA_CipherContext;
/** Handle for symmetric key */
typedef struct KIARA_SymmetricKey KIARA_SymmetricKey;

/** Get cipher by name, use NULL for default cipher */
KIARA_API const KIARA_Cipher * kiaraGetCipher(const char *name);

KIARA_API KIARA_SymmetricKey * kiaraNewSymmetricKey(const KIARA_Cipher *cipher);
KIARA_API KIARA_Result kiaraFreeSymmetricKey(KIARA_SymmetricKey *key);
KIARA_API KIARA_Result kiaraInitSymmetricKeyFromText(KIARA_SymmetricKey *key, const char *text);

KIARA_API KIARA_CipherContext * kiaraNewCipherContext();
KIARA_API KIARA_Result kiaraFreeCipherContext(KIARA_CipherContext *cipherContext);
KIARA_API KIARA_Result kiaraInitEncryption(KIARA_CipherContext *cipherContext, KIARA_SymmetricKey *key);
KIARA_API KIARA_Result kiaraInitDecryption(KIARA_CipherContext *cipherContext, KIARA_SymmetricKey *key);

/** Encrypt data provided in the plaintext buffer and store in the ciphertext buffer */
KIARA_API KIARA_Result kiaraEncrypt(KIARA_CipherContext *cipherContext, kr_dbuffer_t *ciphertext, const kr_dbuffer_t *plaintext);

/** Decrypt data provided in the ciphertext buffer and store in the ciphertext buffer */
KIARA_API KIARA_Result kiaraDecrypt(KIARA_CipherContext *cipherContext, kr_dbuffer_t *plaintext, const kr_dbuffer_t *ciphertext);

/** Returns secret key by its name */
KIARA_API const char * kiaraGetSecretKeyText(KIARA_Connection *connection, const char *keyName);

KIARA_END_EXTERN_C

#endif /* KIARA_SECURITY_H_INCLUDED */
