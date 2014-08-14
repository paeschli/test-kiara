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
 * KDArgument.hpp
 *
 *  Created on: Mar 11, 2010
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_LLVM_KDARGUMENT_HPP_INCLUDED
#define KIARA_LLVM_KDARGUMENT_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <KIARA/LLVM/Utils.hpp>
#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant/bad_visit.hpp>
#include <vector>

namespace KIARA
{

/// KIARA Directive Argument
class KIARA_API KDArgument
{
public:

    enum Type
    {
        EMPTY_ARG,
        INTEGER_ARG,
        FLOAT_ARG,
        STRING_ARG,
        NULL_PTR_ARG,
        LLVM_VALUE_ARG
    };

    typedef int64_t IntegerType;
    typedef double FloatType;
    typedef std::string StringType;
    struct Empty { };
    struct NullPtrType
    {
        llvm::PointerType *type;

        NullPtrType() :
            type(0)
        { }

        NullPtrType(llvm::PointerType *type) :
            type(type)
        { }
    };
    typedef boost::variant<Empty, IntegerType, FloatType, StringType, NullPtrType, const llvm::Value*> Value;

    KDArgument() :
        value(Empty())
    { }

    KDArgument(FloatType f) :
        value(f)
    { }

    KDArgument(IntegerType i) :
        value(i)
    { }

    KDArgument(const char *s) :
        value(StringType(s))
    { }

    KDArgument(const StringType &s) :
        value(s)
    { }

    KDArgument(llvm::PointerType * pt) :
        value(NullPtrType(pt))
    { }

    KDArgument(const NullPtrType &pt) :
        value(pt)
    { }

    KDArgument(const llvm::Value *llvmValue);

    const KDArgument &operator=(const KDArgument &other)
    {
        value = other.value;
        return *this;
    }

    Type getType() const
    {
        return static_cast<Type>(value.which());
    }

    const Value & getValue() const
    {
        return value;
    }

    bool isEmpty() const
    {
        return getType() == EMPTY_ARG;
    }

    IntegerType toInteger() const
    {
        return boost::apply_visitor(ToInteger(), value);
    }

    FloatType toFloat() const
    {
        return boost::apply_visitor(ToFloat(), value);
    }

    StringType toString() const
    {
        return boost::apply_visitor(ToString(), value);
    }

    IntegerType getInteger() const
    {
        return boost::get<IntegerType>(value);
    }

    FloatType getFloat() const
    {
        return boost::get<FloatType>(value);
    }

    StringType getString() const
    {
        return boost::get<StringType>(value);
    }

    NullPtrType getNullPtr() const
    {
        return boost::get<NullPtrType>(value);
    }

    const llvm::Value *getLLVMValue() const
    {
        return boost::get<const llvm::Value *>(value);
    }

    void setEmpty() { value = Empty(); }

    void setFloat(FloatType v) { value = v; }

    void setInteger(IntegerType v) { value = v; }

    void setString(const char *v) { value = StringType(v); }

    void setString(const StringType &v) { value = v; }

    void setNullPtr(llvm::PointerType *pt)
    {
        value = NullPtrType(pt);
    }

    void setNullPtr(const NullPtrType &pt)
    {
        value = pt;
    }

    void setLLVMValue(const llvm::Value *v);

private:

    struct ToInteger : public boost::static_visitor<IntegerType>
    {
        IntegerType operator()(Empty) const
        {
            throw boost::bad_visit();
        }

        IntegerType operator()(IntegerType v) const
        {
            return v;
        }

        IntegerType operator()(FloatType v) const
        {
            return static_cast<IntegerType>(v);
        }

        IntegerType operator()(const StringType &v) const
        {
            return boost::lexical_cast<IntegerType>(v);
        }

        IntegerType operator()(const NullPtrType &v) const
        {
            throw boost::bad_visit();
        }

        IntegerType operator()(const llvm::Value *v) const
        {
            throw boost::bad_visit();
        }
    };

    struct ToFloat : public boost::static_visitor<FloatType>
    {
        FloatType operator()(Empty) const { throw boost::bad_visit(); }

        FloatType operator()(IntegerType v) const
        {
            return static_cast<FloatType>(v);
        }

        FloatType operator()(FloatType v) const { return v; }

        FloatType operator()(const StringType &v) const
        {
            return boost::lexical_cast<FloatType>(v);
        }

        FloatType operator()(const NullPtrType &v) const
        {
            throw boost::bad_visit();
        }

        FloatType operator()(const llvm::Value *v) const
        {
            throw boost::bad_visit();
        }
    };

    struct ToString : public boost::static_visitor<StringType>
    {

        StringType operator()(Empty) const { throw boost::bad_visit(); }

        StringType operator()(IntegerType v) const
        {
            return boost::lexical_cast<StringType>(v);
        }

        StringType operator()(FloatType v) const
        {
            return boost::lexical_cast<StringType>(v);
        }

        StringType operator()(const StringType &v) const
        {
            return v;
        }

        StringType operator()(const NullPtrType &v) const
        {
            return StringType("null of type ")+llvmToString(llvmUpCast(v.type));
        }

        StringType operator()(const llvm::Value *v) const
        {
            return llvmToString(v);
        }

    };

    Value value;
};

} // namespace KIARA

namespace boost
{

#define _KIARA_HAS_NOTHROW_COPY(C)  \
  template <>                      \
  struct has_nothrow_copy< C >     \
    : mpl::true_                   \
  {                                \
  }

_KIARA_HAS_NOTHROW_COPY(::KIARA::KDArgument::Empty);

#undef _KIARA_HAS_NOTHROW_COPY

} // namespace boost

#endif
