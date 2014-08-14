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
 * CodeGenerator.cpp
 *
 *  Created on: Nov 21, 2013
 *      Author: Dmitri Rubinstein
 */

#include "CodeGenerator.hpp"
#include "ASTUtils.hpp"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Attr.h"
#include <sstream>
#include <set>

//#define KIARA_DO_DEBUG
#include "Debug.hpp"

namespace KIARA
{
namespace PP
{

namespace
{

inline std::string toString(std::vector<std::string> &v)
{
    std::ostringstream oss;
    std::ostream_iterator<std::string> out_it (oss, ", ");
    std::copy ( v.begin(), v.end(), out_it );
    return oss.str();
}

inline bool getStringConstant(clang::Expr *expr, std::string &dest)
{
    if (clang::CastExpr *ce = clang::dyn_cast<clang::CastExpr>(expr))
        expr = ce->getSubExpr();

    if (clang::StringLiteral *sl = clang::dyn_cast<clang::StringLiteral>(expr))
    {
        dest = sl->getString();
        return true;
    }

    return false;
}

} // unnamed namespace

AnnotationParam AnnotationParam::fromExpr(clang::Expr *expr)
{
    if (clang::UnaryExprOrTypeTraitExpr *tyExpr =
            llvm::dyn_cast<clang::UnaryExprOrTypeTraitExpr>(expr))
    {
        if (tyExpr->getKind() == clang::UETT_SizeOf)
        {
            if (!tyExpr->isArgumentType())
            {
                // expression
                clang::Expr *tmp = tyExpr->getArgumentExpr()->IgnoreParens();

                clang::DeclRefExpr *declRefExpr = tmp ? llvm::dyn_cast<clang::DeclRefExpr>(tmp) : 0;

                if (declRefExpr && !declRefExpr->refersToEnclosingLocal())
                {
                    return AnnotationParam(declRefExpr->getDecl());
                }
            }
            else
            {
                return AnnotationParam(tyExpr->getTypeOfArgument());
            }
        }
    }

    {
        std::string strConst;
        if (getStringConstant(expr, strConst))
            return AnnotationParam(strConst);
    }

    return AnnotationParam();
}

void AnnotationParam::dump() const
{
    switch (getKind())
    {
        case NO_PARAM: llvm::errs() << "None"; break;
        case STRING_PARAM: llvm::errs() << "\"" << getString() << "\""; break;
        case OBJECT_PARAM: llvm::errs() << "Object {\n"; getObject()->dump(); llvm::errs() << "}"; break;
        case TYPE_PARAM: llvm::errs() << "Type {\n"; getType().dump(); llvm::errs() << "}"; break;
    }
}

void Annotation::clear()
{
    name_.clear();
    params_.clear();
}

void Annotation::dump() const
{
    llvm::errs() << name_ << "(";
    for (AnnotationParamList::const_iterator it = params_.begin(),
        end = params_.end(); it != end; ++it)
    {
        if (it != params_.begin())
            llvm::errs() << ", ";
        it->dump();
    }
    llvm::errs() << ")";
}

class ScanASTVisitor : public clang::RecursiveASTVisitor<ScanASTVisitor>
{
public:
    ScanASTVisitor(CodeGenerator &specs, clang::ASTContext &ctx)
        : registry_(specs)
        , ctx_(ctx)
    {}

    typedef std::vector<std::string> StringVector;
    typedef StringVector::iterator SVIter;

    bool VisitTypedefNameDecl(clang::TypedefNameDecl *d)
    {
        if (d->isInvalidDecl())
            return true;
#if 0
        llvm::errs() << "TYPEDEF: " << d->getName() << "\n";
        d->dump();

        std::string funcTypeName;
        if (!CodeGenerator::isKiaraFunction(d->getUnderlyingType(), &funcTypeName))
            return true;

        llvm::errs() << " TYPE: "<< funcTypeName << "\n";

        clang::TypeSourceInfo *tsi = d->getTypeSourceInfo();
        clang::TypeLoc TL = tsi->getTypeLoc();

        TL.getBeginLoc().print(llvm::errs(), astContext_.getSourceManager());

        llvm::errs() << "\n";

        clang::SourceRange sr = TL.getSourceRange();

        const char *B = astContext_.getSourceManager().getCharacterData(sr.getBegin());
        const char *E = astContext_.getSourceManager().getCharacterData(sr.getEnd());
        clang::StringRef srcStr(B, E - B);

        llvm::errs() << "SRC: " << srcStr << "\n";

        //ctx_.getSourceManager().
#endif
        return true;
    }

