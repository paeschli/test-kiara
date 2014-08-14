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
 * Mangler.cpp
 *
 *  Created on: 05.03.2013
 *      Author: Dmitri Rubinstein
 */
#define KIARA_COMPILER_LIB
#include "Mangler.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>

namespace KIARA
{

namespace Compiler
{

// Type mangling

std::string Mangler::getMangledName(const Type::Ptr &type, MangleMode mode)
{
    const World &world = type->getWorld();

#define TM(tname, str)                              \
    if (canonicallyEqual(type, world.KIARA_JOIN(type_,tname)()))    \
        return str

    TM(void, "v");
    TM(any, "kA");
    TM(i8, "ka");
    TM(u8, "kh");
    TM(i16, "ks");
    TM(u16, "kt");
    TM(i32, "ki");
    TM(u32, "kj");
    TM(i64, "kx");
    TM(u64, "ky");
    TM(float, "kf");
    TM(double, "kd");
    TM(boolean, "kb");
    TM(string, "kS");

    TM(c_char, "c");
    TM(c_wchar_t, "w");
    TM(c_schar, "a");
    TM(c_uchar, "h");
    TM(c_short, "s");
    TM(c_ushort, "t");
    TM(c_int, "i");
    TM(c_uint, "j");
    TM(c_long, "l");
    TM(c_ulong, "m");
    TM(c_longlong, "x");
    TM(c_ulonglong, "y");
    TM(c_float, "f");
    TM(c_double, "d");
    TM(c_longdouble, "e");

    if (PrimValueType::Ptr vty = dyn_cast<PrimValueType>(type))
    {
        const TypedBox &value = vty->getValue();
        switch (value.getType())
        {
            case BT_EMPTY:
            case BT_INT8_T:
                return "La" + boost::lexical_cast<std::string>(value.get_i8()) + "E";
            case BT_UINT8_T:
                return "Lh" + boost::lexical_cast<std::string>(value.get_i8()) + "E";
            case BT_INT16_T:
                return "Ls" + boost::lexical_cast<std::string>(value.get_i8()) + "E";
            case BT_UINT16_T:
                return "Lt" + boost::lexical_cast<std::string>(value.get_i8()) + "E";
            case BT_INT32_T:
                return "Li" + boost::lexical_cast<std::string>(value.get_i8()) + "E";
            case BT_UINT32_T:
                return "Lj" + boost::lexical_cast<std::string>(value.get_i8()) + "E";
            case BT_INT64_T:
                return "Lx" + boost::lexical_cast<std::string>(value.get_double()) + "E";
            case BT_UINT64_T:
                return "Ly" + boost::lexical_cast<std::string>(value.get_double()) + "E";
            case BT_FLOAT:
                return "Lf" + boost::lexical_cast<std::string>(value.get_double()) + "E";
            case BT_DOUBLE:
                return "Ld" + boost::lexical_cast<std::string>(value.get_double()) + "E";
            case BT_BOOL:
                return (value.get_bool() ? "Lb1E" : "Lb0E");
            case BT_STRING:
                return "LN6stringE" + std::string(value.get_string()) + "E";
            case BT_VOIDPTR:
                return "LPv" + boost::lexical_cast<std::string>(value.get_ptr()) + "E";
        }
    }

    if (PtrType::Ptr pty = dyn_cast<PtrType>(type))
        return "P" + getMangledName(pty->getElementType());

    if (RefType::Ptr rty = dyn_cast<RefType>(type))
        return "R" + getMangledName(rty->getElementType());

    if (ArrayType::Ptr aty = dyn_cast<ArrayType>(type))
    {
        return "P" + getMangledName(aty->getElementType());
    }

    if (FixedArrayType::Ptr faty = dyn_cast<FixedArrayType>(type))
    {
        if (mode == FUNC_PARAMETER_TYPE)
            return "P" + getMangledName(faty->getElementType());
        else
            return "A" + boost::lexical_cast<std::string>(faty->getArraySize()) + "_" + getMangledName(faty->getElementType());
    }

    if (FunctionType::Ptr fty = dyn_cast<FunctionType>(type))
    {
        std::string tn = fty->getFullTypeName();
        boost::algorithm::replace_all(tn, ".", "_");
        tn = "_KF" + boost::lexical_cast<std::string>(tn.length()) + tn;

        for (size_t i = 0, num = fty->getNumParams(); i < num; ++i)
        {
            tn += getMangledName(fty->getParamType(i));
        }
        return tn;
    }

    bool uniqueType = false;
    if (StructType::Ptr sty = dyn_cast<StructType>(type))
    {
        uniqueType = sty->isUnique();
    }

    std::string tn = type->getFullTypeName();
    boost::algorithm::replace_all(tn, ".", "_");
    tn = boost::lexical_cast<std::string>(tn.length()) + tn;
    if (!uniqueType)
    {
        size_t numElements = type->getNumElements();
        if (numElements > 0)
        {
            tn+="I";
            for (size_t i = 0; i < numElements; ++i)
            {
                Type::Ptr elem = type->getSafeElementAs<Type>(i);
                tn += getMangledName(elem);
            }
            tn+="E";
        }
    }

    return tn;
}

static inline const char *encodeOp(const std::string &op, int numArgs)
{
    if (op == "+")
            return (numArgs == 1 ? "ps" :
                        (numArgs == 2 ? "pl" : "v2pl"));
    if (op == "-")
            return (numArgs == 1 ? "ng" :
                        (numArgs == 2 ? "mi" : "v2mi"));
    if (op == "&")
            return (numArgs == 1 ? "ad" :
                        (numArgs == 2 ? "an" : "v2an"));
    if (op == "*")
        return (numArgs == 1 ? "de" :
                    (numArgs == 2 ? "ml" : "v2ml"));
    if (op == "~")
        return (numArgs == 2 ? "co" : "v2co");
    if (op == "/")
        return (numArgs == 2 ? "dv" : "v2dv");
    if (op == "%")
        return (numArgs == 2 ? "rm" : "v2rm");
    if (op == "|")
        return (numArgs == 2 ? "or" : "v2or");
    if (op == "^")
        return (numArgs == 2 ? "eo" : "v2eo");
    if (op == "=")
        return (numArgs == 2 ? "aS" : "v2aS");
    if (op == "+=")
        return (numArgs == 2 ? "pL" : "v2pL");
    if (op == "-=")
        return (numArgs == 2 ? "pL" : "v2mI");
    if (op == "*=")
        return (numArgs == 2 ? "mL" : "v2mL");
    if (op == "/=")
        return (numArgs == 2 ? "dV" : "v2dV");
    if (op == "%=")
        return (numArgs == 2 ? "rM" : "v2rM");
    if (op == "&=")
        return (numArgs == 2 ? "aN" : "v2aN");
    if (op == "|=")
        return (numArgs == 2 ? "oR" : "v2oR");
    if (op == "^=")
        return (numArgs == 2 ? "eO" : "v2eO");
    if (op == "<<")
        return (numArgs == 2 ? "ls" : "v2ls");
    if (op == ">>")
        return (numArgs == 2 ? "rs" : "v2rs");
    if (op == "<<=")
        return (numArgs == 2 ? "lS" : "v2lS");
    if (op == ">>=")
        return (numArgs == 2 ? "rS" : "v2rS");
    if (op == "==")
        return (numArgs == 2 ? "eq" : "v2eq");
    if (op == "!=")
        return (numArgs == 2 ? "ne" : "v2ne");
    if (op == "<")
        return (numArgs == 2 ? "lt" : "v2lt");
    if (op == ">")
        return (numArgs == 2 ? "gt" : "v2gt");
    if (op == "<=")
        return (numArgs == 2 ? "le" : "v2le");
    if (op == ">=")
        return (numArgs == 2 ? "ge" : "v2ge");
    if (op == "!")
        return (numArgs == 1 ? "nt" : "v2nt");
    if (op == "&&")
        return (numArgs == 2 ? "aa" : "v2aa");
    if (op == "||")
        return (numArgs == 2 ? "oo" : "v2oo");
    if (op == "++")
        return (numArgs == 1 ? "pp" : "v2pp");
    if (op == "--")
        return (numArgs == 1 ? "mm" : "v2mm");
    if (op == ",")
        return (numArgs == 1 ? "cm" : "v2cm");
    if (op == "->*")
        return (numArgs == 1 ? "pm" : "v2pm");
    if (op == "->")
        return (numArgs == 1 ? "pt" : "v2pt");
    if (op == "()")
        return (numArgs == 1 ? "cl" : "v2cl");
    if (op == "[]")
        return (numArgs == 1 ? "ix" : "v2ix");
    if (op == "?")
        return (numArgs == 3 ? "qu" : "v2qu");
    if (op == ".")
        return "v3dot";
    if (op == ":")
        return "v3col";
    if (op == "$")
        return "v3dol";
    return 0;
}

std::string Mangler::getMangledFuncPrefix(const std::string &name, int numArgs)
{
    const char *opName = encodeOp(name, numArgs);
    if (opName)
        return std::string("_K") + opName;

    std::string buf;

    for (size_t i = 0; i < name.length(); ++i)
    {
        char c = name[i];
        switch (c)
        {
            case '+': buf += "pl"; break;
            case '-': buf += "mi"; break;
            case '&': buf += "an"; break;
            case '*': buf += "ml"; break;
            case '/': buf += "dv"; break;
            case '~': buf += "co"; break;
            case '%': buf += "rm"; break;
            case '|': buf += "or"; break;
            case '^': buf += "eo"; break;
            case '=': buf += "as"; break;
            case '<': buf += "lt"; break;
            case '>': buf += "gt"; break;
            case '!': buf += "nt"; break;
            case ',': buf += "cm"; break;
            case '(': buf += "op"; break;
            case ')': buf += "cp"; break;
            case '[': buf += "ok"; break;
            case ']': buf += "ck"; break;
            case '{': buf += "ob"; break;
            case '}': buf += "cb"; break;
            case '?': buf += "qu"; break;
            case '.': buf += "dt"; break;
            case ':': buf += "cl"; break;
            case '$': buf += "ds"; break;
            default:
                buf+=c;
                break;
        }
    }

    return "_K" + boost::lexical_cast<std::string>(buf.length()) + buf;
}

std::string Mangler::getMangledFuncName(const std::string &name,
                                        ArrayRef<Type::Ptr> argTypes)
{
    typedef ArrayRef<Type::Ptr>::const_iterator Iter;

    std::string mn = getMangledFuncPrefix(name, argTypes.size());
    for (Iter it = argTypes.begin(), end = argTypes.end(); it != end; ++it)
        mn += Mangler::getMangledName(*it, FUNC_PARAMETER_TYPE);
    return mn;
}

std::string Mangler::getMangledFuncName(const std::string &name,
                                        ArrayRef<IR::IRExpr::Ptr> args)
{
    typedef ArrayRef<IR::IRExpr::Ptr>::const_iterator Iter;

    std::string mn = getMangledFuncPrefix(name, args.size());
    for (Iter it = args.begin(), end = args.end(); it != end; ++it)
        mn += Mangler::getMangledName((*it)->getExprType(), FUNC_PARAMETER_TYPE);
    return mn;
}

std::string Mangler::getMangledFuncName(const std::string &name,
                                        ArrayRef<std::pair<std::string, Type::Ptr> > argTypes)
{
    typedef ArrayRef<std::pair<std::string, Type::Ptr> >::const_iterator Iter;

    std::string mn = getMangledFuncPrefix(name, argTypes.size());
    for (Iter it = argTypes.begin(), end = argTypes.end(); it != end; ++it)
        mn += Mangler::getMangledName(it->second, FUNC_PARAMETER_TYPE);
    return mn;
}

} // namespace Compiler

} // namespace KIARA
