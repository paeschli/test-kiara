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
 * Value.hpp
 *
 *  Created on: 28.05.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_DB_VALUE_HPP_INCLUDED
#define KIARA_DB_VALUE_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <KIARA/Common/Types.hpp>
#include <KIARA/Common/TypeTraits.hpp>
#include <KIARA/Utils/Box.hpp>
#include <KIARA/Utils/Error.hpp>
#include <KIARA/DB/Object.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>
#include <boost/any.hpp>
#include <vector>
#include <map>
#include <ostream>
#include <string>

namespace KIARA
{

enum NumberType
{
    NT_INT8_T,
    NT_UINT8_T,
    NT_INT16_T,
    NT_UINT16_T,
    NT_INT32_T,
    NT_UINT32_T,
    NT_INT64_T,
    NT_UINT64_T,
    NT_FLOAT,
    NT_DOUBLE,
    NT_LAST = NT_DOUBLE
};

struct Number
{
public:

    Number() : value_(), type_(NT_INT32_T) {  }
    Number(int8_t  v) : value_(v), type_(NT_INT8_T) { }
    Number(uint8_t  v) : value_(v), type_(NT_UINT8_T) { }
    Number(int16_t  v) : value_(v), type_(NT_INT16_T) { }
    Number(uint16_t  v) : value_(v), type_(NT_UINT16_T) { }
    Number(int32_t  v) : value_(v), type_(NT_INT32_T) { }
    Number(uint32_t  v) : value_(v), type_(NT_UINT32_T) { }
    Number(int64_t  v) : value_(v), type_(NT_INT64_T) { }
    Number(uint64_t  v) : value_(v), type_(NT_UINT64_T) { }
    Number(float   v) : value_(v), type_(NT_FLOAT) { }
    Number(double  v) : value_(v), type_(NT_DOUBLE) { }

    Number(const Number &v)
        : value_(v.value_)
        ,  type_(v.type_)
    {
    }

    ~Number()
    {
    }

    template <typename T>
    static Number create(T value)
    {
        return Number(normalize_value(value));
    }

    bool operator==(const Number& other) const
    {
        if (other.type_ != type_)
            return false;
        return other.value_ == value_;
    }

    bool operator!=(const Number& other) const
    {
        return !(*this == other);
    }

    template <typename T>
    T get() const { return get_impl<typename normalize_type<T>::type>(); }

    template <typename T>
    void set(T value) { set_impl<typename normalize_type<T>::type>(value); }

    template <typename T>
    bool is() const { return is_impl<typename normalize_type<T>::type>(); }

    template <typename T>
    T get_impl() const { KIARA_UNREACHABLE("Unsupported type"); }

    template <typename T>
    void set_impl(T value) { KIARA_UNREACHABLE("Unsupported type"); }

    template <typename T>
    bool is_impl() const { KIARA_UNREACHABLE("Unsupported type"); }

    bool is_i8() const { return type_ == NT_INT8_T; }
    bool is_u8() const { return type_ == NT_UINT8_T; }
    bool is_i16() const { return type_ == NT_INT16_T; }
    bool is_u16() const { return type_ == NT_UINT16_T; }
    bool is_i32() const { return type_ == NT_INT32_T; }
    bool is_u32() const { return type_ == NT_UINT32_T; }
    bool is_i64() const { return type_ == NT_INT64_T; }
    bool is_u64() const { return type_ == NT_UINT64_T; }
    bool is_float() const { return type_ == NT_FLOAT; }
    bool is_double() const { return type_ == NT_DOUBLE; }

    bool isReal() const { return is_float() || is_double(); }
    bool isInteger() const { return !isReal(); }

