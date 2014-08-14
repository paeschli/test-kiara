%name kiaraParser
%token_prefix TOK_

%include {
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

#define KIARA_LIB
#include <KIARA/Core/Exception.hpp>
#include <KIARA/DB/Attributes.hpp>
#include <KIARA/IDL/IDLParserContext.hpp>
#include <KIARA/IDL/kiaray.hpp>
#include <KIARA/DB/DerivedTypes.hpp>
#include <KIARA/Compiler/IR.hpp>
#include <cassert>
#include <cstdlib>

#include <boost/ptr_container/ptr_vector.hpp>
#include <iostream>
#include <sstream>


// #define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

#define PDEBUG(message)                                        \
   DFC_DEBUG("["<<ctx->fileName<<":"<<ctx->lineNum<<"] "       \
        << message)

#define PERROR(message)                                        \
do {                                                           \
   std::ostringstream msgs;                                    \
   msgs << "[ERROR:"<<ctx->fileName<<":"<<ctx->lineNum<<"] "   \
        << message;                                            \
   ctx->addParserError(msgs.str());                            \
} while(false)

namespace {

inline void destroyObject(KIARA::IDLToken *object)
{
	delete object;
}

} // unnamed namespace

#define MODULE (*ctx->getModule())
#define WORLD MODULE.getWorld()

//   std::cerr<<"DESTROY "<<token<<" : "<<(*token)<<std::endl;

namespace KIARA
{

void IDLParserContext::initParser()
{
    assert(parser == 0);
    parser = kiaraParserAlloc(malloc);
}

void IDLParserContext::destroyParser()
{
    assert(parser != 0);
    kiaraParserFree(parser, free);
}

bool IDLParserContext::parse(bool strict)
{
    assert(scanner != 0);
    assert(parser != 0);

    int tokId = 0;
    while ((tokId = lex()) > 0)
    {
        assert(! isScannerError());
        //std::cerr<<"tok="<<tokId<<" yytext="<<getText()<<std::endl;
        //std::cout<<" ID = "<<token.str<<std::endl;

        IDLToken *newToken = new IDLToken(token);
        //std::cerr<<"ALLOC "<<newToken<<" : "<<*newToken<<std::endl;

        kiaraParser(parser, tokId, newToken, this);

        if (parsingFailed)
           break;

        //std::cerr<<std::endl;
    }

    if (isScannerError())
    {
        //DEBUG
        std::cerr<<"SCANNER ERROR:"<<std::endl;
        std::cerr<<getScannerError()<<std::endl;
        //END DEBUG
    }

    // notify parser about EOF
    kiaraParser(parser, 0, 0, this);

    bool error = isScannerError() || parsingFailed;

    if (!parserErrors.empty())
    {
        //DEBUG
        std::cerr<<"PARSER ERROR(S):"<<std::endl;
        for (std::vector<std::string>::const_iterator it = parserErrors.begin(),
             end = parserErrors.end(); it != end; ++it)
        {
            std::cerr<<*it<<std::endl;
        }
        //END DEBUG
        if (strict)
           error = true;
    }

    return !error;
}

} // namespace KIARA

namespace
{

inline void destroyObject(KIARA::Object* object)
{
    if (object)
    {
        object->release();
    }
}


template <class T>
inline T * unwrap(const typename T::Ptr &ptr)
{
    if (ptr)
        ptr->addRef();
    return ptr.get();
}

inline void destroyObject(KIARA::AnnotationList *object)
{
    delete object;
}

inline void destroyObject(KIARA::TypeList *object)
{
    delete object;
}

struct EnumDef
{
    std::string name;
    KIARA::IR::PrimLiteral::Ptr constant;

    EnumDef() { }
    EnumDef(const std::string &name, const KIARA::IR::PrimLiteral::Ptr &constant)
        : name(name)
        , constant(constant)
    { }

    ~EnumDef() { }
};

inline void destroyObject(EnumDef *object)
{
    delete object;
}

struct EnumDefList
{
    typedef boost::ptr_vector<EnumDef>::iterator iterator;
    typedef boost::ptr_vector<EnumDef>::const_iterator const_iterator;
    boost::ptr_vector<EnumDef> constants;
    int64_t nextConst;

    EnumDefList() : constants(), nextConst(0) { }

    const_iterator begin() const { return constants.begin(); }
    const_iterator end() const { return constants.end(); }

    size_t size() const { return constants.size(); }
};

inline void destroyObject(EnumDefList *object)
{
    delete object;
}

typedef int64_t FieldIdentifier;

inline void destroyObject(FieldIdentifier *object)
{
	delete object;
}

struct Field
{
    FieldIdentifier *id;
    KIARA::Type::Ptr type;
    std::string name;
    KIARA::Expr::Ptr value;
    KIARA::AnnotationList *annotationList;

    Field()
        : id(0)
        , type(0)
        , name()
        , value()
        , annotationList(0)
    { }

    ~Field()
    {
        destroyObject(id);
        destroyObject(annotationList);
    }

    const std::string & getName()
    {
        return name;
    }
};

inline void destroyObject(Field *object)
{
    delete object;
}

struct FieldList
{
    typedef boost::ptr_vector<Field>::iterator iterator;
    typedef boost::ptr_vector<Field>::const_iterator const_iterator;
    boost::ptr_vector<Field> fields;

    size_t size() const { return fields.size(); }

    std::vector<KIARA::Type::Ptr> getTypes() const
    {
        std::vector<KIARA::Type::Ptr> result;
        for (const_iterator it = fields.begin(), end = fields.end();
               it != end; ++it)
        {
            result.push_back(it->type);
        }
        return result;
    }

    std::vector<std::string> getNames() const
    {
        std::vector<std::string> result;
        for (const_iterator it = fields.begin(), end = fields.end();
               it != end; ++it)
        {
            result.push_back(it->name);
        }
        return result;
    }

    void initAnnotations(const KIARA::StructType::Ptr &sty)
    {
        const size_t n = fields.size();
        for (size_t i = 0; i < n; ++i)
        {
            KIARA::ElementData & elemData = sty->getElementDataAt(i);
            if (fields[i].value)
                elemData.setAttributeValue<KIARA::DefaultFieldValueAttr>(fields[i].value);
            if (fields[i].annotationList && !fields[i].annotationList->empty())
            {
                elemData.setAttributeValue<KIARA::AnnotationListAttr>(*fields[i].annotationList);
            }
        }
    }

    KIARA::FunctionType::Ptr createFunction(const std::string &name,
                                            const KIARA::Type::Ptr &returnType,
                                            KIARA::AnnotationList *funcAnnotList = 0,
                                            KIARA::AnnotationList *retAnnotList = 0)
    {
        //KIARA::World &world = returnType->getWorld();
        KIARA::FunctionType::ParamTypes paramTypes(fields.size());

        for (size_t i = 0; i < fields.size(); ++i)
        {
            paramTypes[i].second = fields[i].type;
            paramTypes[i].first = fields[i].name;
        }

        KIARA::FunctionType::Ptr fty = KIARA::FunctionType::create(name, returnType, paramTypes);

        // set parameter annotations
        for (size_t i = 0; i < fty->getNumParams(); ++i)
        {
            if (fields[i].annotationList && !fields[i].annotationList->empty())
            {
                fty->getParamElementDataAt(i).setAttributeValue<KIARA::AnnotationListAttr>(*fields[i].annotationList);
            }
        }

        // set return type annotations
        if (retAnnotList && !retAnnotList->empty())
        {
            fty->getReturnElementData().setAttributeValue<KIARA::AnnotationListAttr>(*retAnnotList);
        }

        // set function annotations
        if (funcAnnotList && !funcAnnotList->empty())
        {
            fty->setAttributeValue<KIARA::AnnotationListAttr>(*funcAnnotList);
        }

        return fty;
    }

};

inline void destroyObject(FieldList *object)
{
    delete object;
}

struct Function
{
    KIARA::FunctionType::Ptr type;

    Function()
        : type(0)
    { }

    ~Function()
    {
    }

    std::string getName() const
    {
        return type ? type->getTypeName() : "";
    }
};

inline void destroyObject(Function *object)
{
    delete object;
}

struct FunctionList
{
    typedef boost::ptr_vector<Function>::iterator iterator;
    typedef boost::ptr_vector<Function>::const_iterator const_iterator;
    boost::ptr_vector<Function> functions;

    size_t size() const { return functions.size(); }

    std::vector<KIARA::Type::Ptr> getTypes() const
    {
        std::vector<KIARA::Type::Ptr> result;
        for (const_iterator it = functions.begin(), end = functions.end();
               it != end; ++it)
        {
            result.push_back(it->type);
        }
        return result;
    }

    std::vector<std::string> getNames() const
    {
        std::vector<std::string> result;
        for (const_iterator it = functions.begin(), end = functions.end();
               it != end; ++it)
        {
            result.push_back(it->getName());
        }
        return result;
    }
};

inline void destroyObject(FunctionList *object)
{
	delete object;
}

} // unnamed namespace

}

