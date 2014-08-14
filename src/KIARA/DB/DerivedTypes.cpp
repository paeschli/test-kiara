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
 * DerivedTypes.cpp
 *
 *  Created on: 24.06.2013
 *      Author: Dmitri Rubinstein
 */

#define KIARA_LIB
#include "DerivedTypes.hpp"
#include "World.hpp"
#include <KIARA/Core/Exception.hpp>
#include <KIARA/DB/Attributes.hpp>
#include <KIARA/Utils/IndentingStreambuf.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>


namespace KIARA
{

/// PrimType

DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::PrimType)

const char * PrimType::getNameOfPrimTypeKind(PrimTypeKind kind)
{
    switch (kind)
    {
        case PRIMTYPE_i8:      return "i8";
        case PRIMTYPE_u8:      return "u8";
        case PRIMTYPE_i16:     return "i16";
        case PRIMTYPE_u16:     return "u16";
        case PRIMTYPE_i32:     return "i32";
        case PRIMTYPE_u32:     return "u32";
        case PRIMTYPE_i64:     return "i64";
        case PRIMTYPE_u64:     return "u64";
        case PRIMTYPE_float:   return "float";
        case PRIMTYPE_double:  return "double";
        case PRIMTYPE_boolean: return "boolean";
        case PRIMTYPE_string:  return "string";
        case PRIMTYPE_c_int8_t:   return "c_int8_t";
        case PRIMTYPE_c_uint8_t:  return "c_uint8_t";
        case PRIMTYPE_c_int16_t:  return "c_int16_t";
        case PRIMTYPE_c_uint16_t: return "c_uint16_t";
        case PRIMTYPE_c_int32_t:  return "c_int32_t";
        case PRIMTYPE_c_uint32_t: return "c_uint32_t";
        case PRIMTYPE_c_int64_t:  return "c_int64_t";
        case PRIMTYPE_c_uint64_t: return "c_uint64_t";
        case PRIMTYPE_c_float:    return "c_float";
        case PRIMTYPE_c_double:   return "c_double";
        case PRIMTYPE_c_longdouble:   return "c_longdouble";
        case PRIMTYPE_c_bool: return "c_bool";
        case PRIMTYPE_c_nullptr: return "c_nullptr";
        default:
            KIARA_UNREACHABLE("Unknown primitive type ID");
            break;
    }
}

size_t PrimType::getByteSizeOfPrimTypeKind(PrimTypeKind kind)
{
    switch (kind)
    {
        // For abstract types function should report size of the internal representation
        case PRIMTYPE_i8:      return sizeof(int8_t);
        case PRIMTYPE_u8:      return sizeof(uint8_t);
        case PRIMTYPE_i16:     return sizeof(int16_t);
        case PRIMTYPE_u16:     return sizeof(uint16_t);
        case PRIMTYPE_i32:     return sizeof(int32_t);
        case PRIMTYPE_u32:     return sizeof(uint32_t);
        case PRIMTYPE_i64:     return sizeof(int64_t);
        case PRIMTYPE_u64:     return sizeof(uint64_t);
        case PRIMTYPE_float:   return sizeof(float);
        case PRIMTYPE_double:  return sizeof(double);
        case PRIMTYPE_boolean: return sizeof(bool); //??? Is this correct
        case PRIMTYPE_string:  return sizeof(char*);
        // For native types function should report size measured with sizeof
        case PRIMTYPE_c_int8_t:   return sizeof(int8_t);
        case PRIMTYPE_c_uint8_t:  return sizeof(uint8_t);
        case PRIMTYPE_c_int16_t:  return sizeof(int16_t);
        case PRIMTYPE_c_uint16_t: return sizeof(uint16_t);
        case PRIMTYPE_c_int32_t:  return sizeof(int32_t);
        case PRIMTYPE_c_uint32_t: return sizeof(uint32_t);
        case PRIMTYPE_c_int64_t:  return sizeof(int64_t);
        case PRIMTYPE_c_uint64_t: return sizeof(uint64_t);
        case PRIMTYPE_c_float:    return sizeof(float);
        case PRIMTYPE_c_double:   return sizeof(double);
        case PRIMTYPE_c_longdouble:   return sizeof(long double);
        case PRIMTYPE_c_bool: return sizeof(bool);
        default:
            KIARA_UNREACHABLE("Unknown primitive type ID");
            break;
    }
}

