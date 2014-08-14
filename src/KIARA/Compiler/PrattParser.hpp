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
 * PrattParser.hpp
 *
 *  Created on: 01.03.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMPILER_PRATTPARSER_HPP_INCLUDED
#define KIARA_COMPILER_PRATTPARSER_HPP_INCLUDED

#include "Config.hpp"
#include <KIARA/DB/Expr.hpp>
#include "Token.hpp"
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <stack>

// Based on http://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/

namespace KIARA
{

namespace Compiler
{

class PrattParser;
class Lexer;

// PrefixParseFn is nud (null denotation) in Pratt terminology
// InfixParseFn is led (left denotation) in Pratt terminology
// rbp is right binding power in Pratt terminology
// lbp is left binding power in Pratt terminology

typedef boost::function<Object::Ptr (PrattParser &parser, const Token &token)> PrefixParseFn;
typedef boost::function<Object::Ptr (PrattParser &parser, const Object::Ptr &left, const Token &token)> InfixParseFn;

class BasicParserEntry
{
public:

    typedef boost::shared_ptr<BasicParserEntry> Ptr;

    PrefixParseFn prefixParse;
    InfixParseFn infixParse;
    int lbp;

    BasicParserEntry()
        : prefixParse()
        , infixParse()
        , lbp(0)
    { }

    BasicParserEntry &operator=(const BasicParserEntry &other)
    {
        assign(other);
        return *this;
    }

    virtual void assign(const BasicParserEntry &other)
    {
        prefixParse = other.prefixParse;
        infixParse = other.infixParse;
        lbp = other.lbp;
    }

    virtual ~BasicParserEntry() { }
};

class KIARA_COMPILER_API PrattParser
{
public:

    class ParserToken : public Token
    {
    public:
        BasicParserEntry::Ptr entry;

        template <class T>
        boost::shared_ptr<T> getEntry() const
        {
            BOOST_ASSERT(boost::dynamic_pointer_cast<T>(entry));
            return boost::static_pointer_cast<T>(entry);
        }

        ParserToken()
            : Token()
            , entry()
        { }

        ParserToken(const Token &other)
            : Token(other)
            , entry()
        { }

        ParserToken(const ParserToken &other)
            : Token(other)
            , entry(other.entry)
        { }

        ParserToken & operator=(const Token &other)
        {
            static_cast<Token&>(*this) = static_cast<const Token&>(other);
            entry.reset();
            return *this;
        }

        ParserToken & operator=(const ParserToken &other)
        {
            static_cast<Token&>(*this) = static_cast<const Token&>(other);
            entry = other.entry;
            return *this;
        }
    };

    class ParserState
    {
        friend class PrattParser;
    public:

        ParserState()
            : fileName_()
            , lexer_(0)
        { }

    private:
        std::string fileName_;
        Lexer *lexer_;
        ParserToken currentToken_; // last token read by advance, match
        std::stack<ParserToken> tokenStack_; // arbitrary lookahead
    };

    PrattParser();
    virtual ~PrattParser();

    virtual void initParser(Lexer &lexer, const std::string &fileName = "stdin");

    const std::string & getFileName() const { return state_.fileName_; }
    const Lexer * getLexer() const { return state_.lexer_; }

    const ParserState & getState() const { return state_; }
    void setState(const ParserState &state) { state_ = state; }

    void registerPrefixParser(TokenType tokenType, PrefixParseFn parseFn)
    {
        registerPrefixParser(tokenType, "", parseFn);
    }

    void registerPrefixParser(const std::string &tokenSym, PrefixParseFn parseFn)
    {
        registerPrefixParser(TOK_NAME, tokenSym, parseFn);
    }

    void registerPrefixParser(TokenType tokenType, const std::string &tokenSym, PrefixParseFn parseFn);
    void unregisterPrefixParser(TokenType tokenType, const std::string &tokenSym);

    void registerInfixParser(const std::string &tokenSym, InfixParseFn parseFn)
    {
        registerInfixParser(TOK_NAME, tokenSym, parseFn);
    }

