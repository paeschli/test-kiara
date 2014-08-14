/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2012, 2013  German Research Center for Artificial Intelligence (DFKI)
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
 * kiara.h
 *
 *  Created on: 03.09.2012
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_H_INCLUDED
#define KIARA_H_INCLUDED

#include <KIARA/Common/Config.h>
#include <KIARA/Common/Version.h>
#include <KIARA/Common/stdint.h>
#include <KIARA/Common/stddef.h>
#include <KIARA/CDT/kr_dbuffer.h>
#include <stdlib.h>

/* Common macros */

#ifdef __cplusplus
#define KIARA_EXTERN_C extern "C"
#define KIARA_BEGIN_EXTERN_C extern "C" {
#define KIARA_END_EXTERN_C }
#else
#define KIARA_EXTERN_C
#define KIARA_BEGIN_EXTERN_C
#define KIARA_END_EXTERN_C
#endif

#define _KIARA_DTYPE(what, kiara_type_name) \
    KIARA_JOIN(KIARA_JOIN(kr_, KIARA_JOIN(what, _)), kiara_type_name)

#define KIARA_DECL_FUNC_OBJ(kiara_type_name)            \
    KIARA_BEGIN_EXTERN_C                                \
    struct _KIARA_DTYPE(funcobj, kiara_type_name);      \
    KIARA_END_EXTERN_C

#ifdef __cplusplus
#define KIARA_FUNC_OBJ(kiara_type_name)                 \
        struct ::_KIARA_DTYPE(funcobj, kiara_type_name) *
#else
#define KIARA_FUNC_OBJ(kiara_type_name)                 \
        struct _KIARA_DTYPE(funcobj, kiara_type_name) *
#endif

#define KIARA_FWD_DECL_ENCRYPTED(kiara_type_name)       \
    KIARA_BEGIN_EXTERN_C                                \
    struct _KIARA_DTYPE(enc, kiara_type_name);          \
    KIARA_END_EXTERN_C

#ifdef __cplusplus
#define KIARA_ENCRYPTED_STRUCT(kiara_type_name)         \
        struct ::_KIARA_DTYPE(enc, kiara_type_name)
#define KIARA_ENCRYPTED(kiara_type_name)                \
        struct ::_KIARA_DTYPE(enc, kiara_type_name) *
#else
#define KIARA_ENCRYPTED_STRUCT(kiara_type_name)         \
        struct _KIARA_DTYPE(enc, kiara_type_name)
#define KIARA_ENCRYPTED(kiara_type_name)                \
        struct _KIARA_DTYPE(enc, kiara_type_name) *
#endif

KIARA_BEGIN_EXTERN_C

#define KIARA_SUCCESS   0x0
#define KIARA_FALSE     0x0
#define KIARA_TRUE      0x1

/** Return codes signaling errors */

#define KIARA_NO_ERROR              KIARA_SUCCESS
#define KIARA_FAILURE               (-1)
#define KIARA_GENERIC_ERROR         0x0001
#define KIARA_INPUT_ERROR           0x0100
#define KIARA_OUTPUT_ERROR          0x0200
#define KIARA_CONNECTION_ERROR      0x0300
#define KIARA_API_ERROR             0x0500
#define KIARA_INIT_ERROR            0x0501
#define KIARA_FINI_ERROR            0x0502
#define KIARA_INVALID_VALUE         0x0503
#define KIARA_INVALID_TYPE          0x0504
#define KIARA_INVALID_OPERATION     0x0505
#define KIARA_INVALID_ARGUMENT      0x0506
#define KIARA_UNSUPPORTED_FEATURE   0x0507
#define KIARA_CONFIG_ERROR          0x0508
#define KIARA_SECURITY_ERROR        0x0510
#define KIARA_SYMMETRIC_KEY_INIT_FAILED     0x0511
#define KIARA_ENCRYPTION_FAILED             0x0512
#define KIARA_DECRYPTION_FAILED             0x0513


/* Network error */
#define KIARA_NETWORK_ERROR         0x0600

/* Marshalling errors */
#define KIARA_REQUEST_ERROR         0x0700