bool PrimType::isIntegerPrimTypeKind(PrimTypeKind kind)
{
    return (kind == PRIMTYPE_i8 ||
            kind == PRIMTYPE_u8 ||
            kind == PRIMTYPE_i16 ||
            kind == PRIMTYPE_u16 ||
            kind == PRIMTYPE_i32 ||
            kind == PRIMTYPE_u32 ||
            kind == PRIMTYPE_i64 ||
            kind == PRIMTYPE_u64 ||
            kind == PRIMTYPE_c_int8_t ||
            kind == PRIMTYPE_c_uint8_t ||
            kind == PRIMTYPE_c_int16_t ||
            kind == PRIMTYPE_c_uint16_t ||
            kind == PRIMTYPE_c_int32_t ||
            kind == PRIMTYPE_c_uint32_t ||
            kind == PRIMTYPE_c_int64_t ||
            kind == PRIMTYPE_c_uint64_t);
}

bool PrimType::isSignedIntegerPrimTypeKind(PrimTypeKind kind)
{
    return (kind == PRIMTYPE_i8 ||
            kind == PRIMTYPE_i16 ||
            kind == PRIMTYPE_i32 ||
            kind == PRIMTYPE_i64 ||
            kind == PRIMTYPE_c_int8_t ||
            kind == PRIMTYPE_c_int16_t ||
            kind == PRIMTYPE_c_int32_t ||
            kind == PRIMTYPE_c_int64_t);
}

bool PrimType::isFloatingPointPrimTypeKind(PrimTypeKind kind)
{
    return (kind == PRIMTYPE_float ||
            kind == PRIMTYPE_double ||
            kind == PRIMTYPE_c_float ||
            kind == PRIMTYPE_c_double ||
            kind == PRIMTYPE_c_longdouble);
}

PrimType::PrimType(World& world, PrimTypeKind kind)
    : Type(world, getNameOfPrimTypeKind(kind), kind, 0)
{
}

PrimType::Ptr PrimType::getBooleanType(World &world)
{
    return world.type_boolean();
}

PrimType::Ptr PrimType::getStringType(World &world)
{
    return world.type_string();
}

/// PrimValueType

DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::PrimValueType)

PrimValueType::Ptr PrimValueType::get(World &world, int8_t v) { return world.getOrCreate<PrimValueType>(new PrimValueType(world, v)); }
PrimValueType::Ptr PrimValueType::get(World &world, uint8_t v) { return world.getOrCreate<PrimValueType>(new PrimValueType(world, v)); }
PrimValueType::Ptr PrimValueType::get(World &world, int16_t v) { return world.getOrCreate<PrimValueType>(new PrimValueType(world, v)); }
PrimValueType::Ptr PrimValueType::get(World &world, uint16_t v) { return world.getOrCreate<PrimValueType>(new PrimValueType(world, v)); }
PrimValueType::Ptr PrimValueType::get(World &world, int32_t v) { return world.getOrCreate<PrimValueType>(new PrimValueType(world, v)); }
PrimValueType::Ptr PrimValueType::get(World &world, uint32_t v) { return world.getOrCreate<PrimValueType>(new PrimValueType(world, v)); }
PrimValueType::Ptr PrimValueType::get(World &world, int64_t v) { return world.getOrCreate<PrimValueType>(new PrimValueType(world, v)); }
PrimValueType::Ptr PrimValueType::get(World &world, uint64_t v) { return world.getOrCreate<PrimValueType>(new PrimValueType(world, v)); }
PrimValueType::Ptr PrimValueType::get(World &world, float v) { return world.getOrCreate<PrimValueType>(new PrimValueType(world, v)); }
PrimValueType::Ptr PrimValueType::get(World &world, double v) { return world.getOrCreate<PrimValueType>(new PrimValueType(world, v)); }
PrimValueType::Ptr PrimValueType::get(World &world, bool v) { return world.getOrCreate<PrimValueType>(new PrimValueType(world, v)); }
PrimValueType::Ptr PrimValueType::get(World &world, const_char_ptr v) { return world.getOrCreate<PrimValueType>(new PrimValueType(world, v)); }
PrimValueType::Ptr PrimValueType::get(World &world, const std::string & v) { return world.getOrCreate<PrimValueType>(new PrimValueType(world, v)); }

