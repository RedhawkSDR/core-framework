/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
 *
 * REDHAWK core is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef REDHAWK_SHARED_BUFFER_H
#define REDHAWK_SHARED_BUFFER_H

#include <algorithm>
#include <memory>

#include "refcount_memory.h"
#include "shm/Allocator.h"

#ifdef _RH_SHARED_BUFFER_DEBUG
#include "debug/check.h"
#include "debug/checked_allocator.h"
#include "debug/checked_iterator.h"

#define _RH_SHARED_BUFFER_CHECK(X) _RH_DEBUG_CHECK(X)
#else // !_RH_SHARED_BUFFER_DEBUG
#define _RH_SHARED_BUFFER_CHECK(X)
#endif

namespace redhawk {

    // Forward declaration of read/write buffer class.
    template <typename T>
    class buffer;

    /**
     * @brief  An immutable container that can share its data with other
     *         instances.
     *
     * The %shared_buffer class provides read-only access to a sized array of
     * elements that can be shared between many buffer instances. This enables
     * the transfer of ownership of data without explicit management of
     * references.
     *
     * shared_buffers have reference semantics. Assignment and copy construction
     * do not copy any elements, only the data pointer. A %shared_buffer never
     * peforms any memory allocation of its own, but can take ownership of an
     * existing array. When the last reference to the underlying data goes
     * away, the data is freed.
     *
     * For write access and memory allocation, see buffer.
     */
    template <typename T>
    class shared_buffer {
    public:
        /// @brief The element type (T).
        typedef T value_type;
        /**
         * @brief A random access iterator to const value_type.
         *
         * Note that all access to %shared_buffer is const. This means that
         * %iterator and %const_iterator are equivalent.
         */
#ifdef _RH_SHARED_BUFFER_DEBUG
        typedef ::redhawk::debug::checked_iterator<const value_type*,shared_buffer> iterator;
#else
        typedef const value_type* iterator;
#endif
        /**
         * @brief A random access iterator to const value_type.
         *
         * Note that all access to %shared_buffer is const. This means that
         * %iterator and %const_iterator are equivalent.
         */
        typedef iterator const_iterator;

        /**
         * @brief  Construct an empty %shared_buffer.
         */
        shared_buffer() :
            _M_memory(),
            _M_start(0),
            _M_finish(0)
        {
        }

        /**
         * @brief  Construct a %shared_buffer with an existing pointer.
         * @param data  Pointer to first element.
         * @param size  Number of elements.
         *
         * The newly-created %shared_buffer takes ownership of @a data. When
         * the last %shared_buffer pointing to @a data is destroyed, @a data
         * will be deleted with delete[].
         */
        shared_buffer(value_type* data, size_t size) :
            _M_memory(data, size),
            _M_start(data),
            _M_finish(data + size)
        {
        }

        /**
         * @brief  Construct a %shared_buffer with an existing pointer and a
         *         custom deleter.
         * @param data  Pointer to first element.
         * @param size  Number of elements.
         * @param deleter  Callable object.
         *
         * @a D must by copy-constructible. When the last %shared_buffer
         * pointing to @a data is destroyed, @a deleter will be called on
         * @a data. This can be used to define custom release behavior.
         */
        template <class D>
        shared_buffer(value_type* data, size_t size, D deleter) :
            _M_memory(data, size, deleter),
            _M_start(data),
            _M_finish(data + size)
        {
        }

        /**
         * @brief  Construct a %shared_buffer with an existing pointer known to
         *         be allocated from process-shared memory.
         * @param data  Pointer to first element.
         * @param size  Number of elements.
         * @param deleter  Callable object.
         * @param tag  Indicates that @a data is in process-shared memory.
         *
         * @warning This constructor is intended for internal use only.
         */
        template <class D>
        shared_buffer(value_type* data, size_t size, D deleter, detail::process_shared_tag tag) :
            _M_memory(data, size, deleter, tag),
            _M_start(data),
            _M_finish(data + size)
        {
        }

        /**
         * @brief  %shared_buffer copy constructor.
         * @param other  A %shared_buffer of identical element type.
         *
         * %shared_buffer has reference semantics; after construction, this
         * instance shares the underlying data, increasing its reference count
         * by one.
         */
        shared_buffer(const shared_buffer& other) :
            _M_memory(other._M_memory),
            _M_start(other._M_start),
            _M_finish(other._M_finish)
        {
        }