%token_type {KIARA::IDLToken*}
%extra_argument {KIARA::IDLParserContext *ctx}
%token_destructor { destroyObject($$); }
%parse_failure {
  ctx->parsingFailed = true;
}
%parse_accept {
  ctx->parsingFailed = false;
}

%syntax_error {
    /*UNUSED_PARAMETER(yymajor);  *//* Silence some compiler warnings */
    /*assert( TOKEN.z[0] );  *//* The tokenizer always gives us a token */
    if (TOKEN)
    {
        PERROR("near "<<*TOKEN<<" : syntax error");
    }
    else
    {
        if (ctx->isScannerError())
           PERROR("Invalid token");
        else
           PERROR("Unexpected end of file");
    }
}

%type base_type {KIARA::Type *}
%destructor base_type { destroyObject($$); }
%type field_type {KIARA::Type *}
%destructor field_type { destroyObject($$); }
%type function_type {KIARA::Type *}
%destructor function_type { destroyObject($$); }
%type generic_type {KIARA::Type *}
%destructor generic_type { destroyObject($$); }
%type simple_base_type {KIARA::Type *}
%destructor simple_base_type { destroyObject($$); }
%type enum {KIARA::Type *}
%destructor enum { destroyObject($$); }
%type senum {KIARA::Type *}
%destructor senum { destroyObject($$); }
%type struct {KIARA::Type *}
%destructor struct { destroyObject($$); }
%type xception {KIARA::Type *}
%destructor xception { destroyObject($$); }
%type service {KIARA::Type *}
%destructor service { destroyObject($$); }
%type annotated_type_definition {KIARA::Type *}
%destructor annotated_type_definition { destroyObject($$); }
%type non_annotated_type_definition {KIARA::Type *}
%destructor non_annotated_type_definition { destroyObject($$); }
%type type_definition {KIARA::Type *}
%destructor type_definition { destroyObject($$); }
%type annotation_def {KIARA::Type *}
%destructor annotation_def { destroyObject($$); }
%type typedef {KIARA::Type *}
%destructor typedef { destroyObject($$); }

