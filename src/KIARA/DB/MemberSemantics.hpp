/*
 * MemberSemantics.hpp
 *
 *  Created on: Jan 13, 2014
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_DB_MEMBERSEMANTICS_HPP_INCLUDED
#define KIARA_DB_MEMBERSEMANTICS_HPP_INCLUDED

#include <iostream>
#include "TypeSemantics.hpp"

namespace KIARA
{

class MemberSemantics : public TypeSemantics
{
public:

    MemberSemantics(AllocationMode allocationMode = AM_NONE, ValueInterp valueInterp = VI_NONE)
        : TypeSemantics(allocationMode, valueInterp)
        , numDeref(0)
    {
    }

    void setToDefaults()
    {
        numDeref = 0;
        TypeSemantics::setToDefaults();
    }

    MemberSemantics deref() const
    {
        MemberSemantics msem = *this;
        if (msem.numDeref != 0)
            --msem.numDeref;
        else
            msem.setToDefaults();
        return msem;
    }

    unsigned int numDeref;
};

inline std::ostream &operator<<(std::ostream &out, const MemberSemantics &msem)
{
    out << "MemberSemantics( ";
    out << "num deref: " << msem.numDeref;
    out << ", allocationMode: ";
    switch (msem.allocationMode)
    {
        case MemberSemantics::AM_NONE: out << "AM_NONE"; break;
        case MemberSemantics::AM_MALLOC_FREE: out << "AM_MALLOC_FREE"; break;
        case MemberSemantics::AM_NEW_DELETE: out << "AM_NEW_DELETE"; break;
        case MemberSemantics::AM_CUSTOM: out << "AM_CUSTOM"; break;
    }
    out << ", valueIterp: ";
    switch (msem.valueInterp)
    {
        case MemberSemantics::VI_NONE: out << "VI_NONE"; break;
        case MemberSemantics::VI_ARRAY_PTR: out << "VI_ARRAY_PTR"; break;
        case MemberSemantics::VI_VALUE_PTR: out << "VI_VALUE_PTR"; break;
        case MemberSemantics::VI_CSTRING_PTR: out << "VI_CSTRING_PTR"; break;
    }
    out << " )";
    return out;
}

} // namespace KIARA

#endif /* KIARA_DB_MEMBERSEMANTICS_HPP_INCLUDED */