    int8_t   get_i8() const { BOOST_ASSERT(type_ == NT_INT8_T); return value_.get_i8(); }
    uint8_t  get_u8() const { BOOST_ASSERT(type_ == NT_UINT8_T); return value_.get_u8(); }
    int16_t  get_i16() const { BOOST_ASSERT(type_ == NT_INT16_T); return value_.get_i16(); }
    uint16_t get_u16() const { BOOST_ASSERT(type_ == NT_UINT16_T); return value_.get_u16(); }
    int32_t  get_i32() const { BOOST_ASSERT(type_ == NT_INT32_T); return value_.get_i32(); }
    uint32_t get_u32() const { BOOST_ASSERT(type_ == NT_UINT32_T); return value_.get_u32(); }
    int64_t  get_i64() const { BOOST_ASSERT(type_ == NT_INT64_T); return value_.get_i64(); }
    uint64_t get_u64() const { BOOST_ASSERT(type_ == NT_UINT64_T); return value_.get_u64(); }
    float  get_float() const { BOOST_ASSERT(type_ == NT_FLOAT); return value_.get_float(); }
    double get_double() const { BOOST_ASSERT(type_ == NT_DOUBLE); return value_.get_double(); }

    void set_i8(int8_t v) { type_ = NT_INT8_T; value_.set_i8(v); }
    void set_u8(uint8_t v) { type_ = NT_UINT8_T; value_.set_u8(v); }
    void set_i16(int16_t v) { type_ = NT_INT16_T; value_.set_i16(v); }
    void set_u16(uint16_t v) { type_ = NT_UINT16_T; value_.set_u16(v); }
    void set_i32(int32_t v) { type_ = NT_INT32_T; value_.set_i32(v); }
    void set_u32(uint32_t v) { type_ = NT_UINT32_T; value_.set_u32(v); }
    void set_i64(int64_t v) { type_ = NT_INT64_T; value_.set_i64(v); }
    void set_u64(uint64_t v) { type_ = NT_UINT64_T; value_.set_u64(v); }
    void set_float(float v) { type_ = NT_FLOAT; value_.set_float(v); }
    void set_double(double v) { type_ = NT_DOUBLE; value_.set_double(v); }

#define _KIARA_NUMBER_IDENTITY(X) X
#define _KIARA_NUMBER_CONVERT(F)                    \
        switch (type_)                              \
        {                                           \
            case NT_INT8_T: return F(value_.i8);    \
            case NT_UINT8_T: return F(value_.u8);   \
            case NT_INT16_T: return F(value_.i16);  \
            case NT_UINT16_T: return F(value_.u16); \
            case NT_INT32_T: return F(value_.i32);  \
            case NT_UINT32_T: return F(value_.u32); \
            case NT_INT64_T: return F(value_.i64);  \
            case NT_UINT64_T: return F(value_.u64); \
            case NT_FLOAT: return F(value_.f);      \
            case NT_DOUBLE: return F(value_.d);     \
        }                                           \
        KIARA_UNREACHABLE("invalid type of argument")

    int64_t toInt() const
    {
        _KIARA_NUMBER_CONVERT(_KIARA_NUMBER_IDENTITY);
    }

    uint64_t toUInt() const
    {
        _KIARA_NUMBER_CONVERT(_KIARA_NUMBER_IDENTITY);
    }

    float toFloat() const
    {
        _KIARA_NUMBER_CONVERT(_KIARA_NUMBER_IDENTITY);
    }

    double toDouble() const
    {
        _KIARA_NUMBER_CONVERT(_KIARA_NUMBER_IDENTITY);
    }

    std::string toString() const
    {
        _KIARA_NUMBER_CONVERT(boost::lexical_cast<std::string>);
    }

#undef _KIARA_NUMBER_CONVERT
#undef _KIARA_NUMBER_IDENTITY

    NumberType getType() const { return type_; }
    const Box &getValue() const { return value_; }