%type simple_generic_type {KIARA::Type *}
%destructor simple_generic_type { destroyObject($$); }
%type generic_type_arg {KIARA::Type *}
%destructor generic_type_arg { destroyObject($$); }
%type generic_type_arg_list {KIARA::TypeList *}
%destructor generic_type_arg_list { destroyObject($$); }

%type const_value {KIARA::Expr *}
%destructor const_value { destroyObject($$); }
%type field_value {KIARA::Expr *}
%destructor field_value { destroyObject($$); }

%type annotation {KIARA::Annotation *}
%destructor annotation { destroyObject($$); }

%type annotation_list {KIARA::AnnotationList *}
%destructor annotation_list { destroyObject($$); }
%type non_empty_annotation_list {KIARA::AnnotationList *}
%destructor non_empty_annotation_list { destroyObject($$); }

%type enum_def {EnumDef *}
%destructor enum_def { destroyObject($$); }

%type enum_def_list {EnumDefList *}
%destructor enum_def_list { destroyObject($$); }

%type field_identifier { FieldIdentifier * }
%destructor field_identifier { destroyObject($$); }

%type field {Field *}
%destructor field { destroyObject($$); }

%type field_list {FieldList *}
%destructor field_list { destroyObject($$); }

%type function {Function *}
%destructor function { destroyObject($$); }

%type function_list {FunctionList *}
%destructor function_list { destroyObject($$); }


/* Program */
program ::= header_list definition_list . { PDEBUG("Program -> Headers DefinitionList"); }

capture_doc_text ::= .
destroy_doc_text ::= .

/* Header list */
header_list ::= header_list destroy_doc_text header . { PDEBUG("HeaderList -> HeaderList Header"); }
header_list ::= . { PDEBUG("HeaderList -> "); }

/* Header */
header ::= include . { PDEBUG("Header -> Include"); }
header ::= NAMESPACE IDENTIFIER IDENTIFIER(D) . { PDEBUG("Header -> NAMESPACE IDENTIFIER IDENTIFIER:"<<D->str);
  destroyObject(D);
}
header ::= NAMESPACE STAR IDENTIFIER . { PDEBUG("Header -> NAMESPACE * IDENTIFIER"); }

/*
-- DEPRECATED --
header ::= CPP_NAMESPACE IDENTIFIER .
header ::= CPP_INCLUDE LITERAL .
header ::= PHP_NAMESPACE IDENTIFIER .
header ::= PY_MODULE IDENTIFIER .
header ::= PERL_PACKAGE IDENTIFIER .
header ::= RUBY_NAMESPACE IDENTIFIER .
header ::= SMALLTALK_CATEGORY ST_IDENTIFIER .
header ::= SMALLTALK_PREFIX IDENTIFIER .
header ::= JAVA_PACKAGE IDENTIFIER .
header ::= COCOA_PREFIX IDENTIFIER .
header ::= XSD_NAMESPACE LITERAL .
header ::= CSHARP_NAMESPACE IDENTIFIER .
header ::= DELPHI_NAMESPACE IDENTIFIER .
*/

include ::= INCLUDE LITERAL . {
  PDEBUG("Include -> tok_include tok_literal");
}

definition_list ::= definition_list capture_doc_text definition . {
  PDEBUG("DefinitionList -> DefinitionList Definition");
}
definition_list ::= . { PDEBUG("DefinitionList -> "); }