    bool VisitRecordDecl(clang::RecordDecl *d)
    {
        if (d->isInvalidDecl() || !d->isCompleteDefinition())
            return true;

        if (!d->getName().startswith("kiara_reflect"))
            return true;

        // Get the first attribute for the C++ record
        clang::specific_attr_iterator<clang::TypeTagForDatatypeAttr> k = d->specific_attr_begin<clang::TypeTagForDatatypeAttr>();
        if (k == d->specific_attr_end<clang::TypeTagForDatatypeAttr>())
        {
            registry_.warning("Ill-formed Reflection Spec; no annotation attribute found on the reflection structure");
            return true;
        }

        clang::TypeTagForDatatypeAttr *attr = *k;

        clang::QualType ty = attr->getMatchingCType();

        addTypeInfo(ty);

        return true;
    }

    bool VisitCallExpr(clang::CallExpr *e)
    {
        KIARA_DEBUG("VisitCallExpr: " << e->getStmtClassName()); //???DEBUG
        clang::FunctionDecl *f = e->getDirectCallee();
        if (!f)
            return true;

        // Function name
        clang::DeclarationName DeclName = f->getNameInfo().getName();
        clang::IdentifierInfo *id = DeclName.getAsIdentifierInfo();
        if (!id || (id->getName() != "kiara_reflect_get_type_ptr" &&
                id->getName() != "kiara_reflect_object" &&
                id->getName() != "kiara_reflect_object_ext"))
            return false;

        KIARA_IFDEBUG(f->dump());//???DEBUG
        KIARA_DEBUG("");
        KIARA_IFDEBUG(f->dumpDeclContext());//???DEBUG
        KIARA_DEBUG("");

        clang::Expr *arg0 = e->getArg(0);
        if (!arg0)
            return true;

        KIARA_IFDEBUG(arg0->dump());

        if (clang::CastExpr *ce = clang::dyn_cast<clang::CastExpr>(arg0))
            arg0 = ce->getSubExpr();

        clang::UnaryExprOrTypeTraitExpr *tyExpr =
                llvm::dyn_cast<clang::UnaryExprOrTypeTraitExpr>(arg0);
        if (!tyExpr)
            return true;
        if (tyExpr->getKind() != clang::UETT_SizeOf)
            return true;

        AnnotationList annotations;
        {
            clang::CallExpr::arg_iterator ai = e->arg_begin();
            ++ai;
            clang::CallExpr::arg_iterator aend = e->arg_end();

            std::string strParam;
            Annotation anno;
            AnnotationParam annoParam;
            bool firstParam = true;
            while (ai != aend)
            {
                clang::Expr *expr = *ai;
                annoParam = AnnotationParam::fromExpr(expr);

                // check for 'end of parameters' terminator
                if (annoParam.isString() && annoParam.getString() == "kiara-eop")
                {
                    if (anno.isValid())
                        annotations.push_back(anno);
                    anno.clear();
                    firstParam = true;
                }
                else if (firstParam)
                {
                    firstParam = false;
                    if (annoParam.isString())
                        anno.setName(annoParam.getString());
                }
                else
                {
                    anno.addParam(annoParam);
                }

                ++ai;
            }
        }

        //llvm::errs()<<"UEOTTE:\n";
        //tyExpr->dump();

        //llvm::errs()<<"UEOTTE children:\n";
        //for (clang::UnaryExprOrTypeTraitExpr::const_child_range CI = tyExpr->children(); CI; ++CI)
        //{
        //    llvm::errs()<<"CHILD:\n";
        //    (*CI)->dump();
        //}

        if (!tyExpr->isArgumentType())
        {
            // FIXME adding expression to the RTTI should be performed by a different macro
            KIARA_IFDEBUG(tyExpr->dump());
            // expression
            clang::Expr *tmp = tyExpr->getArgumentExpr()->IgnoreParens();

            clang::DeclRefExpr *declRefExpr = tmp ? llvm::dyn_cast<clang::DeclRefExpr>(tmp) : 0;
            KIARA_IFDEBUG(if (declRefExpr) //???DEBUG
                llvm::errs() << "Local declaration: "<<declRefExpr->refersToEnclosingLocal()<<"\n"); //???DEBUG
            if (declRefExpr && !declRefExpr->refersToEnclosingLocal())
            {
                addObjectInfo(declRefExpr->getDecl());
            }
        }
        else
        {
            clang::QualType argType = tyExpr->getTypeOfArgument();
            addTypeInfo(argType);
            if (!annotations.empty())
                registry_.addTypeAnnotations(argType, annotations);
        }

        return true;
    }

    void addTypeInfo(clang::QualType T)
    {
        registry_.addTypeInfo(T);
    }

