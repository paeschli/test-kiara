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
 * valuetest.cpp
 *
 *  Created on: 28.05.2013
 *      Author: Dmitri Rubinstein
 */
#include <boost/test/minimal.hpp>
#include <KIARA/Common/TypeTraits.hpp>
#include <KIARA/Core/LibraryInit.hpp>
#include <KIARA/Core/CycleCollector.hpp>
#include <KIARA/DB/Object.hpp>
#include <KIARA/DB/Value.hpp>
#include <KIARA/DB/ValueIO.hpp>
#include <KIARA/DB/World.hpp>
#include <DFC/Base/Core/ObjectFactory.hpp>

using namespace KIARA;

class TestValue : public KIARA::Object
{
    DFC_DECLARE_NON_CONSTRUCTIBLE_TYPE(TestValue, KIARA::Object)
public:

    TestValue(const std::string &value, KIARA::World &w)
        : KIARA::Object(w)
        , value_(value)
    {
    }

    ~TestValue()
    {
    }

    void print(std::ostream &out) const
    {
        out<<"TestValue(\""<<value_<<"\")";
    }

protected:
    std::string value_;
};

DFC_DEFINE_NON_CONSTRUCTIBLE_TYPE(TestValue)

struct X
{
    int a;
    float b;
};

bool checkJSON(const Value &value)
{
    std::string s1 = ValueIO::toJSON(value);
    std::string s2 = ValueIO::toJSON(ValueIO::fromJSON(s1));
    std::cout<<"S1 = "<<s1<<"\nS2 = "<<s2<<std::endl;
    return s1 == s2;
}