        /**
         * The dtor releases the %shared_buffer's shared data pointer. If no
         * other %shared_buffer points to the data, the data is released.
         */
        ~shared_buffer()
        {
        }

        /**
         * @brief  %shared_buffer assignment operator.
         * @param other  A %shared_buffer of identical element type.
         *
         * %shared_buffer has reference semantics; after assignment, this
         * instance shares the underlying data, increasing its reference count
         * by one. The prior data pointer is released, deleting the data if
         * this was the last reference. 
         */
        shared_buffer& operator=(const shared_buffer& other)
        {
            // Use copy constructor and swap to handle reference count
            shared_buffer temp(other);
            this->swap(temp);
            return *this;
        }

        /**
         * Returns a read-only iterator that points to the first element in the
         * %shared_buffer.
         */
        iterator begin() const
        {
            return _M_iterator(this->_M_begin());
        }

        /**
         * Returns a read-only iterator that points one past the last element
         * in the %shared_buffer.
         */
        iterator end() const
        {
            return _M_iterator(this->_M_end());
        }

        /**
         * @brief  Subscript access to the data contained in the %shared_buffer.
         * @param index  The index of the element to access
         * @return  Read-only reference to element
         */
        const value_type& operator[] (size_t index) const
        {
            return this->_M_index(index);
        }

        /**
         * Returns the number of elements in the %shared_buffer.
         */
        size_t size() const
        {
            return size_t(this->_M_finish - this->_M_start);
        }

        /**
         * Returns true if the %shared_buffer is empty.
         */
        bool empty() const
        {
            return this->begin() == this->end();
        }

        /**
         * Returns a reference to the backing memory object.
         *
         * @warning This method is intended for internal use only.
         */
        const refcount_memory& get_memory() const
        {
            return _M_memory;
        }

        /**
         * Returns a read-only pointer to the first element.
         */
        const value_type* data() const
        {
            return this->_M_start;
        }

        /**
         * @brief  Returns a %shared_buffer containing a subset of elements.
         * @param start  Index of first element.
         * @param end  Index of last element, exclusive (default end).
         * @return  The new %shared_buffer.
         */
        shared_buffer slice(size_t start, size_t end=size_t(-1)) const
        {
            shared_buffer result(*this);
            result.trim(start, end);
            return result;
        }

        /**
         * @brief  Adjusts the start and end indices of this %shared_buffer.
         * @param start  Index of first element.
         * @param end  Index of last element, exclusive (default end).
         */
        void trim(size_t start, size_t end=size_t(-1))
        {
            if (end == (size_t)-1) {
                end = this->size();
            }
            this->_M_trim(this->_M_begin() + start, this->_M_begin() + end);
        }

        /**
         * @brief  Adjusts the beginning of this %shared_buffer.
         * @param start  Iterator to first element.
         */
        void trim(iterator first)
        {
            this->_M_trim(first, end());
        }

        /**
         * @brief  Adjusts the beginning and end of this %shared_buffer.
         * @param start  Iterator to first element.
         * @param end  Iterator to last element, exclusive.
         */
        void trim(iterator first, iterator last)
        {
            this->_M_trim(first, last);
        }

        /**
         * @brief  Returns a copy of this %shared_buffer.
         *
         * The returned %buffer points to a newly-allocated array.
         */
        buffer<T> copy() const
        {
            buffer<T> result(this->size());
            this->_M_copy(result);
            return result;
        }

        /**
         * @brief  Returns a copy of this %shared_buffer.
         * @param allocator  An allocator object.
         *
         * The returned %buffer points to a new array allocated with
         * @a allocator. @a allocator must be copy-constructible.
         */
        template <class Alloc>
        buffer<T> copy(const Alloc& allocator) const
        {
            buffer<T> result(this->size(), allocator);
            this->_M_copy(result);
            return result;
        }

        /**
         * @brief  Swap contents with another %shared_buffer.
         * @param other  %shared_buffer to swap with.
         */
        void swap(shared_buffer& other)
        {
            this->_M_swap(other);
        }

        /**
         * @brief  Reinterpret a shared_buffer as another data type.
         * @param other  %shared_buffer to reinterpret.
         * @return  The new %shared_buffer.
         *
         * Data is reinterpreted by standard C++ reinterpret_cast semantics.
         * The size of the new %shared_buffer is the floor of the size of
         * @a other multiplied by the ratio sizeof(U)/sizeof(T).
         */
        template <typename U>
        static shared_buffer recast(const shared_buffer<U>& other)
        {
            return _M_recast<shared_buffer>(other);
        }