    size_t hash() const
    {
        size_t seed = 0;
        boost::hash_combine(seed, type_);
        switch (type_)
        {
            case NT_INT8_T:
                boost::hash_combine(seed, value_.i8);
                break;
            case NT_UINT8_T:
                boost::hash_combine(seed, value_.u8);
                break;
            case NT_INT16_T:
                boost::hash_combine(seed, value_.i16);
                break;
            case NT_UINT16_T:
                boost::hash_combine(seed, value_.u16);
                break;
            case NT_INT32_T:
                boost::hash_combine(seed, value_.i32);
                break;
            case NT_UINT32_T:
                boost::hash_combine(seed, value_.u32);
                break;
            case NT_INT64_T:
                boost::hash_combine(seed, value_.i64);
                break;
            case NT_UINT64_T:
                boost::hash_combine(seed, value_.u64);
                break;
            case NT_FLOAT:
                boost::hash_combine(seed, value_.f);
                break;
            case NT_DOUBLE:
                boost::hash_combine(seed, value_.d);
                break;
        }
        return seed;
    }

private:
    Box value_;
    NumberType type_;
};

inline std::size_t hash_value(const Number & b)
{
    return b.hash();
}

inline std::ostream & operator<<(std::ostream &out, const Number &n)
{
    out<<n.toString();
    return out;
}

template <> inline int8_t Number::get_impl<int8_t>() const { return get_i8(); }
template <> inline uint8_t Number::get_impl<uint8_t>() const { return get_u8(); }
template <> inline int16_t Number::get_impl<int16_t>() const { return get_i16(); }
template <> inline uint16_t Number::get_impl<uint16_t>() const { return get_u16(); }
template <> inline int32_t Number::get_impl<int32_t>() const { return get_i32(); }
template <> inline uint32_t Number::get_impl<uint32_t>() const { return get_u32(); }
template <> inline int64_t Number::get_impl<int64_t>() const { return get_i64(); }
template <> inline uint64_t Number::get_impl<uint64_t>() const { return get_u64(); }
template <> inline float Number::get_impl<float>() const { return get_float(); }
template <> inline double Number::get_impl<double>() const { return get_double(); }

template <> inline void Number::set_impl<int8_t>(int8_t v) { set_i8(v); }
template <> inline void Number::set_impl<uint8_t>(uint8_t v) { set_u8(v); }
template <> inline void Number::set_impl<int16_t>(int16_t v) { set_i16(v); }
template <> inline void Number::set_impl<uint16_t >(uint16_t v) { set_u16(v); }
template <> inline void Number::set_impl<int32_t>(int32_t v) { set_i32(v); }
template <> inline void Number::set_impl<uint32_t >(uint32_t v) { set_u32(v); }
template <> inline void Number::set_impl<int64_t>(int64_t v) { set_i64(v); }
template <> inline void Number::set_impl<uint64_t>(uint64_t v) { set_u64(v); }
template <> inline void Number::set_impl<float>(float v) { set_float(v); }
template <> inline void Number::set_impl<double>(double v) { set_double(v); }

template <> inline bool Number::is_impl<int8_t>() const { return is_i8(); }
template <> inline bool Number::is_impl<uint8_t>() const { return is_u8(); }
template <> inline bool Number::is_impl<int16_t>() const { return is_i16(); }
template <> inline bool Number::is_impl<uint16_t>() const { return is_u16(); }
template <> inline bool Number::is_impl<int32_t>() const { return is_i32(); }
template <> inline bool Number::is_impl<uint32_t>() const { return is_u32(); }
template <> inline bool Number::is_impl<int64_t>() const { return is_i64(); }
template <> inline bool Number::is_impl<uint64_t>() const { return is_u64(); }
template <> inline bool Number::is_impl<float>() const { return is_float(); }
template <> inline bool Number::is_impl<double>() const { return is_double(); }

enum ValueType
{
    VT_NULL,
    VT_NUMBER,
    VT_BOOL,
    VT_STRING,
    VT_OBJECT,
    VT_ANY,
    VT_DICT,
    VT_ARRAY
};

struct NullValue { };

class DictValue;
class ArrayValue;

class Value
{
public:

    Value() : value_(NullValue()) { }
    Value(const Number &number) : value_(number) { }
    Value(bool b) : value_(b) { }
    Value(const char *str) : value_(std::string(str)) { }
    Value(const std::string &str) : value_(str) { }
    Value(const Object::Ptr &obj) : value_(obj) { }
    Value(const DictValue &value) : value_(value) { }
    Value(const ArrayValue &value) : value_(value) { }
    Value(const Value &other) : value_(other.value_) { }

