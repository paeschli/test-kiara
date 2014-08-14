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
 * Annotation.cpp
 *
 *  Created on: Aug 16, 2013
 *      Author: rubinste
 */
#define KIARA_LIB
#include "Annotation.hpp"

namespace KIARA
{

DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(KIARA::Annotation)

Annotation::Annotation(World &world)
    : Object(world)
    , annotationType_()
    , values_()
{ }

Annotation::Annotation(const StructType::Ptr &annotationType)
    : Object(annotationType->getWorld())
    , annotationType_(annotationType)
    , values_()
{ }

Annotation::Annotation(const Annotation &other)
    : Object(other.getWorld())
    , annotationType_(other.annotationType_)
    , values_(other.values_)
{ }

void Annotation::setAnnotationType(const StructType::Ptr & annotationType)
{
    annotationType_ = annotationType;
}

void Annotation::setValues(const ArrayValue &values)
{
    values_ = values;
}

void Annotation::print(std::ostream &out) const
{
    out<<"annotation ";
    if (annotationType_)
        out<<annotationType_->getFullTypeName();
    out<<"("<<values_<<")";
}

void Annotation::gcUnlinkRefs()
{
    InheritedType::gcUnlinkRefs();
    gcUnlinkChild(annotationType_);
    // FIXME: process values_
}

void Annotation::gcApplyToChildren(const CollectorCallback &callback)
{
    InheritedType::gcApplyToChildren(callback);
    gcApply(annotationType_, callback);
    // FIXME: process values_
}

Annotation::Ptr getFirstAnnotationOfType(const AnnotationList &annotationList, const StructType::Ptr &type)
{
    for (AnnotationList::const_iterator it = annotationList.begin(), end = annotationList.end(); it != end; ++it)
    {
        if ((*it)->getAnnotationType() == type)
            return *it;
    }
    return Annotation::Ptr();
}

} // namespace KIARA
