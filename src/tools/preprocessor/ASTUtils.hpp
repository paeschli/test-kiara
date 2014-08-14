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
 * ASTUtils.hpp
 *
 *  Created on: Nov 21, 2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_ASTUTILS_HPP_INCLUDED
#define KIARA_ASTUTILS_HPP_INCLUDED

#include <clang/AST/DeclBase.h>
#include <clang/AST/ASTContext.h>
#include <string>

namespace KIARA
{

namespace ASTUtils
{

std::string getLocalName(const clang::DeclContext *DC);

void appendScope(const clang::DeclContext *DC, std::vector<std::string> &Buffer);

class TypePartsGetter
{
public:

    TypePartsGetter(clang::ASTContext &ctx) :
        ctx_(ctx)
    {
    }

#define ABSTRACT_TYPE(CLASS, PARENT)
#define NON_CANONICAL_TYPE(CLASS, PARENT)
#define TYPE(CLASS, PARENT) void getTypeParts(const clang::CLASS##Type *T, std::vector<std::string> &parts);
#include "clang/AST/TypeNodes.def"

    void getTypeParts(clang::QualType T, std::vector<std::string> &parts);
    void getTypeParts(const clang::TagType *T, std::vector<std::string> &parts);
    void getTypeParts(const clang::NamedDecl *ND, std::vector<std::string> &parts);

    static std::string joinTypeParts(const std::vector<std::string> &typeParts);

private:
    clang::ASTContext &ctx_;
};

inline std::string getFullTypeName(clang::QualType T, clang::ASTContext &ctx)
{
    TypePartsGetter tpg(ctx);
    std::vector<std::string> typeParts;
    tpg.getTypeParts(T, typeParts);
    return TypePartsGetter::joinTypeParts(typeParts);
}

inline std::string getFullTypeName(const clang::TagType *T, clang::ASTContext &ctx)
{
    TypePartsGetter tpg(ctx);
    std::vector<std::string> typeParts;
    tpg.getTypeParts(T, typeParts);
    return TypePartsGetter::joinTypeParts(typeParts);
}

inline std::string getFullTypeName(const clang::NamedDecl *ND, clang::ASTContext &ctx)
{
    TypePartsGetter tpg(ctx);
    std::vector<std::string> typeParts;
    tpg.getTypeParts(ND, typeParts);
    return TypePartsGetter::joinTypeParts(typeParts);
}

} // namespace ASTUtils

} // namespace KIARA

#endif /* KIARA_UTILS_HPP_INCLUDED */