        /**
         * @brief  Returns a transient %shared_buffer.
         * @param data  Pointer to first element.
         * @param size  Number of elements.
         *
         * Adapts externally managed memory to work with the %shared_buffer
         * API; however, additional care must be taken to ensure that the data
         * is copied if it needs to be held past the lifetime of the transient
         * %shared_buffer.
         */
        static shared_buffer make_transient(const value_type* data, size_t size)
        {
            shared_buffer result;
            result._M_start = const_cast<value_type*>(data);
            result._M_finish = result._M_start + size;
            return result;
        }

        /**
         * @brief  Returns true if the array's lifetime is not managed.
         *
         * Transient shared_buffers do not own the underlying data. If the
         * receiver of a transient buffer needs to hold on to it past the
         * lifetime of the call, they must make a copy.
         */
        bool transient() const
        {
            return !(this->_M_memory);
        }

    protected:
        /// @cond IMPL

        // Internal implementation of operator[]; supports const and non-const
        // use.
        value_type& _M_index(size_t index) const
        {
            _RH_SHARED_BUFFER_CHECK(index < this->size());
            return this->_M_start[index];
        }

        // Internal implementation of begin; supports const and non-const use.
        value_type* _M_begin() const
        {
            return this->_M_start;
        }

        // Internal implementation of end; supports const and non-const use.
        value_type* _M_end() const
        {
            return this->_M_finish;
        }

        // Converts a pointer into an iterator; if debug is enabled, the ctor
        // requires an additional parameter (a pointer back to the originating
        // conatiner), otherwise, it's a no-op
        inline iterator _M_iterator(value_type* iter) const
        {
#ifdef _RH_SHARED_BUFFER_DEBUG
            return iterator(iter, this);
#else
            return iterator(iter);
#endif
        }

        // Internal implementation of trim to support checked iterators, which
        // are different classes for shared_buffer and buffer
        void _M_trim(const value_type* first, const value_type* last)
        {
            _RH_SHARED_BUFFER_CHECK(last >= first);
            _RH_SHARED_BUFFER_CHECK(last <= this->_M_finish);
            this->_M_start = const_cast<value_type*>(first);
            this->_M_finish = const_cast<value_type*>(last);
        }

        // Internal implementation of swap.
        void _M_swap(shared_buffer& other)
        {
            this->_M_memory.swap(other._M_memory);
            std::swap(this->_M_start, other._M_start);
            std::swap(this->_M_finish, other._M_finish);
        }

        // Internal implementation of copy. Copies the contents of this buffer
        // into another, pre-existing buffer.
        void _M_copy(shared_buffer& dest) const
        {
            std::copy(this->begin(), this->end(), dest._M_begin());
        }

        // Implementation of recast. The output type is a template parameter so
        // that buffer can use this method as well.
        template <class Tout, class U>
        static Tout _M_recast(const U& other)
        {
            // Reinterpret the input class (which is some form of shared_buffer
            // or buffer) via a void pointer so that the compiler doesn't
            // object about strict aliasing rules, which shouldn't matter here.
            // The in-memory layout is always the same irrespective of the
            // individual element type (including the data implementation), so
            // the important effects of the copy constructor like reference
            // counting will still occur.
            const void* ptr = &other;
            Tout result(*reinterpret_cast<const Tout*>(ptr));
            // Truncate any extra bytes from the end of the array so that the
            // end pointer is at an integral offset from the start (otherwise
            // iterators might not meet). This may be a concern when the output
            // type is larger than the input type; e.g., 9 floats yields 4
            // complex floats, with 1 float "lost". Note that size() already
            // truncates the number of elements for us.
            result._M_finish = result._M_start + result.size();
            return result;
        }

        // Internal constructor to allow buffer to request a number of elements
        // and an allocator type for the internal buffer to do an allocation.
        template <typename Alloc>
        shared_buffer(size_t count, const Alloc& allocator) :
            _M_memory(count, allocator),
            _M_start((value_type*) _M_memory.address()),
            _M_finish(_M_start + count)
        {
        }
        /// @endcond

    private:
        // Disallow swap with any other type (mainly, buffer).
        template <class U>
        void swap(U& other);