int test_main (int argc, char **argv)
{
    KIARA::LibraryInit init;
    DFC_REGISTER_TYPE(TestValue);
    KIARA::World world;

    std::cout<<"sizeof(Number) == "<<sizeof(Number)<<std::endl;
    std::cout<<"sizeof(Value) == "<<sizeof(Value)<<std::endl;
    std::cout<<"sizeof(DictValue) == "<<sizeof(DictValue)<<std::endl;
    std::cout<<"sizeof(ArrayValue) == "<<sizeof(ArrayValue)<<std::endl;

    Number a(12.5);
    Number b(13.5);

    Number c(a.toDouble() + b.toDouble());
    BOOST_CHECK(c.toInt() == 26);

    Number d(c.toInt());
    BOOST_CHECK(d.toString() == "26");

    std::cout<<"d = "<<d.toString()<<std::endl;

    Value val;
    BOOST_CHECK(val.isNull());
    BOOST_CHECK(checkJSON(val));
    std::cout<<"val = "<<val<<std::endl;
    std::cout<<"val as JSON: "<<ValueIO::toJSON(val)<<std::endl;

    val = d;
    BOOST_CHECK(val.isNumber());
    BOOST_CHECK(checkJSON(val));
    std::cout<<"val = "<<val<<std::endl;
    std::cout<<"val as JSON: "<<ValueIO::toJSON(val)<<std::endl;

    val = 12;
    BOOST_CHECK(val.isNumber());
    BOOST_CHECK(val.is<int>());
    BOOST_CHECK(val.isIntegerNumber());
    BOOST_CHECK(val.get<int>() == 12);
    BOOST_CHECK(val.getNumber() == Number::create(12));
    BOOST_CHECK(checkJSON(val));
    std::cout<<"val = "<<val<<std::endl;
    std::cout<<"val as JSON: "<<ValueIO::toJSON(val)<<std::endl;

    val = 30.12;
    BOOST_CHECK(val.isNumber());
    BOOST_CHECK(val.is<double>());
    BOOST_CHECK(val.isRealNumber());
    BOOST_CHECK(val.get<double>() == 30.12);
    BOOST_CHECK(val.getNumber() == Number(30.12));
    BOOST_CHECK(checkJSON(val));
    std::cout<<"val = "<<val<<std::endl;
    std::cout<<"val as JSON: "<<ValueIO::toJSON(val)<<std::endl;

    Value val1 = "test";
    BOOST_CHECK(val1.isString());
    BOOST_CHECK(val1.is<std::string>());
    BOOST_CHECK(val1.get<std::string>() == "test");
    BOOST_CHECK(checkJSON(val));
    std::cout<<"val1 = "<<val1<<std::endl;
    std::cout<<"val1 as JSON: "<<ValueIO::toJSON(val1)<<std::endl;

    //std::cout<<boost::lexical_cast<std::string>(val)<<std::endl;

    X x = {12, 22.12};

    val.setAny(x);
    BOOST_CHECK(val.isAny());
    BOOST_CHECK(checkJSON(val));
    std::cout<<"val = "<<val<<std::endl;
    std::cout<<"val as JSON: "<<ValueIO::toJSON(val)<<std::endl;

    val = x;
    BOOST_CHECK(val.isAny());
    BOOST_CHECK(checkJSON(val));
    std::cout<<"val = "<<val<<std::endl;
    std::cout<<"val as JSON: "<<ValueIO::toJSON(val)<<std::endl;

    std::string s = "T1";
    val = s;
    BOOST_CHECK(val.isString());
    BOOST_CHECK(checkJSON(val));
    std::cout<<"val = "<<val<<std::endl;
    std::cout<<"val as JSON: "<<ValueIO::toJSON(val)<<std::endl;

    val = true;
    BOOST_CHECK(val.isTrue());
    BOOST_CHECK(checkJSON(val));
    std::cout<<"val = "<<val<<std::endl;
    std::cout<<"val as JSON: "<<ValueIO::toJSON(val)<<std::endl;
    val = false;
    BOOST_CHECK(val.isFalse());
    BOOST_CHECK(checkJSON(val));
    std::cout<<"val = "<<val<<std::endl;
    std::cout<<"val as JSON: "<<ValueIO::toJSON(val)<<std::endl;

    val.getOrCreateDict()["X"] = x;
    val.getOrCreateDict()["A"] = 90;
    val.getOrCreateDict()["B"] = 3.0;
    BOOST_CHECK(val.isDict());
    BOOST_CHECK(val.getDict().hasKey("X") == true);
    BOOST_CHECK(val.getDict().hasKey("A") == true);
    BOOST_CHECK(val.getDict().hasKey("B") == true);
    BOOST_CHECK(val.getDict().hasKey("Y") == false);
    BOOST_CHECK(checkJSON(val));
    std::cout<<"val = "<<val<<std::endl;
    std::cout<<"val as JSON: "<<ValueIO::toJSON(val)<<std::endl;

    val = sizeof(val);
    BOOST_CHECK(val.isNumber());
    BOOST_CHECK(checkJSON(val));
    std::cout<<"val = "<<val<<std::endl;
    std::cout<<"val as JSON: "<<ValueIO::toJSON(val)<<std::endl;
    std::cout<<"val number type id: "<<val.getNumber().getType()<<std::endl;

    val.getOrCreateArray().push_back(x);
    val.getOrCreateArray().push_back(22);
    val.getOrCreateArray().push_back(1);
    val.getOrCreateArray().push_back(true);
    val.getOrCreateArray().push_back(0);
    val.getOrCreateArray().push_back("Z");
    val.getOrCreateArray().push_back(false);
    BOOST_CHECK(val.isArray());
    BOOST_CHECK(val.getArray().size() == 7);
    BOOST_CHECK(checkJSON(val));
    std::cout<<"val = "<<val<<std::endl;
    std::cout<<"val as JSON: "<<ValueIO::toJSON(val)<<std::endl;

    val.setObject(new TestValue("TEST", world));
    BOOST_CHECK(val.isObject());
    BOOST_CHECK(checkJSON(val));
    std::cout<<"val = "<<val<<std::endl;

    return 0;
}