PrimValueType::PrimValueType(World& world, int8_t  v)
    : Type(world, "value_type_i8("+boost::lexical_cast<std::string>(static_cast<int>(v))+")",
           NODE_PRIMVALUETYPE, 0)
    , value_(v)
{ }

PrimValueType::PrimValueType(World& world, uint8_t  v)
    : Type(world, "value_type_u8("+boost::lexical_cast<std::string>(static_cast<int>(v))+")",
           NODE_PRIMVALUETYPE, 0)
    , value_(v)
{ }

PrimValueType::PrimValueType(World& world, int16_t  v)
    : Type(world, "value_type_i16("+boost::lexical_cast<std::string>(v)+")",
           NODE_PRIMVALUETYPE, 0)
    , value_(v)
{ }

PrimValueType::PrimValueType(World& world, uint16_t  v)
    : Type(world, "value_type_u16("+boost::lexical_cast<std::string>(v)+")",
           NODE_PRIMVALUETYPE, 0)
    , value_(v)
{ }

PrimValueType::PrimValueType(World& world, int32_t  v)
    : Type(world, "value_type_i32("+boost::lexical_cast<std::string>(v)+")",
           NODE_PRIMVALUETYPE, 0)
    , value_(v)
{ }

PrimValueType::PrimValueType(World& world, uint32_t  v)
    : Type(world, "value_type_u32("+boost::lexical_cast<std::string>(v)+")",
           NODE_PRIMVALUETYPE, 0)
    , value_(v)
{ }

PrimValueType::PrimValueType(World& world, int64_t  v)
    : Type(world, "value_type_i64("+boost::lexical_cast<std::string>(v)+")",
           NODE_PRIMVALUETYPE, 0)
    , value_(v)
{ }

PrimValueType::PrimValueType(World& world, uint64_t  v)
    : Type(world, "value_type_u64("+boost::lexical_cast<std::string>(v)+")",
           NODE_PRIMVALUETYPE, 0)
    , value_(v)
{ }

PrimValueType::PrimValueType(World& world, float   v)
    : Type(world, "value_type_float("+boost::lexical_cast<std::string>(v)+")",
           NODE_PRIMVALUETYPE, 0)
    , value_(v)
{ }

PrimValueType::PrimValueType(World& world, double  v)
    : Type(world, "value_type_double("+boost::lexical_cast<std::string>(v)+")",
           NODE_PRIMVALUETYPE, 0)
    , value_(v)
{ }

PrimValueType::PrimValueType(World& world, bool v)
    : Type(world, "value_type_bool("+std::string(v ? "true" : "false")+")",
           NODE_PRIMVALUETYPE, 0)
    , value_(v)
{ }

PrimValueType::PrimValueType(World& world, const_char_ptr v)
    : Type(world, "value_type_string('"+std::string(v ? v : "")+"')",
           NODE_PRIMVALUETYPE, 0)
    , value_(v)
{ }

PrimValueType::PrimValueType(World& world, const std::string & v)
    : Type(world, "value_type_string('"+v+"')",
           NODE_PRIMVALUETYPE, 0)
    , value_(v)
{ }

bool PrimValueType::equals(const Object::Ptr &other) const
{
    PrimValueType::Ptr otherTy = dyn_cast<PrimValueType>(other);
    if (!otherTy)
        return false;
    if (kind() != otherTy->kind())
        return false;
    if (value_ != otherTy->getValue())
        return false;
    return true;
}

size_t PrimValueType::hash() const
{
    size_t seed = 0;
    boost::hash_combine(seed, kind());
    boost::hash_combine(seed, value_);
    return seed;
}

/// VoidType

DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::VoidType)

VoidType::VoidType(World& world)
    : Type(world, "void", NODE_VOIDTYPE, 0)
{
}

VoidType::Ptr VoidType::get(World &world)
{
    return world.type_void();
}

/// TypeType

DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::TypeType)

TypeType::TypeType(World& world)
    : Type(world, "type", NODE_TYPETYPE, 0)
{
}

TypeType::Ptr TypeType::get(World &world)
{
    return world.type_type();
}

/// UnresolvedSymbolType

DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::UnresolvedSymbolType)

UnresolvedSymbolType::UnresolvedSymbolType(World& world)
    : Type(world, "unresolved_symbol", NODE_UNRESOLVEDSYMBOLTYPE, 0)
{
}