        refcount_memory _M_memory;
        value_type* _M_start;
        value_type* _M_finish;
    };


    /**
     * @brief  A shared container data type.
     *
     * The %buffer class extends shared_buffer to provides write access.
     * Multiple buffers and shared_buffers may point to the same underlying
     * data.
     *
     * buffers have reference semantics. Assignment and copy construction do
     * not copy any elements, only the data pointer.
     *
     * Unlike %shared_buffer, %buffer has allocating constructors. When the
     * last reference to the underlying data goes away, the data is freed.
     */ 
    template <class T>
    class buffer : public shared_buffer<T>
    {
    public:
        /// @brief  The equivalent shared buffer type.
        typedef shared_buffer<T> shared_type;
        /// @brief  The element type (T).
        typedef T value_type;
        /// @brief  A random access iterator to value_type.
#ifdef _RH_SHARED_BUFFER_DEBUG
        typedef ::redhawk::debug::checked_iterator<value_type*,buffer> iterator;
#else
        typedef value_type* iterator;
#endif
        /// @brief  A random access iterator to const value_type.
#ifdef _RH_SHARED_BUFFER_DEBUG
        typedef ::redhawk::debug::checked_iterator<const value_type*,buffer> const_iterator;
#else
        typedef const value_type* const_iterator;
#endif
        /// @brief  The default allocator class.
#ifdef _RH_SHARED_BUFFER_DEBUG
        typedef ::redhawk::debug::checked_allocator<T> default_allocator;
#elif defined(_RH_SHARED_BUFFER_USE_STD_ALLOC)
        typedef std::allocator<T> default_allocator;
#else
        typedef ::redhawk::shm::HybridAllocator<T> default_allocator;
#endif

        /**
         * @brief  Construct an empty %buffer.
         */
        buffer() :
            shared_type()
        {
        }

        /**
         * @brief  Construct a %buffer and allocate space.
         * @param size  Number of elements.
         *
         * This constructor allocates memory for @a size elements; no
         * initialization is performed.
         */
        explicit buffer(size_t size) :
            shared_type(size, default_allocator())
        {
        }

        /**
         * @brief  Construct a %buffer and allocate space.
         * @param size  Number of elements.
         * @param allocator  An allocator.
         *
         * This constructor allocates memory for @a size elements using
         * @a allocator; no initialization is performed. @a allocator must be
         * copy-constructible.
         */
        template <class Alloc>
        buffer(size_t size, const Alloc& allocator) :
            shared_type(size, allocator)
        {
        }

        /**
         * @brief  Construct a %buffer with an existing pointer.
         * @param data  Pointer to first element.
         * @param size  Number of elements.
         *
         * The newly-created %buffer takes ownership of @a data. When the last
         * %buffer pointing to @a data is destroyed, @a data will be deleted
         * with delete[].
         */
        buffer(value_type* data, size_t size) :
            shared_type(data, size)
        {
        }

        /**
         * @brief  Construct a %buffer with an existing pointer and a custom
         *         deleter.
         * @param data  Pointer to first element.
         * @param size  Number of elements.
         * @param deleter  Callable object.
         *
         * @a D must be copy-constructible. When the last %buffer pointing to
         * @a data is destroyed, @a deleter will be called on @a data. This can
         * be used to define custom release behavior.
         */
        template <class D>
        buffer(value_type* data, size_t size, D deleter) :
            shared_type(data, size, deleter)
        {
        }

        /**
         * @brief  Construct a %buffer with an existing pointer known to be
         *         allocated from process-shared memory.
         * @param data  Pointer to first element.
         * @param size  Number of elements.
         * @param deleter  Callable object.
         * @param tag  Indicates that @a data is in process-shared memory.
         *
         * @warning This constructor is intended for internal use only.
         */
        template <class D>
        buffer(value_type* data, size_t size, D deleter, detail::process_shared_tag tag) :
            shared_type(data, size, deleter, tag)
        {
        }

        /**
         * The dtor releases the %buffer's shared data pointer. If no other
         * buffers point to the data, the data is released.
         */
        ~buffer()
        {
        }

        /**
         * Returns a read/write iterator that points to the first element in
         * the %buffer.
         */
        iterator begin()
        {
            return _M_iterator(this->_M_begin());
        }

        /**
         * Returns a read-only iterator that points to the first element in the
         * %buffer.
         */
        const_iterator begin() const
        {
            return _M_const_iterator(this->_M_begin());
        }