#define KIARA_RESPONSE_ERROR        0x0800
#define KIARA_INVALID_RESPONSE      0x0801

/** Return codes from a function call */

#define KIARA_EXCEPTION             0x1000

#define KIARA_TO_BOOL(v) ((v)?KIARA_TRUE:KIARA_FALSE)

typedef struct KIARA_Context KIARA_Context;

typedef struct KIARA_Connection KIARA_Connection;

typedef struct KIARA_Server KIARA_Server;

typedef struct KIARA_Service KIARA_Service;

typedef uint64_t KIARA_Size;

/** KIARA_Bool is boolean and can have either 0 (KIARA_FALSE) or 1 (KIARA_TRUE) value.
 *  KIARA_Bool is represented by an int.
 */
typedef int KIARA_Bool;

/* KIARA_Result represent result status of the operation.
 * It have KIARA_SUCCESS value when operation succeed, or non zero value as an error code.
 * KIARA_Result is represented by int.
 */
typedef int KIARA_Result;

typedef struct KIARA_Type KIARA_Type;

typedef struct KIARA_Message KIARA_Message;

typedef struct KIARA_BinaryStream KIARA_BinaryStream;

/** Generic function pointer type for storing arbitrary function pointers */
typedef void (*KIARA_GenericFunc)(void);

/** KIARA_StructDecl is DEPRECATED */
typedef struct KIARA_StructDecl {
    KIARA_Type *type;
    const char *name;
    size_t offset;
} KIARA_StructDecl;

/** KIARA_FuncDecl is DEPRECATED */
typedef struct KIARA_FuncDecl {
    KIARA_Type *type;
    const char *name;
} KIARA_FuncDecl;

/**
 * Opaque object is object which does not contain any type information.
 */
typedef void KIARA_OpaqueObject;

typedef struct KIARA_TypedObject {
    KIARA_Type *type;
    KIARA_OpaqueObject *object;
} KIARA_TypedObject;

/** User APIs
 *
 */

typedef struct KIARA_UserType KIARA_UserType;
typedef int (*KIARA_SetCString)(KIARA_UserType *ustr, const char *cstr);
typedef int (*KIARA_GetCString)(KIARA_UserType *ustr, const char **cstr);
typedef int (*KIARA_SetGenericError)(KIARA_UserType *ustr, int errorCode, const char *errorMessage);
typedef int (*KIARA_GetGenericError)(KIARA_UserType *ustr, int *errorCode, const char **errorMessage);
typedef KIARA_UserType * (*KIARA_AllocateType)(void);
typedef void (*KIARA_DeallocateType)(KIARA_UserType *value);

/*
 * KIARA Function objects (closures)
 */

typedef struct KIARA_FuncObjBase KIARA_FuncObjBase;
typedef struct KIARA_FuncObj KIARA_FuncObj;
typedef KIARA_Result (*KIARA_VAFunc)(KIARA_FuncObj * closure, void *args[], size_t num_args);
typedef KIARA_Result (*KIARA_Func)(KIARA_FuncObj * closure);

typedef struct KIARA_ServiceFuncObjBase KIARA_ServiceFuncObjBase;
typedef struct KIARA_ServiceFuncObj KIARA_ServiceFuncObj;
typedef KIARA_Result (*KIARA_VAServiceFunc)(KIARA_ServiceFuncObj * closure, void *args[], size_t num_args);
typedef KIARA_Result (*KIARA_ServiceFunc)(KIARA_ServiceFuncObj * closure);

typedef KIARA_Result (*KIARA_SyncServiceHandler)(KIARA_ServiceFuncObj *closure, KIARA_Message *outMsg, KIARA_Message *inMsg);

typedef union {
    void *p;
    int i;
    float f;
    double d;
    long l;
    /*long long ll;*/
} KIARA_UserData;

/* Function object for client */
struct KIARA_FuncObjBase {
    KIARA_VAFunc vafunc;
    KIARA_Connection *connection;
    KIARA_Type *funcType;
    KIARA_UserData userData;
};

struct KIARA_FuncObj {
    KIARA_FuncObjBase base;
    KIARA_Func func;
};