    template <typename T>
    Value(const T &other)
    {
        set(other);
    }

    ValueType getType() const
    {
        return static_cast<ValueType>(value_.which());
    }

    bool isNull() const { return getType() == VT_NULL; }
    bool isNumber() const { return getType() == VT_NUMBER; }
    bool isBool() const { return getType() == VT_BOOL; }
    bool isString() const { return getType() == VT_STRING; }
    bool isObject() const { return getType() == VT_OBJECT; }
    bool isAny() const { return getType() == VT_ANY; }
    bool isDict() const { return getType() == VT_DICT; }
    bool isArray() const { return getType() == VT_ARRAY; }

    bool isRealNumber() { return isNumber() && getNumber().isReal(); }
    bool isIntegerNumber() { return isNumber() && getNumber().isInteger(); }
    bool isTrue() const { return isBool() && getBool(); }
    bool isFalse() const { return isBool() && !getBool(); }

    void setNull()
    {
        value_ = NullValue();
    }

    Number & getNumber()
    {
        return boost::get<Number>(value_);
    }

    const Number & getNumber() const
    {
        return boost::get<Number>(value_);
    }

    void setNumber(const Number &value)
    {
        value_ = value;
    }

    bool & getBool()
    {
        return boost::get<bool>(value_);
    }

    bool getBool() const
    {
        return boost::get<bool>(value_);
    }

    void setBool(bool value)
    {
        value_ = value;
    }

    std::string & getString()
    {
        return boost::get<std::string>(value_);
    }

    const std::string & getString() const
    {
        return boost::get<std::string>(value_);
    }

    void setString(const std::string &value)
    {
        value_ = value;
    }

    Object::Ptr & getObject()
    {
        return boost::get<Object::Ptr>(value_);
    }

    const Object::Ptr & getObject() const
    {
        return boost::get<Object::Ptr>(value_);
    }

    void setObject(const Object::Ptr &value)
    {
        value_ = value;
    }

    boost::any & getAny()
    {
        return boost::get<boost::any>(value_);
    }

    const boost::any & getAny() const
    {
        return boost::get<boost::any>(value_);
    }

    void setAny(const boost::any &value)
    {
        value_ = value;
    }

    DictValue & getDict();

    const DictValue & getDict() const;

    DictValue & getOrCreateDict();

    void setDict(const DictValue &value);

    ArrayValue & getArray();

    const ArrayValue & getArray() const;

    ArrayValue & getOrCreateArray();

    void setArray(const ArrayValue &value);

    template <typename T>
    T get() const { return get_impl<typename normalize_type<T>::type>(); }

    template <typename T>
    T get_impl() const { KIARA_UNREACHABLE("Unsupported type"); }


    void set(const char *value)
    {
        setString(value);
    }

    void set(const Object::Ptr &value)
    {
        setObject(value);
    }

    void set(char value) { setNumber(normalize_value(value)); }
    void set(signed char value) { setNumber(normalize_value(value)); }
    void set(unsigned char value) { setNumber(normalize_value(value)); }
    void set(short value) { setNumber(normalize_value(value)); }
    void set(unsigned short value) { setNumber(normalize_value(value)); }
    void set(int value) { setNumber(normalize_value(value)); }
    void set(unsigned int value) { setNumber(normalize_value(value)); }
    void set(long value) { setNumber(normalize_value(value)); }
    void set(unsigned long value) { setNumber(normalize_value(value)); }
    void set(long long value) { setNumber(normalize_value(value)); }
    void set(unsigned long long value) { setNumber(normalize_value(value)); }
    void set(float value) { setNumber(value); }
    void set(double value) { setNumber(value); }

    template <typename T>
    void set(const T &value)
    {
        set_impl<typename normalize_type<T>::type>(value);
    }

    template <typename T>
    void set_impl(const T &value) { value_ = value; }

    template <typename T>
    bool is() const { return is_impl<typename normalize_type<T>::type>(); }

    template <typename T>
    bool is_impl() const { KIARA_UNREACHABLE("Unsupported type"); }

    template <typename T>
    Value & operator=(const T &other)
    {
        set(other);
        return *this;
    }