definition ::= const . { PDEBUG("Definition -> Const"); }
definition ::= type_definition(B) . {
    PDEBUG("Definition -> TypeDefinition");
    destroyObject(B); // this only decrements reference count
}
definition ::= service(B) . {
    PDEBUG("Definition -> Service");
    destroyObject(B); // this only decrements reference count
}

type_definition(A) ::= non_annotated_type_definition(B) . {
    A = B;
}
type_definition(A) ::= annotation_list(AL) annotated_type_definition(B) . {
    if (AL && !AL->empty() && B)
    {
        B->setAttributeValue<KIARA::AnnotationListAttr>(*AL);
    }
    destroyObject(AL);

    A = B;
}

non_annotated_type_definition(A) ::= typedef(B) . { PDEBUG("TypeDefinition -> Typedef"); A = B; }
annotated_type_definition(A) ::= enum(B) . { PDEBUG("TypeDefinition -> Enum"); A = B; }
annotated_type_definition(A) ::= senum(B) . { PDEBUG("TypeDefinition -> Senum"); A = B; }
annotated_type_definition(A) ::= struct(B) . { PDEBUG("TypeDefinition -> Struct"); A = B; }
annotated_type_definition(A) ::= xception(B) . { PDEBUG("TypeDefinition -> Xception"); A = B; }
annotated_type_definition(A) ::= annotation_def(B) . { PDEBUG("TypeDefinition -> AnnotationDef"); A = B; }

typedef(A) ::= TYPEDEF field_type(C) IDENTIFIER(D) . {
    PDEBUG("TypeDef -> TYPEDEF FieldType IDENTIFIER:"<<(D->str));

    if (C)
    {
        DFC_DEBUG("BIND "<<MODULE.getTypeName(C)<<" TO "<<D->str);
        try
        {
            MODULE.bindType(D->str, C);
            MODULE.addTypeDeclaration(KIARA::Module::TYPEDEF, C);
        }
        catch (KIARA::Exception &e)
        {
            PERROR(e.what());
        }
    }

    A = C;
    destroyObject(D);
}

comma_or_semicolon_optional ::= COMMA .
comma_or_semicolon_optional ::= SEMI .
comma_or_semicolon_optional ::= .

enum(A) ::= ENUM IDENTIFIER(ENAME) LBRACE enum_def_list(EDLIST) RBRACE . {
  PDEBUG("Enum -> ENUM IDENTIFIER { EnumDefList }");

  KIARA::EnumType::Ptr ety = KIARA::EnumType::create(WORLD, ENAME->str);
  for (EnumDefList::const_iterator it = EDLIST->begin(), end = EDLIST->end(); it != end; ++it)
  {
      ety->addConstant(it->name, it->constant);
  }
  try
  {
      MODULE.bindType(ENAME->str, ety);
      MODULE.addTypeDeclaration(KIARA::Module::NEWTYPE, ety);
  }
  catch (KIARA::Exception &e)
  {
      PERROR(e.what());
  }

  destroyObject(ENAME);
  destroyObject(EDLIST);
  A = unwrap<KIARA::Type>(ety);
}

enum_def_list(A) ::= enum_def_list(B) enum_def(C) . {
  assert(C != 0);
  assert(B != 0);

  PDEBUG("EnumDefList -> EnumDefList EnumDef");

  if (!C->constant)
  {
    C->constant = new KIARA::IR::PrimLiteral(B->nextConst, WORLD);
  }
  B->nextConst = C->constant->getAsInt() + 1;

  B->constants.push_back(C);
  A = B;
}
enum_def_list(A) ::= . {
  PDEBUG("EnumDefList -> ");
  A = new EnumDefList;
}

enum_def(A) ::= capture_doc_text IDENTIFIER(ENAME) EQ INT_CONSTANT(CVAL) comma_or_semicolon_optional . {
  PDEBUG("EnumDef -> IDENTIFIER = INT_CONSTANT");

  A = new EnumDef(ENAME->str, new KIARA::IR::PrimLiteral(CVAL->val.iconst, WORLD));

  destroyObject(ENAME);
  destroyObject(CVAL);
}
enum_def(A) ::= capture_doc_text IDENTIFIER(ENAME) comma_or_semicolon_optional . {
  PDEBUG("EnumDef -> IDENTIFIER");

  A = new EnumDef(ENAME->str, 0);

  destroyObject(ENAME);
}

senum ::= SENUM IDENTIFIER LBRACE senum_def_list RBRACE . {
  PDEBUG("Senum -> SENUM IDENTIFIER { SenumDefList }");
}

senum_def_list ::= senum_def_list senum_def . {
  PDEBUG("SenumDefList -> SenumDefList SenumDef");
}
senum_def_list ::= . {
  PDEBUG("SenumDefList -> ");
}

senum_def ::= LITERAL comma_or_semicolon_optional . {
  PDEBUG("SenumDef -> LITERAL");
}

