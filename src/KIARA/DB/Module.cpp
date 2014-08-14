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
 * Module.cpp
 *
 *  Created on: 03.08.2012
 *      Author: Dmitri Rubinstein
 */

#define KIARA_LIB
#include "Module.hpp"
#include <KIARA/Core/Exception.hpp>
#include <KIARA/Utils/IndentingStreambuf.hpp>
#include <KIARA/DB/Attributes.hpp>
#include <DFC/Base/Utils/StaticInit.hpp>
#include <DFC/Base/Core/ObjectFactory.hpp>
#include <DFC/Base/Core/ObjectMacros.hpp>
#include <boost/assert.hpp>

namespace KIARA
{

// RTTI
DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::Module)

Module::Module(World &world, const std::string &namespaceName)
    : Object(world)
    , namespace_(Namespace::create(world, namespaceName))
{
    #define ABSTRACT_BUILTIN_TYPE(name)                     \
        if (world.KIARA_JOIN(type_, name)())                \
            namespace_->bindType(KIARA_STRINGIZE(name),     \
                    world.KIARA_JOIN(type_, name)(), false);
    #include <KIARA/DB/Type.def>

    namespace_->bindType("Encrypted", world.getEncryptedAnnotation(), true);
}

Module::~Module()
{
}

void Module::bindType(const std::string &name, const Type::Ptr &type)
{
    namespace_->bindType(name, type);
}

const Type::Ptr Module::lookupType(const std::string &name) const
{
    return namespace_->lookupType(name);
}

const std::string Module::getTypeName(const Type::Ptr &type) const
{
    return namespace_->getTypeName(type);
}

void Module::addTypeDeclaration(TypeDeclarationKind kind, const Type::Ptr &type)
{
    typeDeclarations_.push_back(std::make_pair(kind, type));
}

void Module::print(std::ostream &out) const
{
    out << "Module Namespace " << namespace_->getName() << " {\n";
    {
        IndentingStreambuf isb(out, 1);
        for (Namespace::typemap_const_iterator it = namespace_->typemap_begin(),
                end = namespace_->typemap_end();
                it != end; ++it)
        {

            out << "Type "<<it->first<<" : ";
            it->second->print(out);
            out << "\n";
        }
    }
    out <<"} // Module Namespace " << namespace_->getName() << std::endl;
}

void Module::dump() const
{
    print(std::cerr);
}

void Module::gcUnlinkRefs()
{
    InheritedType::gcUnlinkRefs();
    gcUnlinkChild(namespace_);
    gcUnlinkChildren(GCObject::map_values_tag(), typeDeclarations_.begin(), typeDeclarations_.end());
}

void Module::gcApplyToChildren(const CollectorCallback &callback)
{
    InheritedType::gcApplyToChildren(callback);
    gcApply(namespace_, callback);
    gcApply(GCObject::map_values_tag(), typeDeclarations_.begin(), typeDeclarations_.end(), callback);
}

DFC_STATIC_INIT_FUNC
{
    DFC_REGISTER_TYPE(Module);
}

} // namespace KIARA
