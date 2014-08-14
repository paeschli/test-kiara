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
 * DerivedTypes.hpp
 *
 *  Created on: 24.06.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_DB_DERIVEDTYPES_HPP_INCLUDED
#define KIARA_DB_DERIVEDTYPES_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <KIARA/kiara.h>
#include <KIARA/DB/Type.hpp>
#include <KIARA/DB/Expr.hpp>
#include <boost/bimap.hpp>

namespace KIARA
{

template <class T>
struct PrimTypeID { };

#define _KIARA_PRIMTYPE_ID(Type, ID)        \
template<>                                  \
struct PrimTypeID<Type>                     \
{                                           \
    BOOST_STATIC_CONSTANT(int, value = ID); \
};

_KIARA_PRIMTYPE_ID(int8_t, PRIMTYPE_i8)
_KIARA_PRIMTYPE_ID(uint8_t, PRIMTYPE_u8)
_KIARA_PRIMTYPE_ID(int16_t, PRIMTYPE_i16)
_KIARA_PRIMTYPE_ID(uint16_t, PRIMTYPE_u16)
_KIARA_PRIMTYPE_ID(int32_t, PRIMTYPE_i32)
_KIARA_PRIMTYPE_ID(uint32_t, PRIMTYPE_u32)
_KIARA_PRIMTYPE_ID(int64_t, PRIMTYPE_i64)
_KIARA_PRIMTYPE_ID(uint64_t, PRIMTYPE_u64)
_KIARA_PRIMTYPE_ID(float, PRIMTYPE_float)
_KIARA_PRIMTYPE_ID(double, PRIMTYPE_double)
_KIARA_PRIMTYPE_ID(bool, PRIMTYPE_boolean)
_KIARA_PRIMTYPE_ID(std::string, PRIMTYPE_string)
_KIARA_PRIMTYPE_ID(char *, PRIMTYPE_string)
_KIARA_PRIMTYPE_ID(const char *, PRIMTYPE_string)
_KIARA_PRIMTYPE_ID(void *, PRIMTYPE_c_nullptr)

#undef _KIARA_PRIMTYPE_ID

class KIARA_API PrimType : public Type
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(PrimType, Type)
    friend class World;
public:

    static const char * getNameOfPrimTypeKind(PrimTypeKind kind);

    static size_t getByteSizeOfPrimTypeKind(PrimTypeKind kind);

    static bool isIntegerPrimTypeKind(PrimTypeKind kind);

    static bool isSignedIntegerPrimTypeKind(PrimTypeKind kind);

    static bool isFloatingPointPrimTypeKind(PrimTypeKind kind);

    PrimTypeKind primtype_kind() const { return static_cast<PrimTypeKind>(kind()); }

    size_t getByteSize() const { return getByteSizeOfPrimTypeKind(primtype_kind()); }

    bool isInteger() const
    {
        return isIntegerPrimTypeKind(primtype_kind());
    }

    bool isSignedInteger() const
    {
        return isSignedIntegerPrimTypeKind(primtype_kind());
    }

    bool isFloatingPoint() const
    {
        return isFloatingPointPrimTypeKind(primtype_kind());
    }

    bool isCType() const
    {
        return primtype_kind() >= FIRST_C_PRIMTYPE;
    }

    static PrimType::Ptr getBooleanType(World &world);
    static PrimType::Ptr getStringType(World &world);

private:
    PrimType(World& world, PrimTypeKind kind);
};

class KIARA_API PrimValueType : public Type
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(PrimValueType, Type)
    friend class World;
public:

    static PrimValueType::Ptr get(World &world, int8_t v);
    static PrimValueType::Ptr get(World &world, uint8_t v);
    static PrimValueType::Ptr get(World &world, int16_t v);
    static PrimValueType::Ptr get(World &world, uint16_t v);
    static PrimValueType::Ptr get(World &world, int32_t v);
    static PrimValueType::Ptr get(World &world, uint32_t v);
    static PrimValueType::Ptr get(World &world, int64_t v);
    static PrimValueType::Ptr get(World &world, uint64_t v);
    static PrimValueType::Ptr get(World &world, float v);
    static PrimValueType::Ptr get(World &world, double v);
    static PrimValueType::Ptr get(World &world, bool v);
    static PrimValueType::Ptr get(World &world, const_char_ptr v);
    static PrimValueType::Ptr get(World &world, const std::string & v);

    const TypedBox &getValue() const { return value_; }

    virtual size_t hash() const;

    bool equals(const Object::Ptr &other) const;