    template <typename Visitor>
    typename Visitor::result_type applyVisitor(Visitor &visitor)
    {
        return boost::apply_visitor(visitor, value_);
    }

    template <typename Visitor>
    typename Visitor::result_type applyVisitor(Visitor &visitor) const
    {
        return boost::apply_visitor(visitor, value_);
    }

    template <typename Visitor>
    typename Visitor::result_type applyVisitor(const Visitor &visitor)
    {
        return boost::apply_visitor(visitor, value_);
    }

    template <typename Visitor>
    typename Visitor::result_type applyVisitor(const Visitor &visitor) const
    {
        return boost::apply_visitor(visitor, value_);
    }

    std::string toString() const;

private:
    typedef boost::variant<
            NullValue,
            Number,
            bool,
            std::string,
            Object::Ptr,
            boost::any,
            boost::recursive_wrapper<DictValue>,
            boost::recursive_wrapper<ArrayValue> > ValueVariant;
    ValueVariant value_;
};

template <> inline int8_t Value::get_impl<int8_t>() const { return getNumber().get_i8(); }
template <> inline uint8_t Value::get_impl<uint8_t>() const { return getNumber().get_u8(); }
template <> inline int16_t Value::get_impl<int16_t>() const { return getNumber().get_i16(); }
template <> inline uint16_t Value::get_impl<uint16_t>() const { return getNumber().get_u16(); }
template <> inline int32_t Value::get_impl<int32_t>() const { return getNumber().get_i32(); }
template <> inline uint32_t Value::get_impl<uint32_t>() const { return getNumber().get_u32(); }
template <> inline int64_t Value::get_impl<int64_t>() const { return getNumber().get_i64(); }
template <> inline uint64_t Value::get_impl<uint64_t>() const { return getNumber().get_u64(); }
template <> inline float Value::get_impl<float>() const { return getNumber().get_float(); }
template <> inline double Value::get_impl<double>() const { return getNumber().get_double(); }
template <> inline bool Value::get_impl<bool>() const { return getBool(); }
template <> inline std::string Value::get_impl<std::string>() const { return getString(); }

template <> inline void Value::set_impl<int8_t>(const int8_t &v) { setNumber(Number(v)); }
template <> inline void Value::set_impl<uint8_t>(const uint8_t &v) { setNumber(Number(v)); }
template <> inline void Value::set_impl<int16_t>(const int16_t &v) { setNumber(Number(v)); }
template <> inline void Value::set_impl<uint16_t >(const uint16_t &v) { setNumber(Number(v)); }
template <> inline void Value::set_impl<int32_t>(const int32_t &v) { setNumber(Number(v)); }
template <> inline void Value::set_impl<uint32_t >(const uint32_t &v) { setNumber(Number(v)); }
template <> inline void Value::set_impl<int64_t>(const int64_t &v) { setNumber(Number(v)); }
template <> inline void Value::set_impl<uint64_t>(const uint64_t &v) { setNumber(Number(v)); }
template <> inline void Value::set_impl<float>(const float &v) { setNumber(Number(v)); }
template <> inline void Value::set_impl<double>(const double &v) { setNumber(Number(v)); }
template <> inline void Value::set_impl<bool>(const bool &v) { setBool(v); }
template <> inline void Value::set_impl<std::string>(const std::string &v) { setString(v); }
template <> inline void Value::set_impl<Object::Ptr>(const Object::Ptr &v) { setObject(v); }
template <> inline void Value::set_impl<Value>(const Value &v) { value_ = v.value_; }

template <> inline bool Value::is_impl<int8_t>() const { return isNumber() && getNumber().is_i8(); }
template <> inline bool Value::is_impl<uint8_t>() const { return isNumber() && getNumber().is_u8(); }
template <> inline bool Value::is_impl<int16_t>() const { return isNumber() && getNumber().is_i16(); }
template <> inline bool Value::is_impl<uint16_t>() const { return isNumber() && getNumber().is_u16(); }
template <> inline bool Value::is_impl<int32_t>() const { return isNumber() && getNumber().is_i32(); }
template <> inline bool Value::is_impl<uint32_t>() const { return isNumber() && getNumber().is_u32(); }
template <> inline bool Value::is_impl<int64_t>() const { return isNumber() && getNumber().is_i64(); }
template <> inline bool Value::is_impl<uint64_t>() const { return isNumber() && getNumber().is_u64(); }
template <> inline bool Value::is_impl<float>() const { return isNumber() && getNumber().is_float(); }
template <> inline bool Value::is_impl<double>() const { return isNumber() && getNumber().is_double(); }
template <> inline bool Value::is_impl<bool>() const { return isBool(); }
template <> inline bool Value::is_impl<std::string>() const { return isString(); }
template <> inline bool Value::is_impl<Object::Ptr>() const { return isObject(); }
template <> inline bool Value::is_impl<boost::any>() const { return isAny(); }
template <> inline bool Value::is_impl<DictValue>() const { return isDict(); }
template <> inline bool Value::is_impl<ArrayValue>() const { return isArray(); }

class ArrayValue : public std::vector<Value>
{
public:
    typedef std::vector<Value> InheritedType;