        /**
         * Returns a read/write iterator that points one past the last element
         * in the %buffer.
         */
        iterator end()
        {
            return _M_iterator(this->_M_end());
        }

        /**
         * Returns a read-only iterator that points one past the last element
         * in the %buffer.
         */
        const_iterator end() const
        {
            return _M_const_iterator(this->_M_end());
        }

        /**
         * @brief  Subscript access to the data contained in the %buffer.
         * @param index The index of the element to access
         * @return  Read/write reference to element
         */
        value_type& operator[] (size_t index)
        {
            return this->_M_index(index);
        }

        /**
         * @brief  Subscript access to the data contained in the %buffer.
         * @param index The index of the element to access
         * @return  Read-only reference to element
         */
        const value_type& operator[] (size_t index) const
        {
            return this->_M_index(index);
        }

        /**
         * Returns the number of elements in the %buffer.
         */
        size_t size() const
        {
            return shared_type::size();
        }

        /**
         * Returns true if the %buffer is empty.
         */
        bool empty() const
        {
            return shared_type::empty();
        }

        /**
         * Returns a read-only pointer to the first element.
         */
        const value_type* data() const
        {
            return this->_M_begin();
        }

        /**
         * Returns a read/write pointer to the first element.
         */
        value_type* data()
        {
            return this->_M_begin();
        }

        /**
         * @brief  Returns a %shared_buffer containing a subset of elements.
         * @param start  Index of first element.
         * @param end  Index of last element, exclusive (default end).
         * @return  The new %shared_buffer.
         */
        shared_type slice(size_t start, size_t end=size_t(-1)) const
        {
            return shared_type::slice(start, end);
        }

        /**
         * @brief  Returns a %buffer containing a subset of elements.
         * @param start  Index of first element.
         * @param end  Index of last element, exclusive (default end).
         * @return  The new %buffer.
         */
        buffer slice(size_t start, size_t end=size_t(-1))
        {
            buffer result(*this);
            result.trim(start, end);
            return result;
        }

        /**
         * @brief  Adjusts the start and end indices of this %buffer.
         * @param start  Index of first element.
         * @param end  Index of last element, exclusive (default end).
         */
        void trim(size_t start, size_t end=size_t(-1))
        {
            shared_type::trim(start, end);
        }

        /**
         * @brief  Adjusts the beginning of this %buffer.
         * @param start  Iterator to first element.
         */
        void trim(iterator first)
        {
            this->_M_trim(first, end());
        }

        /**
         * @brief  Adjusts the beginning and end of this %buffer.
         * @param start  Iterator to first element.
         * @param end  Iterator to last element, exclusive (default end).
         */
        void trim(iterator first, iterator last)
        {
            this->_M_trim(first, last);
        }

        /**
         * @brief  Resizes this %buffer.
         * @param size  Number of elements.
         *
         * Allocates new memory using the default allocator. Existing values
         * are copied to the new memory.
         */
        void resize(size_t size)
        {
            buffer temp(size);
            _M_resize(temp);
        }

        /**
         * @brief  Resizes this %buffer.
         * @param size  Number of elements.
         * @param allocator  STL-compliant allocator.
         *
         * Allocates new memory using @a allocator. Existing values are copied
         * to the new memory.
         */
        template <class Alloc>
        void resize(size_t size, Alloc& allocator)
        {
            buffer temp(size, allocator);
            _M_resize(temp);
        }

        /**
         * @brief  Replaces the contents of this %buffer.
         * @param pos  Index of first element to be replaced.
         * @param len  Number of elements to replace.
         * @param src  A shared_buffer from which to copy elements.
         */
        void replace(size_t pos, size_t len, const shared_type& src)
        {
            replace(pos, len, src, 0);
        }

        /**
         * @brief  Replaces the contents of this %buffer.
         * @param pos  Index of first element to be replaced.
         * @param len  Number of elements to replace.
         * @param src  A shared_buffer from which to copy elements.
         * @param srcpos  Index of first element in @p src to copy.
         */
        void replace(size_t pos, size_t len, const shared_type& src, size_t srcpos)
        {
            typename shared_type::const_iterator start = src.begin() + srcpos;
            std::copy(start, start + len, begin() + pos);
        }