/* Function object for server */
struct KIARA_ServiceFuncObjBase {
    KIARA_VAServiceFunc vafunc;
    KIARA_SyncServiceHandler syncHandler;
    KIARA_Connection *connection;
    KIARA_Type *funcType;
    KIARA_UserData userData;
};

struct KIARA_ServiceFuncObj {
    KIARA_ServiceFuncObjBase base;
    KIARA_ServiceFunc func;
};

/*
 * KIARA Static Declaration Types
 */

typedef struct KIARA_DeclAnnotated          KIARA_DeclAnnotated;
typedef struct KIARA_DeclEnum               KIARA_DeclEnum;
typedef struct KIARA_DeclEnumConstant       KIARA_DeclEnumConstant;
typedef struct KIARA_DeclStruct             KIARA_DeclStruct;
typedef struct KIARA_DeclStructMember       KIARA_DeclStructMember;
typedef struct KIARA_DeclFunc               KIARA_DeclFunc;
typedef struct KIARA_DeclFuncArgument       KIARA_DeclFuncArgument;
typedef struct KIARA_DeclService            KIARA_DeclService;
typedef struct KIARA_DeclServiceArgument    KIARA_DeclServiceArgument;
typedef struct KIARA_DeclPtr                KIARA_DeclPtr;
typedef struct KIARA_DeclFixedArray         KIARA_DeclFixedArray;
typedef struct KIARA_DeclFixedArray2D       KIARA_DeclFixedArray2D;
typedef struct KIARA_DeclEncrypted          KIARA_DeclEncrypted;
typedef struct KIARA_DeclAPI                KIARA_DeclAPI;
typedef KIARA_DeclAPI                       KIARA_DeclOpaqueType;
typedef struct KIARA_DeclAPIFunc            KIARA_DeclAPIFunc;
typedef struct KIARA_DeclType               KIARA_DeclType;
typedef KIARA_DeclType * (*KIARA_GetDeclType)(void);

typedef enum KIARA_DeclTypeKind {
    KIARA_TYPE_BUILTIN,
    KIARA_TYPE_ANNOTATED,
    KIARA_TYPE_ENUM,
    KIARA_TYPE_STRUCT,
    KIARA_TYPE_FUNC,
    KIARA_TYPE_SERVICE,
    KIARA_TYPE_POINTER,
    KIARA_TYPE_REFERENCE,
    KIARA_TYPE_ARRAY,
    KIARA_TYPE_FIXED_ARRAY,
    KIARA_TYPE_FIXED_ARRAY_2D,
    KIARA_TYPE_ENCRYPTED,
    KIARA_TYPE_OPAQUE
} KIARA_DeclTypeKind;

struct KIARA_DeclAnnotated {
    KIARA_GetDeclType type;
    const char *annotation;
    const char *semantics;
};

struct KIARA_DeclEnum {
    KIARA_DeclEnumConstant *constants;
    size_t numConstants;
    size_t size; /* sizeof(enum type) */
};

struct KIARA_DeclEnumConstant {
    int64_t value;
    const char *name;
};

struct KIARA_DeclStruct {
    KIARA_DeclStructMember *members;
    size_t numMembers;
    size_t size;
    KIARA_DeclAPI *apiDecl; /* contains optional APIs */
};

struct KIARA_DeclStructMember {
    KIARA_GetDeclType type;
    const char *name;
    size_t offset;
    const char *mainName;
    const char *semantics;
};

struct KIARA_DeclFunc {
    KIARA_Func wrapperFunc;
    KIARA_GetDeclType returnType;
    KIARA_DeclFuncArgument *args;
    size_t numArgs;
};

struct KIARA_DeclFuncArgument {
    KIARA_GetDeclType type;
    const char *name;
    const char *annotation;
    const char *semantics;
};

struct KIARA_DeclService {
    KIARA_ServiceFunc serviceFunc;
    KIARA_VAServiceFunc serviceWrapperFunc;
    KIARA_GetDeclType returnType;
    KIARA_DeclServiceArgument *args;
    size_t numArgs;
};

struct KIARA_DeclServiceArgument {
    KIARA_GetDeclType type;
    const char *name;
    const char *annotation;
    const char *semantics;
};