    // STL Container interface
    typedef InheritedType::allocator_type allocator_type;
    typedef InheritedType::value_type value_type;
    typedef InheritedType::pointer pointer;
    typedef InheritedType::reference reference;
    typedef InheritedType::const_reference const_reference;
    typedef InheritedType::size_type size_type;
    typedef InheritedType::difference_type difference_type;
    typedef InheritedType::iterator iterator;
    typedef InheritedType::const_iterator const_iterator;
    typedef InheritedType::reverse_iterator reverse_iterator;
    typedef InheritedType::const_reverse_iterator const_reverse_iterator;

    ArrayValue() : InheritedType() { }

    ArrayValue(size_type n) : InheritedType(n) { }

    ArrayValue(size_type n, const value_type& element) :
        InheritedType(n, element) { }

    ArrayValue(const InheritedType &container) : InheritedType(container) { }

    ArrayValue(const ArrayValue &container) :
        InheritedType(static_cast<const InheritedType&>(container)) { }

    template <class InputIterator>
    ArrayValue(InputIterator first, InputIterator last) :
        InheritedType(first, last) { }

};

class DictValue : public std::map<std::string, Value>
{
public:

    typedef std::map<std::string, Value> InheritedType;

    // STL Container interface
    typedef InheritedType::key_type key_type;
    typedef InheritedType::mapped_type mapped_type;
    typedef InheritedType::value_type value_type;
    typedef InheritedType::key_compare key_compare;
    typedef InheritedType::value_compare value_compare;
    typedef InheritedType::pointer pointer;
    typedef InheritedType::reference reference;
    typedef InheritedType::const_reference const_reference;
    typedef InheritedType::size_type size_type;
    typedef InheritedType::difference_type difference_type;
    typedef InheritedType::iterator iterator;
    typedef InheritedType::const_iterator const_iterator;
    typedef InheritedType::reverse_iterator reverse_iterator;
    typedef InheritedType::const_reverse_iterator const_reverse_iterator;

    DictValue() : InheritedType() { }

    DictValue(const key_compare &comp) : InheritedType(comp) { }

    template <class InputIterator>
    DictValue(InputIterator first, InputIterator last) :
        InheritedType(first, last) { }

    template <class InputIterator>
    DictValue(InputIterator first, InputIterator last,
             const key_compare &comp) :
        InheritedType(first, last, comp) { }

    DictValue(const InheritedType &container) : InheritedType(container) { }

    DictValue(const DictValue &container) :
        InheritedType(static_cast<const InheritedType&>(container)) { }

