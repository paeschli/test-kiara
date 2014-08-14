//
// header.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef KIARA_TRANSPORT_HEADER_HPP_INCLUDED
#define KIARA_TRANSPORT_HEADER_HPP_INCLUDED

#include <string>

namespace KIARA
{
namespace Transport
{

struct Header
{
    std::string name;
    std::string value;

    Header()
    { }

    Header(const std::string &name, const std::string &value)
        : name(name), value(value)
    { }

    void swap(Header &header)
    {
        name.swap(header.name);
        value.swap(header.value);
    }
};

} // namespace Transport
} // namespace KIARA

#endif // HTTP_SERVER3_HEADER_HPP
