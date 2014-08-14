/*
 * calctest_data_access.c
 *
 *  Created on: Feb 26, 2014
 *      Author: Dmitri Rubinstein
 */

#include "calctest_data_access.h"
#include <KIARA/CDT/kr_dstring.h>

int dstring_SetCString(KIARA_UserType *ustr, const char *cstr)
{
    int result = kr_dstring_assign_str((kr_dstring_t*)ustr, cstr);
    return result ? 0 : 1;
}

int dstring_GetCString(KIARA_UserType *ustr, const char **cstr)
{
    *cstr = kr_dstring_str((kr_dstring_t*)ustr);
    return 0;
}

KIARA_UserType * dstring_Allocate(void)
{
    return (KIARA_UserType *)kr_dstring_new();
}

void dstring_Deallocate(KIARA_UserType *value)
{
    kr_dstring_delete((kr_dstring_t*)value);
}