UnresolvedSymbolType::Ptr UnresolvedSymbolType::get(World &world)
{
    return world.type_unresolved_symbol();
}

/// AnyType

DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::AnyType)

AnyType::AnyType(World& world)
    : Type(world, "any", NODE_ANYTYPE, 0)
{
}

AnyType::Ptr AnyType::get(World &world)
{
    return world.type_any();
}

/// TypedefType

DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::TypedefType)

TypedefType::TypedefType(const std::string &name, const Type::Ptr &typeDeclType)
    : Type(typeDeclType->getWorld(), name, NODE_TYPEDEFTYPE, ArrayRef<Object::Ptr>(typeDeclType),
           typeDeclType->getCanonicalType())
{
}

TypedefType::Ptr TypedefType::create(const std::string &name, const Type::Ptr &typeDeclType)
{
    return typeDeclType->getWorld().getOrCreate<TypedefType>(new TypedefType(name, typeDeclType));
}

void TypedefType::print(std::ostream &out, std::set<const Type*> &visited) const
{
    if (hasAttributes())
    {
        AttributeHolder::print(out);
        out<<" ";
    }
    out << getFullTypeName();
}

/// EnumType

DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::EnumType)

EnumType::EnumType(World &world, const std::string &name)
    : Type(world, name, NODE_ENUMTYPE, 0)
{
}

void EnumType::addConstant(const std::string &name, const Expr::Ptr &expr)
{
    // FIXME: add check that name is not already used
    size_t index = getNumElements();
    resizeElements(index+1);
    elements_[index] = expr;
    nameToIndexBimap_.insert(NameToIndexBimap::value_type(name, index));
}

EnumType::Ptr EnumType::create(World &world, const std::string &name)
{
    return world.getOrCreate<EnumType>(new EnumType(world, name));
}

size_t EnumType::hash() const
{
    return boost::hash_value(this);
}

void EnumType::print(std::ostream &out, std::set<const Type*> &visited) const
{
    out << "enum "<<getTypeName();
    if (hasAttributes())
    {
        out << " ";
        AttributeHolder::print(out);
    }
    out << " {\n";
    {
        IndentingStreambuf isb(out, 1);
        const size_t numElements = getNumElements();
        for (size_t i = 0; i < numElements; ++i)
        {
            Object::Ptr elem = getElementAs<Object>(i);

            out << "  " << getConstantNameAt(i)  << " = ";
            if (elem)
                elem->print(out);
            else
                out << "NULL";

            if (i != numElements-1)
                out << ",\n";
        }
        if (numElements > 0)
            out << "\n";
    }
    out << "}";
}

/// CompoundType

DFC_DEFINE_ABSTRACT_TYPE(KIARA::CompoundType)

CompoundType::CompoundType(World& world, const std::string &name, int kind, size_t num)
    : Type(world, name, kind, num)
{}

CompoundType::CompoundType(World& world, const std::string &name, int kind, ArrayRef<Object::Ptr> elems)
    : Type(world, name, kind, elems)
{
}

#if 0
#define DUMP_COMMA_LIST(p, list)                                                            \
    const BOOST_TYPEOF((list))& l = (list);                                                 \
    if (!l.empty()) {                                                                       \
        for (boost::remove_reference<BOOST_TYPEOF(l)>::type::const_iterator i = l.begin(),  \
                e = l.end() - 1; i != e; ++i) {                                             \
            (*i)->print(p, visited);                                                                 \
            (p) << ", ";                                                                    \
        }                                                                                   \
        l.back()->print(p);                                                                 \
    }

void CompoundType::dumpInner(std::ostream & p, std::set<const Type*> &visited) const
{
    p << "(";
    DUMP_COMMA_LIST(p, getElements());
    p << ")";
}
#endif

/// CompositeType

DFC_DEFINE_ABSTRACT_TYPE(KIARA::CompositeType)

void CompositeType::resizeElements(size_t newSize)
{
    if (!isUnique())
        DFC_THROW_EXCEPTION(Exception, "Structure is not unique");
    Type::resizeElements(newSize);
    elementDataList_.resize(newSize);
}

void CompositeType::setElements(ArrayRef<Type::Ptr> elems)
{
    BOOST_ASSERT(elems.size() == getNumElements());
    std::copy(elems.begin(), elems.end(), elements_.begin());
}