private:
    PrimValueType(World& world, int8_t  v);
    PrimValueType(World& world, uint8_t  v);
    PrimValueType(World& world, int16_t  v);
    PrimValueType(World& world, uint16_t  v);
    PrimValueType(World& world, int32_t  v);
    PrimValueType(World& world, uint32_t  v);
    PrimValueType(World& world, int64_t  v);
    PrimValueType(World& world, uint64_t  v);
    PrimValueType(World& world, float   v);
    PrimValueType(World& world, double  v);
    PrimValueType(World& world, bool v);
    PrimValueType(World& world, const_char_ptr v);
    PrimValueType(World& world, const std::string & v);

    TypedBox value_;
};

/**
 *  There is always just a single VoidType instance.
 *  TODO: Should this be replaced by empty struct ?
 */
class KIARA_API VoidType : public Type
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(VoidType, Type)
    friend class World;
public:

    static VoidType::Ptr get(World &world);

private:
    VoidType(World &world);
};

/**
 *  There is always just a single TypeType instance.
 */
class KIARA_API TypeType : public Type
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(TypeType, Type)
    friend class World;
public:

    static TypeType::Ptr get(World &world);

private:
    TypeType(World &world);
};

class KIARA_API UnresolvedSymbolType : public Type
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(UnresolvedSymbolType, Type)
    friend class World;
public:

    static UnresolvedSymbolType::Ptr get(World &world);

private:
    UnresolvedSymbolType(World &world);
};

/**
 *  There is always just a single AnyType instance.
 */
class KIARA_API AnyType : public Type
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(AnyType, Type)
    friend class World;
public:

    static AnyType::Ptr get(World &world);

private:
    AnyType(World &world);
};

class KIARA_API TypedefType : public Type
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(TypedefType, Type)
    friend class World;
public:

    static TypedefType::Ptr create(const std::string &name, const Type::Ptr &typeDeclType);

    const Type::Ptr getDeclType() const { return getElementAs<Type>(0); }

    virtual void print(std::ostream &out, std::set<const Type*> &visited) const;

private:
    TypedefType(const std::string &name, const Type::Ptr &typeDeclType);
};

class KIARA_API EnumType : public Type
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(EnumType, Type)
    friend class World;
public:

    static const size_t npos = -1;

    static EnumType::Ptr create(World &world, const std::string &name);

    void addConstant(const std::string &name, const Expr::Ptr &expr);

    size_t getNumConstants() const
    {
        return getNumElements();
    }

    const Expr::Ptr getConstantAt(size_t index) const
    {
        return getElementAs<Expr>(index);
    }

    const std::string & getConstantNameAt(size_t index) const
    {
        return nameToIndexBimap_.right.find(index)->second;
    }

    size_t getConstantIndexByName(const std::string &name) const
    {
        NameToIndexBimap::left_const_iterator it = nameToIndexBimap_.left.find(name);
        if (it != nameToIndexBimap_.left.end())
            return it->second;
        else
            return npos;
    }

    virtual size_t hash() const;
    virtual void print(std::ostream &out, std::set<const Type*> &visited) const;

private:
    typedef boost::bimap<std::string, unsigned int> NameToIndexBimap;
    NameToIndexBimap nameToIndexBimap_;

    EnumType(World &world, const std::string &name);
};

class KIARA_API CompoundType : public Type
{
    DFC_DECLARE_ABSTRACT_TYPE(CompoundType, Type)
    friend class World;
public:

protected:
    CompoundType(World& world, const std::string &name, int kind, size_t num);
    CompoundType(World& world, const std::string &name, int kind, ArrayRef<Object::Ptr> elems);

    //void dumpInner(std::ostream & out, std::set<const Type*> &visited) const;
};

class ElementData : public AttributeHolder
{
public:

    ElementData()
        : name_()
    { }

    const std::string & getName() const { return name_; }
    void setName(const std::string &name) { name_ = name; }

private:
    std::string name_;
};

typedef std::vector<ElementData> ElementDataList;

/// CompositeType

class KIARA_API CompositeType : public CompoundType
{
    DFC_DECLARE_ABSTRACT_TYPE(CompositeType, CompoundType)
    friend class World;
public:

    bool isUnique() const { return unique_; }

    bool isOpaque() const { return unique_ && getNumElements() == 0; }

    void makeOpaque()
    {
        resizeElements(0);
    }

    void resizeElements(size_t newSize);

    void setElements(ArrayRef<Type::Ptr> elements);

    static const size_t npos = -1;

