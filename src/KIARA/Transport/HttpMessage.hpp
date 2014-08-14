/*
 * HttpMessage.hpp
 *
 *  Created on: Feb 14, 2014
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_TRANSPORT_HTTPMESSAGE_HPP_INCLUDED
#define KIARA_TRANSPORT_HTTPMESSAGE_HPP_INCLUDED

#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

#include "Header.hpp"
#include "TransportMessage.hpp"

namespace KIARA
{
namespace Transport
{

class HttpMessage : public TransportMessage
{
public:

    typedef std::vector<Header> HeaderList;

    const HeaderList & getHeaders() const { return headers_; }

    HeaderList & getHeaders() { return headers_; }

    /// Convert the response into a vector of buffers. The buffers do not own the
    /// underlying memory blocks, therefore the response object must remain valid and
    /// not be changed until the write operation has completed.
    virtual void toBuffers(std::vector<boost::asio::const_buffer> &buffers);

    HeaderList::const_iterator findHeader(const std::string &name) const
    {
       for (HeaderList::const_iterator it = headers_.begin(), end = headers_.end(); it != end; ++it)
       {
           if (boost::algorithm::iequals(name, it->name))
               return it;
       }
       return headers_.end();
    }

    HeaderList::iterator findHeader(const std::string &name)
    {
       for (HeaderList::iterator it = headers_.begin(), end = headers_.end(); it != end; ++it)
       {
           if (boost::algorithm::iequals(name, it->name))
               return it;
       }
       return headers_.end();
    }

    HeaderList::const_iterator headers_begin() const { return headers_.begin(); }
    HeaderList::const_iterator headers_end() const { return headers_.end(); }

    HeaderList::iterator headers_begin() { return headers_.begin(); }
    HeaderList::iterator headers_end() { return headers_.end(); }

    /// Overwrites the first header with the same name.
    /// The new header will be appended to the end of the list, if no header with the given name can be found.
    void setHeader(const std::string &name, const std::string &value);

    /// Adds a header to this message. The header will be appended to the end of the list.
    void addHeader(const std::string &name, const std::string &value);

    bool getContentLength(size_t &contentLength);

    size_t computeContentLength();

    void clear();

protected:

    HttpMessage(const Transport *transport);

private:
    /// The headers coming from request or to be included in the response.
    HeaderList headers_;
};

} // namespace Transport
} // namespace KIARA

#endif /* KIARA_TRANSPORT_HTTPMESSAGE_HPP_INCLUDED */