/* For Pointers, References and Arrays */
struct KIARA_DeclPtr {
    KIARA_GetDeclType elementType;
};

struct KIARA_DeclFixedArray {
    KIARA_GetDeclType elementType;
    size_t size;
};

struct KIARA_DeclFixedArray2D {
    KIARA_GetDeclType elementType;
    size_t numRows;
    size_t numCols;
};

/* For encrypted type */
struct KIARA_DeclEncrypted {
    KIARA_GetDeclType elementType;
};

struct KIARA_DeclAPI { /* equivalent to KIARA_DeclOpaqueType */
    KIARA_DeclAPIFunc *apiFuncs;
    size_t numAPIFuncs;
};

struct KIARA_DeclAPIFunc {
    const char *memberName;
    const char *apiName;        /* Unique API identifier */
    const char *apiFuncName;    /* Name of the user-defined API function */
    KIARA_GenericFunc apiFunc;  /* Pointer to the user-defined API function */
};

/**
 * KIARA DeclType flags
 */
#define KIARA_DTF_NONE          0
#define KIARA_DTF_CONST_TYPE    0x0001

struct KIARA_DeclType {
    const char *name;
    KIARA_DeclTypeKind typeKind;
    int typeFlags;
    union {
        void *any; /* For disabling type checking */
        KIARA_DeclAnnotated *annotatedDecl; /* for TYPE_ANNOTATED */
        KIARA_DeclEnum *enumDecl;       /* for TYPE_ENUM */
        KIARA_DeclStruct *structDecl;   /* for TYPE_STRUCT */
        KIARA_DeclFunc *funcDecl;       /* for TYPE_FUNC */
        KIARA_DeclService *serviceDecl; /* for TYPE_SERVICE */
        KIARA_DeclPtr *ptrDecl;         /* for TYPE_POINTER, TYPE_REFERENCE, and TYPE_ARRAY */
        KIARA_DeclFixedArray *arrayDecl;
        KIARA_DeclFixedArray2D *array2DDecl;
        KIARA_DeclEncrypted *encryptedDecl;
        KIARA_DeclOpaqueType *opaqueTypeDecl;
    } typeDecl;
};

/* Stringized to empty string */
#define KIARA_NONE

/* Initialization */

/** Initialize KIARA library. Call this only once from main thread.
 *  Returns KIARA_SUCCESS on success or error code when failed.
 *
 *  @param argcp  A pointer to the program's unmodified argc variable from main.
 *                Upon return, the value pointed to by argcp will be updated,
 *                because kiaraInit extracts any command line options intended for the KIARA library.
 *  @param argv   The program's unmodified argv variable from main.
 *                Like argcp, the data for argv will be updated because kiaraInit extracts any command line options
 *                understood by the KIARA library.
 */
KIARA_API int kiaraInit(int *argcp, char **argv);

/** Finalize KIARA library. Call this only once on the same thread 
 *  after successful kiaraInit.
 */
KIARA_API int kiaraFinalize(void);

/** Allocates and initializes a new KIARA context.
 *  Each thread require its own context.
 *
 *  @return new context or NULL if there is no memory.
 */
KIARA_API KIARA_Context * kiaraNewContext(void);

/** Shutdowns context and frees allocated memory.
 *  Fails if there are connections opened.
 *
 *  @return KIARA_SUCCESS if context was successfully freed.
 */
KIARA_API int kiaraFreeContext(KIARA_Context *context);

KIARA_API const char * kiaraGetContextError(KIARA_Context *context);
KIARA_API KIARA_Result kiaraGetContextErrorCode(KIARA_Context *context);
KIARA_API void kiaraClearContextError(KIARA_Context *context);

KIARA_API KIARA_Type * kiaraType_c_int8_t(KIARA_Context *ctx);  /* int8_t */
KIARA_API KIARA_Type * kiaraType_c_uint8_t(KIARA_Context *ctx); /* uint8_t */
KIARA_API KIARA_Type * kiaraType_c_int16_t(KIARA_Context *ctx); /* int16_t */
KIARA_API KIARA_Type * kiaraType_c_uint16_t(KIARA_Context *ctx); /* uint16_t */
KIARA_API KIARA_Type * kiaraType_c_int32_t(KIARA_Context *ctx); /* int32_t */
KIARA_API KIARA_Type * kiaraType_c_uint32_t(KIARA_Context *ctx); /* c_uint32_t */
KIARA_API KIARA_Type * kiaraType_c_int64_t(KIARA_Context *ctx); /* c_int64_t */
KIARA_API KIARA_Type * kiaraType_c_uint64_t(KIARA_Context *ctx); /* c_uint64_t */

