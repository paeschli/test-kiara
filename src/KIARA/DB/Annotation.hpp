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
 * Annotation.hpp
 *
 *  Created on: Aug 16, 2013
 *      Author: rubinste
 */

#ifndef KIARA_DB_ANNOTATION_HPP_INCLUDED
#define KIARA_DB_ANNOTATION_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <KIARA/DB/DerivedTypes.hpp>
#include <KIARA/DB/Value.hpp>
#include <vector>
namespace KIARA
{

class KIARA_API Annotation : public Object
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(Annotation, Object)
public:

    Annotation(World &world);

    Annotation(const StructType::Ptr &type);

    Annotation(const Annotation &other);

    const StructType::Ptr & getAnnotationType() const { return annotationType_; }

    void setAnnotationType(const StructType::Ptr & annotationType);

    const ArrayValue & getValues() const { return values_; }

    void setValues(const ArrayValue &values);

    void print(std::ostream &out) const;

protected:
    virtual void gcUnlinkRefs();
    virtual void gcApplyToChildren(const CollectorCallback &callback);
private:
    StructType::Ptr annotationType_;
    ArrayValue values_;
};

typedef std::vector<Annotation::Ptr> AnnotationList;

KIARA_API Annotation::Ptr getFirstAnnotationOfType(const AnnotationList &annotationList, const StructType::Ptr &type);

} // namespace KIARA

#endif /* KIARA_DB_ANNOTATION_HPP_INCLUDED */