const ::= CONST field_type(FTYPE) IDENTIFIER(CONSTID) EQ const_value(CVAL) comma_or_semicolon_optional . {
  PDEBUG("Const -> CONST FieldType IDENTIFIER = ConstValue");
  destroyObject(FTYPE);
  destroyObject(CONSTID);
  destroyObject(CVAL);
}

const_value(A) ::= INT_CONSTANT(B) . {
  PDEBUG("ConstValue => INT_CONSTANT");
  A = unwrap<KIARA::IR::PrimLiteral>(new KIARA::IR::PrimLiteral(B->val.iconst, WORLD));
  destroyObject(B);
}
const_value(A) ::= DUB_CONSTANT(B) . {
  PDEBUG("ConstValue => DUB_CONSTANT");
  A = unwrap<KIARA::IR::PrimLiteral>(new KIARA::IR::PrimLiteral(B->val.dconst, WORLD));
  destroyObject(B);
}
const_value(A) ::= LITERAL(B) . {
    PDEBUG("ConstValue => LITERAL");
    A = unwrap<KIARA::IR::PrimLiteral>(new KIARA::IR::PrimLiteral(B->str, WORLD));
    destroyObject(B);
}
const_value(A) ::= IDENTIFIER(B) . {
    PDEBUG("ConstValue => IDENTIFIER");
    A = unwrap<KIARA::IR::PrimLiteral>(new KIARA::IR::PrimLiteral(B->str, WORLD));
    destroyObject(B);
}
const_value ::= const_list .   { PDEBUG("ConstValue => ConstList"); }
const_value ::= const_map .    { PDEBUG("ConstValue => ConstMap"); }

const_list ::= LBRACKET const_list_contents RBRACKET . {
  PDEBUG("ConstList => [ ConstListContents ]");
}

const_list_contents ::= const_list_contents const_value comma_or_semicolon_optional . {
  PDEBUG("ConstListContents => ConstListContents ConstValue CommaOrSemicolonOptional");
}
const_list_contents ::= . { PDEBUG("ConstListContents =>"); }

const_map ::= LBRACE const_map_contents RBRACE . {
    PDEBUG("ConstMap => { ConstMapContents }");
}

const_map_contents ::= const_map_contents const_value COLON const_value comma_or_semicolon_optional . {
    PDEBUG("ConstMap => { ConstMapContents }");
}
const_map_contents ::= . {
    PDEBUG("ConstMapContents =>");
}

struct_head ::= STRUCT .
struct_head ::= UNION .

struct(A) ::= struct_head IDENTIFIER(C) xsd_all LBRACE field_list(FL) RBRACE . {
    PDEBUG("Struct -> tok_struct tok_identifier { FieldList }");

    KIARA::StructType::Ptr s = KIARA::StructType::create(WORLD, C->str, FL->size());
    s->setElements(FL->getTypes());
    s->setElementNames(FL->getNames());
    FL->initAnnotations(s);

    try
    {
        MODULE.bindType(C->str, s);
        MODULE.addTypeDeclaration(KIARA::Module::NEWTYPE, s);
    }
    catch (KIARA::Exception &e)
    {
        PERROR(e.what());
        s = 0;
    }

    destroyObject(FL);
    destroyObject(C);
    A = unwrap<KIARA::Type>(s);
}

xsd_all ::= XSD_ALL .
xsd_all ::= .

xsd_optional ::= XSD_OPTIONAL .
xsd_optional ::= .

xsd_nillable ::= XSD_NILLABLE .
xsd_nillable ::= .

xsd_attributes ::= XSD_ATTRS LBRACE field_list RBRACE .
xsd_attributes ::= .

xception(A) ::= XCEPTION IDENTIFIER(XNAME) LBRACE field_list(FL) RBRACE . {
    PDEBUG("Xception -> XCEPTION IDENTIFIER { FieldList }");

    KIARA::StructType::Ptr s = KIARA::StructType::create(WORLD, XNAME->str, FL->size());
    s->setElements(FL->getTypes());
    s->setElementNames(FL->getNames());
    FL->initAnnotations(s);

    s->setAttributeValue<KIARA::ExceptionTypeAttr>(true);

    try
    {
        MODULE.bindType(XNAME->str, s);
        MODULE.addTypeDeclaration(KIARA::Module::NEWTYPE, s);
    }
    catch (KIARA::Exception &e)
    {
        PERROR(e.what());
        s = 0;
    }

    destroyObject(FL);
    destroyObject(XNAME);
    A = unwrap<KIARA::Type>(s);
}

