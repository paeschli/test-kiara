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
 * LangParser.hpp
 *
 *  Created on: 01.03.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMPILER_LANGPARSER_HPP_INCLUDED
#define KIARA_COMPILER_LANGPARSER_HPP_INCLUDED

#include "Config.hpp"
#include "PrattParser.hpp"
#include "IR.hpp"
#include "IRBuilder.hpp"
#include "Scope.hpp"
#include <stack>

namespace KIARA {

namespace Compiler {

typedef PrefixParseFn StmtParseFn;

class KIARA_COMPILER_API LangParserEntry : public BasicParserEntry
{
public:

    typedef boost::shared_ptr<LangParserEntry> Ptr;

    StmtParseFn stmtParse;

    LangParserEntry()
        : BasicParserEntry()
        , stmtParse()
    { }

    LangParserEntry &operator=(const LangParserEntry &other)
    {
        assign(other);
        return *this;
    }

    virtual void assign(const BasicParserEntry &other)
    {
        BasicParserEntry::assign(other);
        BOOST_ASSERT(dynamic_cast<const LangParserEntry*>(&other) != 0);
        stmtParse = static_cast<const LangParserEntry&>(other).stmtParse;
    }

    virtual ~LangParserEntry() { }
};

class KIARA_COMPILER_API LangParser : public PrattParser
{
public:

    LangParser(World &world);
    LangParser(const Scope::Ptr &topScope);
    ~LangParser();

    World & getWorld() const { return world_; }

    Scope::Ptr getScope() const { return builder_.getScope(); }

    void pushScope(const std::string &newName);

    void popScope();

    Type::Ptr parseType();

    // parseStringConstant - returns true if string was parsed
    bool parseStringConstant(std::string &str);

    bool parseAttributeMap(AttributeMap &attributes);

    bool parseAttributeMap(DictValue &attributes);

    IR::IRExpr::Ptr parseStringConstant();

    IR::IRExpr::Ptr parseNumericConstant();

    IR::Prototype::Ptr parsePrototype(bool requireReturnType = false, bool requireArgTypes = true);

    IR::ExternFunction::Ptr parseExtern();

    IR::FunctionDefinition::Ptr parseDefinition();

    StructType::Ptr parseStructType(const Token &firstToken);

    IR::TypeDefinition::Ptr parseStructTypeDefinition(const Token &firstToken);

    IR::TypeDefinition::Ptr parseTypedef(const Token &firstToken);

    bool addDefinition(const IR::FunctionDefinition::Ptr &funcDef);

    Object::Ptr parseStatement();

    bool addFunctionToScope(const IR::FunctionDefinition::Ptr &func); // FIXME should be private

    bool addObjectToScope(const std::string &name, const Object::Ptr &object); // FIXME should be private

    bool addVariableToScope(const IR::DefExpr::Ptr &var); // FIXME should be private

    bool removeFunctionFromScope(const IR::FunctionDeclaration::Ptr &func); // FIXME should be private

    void installOperator(const IR::FunctionDefinition::Ptr &op);

protected:

    void init(const Scope::Ptr &topScope);

    void registerStmtParser(const std::string &tokenSym, StmtParseFn parseFn)
    {
        registerStmtParser(TOK_NAME, tokenSym, parseFn);
    }

    void registerStmtParser(TokenType tokenType, const std::string &tokenSym, StmtParseFn parseFn)
    {
        boost::static_pointer_cast<LangParserEntry>(findOrCreateParserEntry(tokenType, tokenSym))
                ->stmtParse = parseFn;
    }

    void unregisterStmtParser(TokenType tokenType, const std::string &tokenSym)
    {
        LangParserEntry::Ptr entry =
                boost::static_pointer_cast<LangParserEntry>(findParserEntry(tokenType, tokenSym));
        if (entry)
            entry->stmtParse.clear();
    }

    virtual BasicParserEntry::Ptr createParserEntry() const;

    Object::Ptr parseConstant(const Token &token);

    Object::Ptr parseName(const Token &token);

    Object::Ptr parseBinaryOp(int precedence, const Object::Ptr &left, const Token &token);

    Object::Ptr parseUnaryOp(int precedence, const Token &token);

    Object::Ptr parseParenExpr(const Token &token);

    Object::Ptr parseFuncCall(const Object::Ptr &left, const Token &token);

    Object::Ptr parseArrayIndex(const Object::Ptr &left, const Token &token);

    Object::Ptr parseIfExpr(const Token &firstToken);

    Object::Ptr parseLoopExpr(const Token &firstToken);

    Object::Ptr parseForExpr(const Token &firstToken);

    Object::Ptr parseVarExpr(const Token &firstToken);

    Object::Ptr parseBreakExpr(const Token &firstToken);

    Object::Ptr parseBlockExpr(const Token &firstToken);

    Object::Ptr parseBlockExpr(const IR::BlockExpr::Ptr &dest, const SourceLocation *location = 0);

    Object::Ptr parseNamedBlockExpr(const Object::Ptr &left, const Token &token);

    IR::FunctionDeclaration::Ptr parseUndef();

private:
    World &world_;
    IRBuilder builder_;

    IR::IRExpr::Ptr resolveSymbol(const Object::Ptr &expr);
};

} // namespace Compiler

} // namespace KIARA

#endif /* KIARA_COMPILER_LANGPARSER_HPP_INCLUDED */