    void registerInfixParser(TokenType tokenType, const std::string &tokenSym, InfixParseFn parseFn);
    void unregisterInfixParser(TokenType tokenType, const std::string &tokenSym);

    void setLeftBindingPower(const std::string &tokenSym, int precedence)
    {
        setLeftBindingPower(TOK_NAME, tokenSym, precedence);
    }

    void setLeftBindingPower(TokenType tokenType, const std::string &tokenSym, int precedence);

    Object::Ptr expression(int rbp = 0);

    const ParserToken & advance();

    const ParserToken & advance(TokenType expectedType);

    const ParserToken & advance(TokenType expectedType, const std::string &expectedSym);

    const ParserToken & advance(const std::string &expectedSym);

    bool match(TokenType expectedType);

    bool match(TokenType expectedType, const std::string &expectedSym);

    bool match(const std::string &expectedSym);

    bool matchName(std::string &matchedSym)
    {
        return match(TOK_NAME, matchedSym);
    }

    bool matchOperator(std::string &matchedSym)
    {
        return match(TOK_OPERATOR, matchedSym);
    }

    bool match(TokenType expectedType, std::string &matchedSym);

    /** Returns true if the current token is expected symbol after skipping */
    bool skipUntil(const std::string &expectedSym);

    // Returns token parsed by last successful call to advance or match
    const ParserToken & current() const { return state_.currentToken_; }

    // Returns token that was not matched by last call to match
    const ParserToken & next() { return peekToken(); }

protected:

    typedef std::map<std::string, BasicParserEntry::Ptr> ParserEntryMap;

    virtual BasicParserEntry::Ptr createParserEntry() const;

    virtual BasicParserEntry::Ptr createParserEntry(const Token &token) const;

    ParserToken & peekToken();

    void pushToken(const ParserToken &token);

    ParserToken popToken();

    ParserToken getToken()
    {
        return popToken();
    }

    void ungetToken(const ParserToken &token)
    {
        pushToken(token);
    }

    void error(const std::string &msg, const SourceLocation *loc = 0);
    void error(const std::string &msg, const SourceLocation &loc)
    {
        error(msg, &loc);
    }

    BasicParserEntry::Ptr findParserEntry(TokenType tokenType, const std::string &tokenSymbol = "") const
    {
        std::string id = makeTokenId(tokenType, tokenSymbol);
        ParserEntryMap::const_iterator it = entries_.find(id);
        if (it != entries_.end())
            return it->second;
        return BasicParserEntry::Ptr();
    }

    BasicParserEntry::Ptr findParserEntry(const Token &token) const
    {
        return findParserEntry(token.type, token.str);
    }

    BasicParserEntry::Ptr findOrCreateParserEntry(TokenType tokenType, const std::string &tokenSymbol = "")
    {
        std::string id = makeTokenId(tokenType, tokenSymbol);
        ParserEntryMap::iterator it = entries_.find(id);
        if (it != entries_.end())
            return it->second;
        BasicParserEntry::Ptr entry = createParserEntry();
        entries_[id] = entry;
        return entry;
    }

    BasicParserEntry::Ptr findOrCreateParserEntry(const Token &token)
    {
        return findOrCreateParserEntry(token.type, token.str);
    }

    static PrefixParseFn getPrefixParser(const ParserToken &token)
    {
        if (token.entry)
            return token.entry->prefixParse;
        return PrefixParseFn();
    }

    static InfixParseFn getInfixParser(const ParserToken &token)
    {
        if (token.entry)
            return token.entry->infixParse;
        return InfixParseFn();
    }

    static int getLeftBindingPower(const ParserToken &token)
    {
        return token.entry ? token.entry->lbp : 0;
    }

    std::string makeTokenId(TokenType tokenType, const std::string &tokenSym = "") const;

private:
    ParserState state_;
    ParserEntryMap entries_;
    BasicParserEntry::Ptr errorEntry_;
};

} // namespace Compiler

} // namespace KIARA

#endif /* KIARA_COMPILER_PRATTPARSER_HPP_INCLUDED */
