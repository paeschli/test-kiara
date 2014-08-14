/*
 * Buffer.hpp
 *
 *  Created on: Feb 12, 2014
 *      Author: Dmitri Rubinstein
 */

#ifndef KIARA_UTILS_DBUFFER_HPP_INCLUDED
#define KIARA_UTILS_DBUFFER_HPP_INCLUDED

#include <KIARA/CDT/kr_dbuffer.h>

namespace KIARA
{

/// Dynamic buffer
class DBuffer
{
public:

    struct dont_free_tag { };

    DBuffer()
    {
        kr_dbuffer_init(&buf_);
    }

    DBuffer(const DBuffer &other)
    {
        kr_dbuffer_init(&buf_);
        assign(other);
    }

    DBuffer(const kr_dbuffer *other)
    {
        kr_dbuffer_init(&buf_);
        assign(other);
    }

    DBuffer(void *data, size_t size, size_t capacity, kr_dbuffer_free_fn free_fn)
    {
        kr_dbuffer_init_from_data(&buf_, data, size, capacity, free_fn);
    }

    DBuffer(void *data, size_t size, size_t capacity, dont_free_tag)
    {
        kr_dbuffer_init_from_data(&buf_, data, size, capacity, kr_dbuffer_dont_free);
    }

    ~DBuffer()
    {
        kr_dbuffer_destroy(&buf_);
    }

    kr_dbuffer_t * get_dbuffer() { return &buf_; }

    const kr_dbuffer_t * get_dbuffer() const { return &buf_; }

    char * begin() { return kr_dbuffer_data(&buf_); }
    char * end() { return kr_dbuffer_data(&buf_) + kr_dbuffer_size(&buf_); }

    const char * begin() const { return kr_dbuffer_data(&buf_); }
    const char * end() const { return kr_dbuffer_data(&buf_) + kr_dbuffer_size(&buf_); }

    DBuffer & operator=(const DBuffer &other)
    {
        assign(other);
        return *this;
    }

    void set(void *data, size_t size, size_t capacity, kr_dbuffer_free_fn free_fn)
    {
        kr_dbuffer_destroy(&buf_);
        kr_dbuffer_init_from_data(&buf_, data, size, capacity, free_fn);
    }

    void set(void *data, size_t size, size_t capacity, dont_free_tag)
    {
        kr_dbuffer_destroy(&buf_);
        kr_dbuffer_init_from_data(&buf_, data, size, capacity, kr_dbuffer_dont_free);
    }

    const char * data() const { return kr_dbuffer_data(&buf_); }
    char * data() { return kr_dbuffer_data(&buf_); }
    size_t size() const { return kr_dbuffer_size(&buf_); }
    size_t capacity() const { return kr_dbuffer_capacity(&buf_); }
    kr_dbuffer_free_fn free_fn() const { return kr_dbuffer_free_fn(&buf_); }

    char * release() { return kr_dbuffer_release(&buf_); }

    bool empty() { return size() == 0; }

    void clear() { kr_dbuffer_clear(&buf_); }

    bool reserve(size_t newCapacity) { return kr_dbuffer_reserve(&buf_, newCapacity); }

    bool resize(size_t newSize) { return kr_dbuffer_resize(&buf_, newSize); }

    /** Resize buffer but don't copy contents when memory needs to be reallocated */
    bool resize_nocopy(size_t newSize) { return kr_dbuffer_resize_nocopy(&buf_, newSize); }

    bool copy_mem(const void *src, size_t n) { return kr_dbuffer_copy_mem(&buf_, src, n); }
    bool append_mem(const void *src, size_t n) { return kr_dbuffer_append_mem(&buf_, src, n); }
    bool append_byte(int byte) { return kr_dbuffer_append_byte(&buf_, byte); }
    bool assign(const kr_dbuffer_t *src) { return kr_dbuffer_assign(&buf_, src); }
    bool assign(const DBuffer &src) { return kr_dbuffer_assign(&buf_, &src.buf_); }
    bool assign(const char *begin, const char *end) { return kr_dbuffer_copy_mem(&buf_, begin, end-begin); }

    bool append(const kr_dbuffer_t *src) { return kr_dbuffer_append(&buf_, src); }
    bool append(const DBuffer &src) { return kr_dbuffer_append(&buf_, &src.buf_); }

    bool make_c_str() { return kr_dbuffer_make_cstr(&buf_); }

    bool move(kr_dbuffer_t *src) { return kr_dbuffer_move(&buf_, src); }
    bool move(DBuffer &src) { return kr_dbuffer_move(&buf_, &src.buf_); }

    void swap(kr_dbuffer *other) { kr_dbuffer_swap(&buf_, other); }
    void swap(DBuffer &other) { kr_dbuffer_swap(&buf_, &other.buf_); }

private:
    kr_dbuffer_t buf_;
};

} // namespace KIARA

#endif /* BUFFER_HPP_ */
