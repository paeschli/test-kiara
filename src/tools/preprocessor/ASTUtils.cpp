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
 * ASTUtils.cpp
 *
 *  Created on: Nov 21, 2013
 *      Author: Dmitri Rubinstein
 */

#include "ASTUtils.hpp"
#include <llvm/Support/raw_ostream.h>

//#define KIARA_DO_DEBUG
#include "Debug.hpp"

namespace KIARA
{

namespace ASTUtils
{

std::string getLocalName(const clang::DeclContext *DC)
{
    if (DC->isTranslationUnit())
        return "";

    if (const clang::NamespaceDecl *NS = llvm::dyn_cast<clang::NamespaceDecl>(DC)) {
      if ((NS->isAnonymousNamespace() || NS->isInline()))
        return "";
      if (NS->getIdentifier())
        return NS->getNameAsString();
      else
        return "<anonymous>";
    } else if (const clang::ClassTemplateSpecializationDecl *Spec
                 = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(DC)) {
      clang::LangOptions lo;
      clang::PrintingPolicy Policy(lo); //FIXME

      const clang::TemplateArgumentList &TemplateArgs = Spec->getTemplateArgs();
      std::string TemplateArgsStr;
      llvm::raw_string_ostream rso(TemplateArgsStr);
      clang::TemplateSpecializationType::PrintTemplateArgumentList(
                                              rso,
                                              TemplateArgs.data(),
                                              TemplateArgs.size(),
                                              Policy);
      return std::string(Spec->getIdentifier()->getName()) + TemplateArgsStr;
    } else if (const clang::TagDecl *Tag = llvm::dyn_cast<clang::TagDecl>(DC)) {
      if (clang::TypedefNameDecl *Typedef = Tag->getTypedefNameForAnonDecl())
        return Typedef->getIdentifier()->getName();
      else if (Tag->getIdentifier())
        return Tag->getIdentifier()->getName();
      else
        return "";
    }

    return "";
}

/// Appends the given scope to the end of a string vector.
void appendScope(const clang::DeclContext *DC, std::vector<std::string> &parts)
{
  if (DC->isTranslationUnit())
      return;
  appendScope(DC->getParent(), parts);

  std::string localName = getLocalName(DC);
  if (localName.length())
      parts.push_back(localName);
}

void TypePartsGetter::getTypeParts(clang::QualType T, std::vector<std::string> &parts)
{
    KIARA_DEBUG("(1) Call process with type " << T.getAsString());

  if (!T->isInstantiationDependentType() || T->isDependentType())
    T = T.getCanonicalType();
  else {
    // Desugar any types that are purely sugar.
    do {
      // Don't desugar through template specialization types that aren't
      // type aliases. We need to mangle the template arguments as written.
      if (const clang::TemplateSpecializationType *TST
              = llvm::dyn_cast<clang::TemplateSpecializationType>(T))
        if (!TST->isTypeAlias())
          break;

      clang::QualType Desugared
        = T.getSingleStepDesugaredType(ctx_);
      if (Desugared == T)
        break;

      T = Desugared;
    } while (true);
  }

  KIARA_DEBUG("(2) Call process with type " << T.getAsString());

  clang::SplitQualType split = T.split();
  clang::Qualifiers quals = split.Quals;
  const clang::Type *ty = split.Ty;

#if 0
  bool isSubstitutable = quals || !clang::isa<clang::BuiltinType>(T);
  if (isSubstitutable && mangleSubstitution(T))
      return;
#endif

  // If we're mangling a qualified array type, push the qualifiers to
  // the element type.
  if (quals && clang::isa<clang::ArrayType>(T)) {
    ty = ctx_.getAsArrayType(T);
    quals = clang::Qualifiers();

    // Note that we don't update T: we want to add the
    // substitution at the original type.
  }

  if (quals) {
    //mangleQualifiers(quals);
    // Recurse:  even if the qualified type isn't yet substitutable,
    // the unqualified type might be.
    getTypeParts(clang::QualType(ty, 0), parts);
  } else {
      KIARA_DEBUG("Call process with type " << ty->getTypeClassName());

      switch (ty->getTypeClass()) {
#define ABSTRACT_TYPE(CLASS, PARENT)
#define NON_CANONICAL_TYPE(CLASS, PARENT) \
    case clang::Type::CLASS: \
      llvm_unreachable("can't mangle non-canonical type " #CLASS "Type"); \
      return;
#define TYPE(CLASS, PARENT) \
    case clang::Type::CLASS: \
      getTypeParts(static_cast<const clang::CLASS##Type*>(ty), parts); \
      break;
#include "clang/AST/TypeNodes.def"
    }
  }

  // Add the substitution.
//      if (isSubstitutable)
//        addSubstitution(T);
}

namespace {

/// \brief Retrieve the declaration context that should be used when mangling
/// the given declaration.
static const clang::DeclContext *getEffectiveDeclContext(const clang::Decl *D)
{
    // The ABI assumes that lambda closure types that occur within
    // default arguments live in the context of the function. However, due to
    // the way in which Clang parses and creates function declarations, this is
    // not the case: the lambda closure type ends up living in the context
    // where the function itself resides, because the function declaration itself
    // had not yet been created. Fix the context here.
    if (const clang::CXXRecordDecl *RD = clang::dyn_cast<clang::CXXRecordDecl>(D))
    {
        if (RD->isLambda())
            if (clang::ParmVarDecl *ContextParam
                = clang::dyn_cast_or_null<clang::ParmVarDecl>(RD->getLambdaContextDecl()))
                return ContextParam->getDeclContext();
    }

    return D->getDeclContext();
}

static const clang::DeclContext *getEffectiveParentContext(const clang::DeclContext *DC)
{
    return getEffectiveDeclContext(llvm::cast<clang::Decl>(DC));
}

static const clang::DeclContext *IgnoreLinkageSpecDecls(const clang::DeclContext *DC)
{
    while (llvm::isa<clang::LinkageSpecDecl>(DC))
    {
        DC = getEffectiveParentContext(DC);
    }

    return DC;
}

}

void TypePartsGetter::getTypeParts(const clang::NamedDecl *ND, std::vector<std::string> &parts)
{
    const clang::DeclContext *DC = getEffectiveDeclContext(ND);
    DC = IgnoreLinkageSpecDecls(DC);

    appendScope(DC, parts);

    parts.push_back(ND->getIdentifier()->getName());
}

void TypePartsGetter::getTypeParts(const clang::TagType *T, std::vector<std::string> &parts)
{
    TypePartsGetter::getTypeParts(T->getDecl(), parts);
}

void TypePartsGetter::getTypeParts(clang::BuiltinType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}

void TypePartsGetter::getTypeParts(clang::ComplexType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}

void TypePartsGetter::getTypeParts(clang::PointerType const *ty, std::vector<std::string> &parts)
{
    getTypeParts(ty->getPointeeType(), parts);
}

void TypePartsGetter::getTypeParts(clang::BlockPointerType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}

void TypePartsGetter::getTypeParts(clang::LValueReferenceType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}

void TypePartsGetter::getTypeParts(clang::RValueReferenceType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::MemberPointerType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::ConstantArrayType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::IncompleteArrayType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::VariableArrayType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::DependentSizedArrayType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::DependentSizedExtVectorType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::VectorType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::ExtVectorType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::FunctionProtoType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::FunctionNoProtoType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::UnresolvedUsingType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::TypeOfExprType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::TypeOfType const  *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::DecltypeType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::UnaryTransformType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::RecordType const *ty, std::vector<std::string> &parts)
{
    getTypeParts(static_cast<const clang::TagType*>(ty), parts);
}
void TypePartsGetter::getTypeParts(clang::EnumType const *ty, std::vector<std::string> &parts)
{
    getTypeParts(static_cast<const clang::TagType*>(ty), parts);
}
void TypePartsGetter::getTypeParts(clang::TemplateTypeParmType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::SubstTemplateTypeParmPackType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::TemplateSpecializationType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::AutoType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::InjectedClassNameType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::DependentNameType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::DependentTemplateSpecializationType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::PackExpansionType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::ObjCObjectType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::ObjCInterfaceType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::ObjCObjectPointerType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}
void TypePartsGetter::getTypeParts(clang::AtomicType const *ty, std::vector<std::string> &parts)
{
    KIARA_DEBUG("TypePartsGetter: " << ty->getTypeClassName() << " type <" << clang::QualType(ty, 0).getAsString() << ">");
}

std::string TypePartsGetter::joinTypeParts(const std::vector<std::string> &typeParts)
{
    std::string name;
    typedef std::vector<std::string>::const_iterator SVIter;
    for (SVIter it = typeParts.begin(), end = typeParts.end(); it != end; ++it)
    {
        if (!name.empty())
            name += "::";
        name += *it;
    }
    return name;
}

} // namespace ASTUtils

} // namespace KIARA