KIARA_API KIARA_Type * kiaraType_c_char(KIARA_Context *ctx);  /* char */
KIARA_API KIARA_Type * kiaraType_c_wchar(KIARA_Context *ctx); /* wchar */
KIARA_API KIARA_Type * kiaraType_c_schar(KIARA_Context *ctx); /* signed char */
KIARA_API KIARA_Type * kiaraType_c_uchar(KIARA_Context *ctx); /* unsigned char */
KIARA_API KIARA_Type * kiaraType_c_short(KIARA_Context *ctx); /* short */
KIARA_API KIARA_Type * kiaraType_c_ushort(KIARA_Context *ctx); /* ushort */
KIARA_API KIARA_Type * kiaraType_c_int(KIARA_Context *ctx); /* int */
KIARA_API KIARA_Type * kiaraType_c_uint(KIARA_Context *ctx); /* uint */
KIARA_API KIARA_Type * kiaraType_c_long(KIARA_Context *ctx); /* long */
KIARA_API KIARA_Type * kiaraType_c_ulong(KIARA_Context *ctx); /* unsigned long */
KIARA_API KIARA_Type * kiaraType_c_longlong(KIARA_Context *ctx); /* long long */
KIARA_API KIARA_Type * kiaraType_c_ulonglong(KIARA_Context *ctx); /* unsigned long long */
KIARA_API KIARA_Type * kiaraType_c_size_t(KIARA_Context *ctx); /* size_t */
KIARA_API KIARA_Type * kiaraType_c_ssize_t(KIARA_Context *ctx); /* ssize_t */
KIARA_API KIARA_Type * kiaraType_c_void(KIARA_Context *ctx); /* void */
KIARA_API KIARA_Type * kiaraType_c_float(KIARA_Context *ctx); /* float */
KIARA_API KIARA_Type * kiaraType_c_double(KIARA_Context *ctx); /* float */
KIARA_API KIARA_Type * kiaraType_c_longdouble(KIARA_Context *ctx); /* long double */
KIARA_API KIARA_Type * kiaraType_c_ptr(KIARA_Type *elementType);
KIARA_API KIARA_Type * kiaraType_c_char_p(KIARA_Context *ctx);
KIARA_API KIARA_Type * kiaraType_c_void_p(KIARA_Context *ctx);

KIARA_API KIARA_Type * kiaraType_c_int_of_size(KIARA_Context *ctx, size_t size);
KIARA_API KIARA_Type * kiaraType_c_uint_of_size(KIARA_Context *ctx, size_t size);

KIARA_API KIARA_Type * kiaraType_void(KIARA_Context *ctx);
KIARA_API KIARA_Type * kiaraType_any(KIARA_Context *ctx);
KIARA_API KIARA_Type * kiaraType_i8(KIARA_Context *ctx);
KIARA_API KIARA_Type * kiaraType_u8(KIARA_Context *ctx);
KIARA_API KIARA_Type * kiaraType_i16(KIARA_Context *ctx);
KIARA_API KIARA_Type * kiaraType_u16(KIARA_Context *ctx);
KIARA_API KIARA_Type * kiaraType_i32(KIARA_Context *ctx);
KIARA_API KIARA_Type * kiaraType_u32(KIARA_Context *ctx);
KIARA_API KIARA_Type * kiaraType_i64(KIARA_Context *ctx);
KIARA_API KIARA_Type * kiaraType_u64(KIARA_Context *ctx);
KIARA_API KIARA_Type * kiaraType_float(KIARA_Context *ctx);
KIARA_API KIARA_Type * kiaraType_double(KIARA_Context *ctx);
KIARA_API KIARA_Type * kiaraType_boolean(KIARA_Context *ctx);
KIARA_API KIARA_Type * kiaraType_string(KIARA_Context *ctx);

