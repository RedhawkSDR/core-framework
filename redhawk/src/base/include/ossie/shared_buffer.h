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

#include <boost/shared_array.hpp>

namespace redhawk {

    /**
     * @brief  Class to adapt an STL-compliant allocator to a deleter.
     */
    template <class Alloc>
    class allocator_deleter : public Alloc {
    public:
        /// @brief  The allocator type (Alloc).
        typedef Alloc allocator_type;

        /**
         * @brief  Construct an %allocator_deleter.
         *
         * @internal  The STL allocator concept defines deallocate as taking
         * both a pointer and a size; unfortunately, Boost smart pointers
         * expect deleters take just a pointer. This class extends from the
         * template parameter, an allocator (which must be copy-constructible),
         * and stores the size for use with deallocate.
         */
        allocator_deleter(size_t size, const allocator_type& alloc=allocator_type()) :
            allocator_type(alloc),
            _M_size(size)
        {
        }

        /**
         * @brief  Deallocates a pointer.
         */
        void operator() (typename allocator_type::pointer ptr)
        {
            this->deallocate(ptr, _M_size);
        }

    private:
        const size_t _M_size;
    };

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
        typedef const value_type* iterator;
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
            _M_array(),
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
            _M_array(data),
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
            _M_array(data, deleter),
            _M_start(data),
            _M_finish(data + size)
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
         * Returns a read-only iterator that points to the first element in the
         * %shared_buffer.
         */
        iterator begin() const
        {
            return iterator(this->_M_begin());
        }

        /**
         * Returns a read-only iterator that points one past the last element
         * in the %shared_buffer.
         */
        iterator end() const
        {
            return iterator(this->_M_end());
        }

        /**
         * @brief  Subscript access to the data contained in the %shared_buffer.
         * @param index The index of the element to access
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
            this->_M_start += start;
            this->_M_finish = this->_M_start + end - start;
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
         * Adapts externally aquired memory to work with the %shared_buffer
         * API; however, additional care must be taken to ensure that the data
         * is copied if it needs to be held past the lifetime of the call.
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
            return !(this->_M_array);
        }

    protected:
        /// @cond IMPL

        // Internal implementation of operator[]; supports const and non-const
        // use.
        value_type& _M_index(size_t index) const
        {
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

        // Internal implementation of swap.
        void _M_swap(shared_buffer& other)
        {
            this->_M_array.swap(other._M_array);
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
            // individual element type (including the boost::shared_array), so
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
        /// @endcond

    private:
        typedef boost::shared_array<value_type> array_type;

        // Disallow swap with any other type (mainly, buffer).
        template <class U>
        void swap(U& other);

        array_type _M_array;
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
        /// @brief  The read-only buffer type.
        typedef shared_buffer<T> read_type;
        /// @brief  The element type (T).
        typedef T value_type;
        /// @brief  A random access iterator to value_type.
        typedef value_type* iterator;
        /// @brief  A random access iterator to const value_type.
        typedef const value_type* const_iterator;
        /// @brief  The default allocator class.
        typedef std::allocator<T> default_allocator;

        /**
         * @brief  Construct an empty %buffer.
         */
        buffer() :
            read_type()
        {
        }

        /**
         * @brief  Construct a %buffer and allocate space.
         * @param size  Number of elements.
         *
         * This constructor allocates memory for @a size elements; no
         * initialization is performed.
         */
        buffer(size_t size) :
            read_type(this->_M_allocate(size, default_allocator()))
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
            read_type(this->_M_allocate(size, allocator))
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
            read_type(data, size)
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
            read_type(data, size, deleter)
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
            return iterator(this->_M_begin());
        }

        /**
         * Returns a read-only iterator that points to the first element in the
         * %buffer.
         */
        const_iterator begin() const
        {
            return const_iterator(this->_M_begin());
        }

        /**
         * Returns a read/write iterator that points one past the last element
         * in the %buffer.
         */
        iterator end()
        {
            return iterator(this->_M_end());
        }

        /**
         * Returns a read-only iterator that points one past the last element
         * in the %buffer.
         */
        const_iterator end() const
        {
            return const_iterator(this->_M_end());
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
            return read_type::size();
        }

        /**
         * Returns true if the %buffer is empty.
         */
        bool empty() const
        {
            return read_type::empty();
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
        read_type slice(size_t start, size_t end=size_t(-1)) const
        {
            return read_type::slice(start, end);
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
         * @brief  Returns a copy of this %buffer.
         *
         * The returned %buffer points to a newly-allocated array.
         */
        buffer copy() const
        {
            return read_type::copy();
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
            return read_type::copy(allocator);
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
            return read_type::template _M_recast<buffer>(other);
        }

    protected:
        /// @cond IMPL

        // Implementation of allocate.
        template <class Alloc>
        static read_type _M_allocate(size_t size, const Alloc& allocator)
        {
            allocator_deleter<Alloc> deleter(size, allocator);
            return read_type(deleter.allocate(size), size, deleter);
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