annotation_def(A) ::= ANNOTATION IDENTIFIER(ANAME) LBRACE field_list(FL) RBRACE . {
    PDEBUG("AnnotationDef -> ANNOTATION IDENTIFIER { FieldList }");

    KIARA::StructType::Ptr s = KIARA::StructType::create(WORLD, ANAME->str, FL->size());
    s->setElements(FL->getTypes());
    s->setElementNames(FL->getNames());
    FL->initAnnotations(s);

    s->setAttributeValue<KIARA::AnnotationTypeAttr>(true);

    try
    {
        MODULE.bindType(ANAME->str, s);
        MODULE.addTypeDeclaration(KIARA::Module::NEWTYPE, s);
    }
    catch (KIARA::Exception &e)
    {
        PERROR(e.what());
        s = 0;
    }

    destroyObject(ANAME);
    destroyObject(FL);
    A = unwrap<KIARA::Type>(s);
}

service(A) ::= annotation_list(AL) SERVICE IDENTIFIER(SNAME) extends LBRACE flag_args function_list(FL) unflag_args RBRACE . {
    PDEBUG("Service -> SERVICE IDENTIFIER { FunctionList }");
    KIARA::ServiceType::Ptr s = KIARA::ServiceType::create(WORLD, SNAME->str, FL->size());
    s->setElements(FL->getTypes());
    s->setElementNames(FL->getNames());

    if (AL && !AL->empty())
    {
        s->setAttributeValue<KIARA::AnnotationListAttr>(*AL);
    }

    try
    {
        MODULE.bindType(SNAME->str, s);
        MODULE.addTypeDeclaration(KIARA::Module::NEWTYPE, s);
    }
    catch (KIARA::Exception &e)
    {
        PERROR(e.what());
        s = 0;
    }

    destroyObject(AL);
    destroyObject(SNAME);
    destroyObject(FL);
    A = unwrap<KIARA::Type>(s);
}

flag_args ::= .
unflag_args ::= .

extends ::= EXTENDS IDENTIFIER . {
  PDEBUG("Extends -> EXTENDS IDENTIFIER");
}
extends ::= .

function_list(A) ::= function_list(B) function(C) . {
  PDEBUG("FunctionList -> FunctionList Function");
  if (C)
     B->functions.push_back(C);
  A = B;
}
function_list(A) ::= . {
  PDEBUG("FunctionList -> ");
  A = new FunctionList;
}

function(A) ::= capture_doc_text annotation_list(FAL) function_type(RETTYPE) annotation_list(RAL) IDENTIFIER(FNAME) LPAREN field_list(FL) RPAREN throws comma_or_semicolon_optional . {
  PDEBUG("Function");
  A = 0;

  if (RETTYPE)
  {
      A = new Function;
      A->type = FL->createFunction(FNAME->str, RETTYPE, FAL, RAL);
  }
  else
  {
      PERROR("No return type for function "<<(FNAME->str)<<" specified");
  }

  destroyObject(FAL);
  destroyObject(RAL);
  destroyObject(RETTYPE);
  destroyObject(FNAME);
  destroyObject(FL);
}

throws ::= THROWS LPAREN field_list RPAREN . {
  PDEBUG("Throws -> tok_throws ( FieldList )");
}
throws ::= .

field_list(A) ::= field_list(B) field(C) . {
  assert(C != 0);

  PDEBUG("FieldList -> FieldList , Field");

  B->fields.push_back(C);
  A = B;
}
field_list(A) ::= . {
  PDEBUG("FieldList -> ");
  A = new FieldList;
}

field(A) ::= capture_doc_text field_identifier(FID) annotation_list(AL) field_requiredness field_type(FTYPE) IDENTIFIER(FNAME) field_value(FVAL) xsd_optional xsd_nillable xsd_attributes comma_or_semicolon_optional . {
  PDEBUG("INT_CONSTANT : Field -> FieldType IDENTIFIER");

  A = new Field;

  A->id = FID;
  A->type = FTYPE;
  A->name = FNAME->str;
  A->value = FVAL;
  A->annotationList = AL;

  destroyObject(FTYPE);
  destroyObject(FNAME);
  destroyObject(FVAL); // this only decrements reference count
}

field_identifier(A) ::= INT_CONSTANT(B) COLON . {
   A = new FieldIdentifier(B->val.iconst);
}
field_identifier(A) ::= . {
   A = 0;
}

field_requiredness ::= REQUIRED .
field_requiredness ::= OPTIONAL .
field_requiredness ::= .

field_value(A) ::= EQ const_value(B) . { A = B; }
field_value ::= .

function_type(A) ::= field_type(B) . {
  PDEBUG("FunctionType -> FieldType");
  A = B;
}
function_type(A) ::= VOID . {
  PDEBUG("FunctionType -> VOID");
  A = unwrap<KIARA::Type>(WORLD.type_void());
}

field_type(A) ::= IDENTIFIER(B) . {
  PDEBUG("FieldType -> IDENTIFIER:"<<(B->str));
  A = unwrap<KIARA::Type>(MODULE.lookupType( B->str ));

  if (!A)
  {
      // FIXME: Throw exception ?
      PERROR("Unknown type identifier : "<<B->str);
  }

  destroyObject(B);
}
field_type(A) ::= base_type(B) . {
  PDEBUG("FieldType -> BaseType");
  A = B;
}
field_type(A) ::= generic_type(B) . {
  PDEBUG("FieldType -> GenericType");
  A = B;
}