    /// returns npos if no element with specified name found.
    size_t getElementIndexByName(const std::string &name) const
    {
        for (size_t i = 0; i < getNumElements(); ++i)
        {
            if (getElementNameAt(i) == name)
                return i;
        }
        return npos;
    }

    const Type::Ptr getElementAt(size_t index) const
    {
        return getElementAs<Type>(index);
    }

    /** Throws exception when structure is not unique */
    void setElementAt(size_t index, const Type::Ptr &element);

    const ElementData & getElementDataAt(size_t index) const
    {
        BOOST_ASSERT(index < getNumElements());
        return elementDataList_[index];
    }

    ElementData & getElementDataAt(size_t index)
    {
        BOOST_ASSERT(index < getNumElements());
        return elementDataList_[index];
    }

    void setElementNames(ArrayRef<const char *> names)
    {
        BOOST_ASSERT(getNumElements() == names.size());

        elementDataList_.resize(names.size());
        for (size_t i = 0; i < names.size(); ++i)
        {
            elementDataList_[i].setName(names[i]);
        }
    }

    void setElementNames(const std::vector<std::string> &names)
    {
        BOOST_ASSERT(getNumElements() == names.size());

        elementDataList_.resize(names.size());
        for (size_t i = 0; i < names.size(); ++i)
        {
            elementDataList_[i].setName(names[i]);
        }
    }

    const std::vector<std::string> getElementNames() const
    {
        std::vector<std::string> names;
        for (ElementDataList::const_iterator it = elementDataList_.begin(),
                end = elementDataList_.end(); it != end; ++it)
        {
            names.push_back(it->getName());
        }
        return names;
    }

    const std::string & getElementNameAt(size_t index) const { return elementDataList_[index].getName(); }

    void setElementNameAt(size_t index, const std::string &name)
    {
        elementDataList_[index].setName(name);
    }

    virtual void print(std::ostream &out, std::set<const Type*> &visited) const;
    virtual size_t hash() const;
    virtual bool equals(const Object::Ptr &other) const;

protected:

    virtual void gcUnlinkRefs();

    virtual void gcApplyToChildren(const CollectorCallback &callback);

    CompositeType(World& world, const std::string &name, int kind, size_t num, bool unique)
        : CompoundType(world, name, kind, num)
        , unique_(unique)
        , elementDataList_(num)
    {}

    CompositeType(World& world, const std::string &name, int kind, ArrayRef<Object::Ptr> elems, bool unique)
        : CompoundType(world, name, kind, elems)
        , unique_(unique)
        , elementDataList_(elems.size())
    {
    }

    CompositeType(World& world, const std::string &name, int kind, ArrayRef<Object::Ptr> elems, const std::vector<std::string> &names, bool unique)
        : CompoundType(world, name, kind, elems)
        , unique_(unique)
        , elementDataList_(names.size())
    {
        setElementNames(names);
    }

    void setElementAtUnsafe(size_t index, const Type::Ptr &element)
    {
        BOOST_ASSERT(index < getNumElements());
        elements_[index] = element;
    }

    void dumpInner(std::ostream & out, std::set<const Type*> &visited) const;

private:

    bool unique_;
    std::vector<ElementData> elementDataList_;
};

// StructType

class KIARA_API StructType : public CompositeType
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(StructType, CompositeType)
    friend class World;
public:

    /// Create unique struct type
    static StructType::Ptr create(World &world, const std::string &name, size_t numElements);
    static StructType::Ptr create(World &world, const std::string &name);
    static StructType::Ptr get(World &world, const std::string &name, ArrayRef<Object::Ptr> elems);
    static StructType::Ptr get(World &world, const std::string &name, ArrayRef<Object::Ptr> elems, const std::vector<std::string> &names);

    virtual void print(std::ostream &out, std::set<const Type*> &visited) const;

protected:

    StructType(World& world, const std::string &name, ArrayRef<Object::Ptr> elems)
        : CompositeType(world, name, NODE_STRUCTTYPE, elems, /*unique =*/false)
    { }

    StructType(World& world, const std::string &name, ArrayRef<Object::Ptr> elems, const std::vector<std::string> &names)
        : CompositeType(world, name, NODE_STRUCTTYPE, elems, names, /*unique =*/false)
    { }

    StructType(World& world, const std::string &name, size_t num = 0)
        : CompositeType(world, name, NODE_STRUCTTYPE, num, /*unique=*/true)
    { }
};

// PtrType

class KIARA_API PtrType : public CompositeType
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(PtrType, CompositeType)
    friend class World;
