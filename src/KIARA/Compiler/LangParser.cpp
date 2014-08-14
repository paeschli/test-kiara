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
 * LangParser.cpp
 *
 *  Created on: 01.03.2013
 *      Author: Dmitri Rubinstein
 */
#define KIARA_COMPILER_LIB
#include "LangParser.hpp"
#include <KIARA/DB/World.hpp>
#include <DFC/Base/Core/ObjectFactory.hpp>
#include <DFC/Base/Core/ObjectMacros.hpp>
#include <DFC/Base/Utils/StaticInit.hpp>
#include <boost/bind.hpp>
#include "IRUtils.hpp"
#include "Lexer.hpp"
#include "Mangler.hpp"

// #define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

// Based on http://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/

namespace KIARA
{

namespace Compiler
{

#define PARSER_ERROR(message)                                   \
do {                                                            \
   std::ostringstream msgs;                                     \
   msgs << message;                                             \
   error(msgs.str());                                           \
   return 0;                                                    \
} while(false)

#define PARSER_ERROR_AT(message, loc)                           \
do {                                                            \
   std::ostringstream msgs;                                     \
   msgs << message;                                             \
   error(msgs.str(), loc);                                      \
   return 0;                                                    \
} while(false)

#define PARSER_WRONG_TOKEN(token, message)                      \
do {                                                            \
   ungetToken(token);                                           \
   std::ostringstream msgs;                                     \
   msgs << message << " (found token " << token << ")";         \
   error(msgs.str(), token.loc);                                \
   return 0;                                                    \
} while(false)

#define EXPR_ERROR(message) PARSER_ERROR(message)
#define TYPE_ERROR(message) PARSER_ERROR(message)
#define PROTO_ERROR(message) PARSER_ERROR(message)
#define ERROR_TOKEN(token)                                      \
        do {                                                    \
           std::ostringstream msgs;                             \
           msgs << token.getError();                            \
           error(msgs.str(), token.loc);                        \
           return 0;                                            \
        } while(false)

#define EXPR_WRONG_TOKEN(token, message) PARSER_WRONG_TOKEN(token, message)
#define TYPE_WRONG_TOKEN(token, message) PARSER_WRONG_TOKEN(token, message)
#define PROTO_WRONG_TOKEN(token, message) PARSER_WRONG_TOKEN(token, message)
#define FUNC_WRONG_TOKEN(token, message) PARSER_WRONG_TOKEN(token, message)
#define EXTERN_WRONG_TOKEN(token, message) PARSER_WRONG_TOKEN(token, message)
// ScopeGuard

class ScopeGuard
{
public:

    ScopeGuard(const std::string &name, LangParser *parser)
        : parser_(*parser)
    { parser_.pushScope(name); }

    ScopeGuard(const std::string &name, LangParser &parser)
        : parser_(parser)
    { parser_.pushScope(name); }

    ScopeGuard(const char *name, LangParser *parser)
        : parser_(*parser)
    { parser_.pushScope(name); }

    ScopeGuard(const char *name, LangParser &parser)
        : parser_(parser)
    { parser_.pushScope(name); }