base_type(A) ::= simple_base_type(B) . {
  PDEBUG("BaseType -> SimpleBaseType");
  A = B;
}

simple_base_type(A) ::= STRING . {
  PDEBUG("BaseType -> STRING");
  A = unwrap<KIARA::PrimType>(WORLD.type_string());
}
simple_base_type ::= BINARY . { PDEBUG("BaseType -> BINARY"); }
simple_base_type ::= SLIST .  { PDEBUG("BaseType -> SLIST"); }
simple_base_type(A) ::= BOOLEAN . {
  PDEBUG("BaseType -> BOOLEAN");
  A = unwrap<KIARA::PrimType>(WORLD.type_boolean());
}
simple_base_type(A) ::= I8 . {
  PDEBUG("BaseType -> I8");
  A = unwrap<KIARA::PrimType>(WORLD.type_i8());
}
simple_base_type(A) ::= U8 . {
  PDEBUG("BaseType -> U8");
  A = unwrap<KIARA::PrimType>(WORLD.type_u8());
}
simple_base_type(A) ::= I16 . {
  PDEBUG("BaseType -> I16");
  A = unwrap<KIARA::PrimType>(WORLD.type_i16());
}
simple_base_type(A) ::= U16 . {
  PDEBUG("BaseType -> U16");
  A = unwrap<KIARA::PrimType>(WORLD.type_u16());
}
simple_base_type(A) ::= I32 . {
  PDEBUG("BaseType -> I32");
  A = unwrap<KIARA::PrimType>(WORLD.type_i32());
}
simple_base_type(A) ::= U32 . {
  PDEBUG("BaseType -> U32");
  A = unwrap<KIARA::PrimType>(WORLD.type_u32());
}
simple_base_type(A) ::= I64 . {
  PDEBUG("BaseType -> I64");
  A = unwrap<KIARA::PrimType>(WORLD.type_i64());
}
simple_base_type(A) ::= U64 . {
  PDEBUG("BaseType -> U64");
  A = unwrap<KIARA::PrimType>(WORLD.type_u64());
}
simple_base_type(A) ::= FLOAT . {
  PDEBUG("BaseType -> FLOAT");
  A = unwrap<KIARA::PrimType>(WORLD.type_float());
}
simple_base_type(A) ::= DOUBLE . {
  PDEBUG("BaseType -> DOUBLE");
  A = unwrap<KIARA::PrimType>(WORLD.type_double());
}
simple_base_type(A) ::= ANY . {
  PDEBUG("BaseType -> ANY");
  A = unwrap<KIARA::AnyType>(WORLD.type_any());
}

generic_type(A) ::= simple_generic_type(B) . {
  PDEBUG("GenericType -> SimpleGenericType");
  A = B;
}

simple_generic_type(A) ::= IDENTIFIER(TNAME) LT generic_type_arg_list(TL) GT . {
    PDEBUG("SimpleGenericType -> IDENTIFIER < GenericTypeArgList >");

    A = 0;
    if (TNAME->str == "array")
    {
        if (!TL || (TL->size() != 1 && TL->size() != 2))
            PERROR("Array argument list must have single type and optional size");
        else
        {
            KIARA::Type::Ptr elementType = (*TL)[0];
            if (TL->size() == 2)
            {
                KIARA::PrimValueType::Ptr sizeType = KIARA::dyn_cast<KIARA::PrimValueType>((*TL)[1]);
                if (sizeType && sizeType->getValue().getType() == KIARA::BT_INT64_T)
                {
                    A = unwrap<KIARA::Type>(
                        KIARA::FixedArrayType::get(elementType, sizeType->getValue().get_i64()));
                }
                else
                {
                    PERROR("Second argument of array type must be integer constant");
                }
            }
            else
            {
                A = unwrap<KIARA::Type>(KIARA::ArrayType::get(elementType));
            }
        }
    }
    else
    {
        PERROR("Unsupported generic type: "<<TNAME->str);
    }

    destroyObject(TNAME);
    destroyObject(TL);
}