public:

    static PtrType::Ptr get(const Type::Ptr &elementType);

    const Type::Ptr getElementType() const
    {
        return getElementAt(0);
    }

private:
    PtrType(const Type::Ptr &elementType);

    void init();

    virtual void print(std::ostream &out, std::set<const Type*> &visited) const;
};

class KIARA_API RefType : public CompositeType
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(RefType, CompositeType)
    friend class World;
public:

    static RefType::Ptr get(const Type::Ptr &elementType);

    const Type::Ptr getElementType() const
    {
        return getElementAt(0);
    }

private:
    RefType(const Type::Ptr &elementType);

    void init();

    virtual void print(std::ostream &out, std::set<const Type*> &visited) const;
};

class KIARA_API ArrayType : public CompositeType
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(ArrayType, CompositeType)
    friend class World;
public:

    static ArrayType::Ptr get(const Type::Ptr &elementType);

    const Type::Ptr getElementType() const
    {
        return getElementAt(0);
    }

private:
    ArrayType(World& world, const Type::Ptr &elementType);
    void init();
    virtual void print(std::ostream &out, std::set<const Type*> &visited) const;
};

class KIARA_API FixedArrayType : public CompositeType
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(FixedArrayType, CompositeType)
    friend class World;
public:

    static FixedArrayType::Ptr get(const Type::Ptr &elementType, int64_t numElements);

    const Type::Ptr getElementType() const
    {
        return getElementAt(0);
    }

    int64_t getArraySize() const
    {
        return getElementAs<PrimValueType>(1)->getValue().get<int64_t>();
    }

private:
    FixedArrayType(World& world, const Type::Ptr &elementType, int64_t arraySize);
    void init();
    virtual void print(std::ostream &out, std::set<const Type*> &visited) const;
};

class KIARA_API SymbolType : public CompositeType
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(SymbolType, CompositeType)
    friend class World;
public:

    static SymbolType::Ptr get(World &world, const std::string &symbol);

    const std::string getSymbol() const
    {
        return getElementAs<PrimValueType>(0)->getValue().get<std::string>();
    }

private:
    SymbolType(World &world, const std::string &symbol);
};

class KIARA_API FunctionType : public CompositeType
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(FunctionType, CompositeType)
    friend class World;
public:

    typedef std::vector<std::pair<std::string, Type::Ptr> > ParamTypes;

    // Create unique function type
    static FunctionType::Ptr create(const std::string &name, const Type::Ptr &returnType, const ParamTypes &paramTypes);

    // Get unnamed function type (non-unique)
    static FunctionType::Ptr get(const Type::Ptr &returnType, ArrayRef<Type::Ptr> paramTypes);

    // Get unnamed function type (non-unique)
    static FunctionType::Ptr get(const Type::Ptr &returnType, const ParamTypes &paramTypes);

    Type::Ptr getReturnType() const { return getElementAs<Type>(0); }

    const ElementData & getReturnElementData() const
    {
        return getElementDataAt(0);
    }

    ElementData & getReturnElementData()
    {
        return getElementDataAt(0);
    }

    size_t getNumParams() const { return getNumElements()-1; }

    Type::Ptr getParamType(size_t index) const
    {
        return getElementAt(index+1);
    }

    const std::string & getParamName(size_t index) const
    {
        return getElementDataAt(index+1).getName();
    }

    const ElementData & getParamElementDataAt(size_t index) const
    {
        return getElementDataAt(index+1);
    }

    ElementData & getParamElementDataAt(size_t index)
    {
        return getElementDataAt(index+1);
    }

private:
    FunctionType(World& world, const std::string &name,
                 const Type::Ptr &returnType,
                 ArrayRef<Type::Ptr> paramTypes,
                 bool unique);

    FunctionType(World &world, const std::string &name,
                 const Type::Ptr &returnType,
                 const ParamTypes &paramTypes,
                 bool unique);

    void init();
    virtual void print(std::ostream &out, std::set<const Type*> &visited) const;
};

class KIARA_API ServiceType : public CompositeType
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(ServiceType, CompositeType)
    friend class World;
public:

    static ServiceType::Ptr create(World &world, const std::string &name, size_t numElements);

private:

    ServiceType(World& world, const std::string &name, size_t num)
        : CompositeType(world, name, NODE_SERVICETYPE, num, true)
    { }

    virtual void print(std::ostream &out, std::set<const Type*> &visited) const;
};

} // namespace KIARA

#endif /* KIARA_DB_DERIVEDTYPES_HPP_INCLUDED */