    void addObjectInfo(clang::ValueDecl *decl)
    {
        registry_.addObjectInfo(ObjectInfo(decl));
    }

private:
    CodeGenerator &registry_;
    clang::ASTContext &ctx_;
};

CodeGenerator::CodeGenerator(clang::ASTContext &astContext)
    : astContext_(astContext)
{
}

void CodeGenerator::scan()
{
    ScanASTVisitor scanVisitor(*this, astContext_);
    clang::TranslationUnitDecl* tuDecl = astContext_.getTranslationUnitDecl();
    clang::DeclContext::decl_iterator i = tuDecl->decls_begin();
    while (i != tuDecl->decls_end())
    {
        scanVisitor.TraverseDecl(*i);
        ++i;
    }

    KIARA_IFDEBUG(dump()); //???DEBUG
    KIARA_IFDEBUG(dumpTypeAnnotations()); //???DEBUG
}

void CodeGenerator::dump() const
{
    for (TypeInfoList::const_iterator it = typeInfoList_.begin(), end = typeInfoList_.end(); it != end; ++it)
    {
        it->qualType.dump();
        llvm::errs() << "\n";
    }

    for (ObjectInfoList::const_iterator it = objectInfoList_.begin(), end = objectInfoList_.end(); it != end; ++it)
    {
        it->decl->dump();
        llvm::errs() << "\n";
    }
}

void CodeGenerator::addTypeInfo(const TypeInfo &ti)
{
    typeInfoList_.push_back(ti);
}

void CodeGenerator::addObjectInfo(const ObjectInfo &oi)
{
    objectInfoList_.push_back(oi);
}

void CodeGenerator::addTypeAnnotations(clang::QualType &type, AnnotationList &attributes)
{
    typeAnnotations_[type].swap(attributes);
    attributes.clear();
}

void CodeGenerator::dumpTypeAnnotations() const
{
    llvm::errs() << "TypeAnnotations:\n";
    for (TypeAnnotationMap::const_iterator it = typeAnnotations_.begin(),
        end = typeAnnotations_.end(); it != end; ++it)
    {
        llvm::errs() << "Type:\n";
        it->first->dump();
        llvm::errs() << "\n";
        for (AnnotationList::const_iterator ai = it->second.begin(), aend = it->second.end(); ai != aend; ++ai)
        {
            ai->dump();
            llvm::errs() << "\n";
        }
    }
}

void CodeGenerator::log(const std::string &message)
{
    llvm::outs() << "LOG: " << message << "\n";
}

void CodeGenerator::warning(const std::string &message)
{
    llvm::outs() << "WARNING: " << message << "\n";
}

void CodeGenerator::generate(llvm::raw_ostream &out) const
{
    clang::LangOptions lo;
    clang::PrintingPolicy pp(lo);
    pp.Indentation = 0;
    pp.SuppressTagKeyword = true;

    GenContext ctx(pp, out);

    for (TypeInfoList::const_iterator it = typeInfoList_.begin(), end = typeInfoList_.end(); it != end; ++it)
    {
        generate(it->qualType, ctx);
    }

    for (ObjectInfoList::const_iterator it = objectInfoList_.begin(), end = objectInfoList_.end(); it != end; ++it)
    {
        generate(it->decl, ctx);
    }
}

bool CodeGenerator::isKiaraFuncObjectPtrType(clang::QualType type, std::string *funcTypeName)
{
    return type->isPointerType() && isKiaraFuncObjectType(type->getPointeeType(), funcTypeName);
}

bool CodeGenerator::isKiaraFuncObjectType(clang::QualType type, std::string *funcTypeName)
{
    if (!type->isRecordType())
        return false;

    const clang::RecordType *rty = type->getAs<clang::RecordType>();

    clang::RecordDecl *decl = rty->getDecl();

    if (!decl->getName().startswith("kr_funcobj"))
        return false;

    if (funcTypeName)
        *funcTypeName = decl->getName().substr(11);

    return true;
}

bool CodeGenerator::isKiaraFunction(clang::QualType type, std::string *funcTypeName)
{
    // This functions works with function pointer types and function types
    if (!type->isFunctionPointerType() && !type->isFunctionType())
        return false;

    const clang::FunctionProtoType *FTy;
    if (type->isFunctionPointerType())
        FTy = type->getPointeeType().getTypePtr()->getAs<clang::FunctionProtoType>();
    else
        FTy = type.getTypePtr()->getAs<clang::FunctionProtoType>();

    if (!FTy)
        return false; // functions without prototype are not supported

    clang::QualType retType = FTy->getResultType().getUnqualifiedType();

    if (!retType->isBuiltinType())
        return false;

    // FIXME: should be KIARA_Result
    {
        const clang::BuiltinType *bty = retType->getAs<clang::BuiltinType>();
        if (!bty || bty->getKind() != clang::BuiltinType::Int)
            return false;
    }

    if (FTy->getNumArgs() < 1)
        return false;

    return isKiaraFuncObjectPtrType(FTy->getArgType(0), funcTypeName);
}

bool CodeGenerator::isKiaraServiceFunction(clang::QualType type, std::string *funcTypeName)
{
    // This functions works with function pointer types and function types
    if (!type->isFunctionPointerType() && !type->isFunctionType())
        return false;

    const clang::FunctionProtoType *FTy;
    if (type->isFunctionPointerType())
        FTy = type->getPointeeType().getTypePtr()->getAs<clang::FunctionProtoType>();
    else
        FTy = type.getTypePtr()->getAs<clang::FunctionProtoType>();

    if (!FTy)
        return false; // functions without prototype are not supported

    clang::QualType retType = FTy->getResultType().getUnqualifiedType();

    if (!retType->isBuiltinType())
        return false;

    // FIXME: should be KIARA_Result
    {
        const clang::BuiltinType *bty = retType->getAs<clang::BuiltinType>();
        if (!bty || bty->getKind() != clang::BuiltinType::Int)
            return false;
    }

    if (FTy->getNumArgs() < 1)
        return false;

    clang::QualType ty = FTy->getArgType(0);

    if (!ty->isPointerType())
        return false;

    ty = ty->getPointeeType();

    if (!ty->isRecordType())
        return false;

    const clang::RecordType *rty = ty->getAs<clang::RecordType>();
    clang::RecordDecl *decl = rty->getDecl();

    if (decl->getName() != "KIARA_ServiceFuncObj")
        return false;

    if (funcTypeName)
    {
        if (const clang::TypedefType *tdefType = type->getAs<clang::TypedefType>())
        {
            *funcTypeName = tdefType->getDecl()->getName();
        }
    }

    return true;
}

CodeGenerator::GenTypes::iterator CodeGenerator::generate(clang::QualType T, GenContext &ctx) const
{
    const bool isCXX = astContext_.getLangOpts().CPlusPlus;

    CodeGenerator::GenTypes::iterator it = ctx.genTypes.find(T);
    if (it != ctx.genTypes.end())
        return it;

    // check for KIARA-specific types
    {
        std::string funcTypeName;
        if (isKiaraFuncObjectPtrType(T, &funcTypeName))
        {
            return ctx.addType(T, funcTypeName, GenTypeInfo::TI_FUNCOBJ);
        }
    }

    {
        std::string funcObjTypeName;
        if (isKiaraFuncObjectType(T, &funcObjTypeName))
        {
            return ctx.addType(T, funcObjTypeName);
        }
    }

    // get typedef name
    clang::StringRef tdefName;
    const clang::TypedefType *tdefType = T->getAs<clang::TypedefType>();
    if (tdefType)
        tdefName = tdefType->getDecl()->getName();

    if (T->isVoidType())
    {
        return ctx.addType(T, "KIARA_VOID", GenTypeInfo::TI_BUILTIN);
    }

    if (T->isFunctionPointerType())
    {
        const clang::FunctionProtoType *FTy = T->getPointeeType().getTypePtr()->getAs<clang::FunctionProtoType>();
        if (!FTy)
            return ctx.noType(); // functions without prototype are not supported

        std::string fullName = T.getAsString(ctx.pp);

        GenTypes::iterator retTypeHandle = generate(FTy->getResultType(), ctx);
        if (!ctx.isValidType(retTypeHandle))
            return ctx.noType();

        for (clang::FunctionProtoType::arg_type_iterator it = FTy->arg_type_begin(), end = FTy->arg_type_end();
            it != end; ++it)
        {
            GenTypes::iterator typeHandle = generate(*it, ctx);
            if (!ctx.isValidType(typeHandle))
                return ctx.noType();
        }

        ctx.out << "/* Unsupported:\n"
                << " * Function type: "<<fullName << "\n"
                << " * Return type: " << retTypeHandle->second.name << "\n"
                << "*/\n";

        return ctx.addType(T, fullName, GenTypeInfo::TI_FUNCTION);
    }

    if (T->isPointerType())
    {
        clang::QualType pointeeType = T->getPointeeType();

        bool isPtrToConstData = pointeeType.isConstQualified();

        if (isPtrToConstData)
        {
            pointeeType.removeLocalConst();
        }

        GenTypes::iterator typeHandle = generate(pointeeType, ctx);

        if (!ctx.isValidType(typeHandle))
            return ctx.noType();

        if (isCXX)
        {
            std::string fullName = T.getAsString();

            return ctx.addType(T, fullName, GenTypeInfo::TI_REFERENCE);
        }

        // handle builtin pointer types
        if (typeHandle->second.name == "KIARA_CHAR" && !isPtrToConstData)
            return ctx.addType(T, "KIARA_CHAR_PTR", GenTypeInfo::TI_BUILTIN);
        else if (typeHandle->second.name == "KIARA_VOID" && !isPtrToConstData)
            return ctx.addType(T, "KIARA_VOID_PTR", GenTypeInfo::TI_BUILTIN);

        std::string fullName = typeHandle->second.name + "_ptr";
        std::string macroName;

        if (isPtrToConstData)
        {
            fullName = "const_" + fullName;
            macroName = "KIARA_DECL_CONST_PTR";
        }
        else
        {
            macroName = "KIARA_DECL_PTR";
        }

        ctx.out << macroName << "(" << fullName << ", " << typeHandle->second.name << ")\n";

        return ctx.addType(T, fullName, GenTypeInfo::TI_POINTER);
    }

    if (T->isReferenceType() && isCXX)
    {
        clang::QualType pointeeType = T->getPointeeType();

        bool isRefToConstData = pointeeType.isConstQualified();

        if (isRefToConstData)
        {
            pointeeType.removeLocalConst();
        }

        GenTypes::iterator typeHandle = generate(pointeeType, ctx);

        if (!ctx.isValidType(typeHandle))
            return ctx.noType();

        std::string fullName = T.getAsString();

        return ctx.addType(T, fullName, GenTypeInfo::TI_POINTER);
    }

    if (T->isBuiltinType())
    {
        if (tdefType)
        {
            const clang::TypedefType *curTdef = tdefType;
            // walk all typedef-s until we find known name
            while (curTdef)
            {
                clang::StringRef tdefName = curTdef->getDecl()->getName();

                if (isCXX)
                    return ctx.addType(T, tdefName, GenTypeInfo::TI_BUILTIN);

#define _BUILTIN_TYPE(c_name, kiara_name)                                   \
            if (tdefName == #c_name)                                        \
                return ctx.addType(T, #kiara_name, GenTypeInfo::TI_BUILTIN)

                _BUILTIN_TYPE(int8_t, KIARA_INT8_T);
                _BUILTIN_TYPE(uint8_t, KIARA_UINT8_T);
                _BUILTIN_TYPE(int16_t, KIARA_INT16_T);
                _BUILTIN_TYPE(uint16_t, KIARA_UINT16_T);
                _BUILTIN_TYPE(int32_t, KIARA_INT32_T);
                _BUILTIN_TYPE(uint32_t, KIARA_UINT32_T);
                _BUILTIN_TYPE(int64_t, KIARA_INT64_T);
                _BUILTIN_TYPE(uint64_t, KIARA_UINT64_T);
                _BUILTIN_TYPE(size_t, KIARA_SIZE_T);
                _BUILTIN_TYPE(ssize_t, KIARA_SSIZE_T);
#undef _BUILTIN_TYPE

                clang::TypedefNameDecl *tdefDecl = curTdef->getDecl();
                curTdef = tdefDecl->getUnderlyingType()->getAs<clang::TypedefType>();
            }
        }

        const clang::BuiltinType *bty = T->getAs<clang::BuiltinType>();

        switch (bty->getKind())
        {
            case clang::BuiltinType::WChar_S:
            case clang::BuiltinType::WChar_U:
                return ctx.addType(T, "KIARA_WCHAR");
            case clang::BuiltinType::Char_S:
            case clang::BuiltinType::Char_U:
                return ctx.addType(T, "KIARA_CHAR");
            case clang::BuiltinType::SChar:  return ctx.addType(T, isCXX ? "signed char" : "KIARA_SCHAR", GenTypeInfo::TI_BUILTIN);
            case clang::BuiltinType::UChar:  return ctx.addType(T, isCXX ? "unsigned char" : "KIARA_UCHAR", GenTypeInfo::TI_BUILTIN);
            case clang::BuiltinType::Short: return ctx.addType(T, isCXX ? "short" : "KIARA_SHORT", GenTypeInfo::TI_BUILTIN);
            case clang::BuiltinType::UShort: return ctx.addType(T, isCXX ? "unsigned short" : "KIARA_USHORT", GenTypeInfo::TI_BUILTIN);
            case clang::BuiltinType::Int: return ctx.addType(T, isCXX ? "int" : "KIARA_INT", GenTypeInfo::TI_BUILTIN);
            case clang::BuiltinType::UInt: return ctx.addType(T, isCXX ? "unsigned int" : "KIARA_UINT", GenTypeInfo::TI_BUILTIN);
            case clang::BuiltinType::Long: return ctx.addType(T, isCXX ? "long" : "KIARA_LONG", GenTypeInfo::TI_BUILTIN);
            case clang::BuiltinType::ULong: return ctx.addType(T, isCXX ? "unsigned long" : "KIARA_ULONG", GenTypeInfo::TI_BUILTIN);
            case clang::BuiltinType::LongLong: return ctx.addType(T, isCXX ? "long long" : "KIARA_LONGLONG", GenTypeInfo::TI_BUILTIN);
            case clang::BuiltinType::ULongLong: return ctx.addType(T, isCXX ? "unsigned long long" : "KIARA_ULONGLONG", GenTypeInfo::TI_BUILTIN);
            case clang::BuiltinType::Float: return ctx.addType(T, isCXX ? "float" : "KIARA_FLOAT", GenTypeInfo::TI_BUILTIN);
            case clang::BuiltinType::Double: return ctx.addType(T, isCXX ? "double" : "KIARA_DOUBLE", GenTypeInfo::TI_BUILTIN);
            case clang::BuiltinType::LongDouble: return ctx.addType(T, isCXX ? "long double" : "KIARA_LONGDOUBLE", GenTypeInfo::TI_BUILTIN);
            default:
                assert(false && "Unknown builtin");
                return ctx.noType();
        }
    }

    if (T->isRecordType())
        return generateRecord(T, ctx);

    // FIXME: Following code is deprecated
#if 0
    if (const clang::TypedefType *tdefType = T->getAs<clang::TypedefType>())
    {
        clang::StringRef name_ = tdefType->getDecl()->getName();
        if (name_.startswith("_kiara_"))
        {
            // kiara builtin wrapper
#define _BUILTIN_TYPE(c_name, kiara_name)                               \
        if (name == "_kiara_" #c_name)                                  \
            return ctx.addType(T, #kiara_name, GenTypeInfo::TI_BUILTIN)

            _BUILTIN_TYPE(size_t, KIARA_SIZE_T);
            _BUILTIN_TYPE(ssize_t, KIARA_SSIZE_T);
            _BUILTIN_TYPE(int8_t, KIARA_INT8_T);
            _BUILTIN_TYPE(uint8_t, KIARA_UINT8_T);
            _BUILTIN_TYPE(int16_t, KIARA_INT16_T);
            _BUILTIN_TYPE(uint16_t, KIARA_UINT16_T);
            _BUILTIN_TYPE(int32_t, KIARA_INT32_T);
            _BUILTIN_TYPE(uint32_t, KIARA_UINT32_T);
            _BUILTIN_TYPE(int64_t, KIARA_INT64_T);
            _BUILTIN_TYPE(uint64_t, KIARA_UINT64_T);

#undef _BUILTIN_TYPE
        }
        return ctx.addType(T, tdefType->getDecl()->getName());
    }
#endif

    {
        std::string fullName = T.getAsString(ctx.pp);
        ctx.out << "/* Unknown DeclType: " << fullName << ":" << T->getTypeClassName() << "*/\n";

        return ctx.noType();
    }

    return ctx.noType();
}

namespace
{
    struct FuncArgInfo
    {
        std::string typeName;
        std::string argName;
        bool isResult;

        FuncArgInfo(const std::string &typeName, const std::string &argName, bool isResult = false)
            : typeName(typeName)
            , argName(argName)
            , isResult(isResult)
        { }
    };
}

CodeGenerator::GenTypes::iterator CodeGenerator::generate(clang::ValueDecl *decl, GenContext &ctx) const
{
    if (isKiaraFunction(decl->getType()))
    {
        if (clang::FunctionDecl *fdecl = clang::dyn_cast<clang::FunctionDecl>(decl))
        {
            return generateFunction(fdecl, KIARA_FUNCTION, ctx);
        }
    }
    else if (isKiaraServiceFunction(decl->getType()))
    {
        if (clang::FunctionDecl *fdecl = clang::dyn_cast<clang::FunctionDecl>(decl))
        {
            return generateFunction(fdecl, KIARA_SERVICE, ctx);
        }
    }
    else
    {
        ctx.out << "// Object " << decl->getName() << "\n";
    }

    return ctx.noType();
}

namespace
{

bool containsAnnotation(const std::string &name, const AnnotationList &annotations)
{
    for (AnnotationList::const_iterator it = annotations.begin(), end = annotations.end(); it != end; ++it)
        if (it->getName() == name)
            return true;
    return false;
}

clang::FieldDecl * findField(clang::DeclContext* declContext, const std::string &name)
{
    for (clang::DeclContext::decl_iterator i = declContext->decls_begin(); i != declContext->decls_end(); ++i)
    {
        clang::FieldDecl* field_decl = llvm::dyn_cast_or_null<clang::FieldDecl>(*i);
        if (!field_decl)
            continue;
        if (field_decl->getName() == name)
            return field_decl;
    }
    return 0;
}

struct StructArrayMember
{
    std::string ptrTypeName;
    std::string ptrMemberName;
    std::string sizeTypeName;
    std::string sizeMemberName;

    StructArrayMember() { }

    StructArrayMember(const std::string &ptrTypeName,
                      const std::string &ptrMemberName,
                      const std::string &sizeTypeName,
                      const std::string &sizeMemberName)
        : ptrTypeName(ptrTypeName)
        , ptrMemberName(ptrMemberName)
        , sizeTypeName(sizeTypeName)
        , sizeMemberName(sizeMemberName)
    {
    }

};

}

CodeGenerator::GenTypes::iterator CodeGenerator::generateRecord(clang::QualType T, GenContext &ctx) const
{
    const bool isCXX = astContext_.getLangOpts().CPlusPlus;
    const clang::RecordType *rty = T->getAs<clang::RecordType>();

    typedef std::vector<std::pair<std::string, std::string> > TypeAndNameList;
    TypeAndNameList typeAndNameList;

    const AnnotationList *annotations = 0;
    {
        TypeAnnotationMap::const_iterator it = typeAnnotations_.find(T);
        if (it != typeAnnotations_.end())
            annotations = &it->second;
    }

    clang::RecordDecl *decl = rty->getDecl();

    clang::DeclContext* declContext = decl->castToDeclContext(decl);

    clang::CXXRecordDecl *cxxdecl = clang::dyn_cast<clang::CXXRecordDecl>(decl);

    bool opaqueType = false;
    typedef std::vector<std::pair<std::string, std::string> > UserAPIList; /* API Name, Function Name */
    typedef std::vector<StructArrayMember> StructArrayMembers; /* Array Member Name, Size Member Name */
    typedef std::set<std::string> ExcludeNameSet;
    UserAPIList userAPIs;
    StructArrayMembers structArrayMembers;
    ExcludeNameSet excludeNames;

    if (annotations)
    {
        for (AnnotationList::const_iterator it = annotations->begin(), end = annotations->end(); it != end; ++it)
        {
            const Annotation &anno = *it;
            if (anno.getName() == "kiara-opaque-object")
                opaqueType = true;
            else if (anno.getName() == "kiara-user-api")
            {
                if (anno.getNumParams() != 2)
                    continue;
                if (!anno.getParam(0).isString())
                    continue;
                if (!anno.getParam(1).isObject())
                    continue;
                userAPIs.push_back(std::make_pair(anno.getParam(0).getString(), anno.getParam(1).getObject()->getName()));
            }
            else if (anno.getName() == "kiara-struct-array-member")
            {
                if (anno.getNumParams() != 2)
                    continue;
                if (!anno.getParam(0).isString()) // pointer member name
                    continue;
                if (!anno.getParam(1).isString()) // array size member name
                    continue;
                // we will output both member names separately
                excludeNames.insert(anno.getParam(0).getString());
                excludeNames.insert(anno.getParam(1).getString());

                StructArrayMember sam;
                sam.ptrMemberName = anno.getParam(0).getString();
                sam.sizeMemberName = anno.getParam(1).getString();

                clang::FieldDecl *ptrMember = findField(declContext, sam.ptrMemberName);
                if (!ptrMember)
                    continue;
                GenTypes::iterator typeHandle = generate(ptrMember->getType(), ctx);
                if (!ctx.isValidType(typeHandle))
                    continue;
                sam.ptrTypeName = typeHandle->second.name;
                clang::FieldDecl *sizeMember = findField(declContext, sam.sizeMemberName);
                if (!sizeMember)
                    continue;
                typeHandle = generate(sizeMember->getType(), ctx);
                if (!ctx.isValidType(typeHandle))
                    continue;
                sam.sizeTypeName = typeHandle->second.name;
                structArrayMembers.push_back(sam);
            }
        }
    }

    // name of the struct can only be used in C++ mode
    std::string recordTypeName;
    if (cxxdecl)
    {
        if (const clang::TypedefType *tdefType = T->getAs<clang::TypedefType>())
        {
            recordTypeName = ASTUtils::getFullTypeName(tdefType->getDecl(), astContext_);
        }
        else
        {
            recordTypeName = ASTUtils::getFullTypeName(cxxdecl, astContext_);
        }
    }
    else if (const clang::TypedefType *tdefType = T->getAs<clang::TypedefType>())
    {
        recordTypeName = tdefType->getDecl()->getName();
    }
    else
    {
        ctx.out << "/* Error: '"<<decl->getName()<<"' struct does not have typedef name */\n";
        return ctx.noType();
    }

    if (!opaqueType)
    {
        for (clang::DeclContext::decl_iterator i = declContext->decls_begin(); i != declContext->decls_end(); ++i)
        {
            clang::FieldDecl* field_decl = llvm::dyn_cast_or_null<clang::FieldDecl>(*i);
            if (field_decl != 0)
            {
                if (field_decl->isAnonymousStructOrUnion()) // FIXME
                    continue;
                GenTypes::iterator typeHandle = generate(field_decl->getType(), ctx);
                if (!ctx.isValidType(typeHandle))
                    return ctx.noType();

                std::string fieldName = field_decl->getName();

                if (excludeNames.find(fieldName) != excludeNames.end())
                    continue;

                typeAndNameList.push_back(std::make_pair(typeHandle->second.name, fieldName));
            }
        }
    }

    if (userAPIs.empty())
    {
        if (isCXX)
            ctx.out << "KIARA_CXX_DECL_STRUCT(";
        else
            ctx.out << "KIARA_DECL_STRUCT(";
    }
    else
    {
        if (opaqueType)
        {
            if (isCXX)
                ctx.out << "KIARA_CXX_DECL_OPAQUE_TYPE(";
            else
                ctx.out << "KIARA_DECL_OPAQUE_TYPE(";
        }
        else
        {
            if (isCXX)
                ctx.out << "KIARA_CXX_DECL_STRUCT_WITH_API(";
            else
                ctx.out << "KIARA_DECL_STRUCT_WITH_API(";
        }
    }

    ctx.out << recordTypeName << ",\n";

    if (!opaqueType)
    {
        for (TypeAndNameList::const_iterator it = typeAndNameList.begin(), end = typeAndNameList.end();
            it != end; ++it)
        {
            if (isCXX)
                ctx.out << "  KIARA_CXX_STRUCT_MEMBER(" << it->second << ")\n";
            else
                ctx.out << "  KIARA_STRUCT_MEMBER(" << it->first << ", " << it->second << ")\n";
        }

        for (StructArrayMembers::const_iterator it = structArrayMembers.begin(), end = structArrayMembers.end();
            it != end; ++it)
        {
            if (isCXX)
                ctx.out << "  KIARA_CXX_STRUCT_ARRAY_MEMBER("
                        << it->ptrMemberName << ", "
                        << it->sizeMemberName << ")\n";
            else
                ctx.out << "  KIARA_STRUCT_ARRAY_MEMBER("
                        << it->ptrTypeName << ", " << it->ptrMemberName << ", "
                        << it->sizeTypeName << ", " << it->sizeMemberName << ")\n";
        }
    }

    if (!userAPIs.empty())
    {
        if (!opaqueType)
            ctx.out << "  ,\n";

        for (UserAPIList::const_iterator it = userAPIs.begin(), end = userAPIs.end(); it != end; ++it)
        {
            if (isCXX)
                ctx.out << "  KIARA_CXX_USER_API(" << it->first <<", " << it->second << ")\n";
            else
                ctx.out << "  KIARA_USER_API(" << it->first <<", " << it->second << ")\n";
        }
    }

    ctx.out << ")\n";

    return ctx.addType(T, recordTypeName);
}


CodeGenerator::GenTypes::iterator CodeGenerator::generateFunction(clang::FunctionDecl *fdecl, FunctionKind kind, GenContext &ctx) const
{
    const bool isCXX = astContext_.getLangOpts().CPlusPlus;

    typedef std::vector<FuncArgInfo> FuncArgInfoList;
    FuncArgInfoList funcArgInfoList;

    {
        clang::FunctionDecl::param_iterator it = fdecl->param_begin();
        clang::FunctionDecl::param_iterator end = fdecl->param_end();
        ++it;
        while (it != end)
        {
            clang::ParmVarDecl *pvd = *it;

            // Check attributes
            bool isResult = false;
            for (clang::specific_attr_iterator<clang::AnnotateAttr> ai = pvd->specific_attr_begin<clang::AnnotateAttr>(),
                aend = pvd->specific_attr_end<clang::AnnotateAttr>(); ai != aend; ++ai)
            {
                clang::AnnotateAttr* attribute = *ai;
                llvm::StringRef attributeText = attribute->getAnnotation();
                if (attributeText == "kiara-result")
                    isResult = true;
            }

            GenTypes::iterator typeHandle = generate(pvd->getType(), ctx);
            if (!ctx.isValidType(typeHandle))
                return ctx.noType();
            funcArgInfoList.push_back(FuncArgInfo(typeHandle->second.name, pvd->getName(), isResult));

            ++it;
        }
    }

    const char * declMacro;
    const char * argMacro;
    const char * resultMacro;
    llvm::StringRef funcName = fdecl->getName();

    // remove prefix that prevents name collision
    if (funcName.startswith("_kpp_"))
        funcName = funcName.substr(5);

    switch (kind)
    {
        case KIARA_FUNCTION:
            if (isCXX)
            {
                declMacro = "KIARA_CXX_DECL_FUNC";
                argMacro = "KIARA_CXX_FUNC_ARG";
                resultMacro = "KIARA_CXX_FUNC_RESULT";
            }
            else
            {
                declMacro = "KIARA_DECL_FUNC";
                argMacro = "KIARA_FUNC_ARG";
                resultMacro = "KIARA_FUNC_RESULT";
            }
            break;
        case KIARA_SERVICE:
            if (isCXX)
            {
                declMacro = "KIARA_CXX_DECL_SERVICE";
                argMacro = "KIARA_CXX_SERVICE_ARG";
                resultMacro = "KIARA_CXX_SERVICE_RESULT";
            }
            else
            {
                declMacro = "KIARA_DECL_SERVICE";
                argMacro = "KIARA_SERVICE_ARG";
                resultMacro = "KIARA_SERVICE_RESULT";
            }
            break;
    }

    ctx.out << declMacro << "(" << funcName << ",\n";

    for (FuncArgInfoList::const_iterator it = funcArgInfoList.begin(), end = funcArgInfoList.end();
        it != end; ++it)
    {
        // FIXME: 'result' name is used to signal result
        if (it->isResult)
            ctx.out << "  " << resultMacro << "(";
        else
            ctx.out << "  " << argMacro << "(";

        ctx.out << it->typeName << ", " << it->argName << ")\n";
    }

    ctx.out << ")\n";

    return ctx.noType();
}

} // namespace PP
} // namespace KIARA
