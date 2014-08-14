/*
 * api_impl.c
 *
 *  Created on: Mar 11, 2014
 *      Author: Dmitri Rubinstein
 */
#include <KIARA/Components/api.h>

#include <KIARA/Common/Config.h>

//#define KIARA_DO_DEBUG
#if defined(KIARA_DO_DEBUG) && !defined(NDEBUG)
#define KIARA_PING() KIARA_DEBUGF("CALLED: %s\n", KIARA_FUNCID())
#define KIARA_IFDEBUG(code) code
#define KIARA_DEBUGF(...) fprintf(stderr, __VA_ARGS__)
#else
#define KIARA_PING()
#define KIARA_IFDEBUG(code)
#define KIARA_DEBUGF(...)
#endif

KIARA_Connection * getConnection(KIARA_FuncObj *funcObj)
{
    return funcObj->base.connection;
}

KIARA_Connection * getServiceConnection(KIARA_ServiceFuncObj *funcObj)
{
    return funcObj->base.connection;
}