/** Arrays */
KIARA_API KIARA_Type * kiaraType_array(KIARA_Type *elementType);
KIARA_API KIARA_Type * kiaraType_fixed_array(KIARA_Type *elementType, size_t size);

/** Structs, functions and native opaque types */
KIARA_API KIARA_Type * kiaraDeclareStructType(KIARA_Context *ctx, const char *name, int numMembers, KIARA_StructDecl members[]);
KIARA_API KIARA_Type * kiaraDeclareFuncType(KIARA_Context *ctx, const char *name, KIARA_Type *returnType, int numArgs, KIARA_FuncDecl args[]);
KIARA_API KIARA_Type * kiaraDeclareOpaqueType(KIARA_Context *ctx, const char *name);

/** Get named type declared in the context.
 *  Context contains all built-in types and all types declared by user:
 *
 *  kiaraGetContextTypeByName(ctx, "c_char")  == kiaraType_c_char(ctx)
 *  kiaraGetContextTypeByName(ctx, "c_wchar") == kiaraType_c_wchar(ctx)
 *  kiaraGetContextTypeByName(ctx, "void")    == kiaraType_void(ctx)
 *  kiaraGetContextTypeByName(ctx, "any")     == kiaraType_any(ctx)
 *  kiaraGetContextTypeByName(ctx, "i8")      == kiaraType_i8(ctx)
 *  kiaraGetContextTypeByName(ctx, "u8")      == kiaraType_u8(ctx)
 *  ...
 *
 */
KIARA_API KIARA_Type * kiaraGetContextTypeByName(KIARA_Context *ctx, const char *name);

typedef KIARA_Type * (*KIARA_TypeDeclarator)(KIARA_Context *ctx);

KIARA_API KIARA_Type * kiaraGetOrDeclareContextTypeByName(KIARA_Context *ctx, const char *name, KIARA_TypeDeclarator declarator);

KIARA_API KIARA_Result kiaraLoadLLVMModule(KIARA_Context *ctx, const char *name);

/* Type mapping */

KIARA_API int kiaraMapType(KIARA_Type *abstractType, KIARA_Type *nativeType);
KIARA_API int kiaraMapName(KIARA_Connection *connection, const char *abstractTypePath, const char *nativeTypePath);

/** Register existing type with the new name, in the same namespace where type is defined. */
KIARA_API int kiaraTypedef(const char *newTypeName, KIARA_Type *type);

/* Connection */

/** Opens new connection.
 *  Connection inherits all types declared in the context.
 *  Connection contains all types provided in IDL of the connection end point.
 *
 *  @return new connection or NULL if connection could not be established.
 */
KIARA_API KIARA_Connection * kiaraOpenConnection(KIARA_Context *context,
        const char *uri);

/** Closes connection.
 *  @return KIARA_SUCCESS if connection was successfully closed.
 */
KIARA_API int kiaraCloseConnection(KIARA_Connection *connection);

/** Returns name of the error code */
KIARA_API const char * kiaraGetErrorName(KIARA_Result errorCode);

/** Returns description of the error or NULL if no error occurred. */
KIARA_API const char * kiaraGetConnectionError(KIARA_Connection *connection);

/** Reset error state of a connection */
KIARA_API void kiaraClearConnectionError(KIARA_Connection *connection);

/** Get named type declared in the IDL associated with the open connection.
 *  When no type with the specified name is found, function will lookup in
 *  the parent context of the connection.
 */
KIARA_API KIARA_Type * kiaraGetTypeByName(KIARA_Connection *connection, const char *name);

/** Get type handle from static declaration
 */
KIARA_API KIARA_Type * kiaraGetTypeFromContext(KIARA_Context *ctx, KIARA_GetDeclType declTypeGetter);

/** Get type handle from static declaration
 */
KIARA_API KIARA_Type * kiaraGetTypeFromDecl(KIARA_Connection *connection, KIARA_GetDeclType declTypeGetter);

/** Get type handle from static declaration
 */
KIARA_API KIARA_Type * kiaraGetContextTypeFromDecl(KIARA_Context *context, KIARA_GetDeclType declTypeGetter);