        /**
         * @brief  Returns a copy of this %buffer.
         *
         * The returned %buffer points to a newly-allocated array.
         */
        buffer copy() const
        {
            return shared_type::copy();
        }

        /**
         * @brief  Returns a copy of this %buffer.
         * @param allocator  An allocator object.
         *
         * The returned %buffer points to a new array allocated with
         * @a allocator. @a allocator must be copy-constructible.
         */
        template <class Alloc>
        buffer copy(const Alloc& allocator) const
        {
            return shared_type::copy(allocator);
        }

        /**
         * @brief  Swap contents with another %buffer.
         * @param other  %buffer to swap with.
         */
        void swap(buffer& other)
        {
            this->_M_swap(other);
        }

        /**
         * @brief  Reinterpret a %buffer as another data type.
         * @param other  %buffer to reinterpret.
         * @return  The new %buffer.
         *
         * Data is reinterpreted by standard C++ reinterpret_cast semantics.
         * The size of the new %buffer is the floor of the size of @a other
         * multiplied by the ratio sizeof(U)/sizeof(T).
         */
        template <typename U>
        static buffer recast(const buffer<U>& other)
        {
            // Use the base class' template method implementation, with this
            // type as the first template parameter (the second is deduced from
            // the argument).
            return shared_type::template _M_recast<buffer>(other);
        }

    protected:
        /// @cond IMPL

        // Converts a pointer into an iterator (see shared_buffer::_M_iterator
        // for more explanation)
        inline iterator _M_iterator(value_type* iter)
        {
#ifdef _RH_SHARED_BUFFER_DEBUG
            return iterator(iter, this);
#else
            return iterator(iter);
#endif
        }

        // Converts a const pointer into an const_iterator (see above)
        inline const_iterator _M_const_iterator(const value_type* iter) const
        {
#ifdef _RH_SHARED_BUFFER_DEBUG
            return const_iterator(iter, this);
#else
            return const_iterator(iter);
#endif
        }

        // Copies existing elements into a destination buffer then swaps the
        // buffers, as used to perform a resize. The allocation is done inside
        // the public method to ensure that the correct allocator template is
        // used.
        inline void _M_resize(buffer& dest)
        {
            size_t count = std::min(size(), dest.size());
            std::copy(begin(), begin() + count, dest.begin());
            this->swap(dest);
        }
        /// @endcond
    };

    /**
     * @brief  Buffer equality comparison.
     * @param  lhs  A %shared_buffer.
     * @param  rhs  A %shared_buffer of the same type as @a lhs.
     * @return  True iff the size and elements of the shared_buffers are equal.
     */
    template <typename T>
    inline bool operator==(const shared_buffer<T>& lhs, const shared_buffer<T>& rhs)
    {
        if (lhs.size() != rhs.size()) {
            // Different sizes always compare unequal
            return false;
        } else if (lhs.data() == rhs.data()) {
            // If the data pointer is the same (the size is already known to be
            // the same), no further comparison is required
            return true;
        } else {
            // Perform element-wise comparison
            return std::equal(lhs.begin(), lhs.end(), rhs.begin());
        }
    }

    /**
     * @brief  Buffer inequality comparison.
     * @param  lhs  A %shared_buffer.
     * @param  rhs  A %shared_buffer of the same type as @a lhs.
     * @return  True iff the size or elements of the shared_buffers are not equal.
     */
    template <typename T>
    inline bool operator!=(const shared_buffer<T>& lhs, const shared_buffer<T>& rhs)
    {
        return !(lhs == rhs);
    }

    /**
     * @brief  A convenience wrapper for creating a buffer.
     * @param data  Pointer to first element.
     * @param size  Number of elements.
     * @return  A newly-constructed buffer<> of the appropriate type.
     */
    template <class T>
    inline redhawk::buffer<T> make_buffer(T* data, size_t size)
    {
        return redhawk::buffer<T>(data, size);
    }

    /**
     * @brief  A convenience wrapper for creating a buffer with a custom deleter.
     * @param data  Pointer to first element.
     * @param size  Number of elements.
     * @param deleter  Callable object.
     * @return  A newly-constructed buffer<> of the appropriate type.
     */
    template <class T, class D>
    inline redhawk::buffer<T> make_buffer(T* data, size_t size, D deleter)
    {
        return redhawk::buffer<T>(data, size, deleter);
    }

} // namespace redhawk

#endif // REDHAWK_SHARED_BUFFER_H
