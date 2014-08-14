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
 * PrattParser.cpp
 *
 *  Created on: 01.03.2013
 *      Author: Dmitri Rubinstein
 */
#define KIARA_COMPILER_LIB
#include "PrattParser.hpp"
#include "Lexer.hpp"
#include <memory>

// Based on http://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/

namespace KIARA {

namespace Compiler {

#define PARSER_ERROR(message)                                   \
do {                                                            \
   std::ostringstream msgs;                                     \
   msgs << message;                                             \
   error(msgs.str());                                           \
   return 0;                                                    \
} while(false)

#define PARSER_WRONG_TOKEN(token, message)                              \
do {                                                                    \
   lexer_->ungetToken(token);                                           \
   std::ostringstream msgs;                                             \
   msgs << message << " (found token "                                  \
        <<lexer_->getTokenDescription(lexer_->getCurrentToken())<<")";  \
   error(msgs.str());                                                   \
   return 0;                                                            \
} while(false)

// Parser

PrattParser::PrattParser()
    : state_()
    , entries_()
{
}

PrattParser::~PrattParser()
{
}

void PrattParser::initParser(Lexer &lexer, const std::string &fileName)
{
    state_.lexer_ = &lexer;
    state_.fileName_ = fileName;
    if (!errorEntry_)
        errorEntry_ = createParserEntry();
    advance(); // read first token
}

BasicParserEntry::Ptr PrattParser::createParserEntry() const
{
    return BasicParserEntry::Ptr(new BasicParserEntry());
}

BasicParserEntry::Ptr PrattParser::createParserEntry(const Token &token) const
{
    BasicParserEntry::Ptr entry = findParserEntry(token);
    BasicParserEntry::Ptr tokEntry = createParserEntry();

    if (entry)
        *tokEntry = *entry;

    if ((!entry || !tokEntry->prefixParse || !tokEntry->infixParse) && token.isSymbol())
    {
        if (BasicParserEntry::Ptr entry1 = findParserEntry(token.type))
        {
            if (!entry)
                *tokEntry = *entry1;
            else
            {
                if (!tokEntry->prefixParse)
                    tokEntry->prefixParse = entry1->prefixParse;
                if (!tokEntry->infixParse)
                    tokEntry->infixParse = entry1->infixParse;
            }
        }
    }
    return tokEntry;
}

void PrattParser::registerPrefixParser(TokenType tokenType, const std::string &tokenSym, PrefixParseFn parseFn)
{
    findOrCreateParserEntry(tokenType, tokenSym)->prefixParse = parseFn;
}

void PrattParser::unregisterPrefixParser(TokenType tokenType, const std::string &tokenSym)
{
    BasicParserEntry::Ptr entry = findParserEntry(tokenType, tokenSym);
    if (entry)
        entry->prefixParse.clear();
}

void PrattParser::registerInfixParser(TokenType tokenType, const std::string &tokenSym, InfixParseFn parseFn)
{
    findOrCreateParserEntry(tokenType, tokenSym)->infixParse = parseFn;
}

void PrattParser::unregisterInfixParser(TokenType tokenType, const std::string &tokenSym)
{
    BasicParserEntry::Ptr entry = findParserEntry(tokenType, tokenSym);
    if (entry)
        entry->infixParse.clear();
}

void PrattParser::setLeftBindingPower(TokenType tokenType, const std::string &tokenSym, int precedence)
{
    findOrCreateParserEntry(tokenType, tokenSym)->lbp = precedence;
}

Object::Ptr PrattParser::expression(int rbp)
{
    ParserToken t = current();
    advance();

    BOOST_ASSERT(t.entry);
    PrefixParseFn prefix = t.entry->prefixParse;

    if (!prefix)
    {
        error("Could not parse, no prefix parser for \"" + t.getDescr() + "\".", t.loc);
        return 0;
    }

    Object::Ptr left = prefix(*this, t);
    if (!left)
        return 0;

    BOOST_ASSERT(current().entry);
    while (rbp < current().entry->lbp)
    {
        t = current();
        advance();

        BOOST_ASSERT(t.entry);
        InfixParseFn infix = t.entry->infixParse;
        if (!infix)
        {
            error("Could not parse, no infix parser for \"" + t.getDescr() + "\".", t.loc);
            return 0;
        }

        left = infix(*this, left, t);
        if (!left)
            return 0;
    }

    return left;
}

const PrattParser::ParserToken & PrattParser::advance()
{
    state_.currentToken_ = getToken();
    state_.currentToken_.entry = createParserEntry(state_.currentToken_);
    return state_.currentToken_;
}

const PrattParser::ParserToken & PrattParser::advance(TokenType expectedType)
{
    const Token &token = current();
    if (token.type != expectedType)
    {
        std::string errorMsg =
                std::string("Expected token ")+Token::getTypeDescr(expectedType)+
                " but found "+token.getDescr();
        error(errorMsg, token.loc);
        state_.currentToken_ = Token(TOK_ERROR, errorMsg, token.loc);
        state_.currentToken_.entry = errorEntry_;
        return state_.currentToken_;
    }
    return advance();
}

const PrattParser::ParserToken & PrattParser::advance(TokenType expectedType, const std::string &expectedSym)
{
    const Token &token = current();
    if (token.type != expectedType || token.str != expectedSym)
    {
        std::string errorMsg = std::string("Expected token ")+Token::getTypeDescr(expectedType)+
                " "+expectedSym+" but found "+token.getDescr();
        error(errorMsg, token.loc);
        state_.currentToken_ = Token(TOK_ERROR, errorMsg, token.loc);
        state_.currentToken_.entry = errorEntry_;
        return state_.currentToken_;
    }
    return advance();
}

const PrattParser::ParserToken & PrattParser::advance(const std::string &expectedSym)
{
    const Token &token = current();
    if (!token.isSymbol(expectedSym))
    {
        std::string errorMsg = std::string("Expected symbol token '")+
                expectedSym+"' but found "+token.getDescr();
        error(errorMsg, token.loc);
        state_.currentToken_ = Token(TOK_ERROR, errorMsg, token.loc);
        state_.currentToken_.entry = errorEntry_;
        return state_.currentToken_;
    }
    return advance();
}

bool PrattParser::match(TokenType expectedType)
{
    const Token &token = current();
    if (token.type != expectedType)
        return false;
    advance();
    return true;
}

bool PrattParser::match(TokenType expectedType, const std::string &expectedSym)
{
    const Token &token = current();
    if (token.type != expectedType)
        return false;
    if (token.getStringValue() != expectedSym)
        return false;
    advance();
    return true;
}

bool PrattParser::match(const std::string &expectedSym)
{
    const Token &token = current();
    if (!token.isSymbol(expectedSym))
        return false;
    advance();
    return true;
}

bool PrattParser::match(TokenType expectedType, std::string &matchedSym)
{
    const Token &token = current();
    if (token.type != expectedType)
        return false;
    matchedSym = token.getStringValue();
    advance();
    return true;
}

bool PrattParser::skipUntil(const std::string &expectedSym)
{
    while (!current().isSymbol(expectedSym) && !current().isEof())
        advance();
    return !current().isEof();
}

PrattParser::ParserToken & PrattParser::peekToken()
{
    if (state_.tokenStack_.empty())
    {
        ParserToken newToken;
        state_.lexer_->next(newToken);
        state_.tokenStack_.push(newToken);
    }
    return state_.tokenStack_.top();
}

void PrattParser::pushToken(const ParserToken &token)
{
    state_.tokenStack_.push(token);
}

PrattParser::ParserToken PrattParser::popToken()
{
    ParserToken token = peekToken();
    state_.tokenStack_.pop();
    return token;
}

void PrattParser::error(const std::string &msg, const SourceLocation *loc)
{
    std::cerr<<state_.fileName_<<":"<<source_location(loc)<<": error: "<<msg<<std::endl;
}

std::string PrattParser::makeTokenId(TokenType tokenType, const std::string &tokenSym) const
{
    switch (tokenType)
    {
        case TOK_EMPTY:     return "empty";
        case TOK_ERROR:     return "error";
        case TOK_EOF:       return "eof";
        case TOK_NAME:
        case TOK_OPERATOR:
            return "sym:"+tokenSym;
        case TOK_ICONST:    return "iconst";
        case TOK_FCONST:    return "fconst";
        case TOK_SCONST:    return "sconst";
    }
    return "";
}

} // namespace Compiler

} // namespace KIARA