void CompositeType::setElementAt(size_t index, const Type::Ptr &element)
{
    BOOST_ASSERT(index < getNumElements());
    if (!isUnique())
        DFC_THROW_EXCEPTION(Exception, "Structure is not unique");
    elements_[index] = element;
}

void CompositeType::gcUnlinkRefs()
{
    InheritedType::gcUnlinkRefs();
    typedef std::vector<ElementData>::iterator Iter;
//    for (Iter it = elementDataList_.begin(), end = elementDataList_.end(); it != end; ++it)
//    {
//        gcUnlinkChildren(it->attr_begin(), it->attr_end());
//    }
}

void CompositeType::gcApplyToChildren(const CollectorCallback &callback)
{
    InheritedType::gcApplyToChildren(callback);
    typedef std::vector<ElementData>::iterator Iter;
//    for (Iter it = elementDataList_.begin(), end = elementDataList_.end(); it != end; ++it)
//    {
//        gcApply(it->attr_begin(), it->attr_end(), callback);
//    }
}

void CompositeType::dumpInner(std::ostream & p, std::set<const Type*> &visited) const
{
    if (visited.find(this) != visited.end())
    {
        return;
    }
    visited.insert(this);

    if (hasAttributes())
    {
        p << " ";
        AttributeHolder::print(p);
    }
    if (!isOpaque())
    {
        p << " {\n";
        {
            IndentingStreambuf isb(p, 1);
            const size_t numElements = getNumElements();
            for (size_t i = 0; i < numElements; ++i)
            {
                Object::Ptr elem = getElementAs<Object>(i);
                const ElementData &edata = getElementDataAt(i);

                if (!edata.getName().empty())
                    p << edata.getName() << " : ";
                if (edata.hasAttributes())
                {
                    p << "[";
                    edata.print(p);
                    p << "] ";
                }
                if (elem)
                {
                    if (Type::Ptr ty = dyn_cast<Type>(elem))
                        ty->print(p, visited);
                    else
                        elem->print(p);
                }
                else
                    p << "NULL";

                if (i != numElements-1)
                    p << ",\n";
            }
            if (numElements > 0)
                p << "\n";
        }
        p << "}";
    }
}

void CompositeType::print(std::ostream & out, std::set<const Type*> &visited) const
{
    out << "composite "<<getTypeName();
    dumpInner(out, visited);
}

size_t CompositeType::hash() const
{
    if (unique_)
        return boost::hash_value(this);
    else
        return CompoundType::hash();
}

bool CompositeType::equals(const Object::Ptr &other) const
{
    if (unique_)
        return this == other;
    else
        return CompoundType::equals(other);
}

/// StructType

DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::StructType)

StructType::Ptr StructType::create(World &world, const std::string &name, size_t numElements)
{
    return world.getOrCreate<StructType>(new StructType(world, name, numElements));
}

StructType::Ptr StructType::create(World &world, const std::string &name)
{
    return world.getOrCreate<StructType>(new StructType(world, name));
}

StructType::Ptr StructType::get(World &world, const std::string &name, ArrayRef<Object::Ptr> elems)
{
    return world.getOrCreate<StructType>(new StructType(world, name, elems));
}

StructType::Ptr StructType::get(World &world, const std::string &name, ArrayRef<Object::Ptr> elems, const std::vector<std::string> &names)
{
    return world.getOrCreate<StructType>(new StructType(world, name, elems, names));
}

void StructType::print(std::ostream & out, std::set<const Type*> &visited) const
{
    out << "struct "<<getTypeName();
    dumpInner(out, visited);
}

// PtrType

DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::PtrType)

PtrType::Ptr PtrType::get(const Type::Ptr &elementType)
{
    PtrType::Ptr pt(new PtrType(elementType));
    pt->init();
    return elementType->getWorld().getOrCreate<PtrType>(pt);
}

PtrType::PtrType(const Type::Ptr &elementType)
    : CompositeType(elementType->getWorld(), "", NODE_PTRTYPE, 1, false)
{
    BOOST_ASSERT(elementType.get() != 0);
    Type::Ptr elements[] = { elementType };
    //const char * names[] = {"Element"};
    setElements(elements);
    //setElementNames(names);

}

