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
 * Compiler.hpp
 *
 *  Created on: 19.03.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_COMPILER_COMPILER_HPP_INCLUDED
#define KIARA_COMPILER_COMPILER_HPP_INCLUDED

#include "Config.hpp"
#include "IR.hpp"
#include "IRTransformer.hpp"
#include <map>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>

namespace KIARA
{

namespace Compiler
{

class LangParser;
class CompilerPhase;
class Compiler;
class CodeGen;
typedef boost::shared_ptr<CompilerPhase> CompilerPhasePtr;

class CompilationContext
{
    friend class Compiler;
public:

    typedef std::map<std::string, boost::any> DataTransferObject;
    DataTransferObject dto;

    CompilationContext(size_t phase = 0)
        : dto()
        , phase_(phase)
        , endOfCompilation_(false)
    {
    }

    void endCompilation(bool flag = true)
    {
        endOfCompilation_ = flag;
    }

    bool isEndOfCompilation() { return endOfCompilation_; }

    template <typename ValueType>
    ValueType get(const std::string &name, ValueType defaultValue) const
    {
        DataTransferObject::const_iterator it = dto.find(name);
        if (it == dto.end() || it->second.empty())
            return defaultValue;
        boost::any tmp(it->second);
        ValueType *value = boost::any_cast<ValueType>(&tmp);
        if (!value)
            return defaultValue;
        return *value;
    }

    template <typename ValueType>
    void set(const std::string &name, const ValueType &value)
    {
        dto[name] = value;
    }

    void clear(const std::string &name)
    {
        DataTransferObject::iterator it = dto.find(name);
        if (it != dto.end())
            it->second = boost::any();
    }

    void erase(const std::string &name)
    {
        dto.erase(name);
    }

    void clear()
    {
        dto.clear();
        phase_ = 0;
    }

    size_t getPhase() const { return phase_; }

private:
    size_t phase_;
    bool endOfCompilation_;
};

class KIARA_COMPILER_API Compiler
{
    friend class CompilerPhase;
public:

    Compiler();
    virtual ~Compiler();

    void addPhase(const CompilerPhasePtr &stage);

    void removeAllPhases();

    void runPhases(CompilationContext &ctx, const Object::Ptr &object = 0);

    void emit(CompilationContext &ctx, const Object::Ptr &object);

private:
    std::vector<CompilerPhasePtr> phases_;
};

// CompilerPhase

class KIARA_COMPILER_API CompilerPhase
{
public:
    typedef CompilerPhasePtr Ptr;

    virtual ~CompilerPhase();
    virtual void runPhase(CompilationContext &ctx, const Object::Ptr &object) = 0;

protected:
    CompilerPhase(Compiler &compiler);
    Compiler &compiler_;

    void emit(CompilationContext &ctx, const Object::Ptr &object)
    {
        compiler_.emit(ctx, object);
    }

    void endCompilation(CompilationContext &ctx)
    {
        ctx.endCompilation();
    }
};

// ParserPhase

class ParserPhase : public CompilerPhase
{
public:

    typedef boost::shared_ptr<ParserPhase> Ptr;

    ParserPhase(LangParser &parser, Compiler &compiler);

    void runPhase(CompilationContext &ctx, const Object::Ptr &object);

private:
    LangParser &parser_;
};

// TransformPhase

template <class Transformer>
class TransformPhase : public CompilerPhase, public Transformer
{
public:

    typedef boost::shared_ptr<TransformPhase> Ptr;

    TransformPhase(Compiler &compiler)
        : CompilerPhase(compiler)
        , Transformer()
    {
    }

    void runPhase(CompilationContext &ctx, const Object::Ptr &object)
    {
        this->reset();
        this->apply(object);
        this->reset();
        emit(ctx, object);
    }
};

} // namespace Compiler

} // namespace KIARA

#endif /* KIARA_COMPILER_COMPILER_HPP_INCLUDED */
