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
 * Compiler.cpp
 *
 *  Created on: 19.03.2013
 *      Author: Dmitri Rubinstein
 */

#define KIARA_COMPILER_LIB

#include "Compiler.hpp"
#include "LangParser.hpp"
#include <KIARA/Utils/VarGuard.hpp>

namespace KIARA
{

namespace Compiler
{

// Compiler

Compiler::Compiler()
    : phases_()
{
}

Compiler::~Compiler()
{
}

void Compiler::addPhase(const CompilerPhasePtr &stage)
{
    if (stage)
    {
        // TODO check that currently no compilation happens
        phases_.push_back(stage);
    }
}

void Compiler::removeAllPhases()
{
    phases_.clear();
}

// CompilerPhase

CompilerPhase::CompilerPhase(Compiler &compiler)
    : compiler_(compiler)
{
}

CompilerPhase::~CompilerPhase()
{
}

void Compiler::runPhases(CompilationContext &ctx, const Object::Ptr &object)
{
    if (ctx.isEndOfCompilation() || ctx.getPhase() >= phases_.size())
        return;

    VarGuard<size_t> g(ctx.phase_);
    CompilerPhase::Ptr stage = phases_[ctx.phase_++];
    BOOST_ASSERT(stage);
    stage->runPhase(ctx, object);
}

void Compiler::emit(CompilationContext &ctx, const Object::Ptr &object)
{
    runPhases(ctx, object);
}

// ParserPhase

ParserPhase::ParserPhase(LangParser &parser, Compiler &compiler)
    : CompilerPhase(compiler)
    , parser_(parser)
{
}

void ParserPhase::runPhase(CompilationContext &ctx, const Object::Ptr &object)
{
    ctx.clear("parser.result");

    const Token &tok = parser_.current();
    if (tok.isEof() || tok.isError())
    {
        endCompilation(ctx);
        return;
    }

    Object::Ptr stmt = parser_.parseStatement();
    if (!stmt)
    {
        endCompilation(ctx);
        return;
    }

    ctx.set("parser.result", stmt);
    emit(ctx, stmt);
}

} // namespace Compiler

} // namespace KIARA
