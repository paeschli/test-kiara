/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2012, 2013  German Research Center for Artificial Intelligence (DFKI)
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
 * ArrayRef.hpp
 *
 *  Created on: 16.08.2012
 *      Author: Dmitri Rubinstein
 */
// Based on code from AnyDSL

#ifndef KIARA_UTILS_ARRAYREF_HPP_INCLUDED
#define KIARA_UTILS_ARRAYREF_HPP_INCLUDED

#include <cassert>
#include <cstdlib>
#include <cstddef>

namespace KIARA
{

template<class LEFT, class RIGHT>
inline LEFT const& deref_hook(const RIGHT* ptr) { return *ptr; }

template<class T, class Deref = T, Deref const& (*Hook)(const T*) = deref_hook<T, T> >
class ArrayRef
{
public:

    class const_iterator
    {
    public:

        typedef std::random_access_iterator_tag iterator_category;
        typedef const Deref value_type;
        typedef ptrdiff_t difference_type;
        typedef const T* pointer;
        typedef const Deref& reference;

        const_iterator(const const_iterator& i) : base_(i.base_) {}
        const_iterator(pointer base) : base_(base) {}

        const_iterator& operator ++ () { ++base_; return *this; }
        const_iterator  operator ++ (int) { const_iterator i(*this); ++(*this); return i; }

        const_iterator& operator -- () { --base_; return *this; }
        const_iterator  operator -- (int) { const_iterator i(*this); --(*this); return i; }

        difference_type operator + (const_iterator i) { return difference_type(base_ + i.base()); }
        difference_type operator - (const_iterator i) { return difference_type(base_ - i.base()); }

        const_iterator operator + (difference_type d) { return const_iterator(base_ + d); }
        const_iterator operator - (difference_type d) { return const_iterator(base_ - d); }

        const_iterator& operator += (difference_type d) { base_ += d; return *this; }
        const_iterator& operator -= (difference_type d) { base_ -= d; return *this; }

        bool operator <  (const const_iterator& i) { return base_ <  i.base_; }
        bool operator <= (const const_iterator& i) { return base_ <= i.base_; }
        bool operator >  (const const_iterator& i) { return base_ >  i.base_; }
        bool operator >= (const const_iterator& i) { return base_ >= i.base_; }
        bool operator == (const const_iterator& i) { return base_ == i.base_; }
        bool operator != (const const_iterator& i) { return base_ != i.base_; }

        reference operator *  () { return Hook(base_); }
        pointer operator -> () { return &Hook(base_); }

        pointer base() const { return base_; }

    private:

        pointer base_;
    };

    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    ArrayRef() : ptr_(0), size_(0) {}

    ArrayRef(const T &oneElt)
          : ptr_(&oneElt), size_(1)
    {}

    template<size_t N>
    ArrayRef(T (&array)[N])
        : ptr_(&array[0])
        , size_(N)
    {}

    ArrayRef(const std::vector<T>& vector)
        : ptr_(vector.empty() ? 0 : &vector[0])
        , size_(vector.size())
    {}

    ArrayRef(const T* ptr, size_t size)
        : ptr_(ptr)
        , size_(size)
    {}

    ArrayRef(const T *begin, const T *end)
      : ptr_(begin), size_(end - begin)
    {}

//    ArrayRef(const Array<T>& array)
//        : ptr_(array.begin())
//        , size_(array.size())
//    {}
    template<class Deref_, Deref const& (*Hook_)(const T*)>
    ArrayRef(const ArrayRef<T, Deref_, Hook_>& array)
        : ptr_(array.begin().base())
        , size_(array.size())
    {}

    const_iterator begin() const { return ptr_; }
    const_iterator end() const { return ptr_ + size_; }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    Deref const& operator [] (size_t i) const
    {
        assert(i < size() && "index out of bounds");
        return Hook(ptr_ + i);
    }

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    const T * data() const { return ptr_; }

    T const& front() const { assert(!empty()); return ptr_[0]; }
    T const& back()  const { assert(!empty()); return ptr_[size_ - 1]; }

    ArrayRef<T> slice(size_t begin, size_t end) const { return ArrayRef<T>(ptr_ + begin, end - begin); }
    ArrayRef<T> slice_front(size_t end) const { return ArrayRef<T>(ptr_, end); }
    ArrayRef<T> slice_back(size_t begin) const { return ArrayRef<T>(ptr_ + begin, size_ - begin); }

    operator ArrayRef<T> () { return ArrayRef<T>(ptr_, size_); }

private:

    const T* ptr_;
    size_t size_;
};

/// Construct an ArrayRef from a single element.
template<typename T>
ArrayRef<T> makeArrayRef(const T &oneElt)
{
    return oneElt;
}

/// Construct an ArrayRef from a pointer and length.
template<typename T>
ArrayRef<T> makeArrayRef(const T *data, size_t length)
{
    return ArrayRef<T>(data, length);
}

/// Construct an ArrayRef from a range.
template<typename T>
ArrayRef<T> makeArrayRef(const T *begin, const T *end)
{
    return ArrayRef<T>(begin, end);
}

/// Construct an ArrayRef from a std::vector.
template<typename T>
ArrayRef<T> makeArrayRef(const std::vector<T> &vec)
{
    return vec;
}

/// Construct an ArrayRef from a C array.
template<typename T, size_t N>
ArrayRef<T> makeArrayRef(const T (&arr)[N])
{
    return ArrayRef<T>(arr);
}

} // namespace KIARA

#endif /* KIARA_UTILS_ARRAYREF_HPP_INCLUDED */