    bool hasKey(const std::string &key) const
    {
        return find(key) != end();
    }

};

inline DictValue & Value::getDict()
{
    return boost::get<DictValue>(value_);
}

inline const DictValue & Value::getDict() const
{
    return boost::get<DictValue>(value_);
}

inline DictValue & Value::getOrCreateDict()
{
    if (DictValue *dv = boost::get<DictValue>(&value_))
    {
        return *dv;
    }
    value_ = DictValue();
    return boost::get<DictValue>(value_);
}

inline void Value::setDict(const DictValue &value)
{
    value_ = value;
}

inline ArrayValue & Value::getArray()
{
    return boost::get<ArrayValue>(value_);
}

inline const ArrayValue & Value::getArray() const
{
    return boost::get<ArrayValue>(value_);
}

inline ArrayValue & Value::getOrCreateArray()
{
    if (ArrayValue *av = boost::get<ArrayValue>(&value_))
    {
        return *av;
    }
    value_ = ArrayValue();
    return boost::get<ArrayValue>(value_);
}

inline void Value::setArray(const ArrayValue &value)
{
    value_ = value;
}

class ValueOutputVisitor : public boost::static_visitor<>
{
public:

    ValueOutputVisitor(std::ostream &out) : out(out) { }

    void operator()(NullValue) const { out<<"null"; }

    void operator()(const Number &n) const { out<<n; }

    void operator()(const bool &b) const { out<<(b ? "true" : "false"); }

    void operator()(const std::string &s) const { out<<"\""<<s<<"\""; }

    void operator()(const Object::Ptr &objPtr) const
    {
        if (objPtr)
        {
            out<<"object<";
            objPtr->printRepr(out);
            out<<">";
        }
        else
            out<<"object<null>";
    }

    void operator()(const boost::any &anyVal) const
    {
        out<<"any<type:"<<anyVal.type().name()<<">";
    }

    void operator()(const DictValue &d) const
    {
        out<<"{";

        if (!d.empty())
        {
            DictValue::const_iterator last = d.end();
            --last;

            for (DictValue::const_iterator it = d.begin(), end = d.end();
                 it != end; ++it)
            {
                out<<it->first<<" : ";
                //boost::apply_visitor(*this, it->second);
                it->second.applyVisitor(*this);
                if (it != last)
                    out<<", ";
            }
        }
        out<<"}";
    }

    void operator()(const ArrayValue &v) const
    {
        out<<"[";
        for (ArrayValue::const_iterator it = v.begin();
             it != v.end(); ++it)
        {
            //boost::apply_visitor(*this, *it);
            it->applyVisitor(*this);
            if (it+1 != v.end())
                out<<", ";
        }
        out<<"]";
    }

private:
    std::ostream &out;
};

inline std::string Value::toString() const
{
    return boost::lexical_cast<std::string>(*this);
}

inline std::ostream &operator<<(std::ostream &o, const Value &value)
{
    //boost::apply_visitor(ValueOutputVisitor(o), value);
    value.applyVisitor(ValueOutputVisitor(o));
    return o;
}

inline std::ostream &operator<<(std::ostream &o, const DictValue &value)
{
    ValueOutputVisitor ov(o);
    ov(value);
    return o;
}

inline std::ostream &operator<<(std::ostream &o, const ArrayValue &value)
{
    ValueOutputVisitor ov(o);
    ov(value);
    return o;
}

inline std::ostream &operator<<(std::ostream &o,
                                const std::vector<Value> &value)
{
    ValueOutputVisitor ov(o);

    o<<"[";
    for (std::vector<Value>::const_iterator it = value.begin(), end = value.end();
         it != end; ++it)
    {
        //boost::apply_visitor(ov, *it);
        it->applyVisitor(ov);
        if (it+1 != value.end())
            o<<", ";
    }
    o<<"]";
    return o;
}

} // namespace KIARA

namespace boost
{

#define _KIARA_HAS_NOTHROW_COPY_(C)             \
    template <>                                 \
    struct has_nothrow_copy< C >                \
        : mpl::true_                            \
    {                                           \
    }

_KIARA_HAS_NOTHROW_COPY_(::KIARA::NullValue);
_KIARA_HAS_NOTHROW_COPY_(::KIARA::ArrayValue);
_KIARA_HAS_NOTHROW_COPY_(::KIARA::DictValue);

#undef SEI_HAS_NOTHROW_COPY_

} // namespace boost

#endif /* KIARA_UTILS_VALUE_HPP_INCLUDED */
