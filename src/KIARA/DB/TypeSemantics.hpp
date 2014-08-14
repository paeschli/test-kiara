/*
 * TypeSemantics.hpp
 *
 *  Created on: Jan 13, 2014
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_DB_TYPESEMANTICS_HPP_INCLUDED
#define KIARA_DB_TYPESEMANTICS_HPP_INCLUDED

#include <iostream>

namespace KIARA
{

class TypeSemantics
{
public:

    enum AllocationMode
    {
        AM_NONE         = 0,
        AM_MALLOC_FREE  = 1,
        AM_NEW_DELETE   = 2,
        AM_CUSTOM       = 3
    };

    enum ValueInterp
    {
        VI_NONE             = 0,
        VI_ARRAY_PTR        = 1,
        VI_VALUE_PTR        = 2,
        VI_CSTRING_PTR      = 3
    };

    TypeSemantics(AllocationMode allocationMode = AM_NONE, ValueInterp valueInterp = VI_NONE)
        : allocationMode(allocationMode)
        , valueInterp(valueInterp)
    {
    }

    void setToDefaults()
    {
        allocationMode = AM_NONE;
        valueInterp = VI_NONE;
    }

    AllocationMode allocationMode;
    ValueInterp valueInterp;
};

inline std::ostream &operator<<(std::ostream &out, const TypeSemantics &msem)
{
    out << "TypeSemantics( ";
    out << "allocationMode: ";
    switch (msem.allocationMode)
    {
        case TypeSemantics::AM_NONE: out << "AM_NONE"; break;
        case TypeSemantics::AM_MALLOC_FREE: out << "AM_MALLOC_FREE"; break;
        case TypeSemantics::AM_NEW_DELETE: out << "AM_NEW_DELETE"; break;
        case TypeSemantics::AM_CUSTOM: out << "AM_CUSTOM"; break;
    }
    out << ", valueIterp: ";
    switch (msem.valueInterp)
    {
        case TypeSemantics::VI_NONE: out << "VI_NONE"; break;
        case TypeSemantics::VI_ARRAY_PTR: out << "VI_ARRAY_PTR"; break;
        case TypeSemantics::VI_VALUE_PTR: out << "VI_VALUE_PTR"; break;
        case TypeSemantics::VI_CSTRING_PTR: out << "VI_CSTRING_PTR"; break;
    }
    out << " )";
    return out;
}

} // namespace KIARA

#endif /* KIARA_DB_TYPESEMANTICS_HPP_INCLUDED */
