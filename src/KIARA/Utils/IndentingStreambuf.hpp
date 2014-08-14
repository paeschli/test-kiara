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
 * IndentingStreambuf.hpp
 *
 *  Created on: 23.03.2013
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_UTILS_INDENTINGSTREAMBUF_HPP_INCLUDED
#define KIARA_UTILS_INDENTINGSTREAMBUF_HPP_INCLUDED

// based on http://www.c-plusplus.de/forum/p1471584
#include <streambuf>
#include <ostream>

namespace KIARA
{

template <typename char_type,
          typename traits = std::char_traits<char_type> >
struct BasicIndentingStreambuf : public std::basic_streambuf<char_type,traits>
{
public:
    typedef typename traits::int_type int_type;
    typedef typename traits::pos_type pos_type;
    typedef typename traits::off_type off_type;

private:
    typedef BasicIndentingStreambuf<char_type, traits>  this_type;
    typedef std::basic_streambuf<char_type, traits>     streambuf_type;

public:
    explicit BasicIndentingStreambuf(
        streambuf_type* dest, int indent_count = 0, int indent_width = 4)
      : streambuf_(dest)
      , owner_(0)
      , fill_char_(' ')
      , indent_count_(indent_count)
      , indent_width_(indent_width)
      , set_indent_(true)
    {
    }

    explicit BasicIndentingStreambuf(
            std::basic_ostream<char_type, traits> &dest, int indent_count = 0, int indent_width = 4)
      : streambuf_(dest.rdbuf())
      , owner_(&dest)
      , fill_char_(' ')
      , indent_count_(indent_count)
      , indent_width_(indent_width)
      , set_indent_(true)
    {
        owner_->rdbuf(this);
    }

    void setIndentFlag(bool flag)
    {
        set_indent_ = flag;
    }

    bool getIndentFlag() const
    {
        return set_indent_;
    }

    virtual ~BasicIndentingStreambuf()
    {
        if (owner_ != 0)
            owner_->rdbuf(streambuf_);
    }

    void copy_indentation_attributes(const this_type& rhs)
    {
        fill_char_      = rhs.fill_char_;
        indent_count_   = rhs.indent_count_;
        indent_width_   = rhs.indent_width_;
        set_indent_     = rhs.set_indent_;
    }

    char_type fill_char() const
    {
        return (fill_char_);
    }

    void fill_char(char_type c)
    {
        fill_char_ = c;
    }

    int indentation() const
    {
        return indent_count_;
    }

    void indentation(int i)
    {
        indent_count_ = i;
    }

    void incr_indentation(int i)
    {
        indent_count_ += i;
        if (indent_count_ < 0)
            indent_count_ = 0;
    }

    int indentation_width() const
    {
        return indent_width_;
    }

    void indentation_width(int i)
    {
        indent_width_ = i;
    }

    streambuf_type* original_rdbuf()
    {
        return (streambuf_);
    }

private:
    BasicIndentingStreambuf(const this_type& rhs)
      : streambuf_(rhs.streambuf_),
        fill_char_(rhs.fill_char_),
        indent_count_(rhs.indent_count_),
        indent_width_(rhs.indent_width_),
        set_indent_(rhs.set_indent_)
    {
    }

    // declared, never defined
    this_type & operator=(const this_type &);

    // override std::streambuf::overflow
    virtual int_type overflow(int_type c = traits::eof())
    {
        if (traits::eq_int_type(c, traits::eof()))
        {
            return streambuf_->sputc(static_cast<char_type>(c));
        }
        if (set_indent_)
        {
            std::fill_n(std::ostreambuf_iterator<char_type>(streambuf_), indent_count_ * indent_width_, fill_char_);
            set_indent_ = false;
        }
        if (traits::eq_int_type(streambuf_->sputc(static_cast<char_type>(c)), traits::eof()))
        {
            return traits::eof();
        }
        if (traits::eq_int_type(c, traits::to_char_type('\n')))
        {
            set_indent_ = true;
        }
        return traits::not_eof(c);
    }

private:
    streambuf_type *streambuf_;
    std::ostream *owner_;

    char_type       fill_char_;
    int             indent_count_;
    int             indent_width_;
    bool            set_indent_;
};

typedef BasicIndentingStreambuf<char>       IndentingStreambuf;

} // namespace KIARA

#endif /* KIARA_UTILS_INDENTINGSTREAMBUF_HPP_INCLUDED */