generic_type_arg(A) ::= IDENTIFIER(B) . {
    PDEBUG("generic_type_arg -> IDENTIFIER:"<<(B->str));
    A = unwrap<KIARA::Type>(MODULE.lookupType( B->str ));

    if (!A)
    {
        // FIXME: Throw exception ?
        PERROR("Unknown type identifier : "<<B->str);
    }

    destroyObject(B);
}
generic_type_arg(A) ::= VOID . {
    PDEBUG("generic_type_arg -> VOID");
    A = unwrap<KIARA::Type>(WORLD.type_void());
}
generic_type_arg(A) ::= base_type(B) . {
    A = B;
}
generic_type_arg(A) ::= generic_type(B) . {
    A = B;
}
generic_type_arg(A) ::= INT_CONSTANT(B) . {
    A = unwrap<KIARA::Type>(KIARA::PrimValueType::get(WORLD, B->val.iconst));
    destroyObject(B);
}
generic_type_arg(A) ::= DUB_CONSTANT(B) . {
    A = unwrap<KIARA::Type>(KIARA::PrimValueType::get(WORLD, B->val.dconst));
    destroyObject(B);
}
generic_type_arg(A) ::= LITERAL(B) . {
    A = unwrap<KIARA::Type>(KIARA::PrimValueType::get(WORLD, B->str));
    destroyObject(B);
}
generic_type_arg ::= const_list .
generic_type_arg ::= const_map .

generic_type_arg_list(A) ::= generic_type_arg_list(B) COMMA generic_type_arg(C) . {
    PDEBUG("GenericTypeArgList -> GenericTypeArgList , FieldType");

    if (C)
    {
        B->push_back(C);
        destroyObject(C); // this only decrements reference count
    }
    A = B;
}

generic_type_arg_list(A) ::= generic_type_arg(B) . {
    PDEBUG("GenericTypeArgList -> FieldType");

    A = new KIARA::TypeList;
    if (B)
    {
        A->push_back(B);
        destroyObject(B); // this only decrements reference count
    }
}

annotation_list(A) ::= LBRACKET non_empty_annotation_list(B) RBRACKET . {
    PDEBUG("AnnotationList -> [ NonEmptyAnnotationList ]");
    
    A = B;
}

annotation_list(A) ::= . {
    PDEBUG("AnnotationList -> ");

    A = new KIARA::AnnotationList;
}

non_empty_annotation_list(A) ::= non_empty_annotation_list(B) COMMA annotation(C) . {
    PDEBUG("NonEmptyAnnotationList -> NonEmptyAnnotationList , Annotation");

    if (C)
    {
        B->push_back(C);
        destroyObject(C); // this only decrements reference count
    }
    A = B;
}

non_empty_annotation_list(A) ::= annotation(B) . {
    A = new KIARA::AnnotationList;
    if (B)
    {
        A->push_back(B);
        destroyObject(B); // this only decrements reference count
    }
}

annotation(A) ::= IDENTIFIER(ANAME) . {
  PDEBUG("Annotation -> IDENTIFIER:"<<(ANAME->str));

  A = 0;

  KIARA::Type::Ptr ty = MODULE.lookupType( ANAME->str );
  if (!ty)
  {
      // FIXME: Throw exception ?
      PERROR("Unknown type identifier : "<<ANAME->str);
  }
  else
  {
      KIARA::StructType::Ptr sty = KIARA::dyn_cast<KIARA::StructType>(ty);
      if (!sty || !sty->getAttributeValue<KIARA::AnnotationTypeAttr>())
      {
          PERROR("Type '"<<(ANAME->str)<<"' is not an annotation type");
      }
      else
      {
          A = unwrap<KIARA::Annotation>(new KIARA::Annotation(sty));
      }
  }

  destroyObject(ANAME);
}

annotation(A) ::= IDENTIFIER(ANAME) LPAREN annotation_arg_list RPAREN . {
  PDEBUG("Annotation -> IDENTIFIER:"<<(ANAME->str)<<" ( AnnotationArgList )");

  A = 0;

  KIARA::Type::Ptr ty = MODULE.lookupType( ANAME->str );
  if (!ty)
  {
      // FIXME: Throw exception ?
      PERROR("Unknown type identifier : "<<ANAME->str);
  }
  else
  {
      KIARA::StructType::Ptr sty = KIARA::dyn_cast<KIARA::StructType>(ty);
      if (!sty || !sty->getAttributeValue<KIARA::AnnotationTypeAttr>())
      {
          PERROR("Type '"<<(ANAME->str)<<"' is not an annotation type");
      }
      else
      {
          // FIXME : parse argument list
          A = unwrap<KIARA::Annotation>(new KIARA::Annotation(sty));
      }
  }

  destroyObject(ANAME);
}

annotation_arg ::= IDENTIFIER EQ const_value .
annotation_arg ::= const_value .

annotation_arg_list ::= non_empty_annotation_arg_list .
annotation_arg_list ::= .

non_empty_annotation_arg_list ::= non_empty_annotation_arg_list COMMA annotation_arg . {
  PDEBUG("NonEmptyAnnotationArgList -> NonEmptyAnnotationArgList , AnnotationArg");
}
non_empty_annotation_arg_list ::= annotation_arg . {
  PDEBUG("NonEmptyAnnotationArgList -> AnnotationArg");
}

// Local Variables:
// tab-width:4
// indent-tabs-mode:nil
// End:
// vim:ts=4:sts=4:sw=4:tw=80:expandtab
