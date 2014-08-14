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
 * IDLWriter.cpp
 *
 *  Created on: Jul 19, 2013
 *      Author: Dmitri Rubinstein
 */
#define KIARA_LIB
#include "IDLWriter.hpp"
#include <KIARA/DB/Attributes.hpp>
#include <KIARA/DB/Expr.hpp>
#include <KIARA/Utils/IndentingStreambuf.hpp>
#include <boost/lexical_cast.hpp>

namespace KIARA
{

IDLWriter::IDLWriter(const Module::Ptr &module)
    : module_(module)
{ }

IDLWriter::~IDLWriter()
{
}

bool IDLWriter::write(std::ostream &out)
{
    const Namespace::Ptr &ns =  module_->getNamespace();
    out << "// Module Namespace " << ns->getName() << "\n";
    {
        const Module::TypeDeclarationList &decls = module_->getTypeDeclatations();
        for (Module::TypeDeclarationList::const_iterator it = decls.begin(),
                end = decls.end(); it != end; ++it)
        {
            std::string ownTypeName = getTypeName(it->second);
            std::string nsTypeName = ns->getTypeName(it->second);
            if (it->first == Module::TYPEDEF)
            {
                out << "typedef " << ownTypeName << " " << nsTypeName << "\n";
            }
            else
            {
                if (StructType::Ptr ty = dyn_cast<StructType>(it->second))
                {
                    writeType(ty, out);
                }
                else if (ServiceType::Ptr ty = dyn_cast<ServiceType>(it->second))
                {
                    writeType(ty, out);
                }
                else if (EnumType::Ptr ty = dyn_cast<EnumType>(it->second))
                {
                    writeType(ty, out);
                }
            }
        }
#if 0
        //IndentingStreambuf isb(out, 1);
        for (Namespace::typemap_const_iterator it = ns->typemap_begin(),
                end = ns->typemap_end();
                it != end; ++it)
        {
            std::string typeName = getTypeName(it->second);

            if (it->second->getNamespace() == ns)
            {
                if (StructType::Ptr sty = dyn_cast<StructType>(it->second))
                {
                    writeType(sty, out);
                }
            }

            if (typeName != it->first)
            {
                out << "typedef " << typeName << " " << it->first << "\n";
            }

        }
#endif
    }
    return true;
}

std::string IDLWriter::getTypeName(const TypePtr &type)
{
    // check generic types
    if (ArrayType::Ptr ty = dyn_cast<ArrayType>(type))
    {
        return "array<" + getTypeName(ty->getElementType()) + ">";
    }
    else if (FixedArrayType::Ptr ty = dyn_cast<FixedArrayType>(type))
    {
        return "array<" + getTypeName(ty->getElementType()) + ", " +
            boost::lexical_cast<std::string>(ty->getArraySize()) + ">";
    }

    return type->getTypeName();
}

void IDLWriter::writeType(const StructType::Ptr &type, std::ostream &out)
{
    if (type)
        writeAnnotationList(*type, out);

    if (type->getAttributeValue<ExceptionTypeAttr>())
        out<<"exception ";
    else if (type->getAttributeValue<AnnotationTypeAttr>())
        out<<"annotation ";
    else
        out<<"struct ";
    out<<getTypeName(type)<<" {\n";
    {
        IndentingStreambuf isb(out, 1);
        writeTypeMembers(type, out);
    }
    out<<"}\n";
}

void IDLWriter::writeType(const ServiceType::Ptr &type, std::ostream &out)
{
    if (type)
        writeAnnotationList(*type, out);

    out<<"service "<<getTypeName(type)<<" {\n";
    {
        IndentingStreambuf isb(out, 1);
        writeTypeMembers(type, out);
    }
    out<<"}\n";
}

void IDLWriter::writeType(const EnumType::Ptr &enumType, std::ostream &out)
{
    if (enumType)
        writeAnnotationList(*enumType, out);

    out<<"enum "<<getTypeName(enumType)<<" {\n";
    {
        IndentingStreambuf isb(out, 1);
        const size_t n = enumType->getNumConstants();
        for (size_t i = 0; i < n; ++i)
        {
            if (i != 0)
                out << ",\n";
            Expr::Ptr expr = enumType->getConstantAt(i);
            std::string name = enumType->getConstantNameAt(i);
            out << name << " = ";
            expr->print(out);
        }
        if (n != 0)
            out << "\n";
    }
    out<<"}\n";
}

void IDLWriter::writeTypeMembers(const CompositeType::Ptr &type, std::ostream &out)
{
    const size_t n = type->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
        Type::Ptr elemTy = type->getElementAt(i);
        const ElementData &elemData = type->getElementDataAt(i);
        if (i != 0)
            out << "\n";
        if (FunctionType::Ptr fty = dyn_cast<FunctionType>(elemTy))
        {
            writeAnnotationList(*fty, out);

            out << getTypeName(fty->getReturnType()) << " ";

            writeAnnotationList(fty->getReturnElementData(), out, " ");

            out << elemData.getName() << "(";

            const size_t nargs = fty->getNumParams();
            for (size_t i = 0; i < nargs; ++i)
            {
                if (i != 0)
                    out << ", ";
                const ElementData &paramElementData = fty->getParamElementDataAt(i);
                writeAnnotationList(paramElementData, out, " ");
                out << getTypeName(fty->getParamType(i)) << " " << fty->getParamName(i);
            }
            out << ")";
        }
        else
        {
            writeAnnotationList(elemData, out);
            out << getTypeName(elemTy) << " " << elemData.getName();
            if (elemData.hasAttributeValue<DefaultFieldValueAttr>())
            {
                Expr::Ptr expr = elemData.getAttributeValue<DefaultFieldValueAttr>();
                out <<" = ";
                expr->print(out);
            }
        }
        out << ";";
    }
    if (n != 0)
        out<<"\n";
}

void IDLWriter::writeAnnotationList(const AttributeHolder &attributeHolder, std::ostream &out, const char *sep)
{
    if (attributeHolder.hasAttributeValue<AnnotationListAttr>())
    {
        AnnotationList alist = attributeHolder.getAttributeValue<AnnotationListAttr>();
        writeAnnotationList(alist, out, sep);
    }
}

void IDLWriter::writeAnnotationList(const AnnotationList &alist, std::ostream &out, const char *sep)
{
    if (alist.empty())
        return;
    out << "[";

    bool first = true;

    for (AnnotationList::const_iterator it = alist.begin(), end = alist.end(); it != end; ++it)
    {
        Annotation::Ptr annotation = *it;
        if (!annotation)
            continue;
        if (!first)
            out<<", ";
        else
            first = false;
        out<<getTypeName(annotation->getAnnotationType());
        // FIXME Implement output of arguments
    }

    out << "]";
    if (sep)
        out << sep;
}

} // namespace KIARA
