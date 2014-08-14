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
 * Module.hpp
 *
 *  Created on: 03.08.2012
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_DB_MODULE_HPP_INCLUDED
#define KIARA_DB_MODULE_HPP_INCLUDED

#include "World.hpp"
#include <DFC/Base/Core/Object.hpp>
#include <vector>
#include <string>
#include <iostream>

namespace KIARA
{

class KIARA_API Module : public Object
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(Module, Object)
public:

    enum TypeDeclarationKind
    {
        TYPEDEF,
        NEWTYPE
    };
    typedef std::vector<std::pair<TypeDeclarationKind, Type::Ptr> > TypeDeclarationList;

    Module(World &world, const std::string &namespaceName);
    ~Module();

    const Namespace::Ptr & getNamespace() const { return namespace_; }

    void bindType(const std::string &name, const Type::Ptr &type);
    const Type::Ptr lookupType(const std::string &name) const;
    const std::string getTypeName(const Type::Ptr &type) const;

    void addTypeDeclaration(TypeDeclarationKind kind, const Type::Ptr &type);
    const TypeDeclarationList & getTypeDeclatations() const { return typeDeclarations_; }

    void print(std::ostream &out) const;
    void dump() const;

protected:
    virtual void gcUnlinkRefs();
    virtual void gcApplyToChildren(const CollectorCallback &callback);
private:
    Namespace::Ptr namespace_;
    TypeDeclarationList typeDeclarations_;
};

} // namespace KIARA

#endif /* MODULE_HPP_INCLUDED */