void PtrType::init()
{
    if (!getElementType()->isCanonicalType())
        setCanonicalTypeUnsafe(PtrType::get(getElementType()->getCanonicalType()));
}

void PtrType::print(std::ostream & p, std::set<const Type*> &visited) const
{
    p << "pointer";
    if (hasAttributes())
    {
        p << " ";
        AttributeHolder::print(p);
    }
    p << "(";
    if (getElementType())
        getElementType()->print(p, visited);
    p << ")";
    //dumpInner(p);
}

// RefType

DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::RefType)

RefType::Ptr RefType::get(const Type::Ptr &elementType)
{
    RefType::Ptr rt(new RefType(elementType));
    rt->init();
    return elementType->getWorld().getOrCreate<RefType>(rt);
}

RefType::RefType(const Type::Ptr &elementType)
    : CompositeType(elementType->getWorld(), "", NODE_REFTYPE, 1, false)
{
    BOOST_ASSERT(elementType.get() != 0);
    Type::Ptr elements[1] = { elementType };
    setElements(elements);
}

void RefType::init()
{
    if (!getElementType()->isCanonicalType())
        setCanonicalTypeUnsafe(RefType::get(getElementType()->getCanonicalType()));
}

void RefType::print(std::ostream & p, std::set<const Type*> &visited) const
{
    p << "reference";
    if (hasAttributes())
    {
        p << " ";
        AttributeHolder::print(p);
    }
    p << "(";
    if (getElementType())
        getElementType()->print(p, visited);
    p << ")";
    //dumpInner(p);
}

// ArrayType

DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::ArrayType)

ArrayType::Ptr ArrayType::get(const Type::Ptr &elementType)
{
    World &world = elementType->getWorld();
    ArrayType::Ptr ty(new ArrayType(world, elementType));
    ty->init();
    return world.getOrCreate<ArrayType>(ty);
}

ArrayType::ArrayType(World& world, const Type::Ptr &elementType)
    : CompositeType(world, "array", NODE_ARRAYTYPE, 1, false)
{
    BOOST_ASSERT(elementType.get() != 0);
    Type::Ptr elements[] = { elementType };
    //const char * names[] = {"Element"};
    setElements(elements);
    //setElementNames(names);
}

void ArrayType::init()
{
    if (!getElementType()->isCanonicalType())
        setCanonicalTypeUnsafe(PtrType::get(getElementType()->getCanonicalType()));
}

void ArrayType::print(std::ostream &out, std::set<const Type*> &visited) const
{
    out << "array";
    if (hasAttributes())
    {
        out << " ";
        AttributeHolder::print(out);
    }
    out << "(";
    if (getElementType())
        getElementType()->print(out, visited);
    out << ")";
    //dumpInner(out);
}

// FixedArrayType

DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::FixedArrayType)

FixedArrayType::Ptr FixedArrayType::get(const Type::Ptr &elementType, int64_t numElements)
{
    World &world = elementType->getWorld();
    FixedArrayType::Ptr ty(new FixedArrayType(world, elementType, numElements));
    ty->init();
    return world.getOrCreate<FixedArrayType>(ty);
}

FixedArrayType::FixedArrayType(World& world, const Type::Ptr &elementType, int64_t numElements)
    : CompositeType(world, "fixedArray", NODE_FIXEDARRAYTYPE, 2, false)
{
    BOOST_ASSERT(elementType.get() != 0);
    Type::Ptr elements[] = { elementType, PrimValueType::get(world, numElements) };
    const char * names[] = {"Element", "Size"};
    setElements(elements);
    setElementNames(names);
}

void FixedArrayType::init()
{
    if (!getElementType()->isCanonicalType())
        setCanonicalTypeUnsafe(FixedArrayType::get(getElementType()->getCanonicalType(), getNumElements()));
}

void FixedArrayType::print(std::ostream &out, std::set<const Type*> &visited) const
{
    out << "fixedArray";
    if (hasAttributes())
    {
        out << " ";
        AttributeHolder::print(out);
    }
    out << "(";
    if (getElementType())
        getElementType()->print(out, visited);
    out<<" , "<<getArraySize()<< ")";
    //dumpInner(out);
}

/// SymbolType

DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::SymbolType)

