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
 * kiara_derived_impl.c
 *
 *  Created on: 21.11.2012
 *      Author: Dmitri Rubinstein
 */
#define KIARA_LIB
#include <KIARA/Common/Config.h>
#include <KIARA/kiara.h>
#include <stdio.h>

const char * kiaraGetErrorName(KIARA_Result errorCode)
{
    switch (errorCode)
    {
        case KIARA_NO_ERROR: return "no error";
        case KIARA_GENERIC_ERROR: return "generic error";
        case KIARA_INPUT_ERROR: return "input error";
        case KIARA_OUTPUT_ERROR: return "output error";
        case KIARA_CONNECTION_ERROR: return "connection error";
        case KIARA_API_ERROR: return "API error";
        case KIARA_INIT_ERROR: return "initialization error";
        case KIARA_FINI_ERROR: return "finalization error";
        case KIARA_INVALID_VALUE: return "invalid value";
        case KIARA_INVALID_TYPE: return "invalid type";
        case KIARA_INVALID_OPERATION: return "invalid operation";
        case KIARA_INVALID_ARGUMENT: return "invalid argument";
        case KIARA_UNSUPPORTED_FEATURE: return "unsupported feature";
        case KIARA_CONFIG_ERROR: return "config error";
        case KIARA_NETWORK_ERROR: return "network error";
        case KIARA_REQUEST_ERROR: return "request error";
        case KIARA_RESPONSE_ERROR: return "response error";
        case KIARA_INVALID_RESPONSE: return "invalid response";
        case KIARA_EXCEPTION: return "exception response";
        default:
            break;
    }
    return "<unknown error code>";
}

static const char * getDeclTypeName(KIARA_GetDeclType decl, const char *defaultName)
{
    return decl ? decl()->name : defaultName;
}

void kiaraDbgDumpDeclType(const KIARA_DeclType *declType)
{
    if (!declType)
        return;

    printf("KIARA DeclType: %s", declType->name);
    switch (declType->typeKind)
    {
        case KIARA_TYPE_BUILTIN: printf(" is builtin\n"); break;

        case KIARA_TYPE_ANNOTATED:
        {
            KIARA_DeclAnnotated *annotatedDecl = declType->typeDecl.annotatedDecl;

            printf(" is annotated type decl\n");
            printf(" annotation for type: %s\nannotation: %s\nsemantics: %s\n",
                   getDeclTypeName(annotatedDecl->type, "NULL"),
                   annotatedDecl->annotation,
                   annotatedDecl->semantics);
        }
        break;

        case KIARA_TYPE_ENUM:
        {
            KIARA_DeclEnum *enumDecl = declType->typeDecl.enumDecl;
            KIARA_DeclEnumConstant *constant;
            size_t i;

            printf(" is enum\n");
            printf("  size: %lu\n", (unsigned long)enumDecl->size);
            for (i = 0; i < enumDecl->numConstants; ++i)
            {
                constant = &enumDecl->constants[i];
                printf("  %li: %s = %li\n",
                       (long)i, constant->name, (long)constant->value);
            }
        }
        break;

        case KIARA_TYPE_STRUCT:
        {
            KIARA_DeclStruct *structDecl = declType->typeDecl.structDecl;
            KIARA_DeclStructMember *member;
            size_t i;

            printf(" is struct\n");
            printf("  size: %li\n", (long)structDecl->size);
            for (i = 0; i < structDecl->numMembers; ++i)
            {
                member = &structDecl->members[i];
                if (member->mainName)
                    printf("  %li: %s : offset = %li type = %s main = %s\n", (long)i,
                           member->name, (long)member->offset, getDeclTypeName(member->type, "NULL"),
                           member->mainName);
                else
                    printf("  %li: %s : offset = %li type = %s\n", (long)i,
                           member->name, (long)member->offset, getDeclTypeName(member->type, "NULL"));
            }
        }
        break;

        case KIARA_TYPE_FUNC:
        {
            KIARA_DeclFunc *funcDecl = declType->typeDecl.funcDecl;
            KIARA_DeclFuncArgument *arg;
            size_t i;

            printf(" is function\n");

            printf("  return type: %s\n", getDeclTypeName(funcDecl->returnType, "NULL"));

            for (i = 0; i < funcDecl->numArgs; ++i)
            {
                arg = &funcDecl->args[i];
                printf("  %li: %s : type = %s\n", (long)i, arg->name,
                       getDeclTypeName(arg->type, "NULL"));
            }
        }
        break;

        case KIARA_TYPE_SERVICE:
        {
            printf(" is service\n");
        }
        break;

        case KIARA_TYPE_POINTER:
        {
            KIARA_DeclPtr *ptrDecl = declType->typeDecl.ptrDecl;
            printf(" is pointer to %s\n", getDeclTypeName(ptrDecl->elementType, "NULL"));
        }
        break;
        case KIARA_TYPE_REFERENCE:
        {
            KIARA_DeclPtr *refDecl = declType->typeDecl.ptrDecl;
            printf(" is reference to %s\n", getDeclTypeName(refDecl->elementType, "NULL"));
        }
        break;
        case KIARA_TYPE_ARRAY:
        {
            KIARA_DeclPtr *arrayDecl = declType->typeDecl.ptrDecl;
            printf(" is array with elements of type %s\n", getDeclTypeName(arrayDecl->elementType, "NULL"));
        }
        break;
        case KIARA_TYPE_FIXED_ARRAY:
        {
            KIARA_DeclFixedArray *arrayDecl = declType->typeDecl.arrayDecl;
            printf(" is fixed array\n  type = %s size = %li\n",
                   getDeclTypeName(arrayDecl->elementType, "NULL"),
                   arrayDecl->size);
        }
        break;
        case KIARA_TYPE_FIXED_ARRAY_2D: printf(" is fixed 2D array\n");
        {
            KIARA_DeclFixedArray2D *array2DDecl = declType->typeDecl.array2DDecl;
            printf(" is fixed 2D array\n  type = %s rows = %li cols = %li\n",
                   getDeclTypeName(array2DDecl->elementType, "NULL"),
                   array2DDecl->numRows, array2DDecl->numCols);
        }
        break;
        case KIARA_TYPE_OPAQUE: printf(" is opaque\n"); break;
        case KIARA_TYPE_ENCRYPTED:
        {
            KIARA_DeclEncrypted *encryptedDecl = declType->typeDecl.encryptedDecl;
            printf(" is encrypted type = %s\n",
                   getDeclTypeName(encryptedDecl->elementType, "NULL"));
        }
        break;
    }
}

void kiaraDbgDumpDeclTypeGetter(KIARA_GetDeclType declTypeGetter)
{
    KIARA_DeclType *declType;
    if (!declTypeGetter)
        return;
    // FIXME For C++ we need to use global lock around declTypeGetter.
    declType = declTypeGetter();
    if (!declType)
        return;
    kiaraDbgDumpDeclType(declType);
}
