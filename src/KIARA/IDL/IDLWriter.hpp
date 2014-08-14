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
 * IDLWriter.hpp
 *
 *  Created on: Jul 19, 2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_IDL_IDLWRITER_HPP_INCLUDED
#define KIARA_IDL_IDLWRITER_HPP_INCLUDED

#include <KIARA/DB/Module.hpp>
#include <KIARA/DB/Annotation.hpp>

namespace KIARA
{

class KIARA_API IDLWriter
{
public:

    IDLWriter(const Module::Ptr &module);
    ~IDLWriter();

    bool write(std::ostream &out);

private:
    std::string getTypeName(const TypePtr &type);
    void writeType(const StructType::Ptr &structType, std::ostream &out);
    void writeType(const ServiceType::Ptr &serviceType, std::ostream &out);
    void writeType(const EnumType::Ptr &enumType, std::ostream &out);
    void writeTypeMembers(const CompositeType::Ptr &type, std::ostream &out);
    void writeAnnotationList(const AttributeHolder &attributeHolder, std::ostream &out, const char *sep = "\n");
    void writeAnnotationList(const AnnotationList &alist, std::ostream &out, const char *sep = "\n");

    Module::Ptr module_;
};

} // namespace KIARA

#endif /* KIARA_IDL_IDLWRITER_HPP_INCLUDED */