SymbolType::SymbolType(World& world, const std::string &symbol)
    : CompositeType(world, "symbol", NODE_SYMBOLTYPE, 1, false)
{
    Type::Ptr elements[] = { PrimValueType::get(world, symbol) };
    const char * names[] = {"Symbol"};
    setElements(elements);
    setElementNames(names);
}

SymbolType::Ptr SymbolType::get(World &world, const std::string &symbol)
{
    return world.getOrCreate<SymbolType>(new SymbolType(world, symbol));
}

// FunctionType

DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::FunctionType)

FunctionType::FunctionType(World& world, const std::string &name,
             const Type::Ptr &returnType,
             ArrayRef<Type::Ptr> paramTypes,
             bool unique)
    : CompositeType(world, name, NODE_FUNCTYPE, paramTypes.size()+1, unique)
{
    BOOST_ASSERT(returnType.get() != 0);

    setElementAtUnsafe(0, returnType);

    for (size_t i = 0; i < paramTypes.size(); ++i)
    {
        setElementAtUnsafe(i+1, paramTypes[i]);
    }
}

FunctionType::FunctionType(World &world, const std::string &name,
             const Type::Ptr &returnType,
             const ParamTypes &paramTypes,
             bool unique)
    : CompositeType(world, name, NODE_FUNCTYPE, paramTypes.size()+1, unique)
{
    setElementAtUnsafe(0, returnType);

    for (size_t i = 0; i < paramTypes.size(); ++i)
    {
        setElementAtUnsafe(i+1, paramTypes[i].second);
        setElementNameAt(i+1, paramTypes[i].first);
    }
}

void FunctionType::init()
{
    //if (!getElementType()->isCanonicalType())
    //    setCanonicalTypeUnsafe(PtrType::get(getElementType()->getCanonicalType()));
    bool isCanonical = true;
    const size_t numElements = getNumElements();
    for (size_t i = 0; i < numElements; ++i)
    {
        if (!getElementAt(i)->isCanonicalType())
        {
            isCanonical = false;
            break;
        }
    }

    if (isCanonical)
        return;

    const size_t numParams = getNumParams();
    ParamTypes paramTypes(numParams);
    for (size_t i = 0; i < numParams; ++i)
    {
        paramTypes[i] = std::make_pair(getParamName(i), getParamType(i)->getCanonicalType());
    }

    FunctionType::Ptr cty(isUnique() ?
        FunctionType::create(getTypeName()+"_CANONICAL", getReturnType()->getCanonicalType(), paramTypes) :
        FunctionType::get(getReturnType()->getCanonicalType(), paramTypes));

    setCanonicalTypeUnsafe(cty);
}

// Create unique function type
FunctionType::Ptr FunctionType::create(const std::string &name, const Type::Ptr &returnType, const FunctionType::ParamTypes &paramTypes)
{
    World &world = returnType->getWorld();
    FunctionType::Ptr ty(new FunctionType(world, name, returnType, paramTypes, true));
    ty->init();
    return world.getOrCreate<FunctionType>(ty);
}

// Get unnamed function type (non-unique)
FunctionType::Ptr FunctionType::get(const Type::Ptr &returnType, ArrayRef<Type::Ptr> paramTypes)
{
    World &world = returnType->getWorld();
    FunctionType::Ptr ty(new FunctionType(world, "", returnType, paramTypes, false));
    ty->init();
    return world.getOrCreate<FunctionType>(ty);
}

// Get unnamed function type (non-unique)
FunctionType::Ptr FunctionType::get(const Type::Ptr &returnType, const FunctionType::ParamTypes &paramTypes)
{
    World &world = returnType->getWorld();
    FunctionType::Ptr ty(new FunctionType(world, "", returnType, paramTypes, false));
    ty->init();
    return world.getOrCreate<FunctionType>(ty);
}

void FunctionType::print(std::ostream & p, std::set<const Type*> &visited) const
{
    p << "function "<<getTypeName();
    dumpInner(p, visited);
}

// ServiceType

DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::ServiceType)

ServiceType::Ptr ServiceType::create(World &world, const std::string &name, size_t numElements)
{
    return world.getOrCreate<ServiceType>(new ServiceType(world, name, numElements));
}

void ServiceType::print(std::ostream & p, std::set<const Type*> &visited) const
{
    p << "service "<<getTypeName()<<" ";
    dumpInner(p, visited);
}

} // namespace KIARA
