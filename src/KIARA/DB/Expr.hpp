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
 * Expr.hpp
 *
 *  Created on: 22.01.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_DB_EXPR_HPP_INCLUDED
#define KIARA_DB_EXPR_HPP_INCLUDED

#include <KIARA/Common/Config.hpp>
#include <KIARA/DB/Object.hpp>
#include <KIARA/DB/Type.hpp>

namespace KIARA
{

class KIARA_API Expr : public Object
{
    DFC_DECLARE_ABSTRACT_TYPE(Expr, Object)
public:

    virtual const Type::Ptr & getExprType() const { return exprType_; }

    /** replaceExpr returns true if replacement was successfully performed */
    virtual bool replaceExpr(const Object::Ptr &oldExpr, const Object::Ptr &newExpr);

protected:

    Expr(const Type::Ptr &exprType);
    Expr(const Type::Ptr &exprType, World &world);

    virtual void gcUnlinkRefs();

    virtual void gcApplyToChildren(const CollectorCallback &callback);

    void setExprType(const Type::Ptr & type)
    {
        exprType_ = type;
    }

private:
    Type::Ptr exprType_;
};

template <class T>
inline typename DFC::PointerTraits<T>::Ptr getExprTypeAs(const Expr::Ptr &expr)
{
    if (!expr)
        return 0;
    return getTypeAs<T>(expr->getExprType());
}

} // namespace KIARA

#endif /* KIARA_IDL_EXPR_HPP_INCLUDED */