    ~ScopeGuard() { parser_.popScope(); }

private:
    LangParser &parser_;
};

// Parser

LangParser::LangParser(World &world)
    : PrattParser()
    , world_(world)
    , builder_(world)
{
    init(0);
}

LangParser::LangParser(const Scope::Ptr &topScope)
    : PrattParser()
    , world_(topScope->getWorld())
    , builder_(topScope->getWorld())
{
    init(topScope);
}

LangParser::~LangParser()
{
}

void LangParser::init(const Scope::Ptr &topScope)
{
    World &world = getWorld();
    if (topScope)
        builder_.pushScope(topScope);
    else
        builder_.pushScope(new Scope(world, "main"));

    IR::IRUtils::addDefaultTypesToScope(getScope());

    // register syntax rules
    setLeftBindingPower("(", 150); // 80 ?
    registerPrefixParser("(", boost::bind(&LangParser::parseParenExpr, this, _2));
    registerInfixParser("(", boost::bind(&LangParser::parseFuncCall, this, _2, _3));

    setLeftBindingPower("[", 80); // 80 ?
    registerInfixParser("[", boost::bind(&LangParser::parseArrayIndex, this, _2, _3));

    registerPrefixParser(TOK_NAME, boost::bind(&LangParser::parseName, this, _2));

    registerPrefixParser(TOK_ICONST, boost::bind(&LangParser::parseConstant, this, _2));
    registerPrefixParser(TOK_FCONST, boost::bind(&LangParser::parseConstant, this, _2));
    registerPrefixParser(TOK_SCONST, boost::bind(&LangParser::parseConstant, this, _2));

    registerStmtParser("struct", boost::bind(&LangParser::parseStructTypeDefinition, this, _2));
    registerStmtParser("def", boost::bind(&LangParser::parseDefinition, this));
    registerStmtParser("extern", boost::bind(&LangParser::parseExtern, this));
    registerStmtParser("undef", boost::bind(&LangParser::parseUndef, this));
    registerStmtParser("typedef", boost::bind(&LangParser::parseTypedef, this, _2));

    registerPrefixParser("if", boost::bind(&LangParser::parseIfExpr, this, _2));
    registerPrefixParser("loop", boost::bind(&LangParser::parseLoopExpr, this, _2));
    registerPrefixParser("for", boost::bind(&LangParser::parseForExpr, this, _2));
    registerPrefixParser("var", boost::bind(&LangParser::parseVarExpr, this, _2));
    registerPrefixParser("break", boost::bind(&LangParser::parseBreakExpr, this, _2));

    registerPrefixParser("{", boost::bind(&LangParser::parseBlockExpr, this, _2));
    registerStmtParser("{", boost::bind(&LangParser::parseBlockExpr, this, _2));

    setLeftBindingPower(":", 150);
    registerInfixParser(":", boost::bind(&LangParser::parseNamedBlockExpr, this, _2, _3));
}

void LangParser::pushScope(const std::string &newName)
{
    builder_.pushScope(newName);
}

void LangParser::popScope()
{
    builder_.popScope();
}

Type::Ptr LangParser::parseType()
{
    if (!current().isSymbol())
        TYPE_WRONG_TOKEN(current(), "expected type identifier");
    SourceLocation loc = current().getLocation();
    std::string tstr = current().str;
    advance();

    // type constructors:
    // ptr, ref, array
    if (tstr == "ptr")
    {
        if (!match("("))
            TYPE_WRONG_TOKEN(current(), "pointer type: expected '('");
        loc = current().getLocation();
        Type::Ptr elemType = parseType();
        if (!elemType)
            PARSER_ERROR_AT("pointer type: expected element type", loc);
        if (!match(")"))
            TYPE_WRONG_TOKEN(current(), "pointer type: expected ')'");
        return getWorld().type_c_ptr(elemType);
    }
    if (tstr == "ref")
    {
        if (!match("("))
            TYPE_WRONG_TOKEN(current(), "reference type: expected '('");
        loc = current().getLocation();
        Type::Ptr elemType = parseType();
        if (!elemType)
            PARSER_ERROR_AT("reference type: expected element type", loc);
        if (!match(")"))
            TYPE_WRONG_TOKEN(current(), "reference type: expected ')'");
        return getWorld().type_c_ref(elemType);
    }
    if (tstr == "array")
    {
        // array(type)
        // array(type, size)
        if (!match("("))
            TYPE_WRONG_TOKEN(current(), "reference type: expected '('");
        loc = current().getLocation();
        Type::Ptr elemType = parseType();
        if (!elemType)
            PARSER_ERROR_AT("array type: expected element type", loc);
        int64_t numElements = 0;
        if (match(","))
        {
            if (current().isIntegerConstant())
            {
                numElements = current().getIntValue();
                if (numElements < 0)
                    TYPE_WRONG_TOKEN(current(), "array type size can't be negative");
                advance();
            }
            else
            {
                TYPE_WRONG_TOKEN(current(), "array type: expected array size after ','");
            }
        }
        if (!match(")"))
            TYPE_WRONG_TOKEN(current(), "reference type: expected ')'");
        if (numElements > 0)
            return FixedArrayType::get(elemType, numElements);
        return ArrayType::get(elemType);
    }
    if (tstr == "symbol")
    {
        // symbol(string)
        if (!match("("))
            TYPE_WRONG_TOKEN(current(), "symbol type: expected '('");
        std::string symbol;
        if (!parseStringConstant(symbol))
            TYPE_WRONG_TOKEN(current(), "symbol type: expected string literal");
        if (!match(")"))
            TYPE_WRONG_TOKEN(current(), "symbol type: expected ')'");
        return SymbolType::get(getWorld(), symbol);
    }
    if (tstr == "fun" || tstr == "fn")
    {
        if (!match("("))
            TYPE_WRONG_TOKEN(current(), "function type: expected '('");

        std::vector<IR::Prototype::Arg> args;
        IR::Prototype::Arg arg;

        Token token;
        if (!match(")")) // match and eat ')'
        {
            while (true)
            {
                token = current();
                advance();
                if (match(":"))
                {
                     if (token.type != TOK_NAME)
                         PARSER_WRONG_TOKEN(token,
                                           "in function type expected identifier before ':'");
                     arg.first = token.getStringValue();
                }
                else
                {
                    // not a name, reset parsing
                    ungetToken(current());
                    ungetToken(token);
                    advance(); // current() == token

                    arg.first = "";
                }

                arg.second = parseType();
                if (arg.second == 0)
                    return 0;

                args.push_back(arg);

                if (match(","))
                    continue;
                else if (match(")"))
                    break;
                else
                    TYPE_WRONG_TOKEN(current(),
                                      "in function type expected ',' or ')'");
            }
        }

        Type::Ptr returnType;
        if (match(":") || match("->"))
        {
            returnType = parseType();
        }
        else
        {
            TYPE_WRONG_TOKEN(current(), "no return type specified in function type");
        }

        return FunctionType::get(returnType, args);
    }
    if (Object::Ptr obj = getScope()->lookupObject(tstr))
    {
        // TODO do this also for TypeExpr ?
        if (Type::Ptr type = dyn_cast<Type>(obj))
            return type;
    }

    PARSER_ERROR_AT("Unknown type identifier: "<<tstr, loc);
#undef RET_CTYPE
}

bool LangParser::parseStringConstant(std::string &str)
{
    if (current().type != TOK_SCONST)
        FUNC_WRONG_TOKEN(current(), "string constant expected");
    str = current().str;
    advance();
    while (current().isStringConstant())
    {
        str += current().str;
        advance();
    }
    return true;
}

// '[' . name ('=' (name | string))? (',' name ('=' (name | string))?)* ']'
bool LangParser::parseAttributeMap(AttributeMap &attributes)
{
    std::string key;

    // parse attributes
    if (!match("]")) // match and eat ']'
        while (true)
        {
            if (!current().isName())
                PROTO_WRONG_TOKEN(current(),
                                  "attribute must have form: name ('=' (string_value | name))?");

            key = current().getStringValue();


            advance(); // read next token

            if (match("="))
            {
                if (!current().isName() && !current().isStringConstant())
                    PROTO_WRONG_TOKEN(current(),
                                      "expected string value or name after '=' in attribute list");
                attributes[key] = current().getStringValue();
                advance();
            }
            else
            {
                attributes[key] = "true";
            }
            if (match(","))
                continue;
            else if (match("]"))
                break;
            else
                PROTO_WRONG_TOKEN(current(),
                                  "expected '=', ',', or ']' in attribute list");
        }
    return true;
}

// '[' . name ('=' (name | string))? (',' name ('=' (name | string))?)* ']'
bool LangParser::parseAttributeMap(DictValue &attributes)
{
    std::string key;

    // parse attributes
    if (!match("]")) // match and eat ']'
        while (true)
        {
            if (!current().isName())
                PROTO_WRONG_TOKEN(current(),
                                  "attribute must have form: name ('=' (string_value | name))?");

            key = current().getStringValue();

            advance(); // read next token

            if (match("="))
            {
                if (!current().isName() && !current().isStringConstant())
                    PROTO_WRONG_TOKEN(current(),
                                      "expected string value or name after '=' in attribute list");
                attributes[key] = current().getStringValue();
                advance();
            }
            else
            {
                attributes[key] = true;
            }
            if (match(","))
                continue;
            else if (match("]"))
                break;
            else
                PROTO_WRONG_TOKEN(current(),
                                  "expected '=', ',', or ']' in attribute list");
        }
    return true;
}

IR::IRExpr::Ptr LangParser::parseStringConstant()
{
    std::string str;
    bool ok = parseStringConstant(str);
    if (ok)
        return new IR::PrimLiteral(str, getWorld());
    return 0;
}

/// numberexpr ::= number
IR::IRExpr::Ptr LangParser::parseNumericConstant()
{
    advance();

    IR::IRExpr::Ptr result;
    if (current().type == TOK_FCONST)
        result = new IR::PrimLiteral(current().value.fconst, getWorld());
    else if (current().type == TOK_ICONST)
    {
        // FIXME add a way to specify type of the constant
        result = new IR::PrimLiteral(static_cast<int32_t>(current().getIntValue()), getWorld());
    }
    else
    {
        EXPR_WRONG_TOKEN(current(), "Numeric constant expected");
    }
    return result;
}

/// prototype
///   ::= id '(' id* ')'
///   ::= binary LETTER number? (id, id)
///   ::= unary LETTER (id)
IR::Prototype::Ptr LangParser::parsePrototype(bool requireReturnType, bool requireArgTypes)
{
    std::string FnName;

    unsigned Kind = 0; // 0 = identifier, 1 = unary, 2 = binary.
    unsigned BinaryPrecedence = 30;

    Token token = current();
    SourceLocation loc = token.getLocation();

    IR::Prototype::AttributeMap attrs;

    if (token.isSymbol("["))
    {
        // parse attributes
        advance();
        if (!match("]")) // match and eat ']'
            while (true)
            {
                token = current();
                if (token.type != TOK_NAME)
                    PROTO_WRONG_TOKEN(token,
                                      "in function '"<<FnName<<"' expected attribute in prototype");

                attrs[token.str] = "true";

                advance(); // read next token

                if (match(","))
                    continue;
                else if (match("]"))
                    break;
                else
                    PROTO_WRONG_TOKEN(current(),
                                      "in function '"<<FnName<<"' expected ',' or ']' in attribute list");
            }
    }

    token = current();

    if (!token.isSymbol())
    {
        PROTO_WRONG_TOKEN(token, "expected function name in prototype");
    }

    if (token.str == "unary")
    {
        token = advance();
        if (!token.isSymbol())
            PROTO_WRONG_TOKEN(token, "expected symbol after 'unary' in prototype");

        FnName = token.str;
        Kind = 1;
        token = advance();
    }
    else if (token.str == "binary")
    {
        token = advance();
        if (!token.isSymbol())
            PROTO_WRONG_TOKEN(token, "expected symbol after 'binary' in prototype");
        FnName = token.str;
        Kind = 2;
        token = advance();

        // Read the precedence if present.
        if (token.type == TOK_ICONST)
        {
            if (token.value.iconst < 1 || token.value.iconst > 100)
                PROTO_ERROR("Invalid precedence: must be 1..100");
            BinaryPrecedence = (unsigned) token.value.iconst;
            token = advance(); // must be '('
        }
    }
    else
    {
        FnName = token.str;
        Kind = 0;
        token = advance(); // must be '('
    }

    if (!token.isSymbol("("))
        PROTO_WRONG_TOKEN(token, "in function '"<<FnName<<"': expected '(' in prototype");

    std::vector<IR::Prototype::Arg> Args;
    IR::Prototype::Arg arg;
    advance();
    if (!match(")")) // match and eat ')'
        while (true)
        {
            token = current();
            if (token.type != TOK_NAME)
                PROTO_WRONG_TOKEN(token,
                                  "in function '"<<FnName<<"' expected identifier in prototype");

            arg.first = token.str;
            arg.second = 0;

            advance(); // read next token
            if (match(":")) // match and eat ':'
            {
                arg.second = parseType();
                if (arg.second == 0)
                    return 0;
            }
            else if (requireArgTypes)
            {
                PROTO_ERROR("In function '"<<FnName<<"': No type for argument '"<<arg.first<<"' specified");
            }

            Args.push_back(arg);

            if (match(","))
                continue;
            else if (match(")"))
                break;
            else
                PROTO_WRONG_TOKEN(current(),
                                  "in function '"<<FnName<<"' expected ',' or ')' in prototype");
        }

    Type::Ptr returnType;
    if (match(":") || match("->"))
    {
        returnType = parseType();
    }
    else if (requireReturnType)
    {
        PROTO_ERROR("In function '"<<FnName<<"': No return type specified");
    }

    // Verify right number of names for operator.
    if (Kind && Args.size() != Kind)
        PROTO_ERROR("In function '"<<FnName<<"': Invalid number of operands for operator");

    std::string mangledName;

    if (attrs.find("C") != attrs.end() || attrs.find("notmangled") != attrs.end())
        mangledName = FnName;
    else
        mangledName = Mangler::getMangledFuncName(FnName, Args);
    IR::Prototype::Ptr proto = new IR::Prototype(FnName, mangledName, returnType, Args, getWorld(), Kind != 0, BinaryPrecedence);
    proto->setAttributes(attrs);
    proto->setLocation(loc);
    return proto;
}

/// external ::= 'extern' . prototype
IR::ExternFunction::Ptr LangParser::parseExtern()
{
    IR::Prototype::Ptr proto = parsePrototype(true, true);
    if (!proto)
        return 0;
    if (!match(";"))
        EXTERN_WRONG_TOKEN(current(), "';' expected");
    IR::ExternFunction::Ptr externFunc = new IR::ExternFunction(proto);
    if (!addDefinition(externFunc))
        return 0;
    return externFunc;
}

/// definition ::= 'def' . ['intrinsic'] prototype expression
IR::FunctionDefinition::Ptr LangParser::parseDefinition()
{
    bool intrinsic = match("intrinsic");

    bool requireReturnType = intrinsic;
    IR::Prototype::Ptr proto = parsePrototype(requireReturnType);
    if (proto == 0)
        return 0;

    // FIXME This is workaround, possibly parsePrototype should set any
    // not yet known /unresolved types to AnyType (or UnresolvedType/TopType ?)
    if (!proto->getReturnType())
        proto->setReturnType(AnyType::get(proto->getWorld()));

    IR::FunctionDefinition::Ptr funcDef;

    if (intrinsic)
    {
        std::string body;
        if (!parseStringConstant(body))
            PARSER_ERROR("intrinsic string(s) expected");
        if (!match(";"))
            FUNC_WRONG_TOKEN(current(), "';' expected");

        funcDef = new IR::Intrinsic(proto, body, getWorld());
    }
    else
    {
        DFC_DEBUG("DUMP SCOPE");
        DFC_IFDEBUG(getScope()->dump());

        {
        ScopeGuard g("func_"+std::string(proto->getName()), this);

//        // FIXME: This should be moved into prototype !?
//
//        for (size_t i = 0; i < numArgs; ++i)
//        {
//            addVariableToScope(IR::IRUtils::createVariable(proto->getArg(i).first, proto->getArg(i).second));
//        }

        IR::Function::Ptr func = new IR::Function(proto, 0);
        const size_t numArgs = func->getNumArgs();
        for (size_t i = 0; i < numArgs; ++i)
        {
            addVariableToScope(func->getArg(i));
        }
        func->setLocation(proto->getLocation());
        addFunctionToScope(func);

        if (IR::IRExpr::Ptr E = resolveSymbol(expression()))
        {
            if (isa<IR::BlockExpr>(E))
                match(";"); // ignore optional ';'
            else if (!match(";"))
                FUNC_WRONG_TOKEN(current(), "';' expected");

            DFC_DEBUG("DBG FUNC "<<proto->getName()
                      <<" (mangled "<<proto->getMangledName()<<")");
            DFC_DEBUG("DUMP SCOPE");
            DFC_IFDEBUG(getScope()->dump());
            DFC_DEBUG(" BODY: "<<E->toString());
            DFC_DEBUG(" TYPE: "<<IR::IRUtils::getTypeName(E->getExprType()));

            Type::Ptr returnType = proto->getReturnType();

            if (!canonicallyEqual(returnType, AnyType::get(returnType->getWorld())) &&
                !canonicallyEqual(returnType, E->getExprType()))
            {
                // FIXME should we try to convert here ?
                PARSER_ERROR_AT("Type mismatch, cannot convert expression of type "
                                <<IR::IRUtils::getTypeName(E->getExprType())
                                <<" to return type "
                                <<IR::IRUtils::getTypeName(returnType),
                                E->getLocation());
            }

            func->setBody(E);

            if (!func->getFunctionType())
                PARSER_ERROR_AT("Could not deduce function type", E->getLocation());

            funcDef = func;
        }
        }
    }
    if (funcDef && !addDefinition(funcDef))
        return 0;
    return funcDef;
}

/// struct_type ::= 'struct' . name attributes? '{' (name (',' name)* ':' type ';')* '}' ';'
StructType::Ptr LangParser::parseStructType(const Token &firstToken)
{
    DictValue attributes;
    if (match("["))
    {
        if (!parseAttributeMap(attributes))
            return 0;
    }

    // struct name is required.
    std::string structName;
    if (!match(TOK_NAME, structName))
        EXPR_WRONG_TOKEN(current(), "expected identifier after 'struct'");

    StructType::Ptr structType;
    if (Object::Ptr object = getScope()->lookupObject(structName))
    {
        structType = dyn_cast<StructType>(object);
        if (!structType)
            PARSER_ERROR_AT("object with name '"<<structName<<"' already exists: "<<*object,
                    current().getLocation());
    }

    if (match(";"))
    {
        // forward declaration / opaque struct
        if (!structType)
        {
            structType = StructType::create(getWorld(), structName);
            getScope()->addObject(structName, structType);
        }
        structType->setAttributes(attributes);

        return structType;
    }

    if (!match("{"))
        PARSER_WRONG_TOKEN(current(), "expected '{' in  struct");

    if (structType && structType->getNumElements())
        PARSER_ERROR_AT("struct type '"<<structName<<"' already defined", current().getLocation());

    std::string memberName;
    Type::Ptr memberType;
    std::vector<Type::Ptr> memberTypes;
    std::vector<std::string> memberNames;

    // parse struct elements
    if (!match("}")) // match and eat '}'
        while (true)
        {
            if (!match(TOK_NAME, memberName))
                PARSER_WRONG_TOKEN(current(),
                                  "struct type member must have form: (name (',' name)* ':' type ';')*");



            if (match(":"))
            {
                SourceLocation loc = current().getLocation();
                memberType = parseType();
                if (!memberType)
                    return 0;
                if (memberType == VoidType::get(getWorld()))
                    PARSER_ERROR_AT("void type cannot be a member of a struct type", loc);
            }
            else
                PARSER_WRONG_TOKEN(current(), "expected ':' in the struct member");

            if (!match(";"))
                PARSER_WRONG_TOKEN(current(), "expected ';' at the end of struct member");

            memberNames.push_back(memberName);
            memberTypes.push_back(memberType);

            if (match("}"))
                break;
        }

    if (!match(";"))
        PARSER_WRONG_TOKEN(current(), "expected ';' at the end of struct declaration");

    if (structType)
        structType->resizeElements(memberTypes.size());
    else
    {
        structType = StructType::create(getWorld(), structName, memberTypes.size());
        getScope()->addObject(structName, structType);
    }
    structType->setElements(memberTypes);
    structType->setElementNames(memberNames);
    structType->setAttributes(attributes);

    return structType;
}

IR::TypeDefinition::Ptr LangParser::parseStructTypeDefinition(const Token &firstToken)
{
    if (StructType::Ptr sty = parseStructType(firstToken))
        return new IR::TypeDefinition(sty);
    return 0;
}

// 'typedef' type name;
IR::TypeDefinition::Ptr LangParser::parseTypedef(const Token &firstToken)
{
    Type::Ptr type = parseType();
    if (!type)
        return 0;
    // struct name is required.
    std::string typeName;
    if (!match(TOK_NAME, typeName))
        TYPE_WRONG_TOKEN(current(), "expected identifier after type in 'typedef'");
    if (Object::Ptr object = getScope()->lookupObject(typeName))
        PARSER_ERROR_AT("object with name '"<<typeName<<"' already exists: "<<*object,
                current().getLocation());
    if (!match(";"))
        PARSER_WRONG_TOKEN(current(), "expected ';' at the end of typedef declaration");

    type = TypedefType::create(typeName, type);
    getScope()->addObject(typeName, type);
    return new IR::TypeDefinition(typeName, type);
}

bool LangParser::addDefinition(const IR::FunctionDefinition::Ptr &funcDef)
{
    if (!addFunctionToScope(funcDef))
        return false;
    installOperator(funcDef);
    return true;
}

Object::Ptr LangParser::parseStatement()
{
    ParserToken t = current();
    StmtParseFn stmtParse = t.getEntry<LangParserEntry>()->stmtParse;
    if (stmtParse)
    {
        advance();
        return stmtParse(*this, t);
    }
    Object::Ptr e = expression(0);
    if (e)
        match(";"); // Should ';' be optional ? Alternative: advance(";");
    else
    {
        // error recovery
        if (skipUntil(";"))
            advance();
    }
    return e;
}

bool LangParser::addFunctionToScope(const IR::FunctionDefinition::Ptr &func)
{
    std::string errorMsg;
    bool result = IR::IRUtils::addFunctionToScope(func, getScope(), &errorMsg);

    if (!result)
        error(errorMsg);

    return result;
}

bool LangParser::addObjectToScope(const std::string &name, const Object::Ptr &object)
{
    std::string errorMsg;
    bool result = IR::IRUtils::addObjectToScope(name, object, getScope(), &errorMsg);

    if (!result)
        error(errorMsg);

    return result;
}

bool LangParser::addVariableToScope(const IR::DefExpr::Ptr &var)
{
    return addObjectToScope(var->getName(), var);
}

bool LangParser::removeFunctionFromScope(const IR::FunctionDeclaration::Ptr &func)
{
    Scope::Ptr scope = getScope();
    std::string funcName = func->getName();
    std::string mangledName = func->getMangledName();

    std::pair<Object::Ptr, const Scope *> objAndScope = scope->lookupObjectAndScope(funcName);
    if (objAndScope.first && objAndScope.second == scope)
    {
        OverloadedObjectMap::Ptr funcMap = DFC::safe_object_cast<OverloadedObjectMap>(objAndScope.first);
        return funcMap->removeObject(mangledName);
    }

    return false;
}

void LangParser::installOperator(const IR::FunctionDefinition::Ptr &op)
{
    // If this is an operator, install it.
    const IR::Prototype::Ptr &proto = op->getProto();
    if (proto->isBinaryOp())
    {
        // FIXME there should be only single registration
        setLeftBindingPower(TOK_OPERATOR, proto->getOperatorName(), proto->getBinaryPrecedence());
        setLeftBindingPower(TOK_NAME, proto->getOperatorName(), proto->getBinaryPrecedence());
        registerInfixParser(TOK_OPERATOR, proto->getOperatorName(), boost::bind(&LangParser::parseBinaryOp, this, proto->getBinaryPrecedence(), _2, _3));
        registerInfixParser(TOK_NAME, proto->getOperatorName(), boost::bind(&LangParser::parseBinaryOp, this, proto->getBinaryPrecedence(), _2, _3));
    }
    if (proto->isUnaryOp())
    {
        // FIXME replace constant 70
        registerPrefixParser(TOK_OPERATOR, proto->getOperatorName(), boost::bind(&LangParser::parseUnaryOp, this, 70, _2));
        registerPrefixParser(TOK_NAME, proto->getOperatorName(), boost::bind(&LangParser::parseUnaryOp, this, 70, _2));
    }
}

BasicParserEntry::Ptr LangParser::createParserEntry() const
{
    return BasicParserEntry::Ptr(new LangParserEntry);
}

Object::Ptr LangParser::parseConstant(const Token &token)
{
    IR::PrimLiteral::Ptr literal;
    if (token.type == TOK_FCONST)
        literal = new IR::PrimLiteral(token.getFloatValue(), getWorld());
    else if (token.type == TOK_ICONST)
        // FIXME add a way to specify type of the constant
        literal = new IR::PrimLiteral(static_cast<int32_t>(token.getIntValue()), getWorld());
    else if (token.type == TOK_SCONST)
        literal = new IR::PrimLiteral(token.getStringValue(), getWorld());
    else
    {
        EXPR_WRONG_TOKEN(token, "Numeric constant expected");
    }
    literal->setLocation(token.getLocation());
    return literal;
}

Object::Ptr LangParser::parseName(const Token &token)
{
    if (token.isSymbol())
    {
        IR::IRExpr::Ptr sym;
        if (token.isSymbol("nullptr"))
        {
            sym = IR::PrimLiteral::getNullPtr(getWorld());
        }
        else
            sym = new IR::SymbolExpr(token.getStringValue(), getScope());
        sym->setLocation(token.getLocation());
        return sym;
    }
    EXPR_WRONG_TOKEN(token, "Symbol expected");
}

Object::Ptr LangParser::parseBinaryOp(int precedence, const Object::Ptr &left, const Token &token)
{
    Object::Ptr right = expression(precedence);
    if (!right)
        return 0;
    std::vector<IR::IRExpr::Ptr> args;
    args.push_back(resolveSymbol(left));
    args.push_back(resolveSymbol(right));
    return builder_.createCall(token.str, getScope(), args, &token.getLocation());
}

Object::Ptr LangParser::parseUnaryOp(int precedence, const Token &token)
{
    Object::Ptr right = expression(precedence);
    if (!right)
        return 0;
    std::vector<IR::IRExpr::Ptr> args;
    args.push_back(resolveSymbol(right));
    return builder_.createCall(token.str, getScope(), args, &token.getLocation());
}

Object::Ptr LangParser::parseParenExpr(const Token &token)
{
    Object::Ptr expr = expression(0);
    if (advance(")").isError())
        ERROR_TOKEN(current());
    return expr;
}

Object::Ptr LangParser::parseFuncCall(const Object::Ptr &left, const Token &token)
{
    std::vector<IR::IRExpr::Ptr> args;
    if (!match(")"))
    {
        do {
            IR::IRExpr::Ptr e = resolveSymbol(expression(0));
            if (!e)
                return 0;
            args.push_back(e);
        } while (match(","));
        if (advance(")").isError())
            ERROR_TOKEN(current());
    }
    IR::SymbolExpr::Ptr sym = dyn_cast<IR::SymbolExpr>(left);
    if (!sym)
        EXPR_ERROR("No identifier : "<<*left);
    const std::string &name = sym->getName();
    Scope::Ptr scope = dyn_cast<Scope>(sym->getScope());

    return builder_.createCall(name, scope, args, &token.getLocation());
}

Object::Ptr LangParser::parseArrayIndex(const Object::Ptr &left, const Token &token)
{
    std::vector<IR::IRExpr::Ptr> args;
    if (!match("]"))
    {
        do {
            IR::IRExpr::Ptr e = resolveSymbol(expression(0));
            if (!e)
                return 0;
            args.push_back(e);
        } while (match(","));
        if (!advance("]"))
            ERROR_TOKEN(current());
    }
    IR::SymbolExpr::Ptr sym = dyn_cast<IR::SymbolExpr>(left);
    if (!sym)
        EXPR_ERROR("No identifier : "<<*left);
    const std::string &name = sym->getName();
    Scope::Ptr scope = dyn_cast<Scope>(sym->getScope());

    return builder_.createCall(name, scope, args, &token.getLocation(), "__index__");
}

/// ifexpr ::= 'if' . expression 'then' expression 'else' expression
Object::Ptr LangParser::parseIfExpr(const Token &ifToken)
{
    // condition.
    IR::IRExpr::Ptr cond = resolveSymbol(expression());
    if (!cond)
        return 0;

    if (!match("then"))
        EXPR_WRONG_TOKEN(current(), "expected 'then'");

    IR::IRExpr::Ptr then = resolveSymbol(expression());
    if (then == 0)
        return 0;

    IR::IRExpr::Ptr _else;

    //if (!match("else"))
    //    EXPR_WRONG_TOKEN(current(), "expected 'else'");

    if (match("else"))
    {
        _else = resolveSymbol(expression());
        if (!_else)
            return 0;
    }

    IR::IRExpr::Ptr result = new IR::IfExpr(cond, then, _else, world_);
    result->setLocation(ifToken.getLocation());
    return result;
}

/// loopexpr ::= 'loop' . expression
Object::Ptr LangParser::parseLoopExpr(const Token &loopToken)
{
    IR::IRExpr::Ptr body = resolveSymbol(expression());
    if (body == 0)
        return 0;

    IR::LoopExpr::Ptr result = new IR::LoopExpr(body);
    result->setLocation(loopToken.getLocation());
    return result;
}

/// forexpr ::= 'for' . identifier [':' type ] '=' expr ',' expr (',' expr)? 'in' expression
Object::Ptr LangParser::parseForExpr(const Token &forToken)
{
    if (!current().isSymbol())
        EXPR_WRONG_TOKEN(current(), "expected identifier after for");

    std::string varName = current().getStringValue();
    advance();

    Type::Ptr varType;
    if (match(":"))
    {
        varType = parseType();
        if (varType == 0)
            return 0;
    }

    if (!match("="))
        EXPR_WRONG_TOKEN(current(), "expected '=' after for");

    IR::IRExpr::Ptr start = resolveSymbol(expression());
    if (start == 0)
        return 0;

    if (!varType)
        varType = start->getExprType();

    if (!match(","))
        EXPR_WRONG_TOKEN(current(), "expected ',' after for start value");

    IR::IRExpr::Ptr end, step, body;

    IR::DefExpr::Ptr var = new IR::DefExpr(varName, varType, true);
    {
        ScopeGuard g("for", this);
        addVariableToScope(var);

        end = resolveSymbol(expression());
        if (end == 0)
            return 0;

        // The step value is optional.
        if (match(","))
        {
            step = resolveSymbol(expression());
            if (step == 0)
                return 0;
        }

        if (!match("in"))
            EXPR_WRONG_TOKEN(current(), "expected 'in' after for");

        body = resolveSymbol(expression());
        if (body == 0)
            return 0;
    }

    IR::ForExpr::Ptr result = new IR::ForExpr(var, start, end, step, body);
    result->setLocation(forToken.getLocation());
    return result;
}

/// varexpr ::= 'var' . identifier (':' type)? ('=' expression)?
//                    (',' identifier (':' type)? ('=' expression)?)* 'in' expression
Object::Ptr LangParser::parseVarExpr(const Token &varToken)
{
    typedef std::vector<std::pair<IR::DefExpr::Ptr, IR::IRExpr::Ptr> > VarList;
    VarList vars;
    std::vector<std::pair<std::string, IR::IRExpr::Ptr> > VarNames;

    // At least one variable name is required.
    if (!current().isSymbol())
        EXPR_WRONG_TOKEN(current(), "expected identifier after var");

    IR::IRExpr::Ptr body;
    {
        ScopeGuard g("var", this);
        while (1)
        {
            std::string Name = current().getStringValue();
            advance();

            Type::Ptr varType;
            // Read the optional type
            if (match(":"))
            {
                varType = parseType();
                if (varType == 0)
                    return 0;
            }

            // Read the optional initializer.
            IR::IRExpr::Ptr initExpr = 0;

            if (match("="))
            {
                initExpr = resolveSymbol(expression());
                if (initExpr == 0)
                    return 0;
            }

            if (!varType && initExpr)
                varType = IR::IRUtils::makeVariableType(initExpr);

            if (!varType)
                EXPR_ERROR("Neither variable type nor initializer specified");

            // Check types
            if (initExpr)
            {
                IR::IRExpr::Ptr newInitExpr = builder_.convertValue(initExpr, varType);
                if (!newInitExpr)
                {
                    PARSER_ERROR_AT("Type mismatch, cannot convert "
                            <<IR::IRUtils::getTypeName(initExpr->getExprType())
                            <<" to "<<IR::IRUtils::getTypeName(varType),
                            initExpr->getLocation());
                }
                initExpr = newInitExpr;
            }

            IR::DefExpr::Ptr def = IR::IRUtils::createVariable(Name, varType);
            addVariableToScope(def);

            vars.push_back(std::make_pair(def, initExpr));

            // End of var list, exit loop.
            if (!match(","))
                break;

            if (!current().isSymbol())
                EXPR_WRONG_TOKEN(current(), "expected identifier list after var");
        }

        if (!match("in"))
            EXPR_WRONG_TOKEN(current(), "expected 'in' keyword after 'var'");

        body = resolveSymbol(expression());
        if (body == 0)
            return 0;
    }

    for (VarList::reverse_iterator it = vars.rbegin(), end = vars.rend();
            it != end; ++it)
    {
        body = new IR::LetExpr(it->first, it->second, body);
    }

    body->setLocation(varToken.getLocation());

    return body;
}

Object::Ptr LangParser::parseBreakExpr(const Token &firstToken)
{
    bool hasParen = match("(");

    // At least one label is required.
    if (!current().isSymbol())
        EXPR_WRONG_TOKEN(current(), "expected identifier after var");

    std::string name = current().getStringValue();
    advance();

    Object::Ptr labelObject = getScope()->lookupObject(name);
    if (!labelObject)
    {
        PARSER_ERROR_AT("No block named '"<<name<<"' in current scope", current().getLocation());
    }

    IR::BlockExpr::Ptr block = dyn_cast<IR::BlockExpr>(labelObject);
    if (!block)
        EXPR_ERROR("Object '"<<name<<"' in current scope is not a block");

    IR::IRExpr::Ptr value;

    if (hasParen)
    {
        if (match(","))
        {
            value = resolveSymbol(expression());
            if (!value)
                return 0;
        }
        if (advance(")").isError())
            ERROR_TOKEN(current());
    }

    return new IR::BreakExpr(block, value);
}

Object::Ptr LangParser::parseBlockExpr(const Token &firstToken)
{
    return parseBlockExpr(new IR::BlockExpr(world_), &firstToken.getLocation());
}

Object::Ptr LangParser::parseBlockExpr(const IR::BlockExpr::Ptr &dest, const SourceLocation *location)
{
    IR::IRExpr::Ptr expr;
    IR::BlockExpr::ExprList exprList;
    while (true)
    {
        if (match("}"))
            break;
        expr = resolveSymbol(parseStatement());
        if (!expr)
            return 0;
        exprList.push_back(expr);
    }
    dest->setExprList(exprList);
    if (location)
        dest->setLocation(*location);
    return dest;
}

Object::Ptr LangParser::parseNamedBlockExpr(const Object::Ptr &left, const Token &token)
{
    IR::SymbolExpr::Ptr sym = dyn_cast<IR::SymbolExpr>(left);
    if (!sym)
        EXPR_ERROR("No identifier : "<<*left);
    const std::string &name = sym->getName();

    if (!token.isSymbol(":"))
        EXPR_ERROR("No ':' after label identifier");

    if (!current().isSymbol("{"))
        EXPR_ERROR("No '{' after ':'");

    Token curTok = current();

    if (advance("{").isError())
        EXPR_ERROR("No '{' after ':'");

    IR::BlockExpr::Ptr block = new IR::BlockExpr(world_, name);

    Object::Ptr expr;
    {
        ScopeGuard g("block", this);
        addObjectToScope(name, block);
        expr = parseBlockExpr(block, &curTok.getLocation());
    }

    return expr;
}

IR::FunctionDeclaration::Ptr LangParser::parseUndef()
{
    // intrinsics and externs both don't do mangling

    bool isExtern = match("intrinsic") || match("extern");

    IR::Prototype::Ptr proto = parsePrototype(true);
    if (proto == 0)
        return 0;

    if (!isExtern)
    {
        std::string mangledName = Mangler::getMangledFuncName(proto->getName(), ArrayRef<IR::Prototype::Arg>(proto->getArgs()));
        proto->setMangledName(mangledName);
    }
    if (!match(";"))
        EXPR_WRONG_TOKEN(current(), "expected ';'");

    IR::FunctionDeclaration::Ptr funcDecl = new IR::FunctionDeclaration(proto, isExtern);
    removeFunctionFromScope(funcDecl);

    return funcDecl;
}

IR::IRExpr::Ptr LangParser::resolveSymbol(const Object::Ptr &expr)
{
    IR::SymbolExpr::Ptr sym = dyn_cast<IR::SymbolExpr>(expr);
    if (sym)
    {
        Scope::Ptr scope = dyn_cast<Scope>(sym->getScope());
        Object::Ptr obj;
        if (scope)
            obj = scope->lookupObject(sym->getName());
        if (IR::IRExpr::Ptr resolvedExpr = dyn_cast<IR::IRExpr>(obj))
            return resolvedExpr;
        else if (OverloadedObjectMap::Ptr funcMap = dyn_cast<OverloadedObjectMap>(obj))
        {
            if (funcMap->getNumObjects() == 1)
                return dyn_cast<IR::IRExpr>(funcMap->begin()->second);
        }
        else if (Type::Ptr typeValue = dyn_cast<Type>(obj))
        {
            IR::TypeExpr::Ptr tyexpr = new IR::TypeExpr(typeValue);
            tyexpr->setLocation(sym->getLocation());
            return tyexpr;
        }
    }

    return dyn_cast<IR::IRExpr>(expr);
}

} // namespace Compiler

} // namespace KIARA
