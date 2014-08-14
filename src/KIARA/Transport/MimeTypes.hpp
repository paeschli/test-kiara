//
// mime_types.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef KIARA_TRANSPORT_MIMETYPES_HPP_INCLUDED
#define KIARA_TRANSPORT_MIMETYPES_HPP_INCLUDED

#include <string>

namespace KIARA
{
namespace Transport
{
namespace MimeTypes
{

/// Convert a file extension into a MIME type.
std::string extensionToType(const std::string& extension);

} // namespace MimeTypes
} // namespace Transport
} // namespace KIARA

#endif // KIARA_SERVER_MIMETYPES_HPP_INCLUDED
