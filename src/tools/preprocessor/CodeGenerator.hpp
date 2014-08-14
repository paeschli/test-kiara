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
 * CodeGenerator.hpp
 *
 *  Created on: Nov 21, 2013
 *      Author: rubinste
 */

#ifndef KIARA_CODEGENERATOR_HPP_INCLUDED
#define KIARA_CODEGENERATOR_HPP_INCLUDED

#include "clang/AST/Type.h"
#include "clang/AST/TypeOrdering.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/PointerUnion.h"
#include <string>
#include <map>
#include <vector>
#include <boost/variant.hpp>

namespace clang
{
class ASTContext;
class ValueDecl;
class FunctionDecl;
class Expr;
}

namespace KIARA
{

namespace PP
{

struct ObjectInfo
{
    clang::ValueDecl *decl;

    ObjectInfo(clang::ValueDecl *decl) : decl(decl) { }
};

struct TypeInfo
{
    clang::QualType qualType;

    TypeInfo(clang::QualType qualType) : qualType(qualType) { }
};

class AnnotationParam
{
public:

    enum Kind
    {
        NO_PARAM,
        STRING_PARAM,
        OBJECT_PARAM,
        TYPE_PARAM
    };

    struct None { };

    AnnotationParam() : value_(None()) { }
    AnnotationParam(const std::string &str) : value_(str) { }
    AnnotationParam(const char *str) : value_(str) { }
    AnnotationParam(clang::ValueDecl *decl) : value_(decl) { }
    AnnotationParam(clang::QualType qualType) : value_(qualType) { }

    const std::string & getString() const
    {
        return boost::get<std::string>(value_);
    }

    clang::ValueDecl * getObject() const
    {
        return boost::get<clang::ValueDecl *>(value_);
    }

    clang::QualType getType() const
    {
        return boost::get<clang::QualType>(value_);
    }

    void clear()
    {
        value_ = None();
    }

    AnnotationParam & operator=(const std::string &str)
    {
        value_ = str;
        return *this;
    }

    AnnotationParam & operator=(clang::ValueDecl *decl)
    {
        value_ = decl;
        return *this;
    }

    AnnotationParam & operator=(clang::QualType qualType)
    {
        value_ = qualType;
        return *this;
    }

    Kind getKind() const
    {
        return static_cast<Kind>(value_.which());
    }

    bool isNone() const { return getKind() == NO_PARAM; }

    bool isString() const { return getKind() == STRING_PARAM; }

    bool isObject() const { return getKind() == OBJECT_PARAM; }

    bool isType() const { return getKind() == TYPE_PARAM; }

    static AnnotationParam fromExpr(clang::Expr *expr);

    void dump() const;

private:
    boost::variant<None, std::string, clang::ValueDecl *, clang::QualType> value_;
};

class Annotation
{
public:

    typedef std::vector<AnnotationParam> AnnotationParamList;

    Annotation() : name_(), params_() { }
    Annotation(const std::string &name) : name_(name), params_() { }
    Annotation(const std::string &name, const AnnotationParam &param) : name_(name), params_(1, param) { }

    const std::string & getName() const { return name_; }

    void setName(const std::string &name) { name_ = name; }

    bool isValid() const { return !getName().empty(); }

    unsigned int getNumParams() const { return params_.size(); }

    const AnnotationParam & getParam(unsigned int i) const { return params_[i]; }

    void addParam(const AnnotationParam &param)
    {
        params_.push_back(param);
    }

    const AnnotationParamList & getParams() const { return params_; }

    void clear();

    void dump() const;

private:
    std::string name_;
    AnnotationParamList params_;
};

typedef std::vector<Annotation> AnnotationList;

class CodeGenerator
{
public:
    CodeGenerator(clang::ASTContext &astContext);

    void scan();

    void dump() const;

    void generate(llvm::raw_ostream &out) const;

    static bool isKiaraFuncObjectPtrType(clang::QualType type, std::string *funcTypeName = 0);

    static bool isKiaraFuncObjectType(clang::QualType type, std::string *funcTypeName = 0);

    static bool isKiaraFunction(clang::QualType type, std::string *funcTypeName = 0);

    static bool isKiaraServiceFunction(clang::QualType type, std::string *funcTypeName = 0);

private:
    friend class ScanASTVisitor;

    typedef std::vector<TypeInfo> TypeInfoList;
    typedef std::vector<ObjectInfo> ObjectInfoList;

    typedef std::map<clang::QualType, AnnotationList, clang::QualTypeOrdering> TypeAnnotationMap;

    clang::ASTContext &astContext_;

    TypeInfoList typeInfoList_;
    ObjectInfoList objectInfoList_;
    TypeAnnotationMap typeAnnotations_;

    void addTypeInfo(const TypeInfo &ti);
    void addObjectInfo(const ObjectInfo &oi);
    /** This function modifies value of attrList */
    void addTypeAnnotations(clang::QualType &type, AnnotationList &attributes);

    void dumpTypeAnnotations() const;

    void log(const std::string &message);
    void warning(const std::string &message);

    struct GenTypeInfo
    {

        enum Kind {
            TI_DEFAULT,
            TI_BUILTIN,
            TI_FUNCOBJ,
            TI_FUNCTION,
            TI_POINTER,
            TI_REFERENCE
        };

        Kind kind;
        std::string name;

        GenTypeInfo(const std::string &name, Kind kind = TI_DEFAULT)
            : kind(kind)
            , name(name)
        { }
    };

    typedef std::map<clang::QualType, GenTypeInfo, clang::QualTypeOrdering> GenTypes;

    struct GenContext
    {
        clang::PrintingPolicy &pp;
        llvm::raw_ostream &out;
        GenTypes genTypes;

        GenContext(clang::PrintingPolicy &pp, llvm::raw_ostream &out)
            : pp(pp)
            , out(out)
        { }

        bool isValidType(GenTypes::iterator typeIt) const
        {
            return typeIt != genTypes.end();
        }

        GenTypes::iterator noType() { return genTypes.end(); }

        GenTypes::iterator addType(clang::QualType type, const std::string &name, GenTypeInfo::Kind kind = GenTypeInfo::TI_DEFAULT)
        {
            return genTypes.insert(std::make_pair(type, GenTypeInfo(name, kind))).first;
        }
    };

    /** Generate declaration of the specified type and returns its name.
     *
     */
    GenTypes::iterator generate(clang::QualType type, GenContext &ctx) const;

    /** Generate declaration of the specified type and returns its name.
     *
     */
    GenTypes::iterator generate(clang::ValueDecl *decl, GenContext &ctx) const;

    /** Generate RecordType */
    GenTypes::iterator generateRecord(clang::QualType type, GenContext &ctx) const;

    enum FunctionKind
    {
        KIARA_FUNCTION,
        KIARA_SERVICE
    };

    GenTypes::iterator generateFunction(clang::FunctionDecl *fdecl, FunctionKind kind, GenContext &ctx) const;

};

} // namespace PP

} // namespace KIARA

namespace boost
{

#define _KIARA_HAS_NOTHROW_COPY(C) \
  template <>                      \
  struct has_nothrow_copy< C >     \
    : mpl::true_                   \
  {                                \
  }

_KIARA_HAS_NOTHROW_COPY(::KIARA::PP::AnnotationParam::None);

#undef _KIARA_HAS_NOTHROW_COPY

} // namespace boost

#endif /* KIARA_CODEGENERATOR_HPP_INCLUDED */