/** Get parent context of the connection. */
KIARA_API KIARA_Context * kiaraGetContext(KIARA_Connection *connection);

/** Generate synchronous client function object accordingly to the IDL and bound datatypes */
KIARA_API KIARA_FuncObj * kiaraGenerateClientFuncObj(KIARA_Connection *connection, const char *idlMethodName, KIARA_GetDeclType declTypeGetter, const char *mapping);

/* Service */

/** Creates new service.
 *  Connection inherits all types declared in the context.
 *  Connection contains all types provided in the IDL.
 *
 *  @return new service or NULL if service could not be created.
 */
KIARA_API KIARA_Service * kiaraNewService(KIARA_Context *context);

/** Destroys service.
 *  @return KIARA_SUCCESS if connection was successfully closed.
 */
KIARA_API KIARA_Result kiaraFreeService(KIARA_Service *service);

/** Returns description of the error or NULL if no error occurred. */
KIARA_API const char * kiaraGetServiceError(KIARA_Service *service);

/** Reset error state of a connection */
KIARA_API void kiaraClearServiceError(KIARA_Service *service);

/** Get named type declared in the IDL associated with the service.
 *  When no type with the specified name is found, function will lookup in
 *  the parent context of the connection.
 */
KIARA_API KIARA_Type * kiaraGetServiceTypeByName(KIARA_Service *service, const char *name);

/** Get type handle from static declaration
 */
KIARA_API KIARA_Type * kiaraGetServiceTypeFromDecl(KIARA_Service *service, KIARA_GetDeclType declTypeGetter);

/** Get parent context of the service. */
KIARA_API KIARA_Context * kiaraGetServiceContext(KIARA_Service *service);

/** Load IDL file describing service(s) provided */
KIARA_API KIARA_Result kiaraLoadServiceIDL(KIARA_Service *service, const char *fileName);

/** Load IDL from string describing service(s) provided */
KIARA_API KIARA_Result kiaraLoadServiceIDLFromString(KIARA_Service *service, const char *idlLanguage, const char *idlContents);

/** Register synchronous service function */
KIARA_API KIARA_Result kiaraRegisterServiceFunc(KIARA_Service *service, const char *idlMethodName, KIARA_GetDeclType declTypeGetter, const char *mapping, KIARA_ServiceFunc func);

/* Server */

KIARA_API KIARA_Server * kiaraNewServer(KIARA_Context *context, const char *host, unsigned int port, const char *configPath);

KIARA_API KIARA_Result kiaraAddService(KIARA_Server *server, const char *path, const char *protocol, KIARA_Service *service);

KIARA_API KIARA_Result kiaraRemoveService(KIARA_Server *server, KIARA_Service *service);

/** Returns description of the error or NULL if no error occurred. */
KIARA_API const char * kiaraGetServerError(KIARA_Server *server);

/** Reset error state of a connection */
KIARA_API void kiaraClearServerError(KIARA_Server *server);

/** Get parent context of the server. */
KIARA_API KIARA_Context * kiaraGetServerContext(KIARA_Server *server);

KIARA_API KIARA_Result kiaraFreeServer(KIARA_Server * server);

KIARA_API KIARA_Result kiaraRunServer(KIARA_Server * server);

/** Get major version number */
KIARA_API int kiaraGetVersionMajor(void);
/** Get minor version number */
KIARA_API int kiaraGetVersionMinor(void);
/** Get patch version number */
KIARA_API int kiaraGetVersionPatch(void);
/** Get repository revision which was used to build KIARA */
KIARA_API const char * kiaraGetRepositoryRevision(void);

/* Debug functions */
KIARA_API void kiaraDbgDumpType(KIARA_Type *type);

KIARA_API void kiaraDbgDumpDeclTypeGetter(KIARA_GetDeclType declTypeGetter);

KIARA_API void kiaraDbgDumpDeclType(const KIARA_DeclType *declType);

KIARA_API void kiaraDbgSimulateCall(KIARA_Service *service, const char *requestData);

KIARA_END_EXTERN_C

#endif /* KIARA_H_INCLUDED */
