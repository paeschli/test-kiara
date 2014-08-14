/*
 * calctest_data_access.h
 *
 *  Created on: Feb 26, 2014
 *      Author: Dmitri Rubinstein
 */

#ifndef CALCTEST_DATA_ACCESS_H_INCLUDED
#define CALCTEST_DATA_ACCESS_H_INCLUDED

#include <KIARA/kiara.h>

int KIARA_LLVM_BITCODE_INLINE dstring_SetCString(KIARA_UserType *ustr, const char *cstr);
int KIARA_LLVM_BITCODE_INLINE dstring_GetCString(KIARA_UserType *ustr, const char **cstr);

KIARA_UserType * KIARA_LLVM_BITCODE_INLINE  dstring_Allocate(void);
void KIARA_LLVM_BITCODE_INLINE dstring_Deallocate(KIARA_UserType *value);

#endif /* CALCTEST_DATA_ACCESS_H_INCLUDED */
