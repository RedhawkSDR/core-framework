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

#ifndef REDHAWK_BITBUFFER_H
#define REDHAWK_BITBUFFER_H

#include <stdint.h>
#include <stddef.h>

#include "refcount_memory.h"
#include "bitops.h"

namespace redhawk {

    class bitbuffer;

    class shared_bitbuffer
    {
    public:
        /// @brief  The backing memory data type.
        typedef unsigned char data_type;
        /// @brief  Value used to represent invalid bit indices.
        static const size_t npos = static_cast<size_t>(-1);

        /**
         * Construct an empty shared_bitbuffer.
         */
        shared_bitbuffer();

        /**
         * @brief  Construct a shared_bitbuffer with an existing pointer.
         * @param ptr   Pointer to first byte of bit data.
         * @param size  Number of bits.
         *
         * The newly-created %sharedbit_buffer takes ownership of @a ptr. When
         * the last %shared_buffer pointing to @a ptr is destroyed, @a ptr will
         * be deleted with delete[].
         */
        shared_bitbuffer(data_type* ptr, size_t bits);

        /**
         * @brief  Construct a shared_bitbuffer with an existing pointer and a
         *         custom deleter.
         * @param ptr      Pointer to first byte of bit data.
         * @param size     Number of bits.
         * @param deleter  Callable object.
         *
         * @a D must by copy-constructible. When the last %shared_bitbuffer
         * pointing to @a ptr is destroyed, @a deleter will be called on
         * @a ptr. This can be used to define custom release behavior.
         */
        template <class D>
        shared_bitbuffer(data_type* ptr, size_t bits, D deleter) :
            _M_memory(ptr, _M_bits_to_bytes(bits), deleter),
            _M_base(ptr),
            _M_offset(0),
            _M_size(bits)
        {
        }

        /**
         * @brief  Construct a shared_bitbuffer with an existing pointer known
         *         to be allocated from process-shared memory.
         * @param ptr      Pointer to first byte of bit data.
         * @param size     Number of bits.
         * @param deleter  Callable object.
         * @param tag      Indicates that @a ptr is in process-shared memory.
         *
         * @warning This constructor is intended for internal use only.
         */
        template <class D>
        shared_bitbuffer(data_type* ptr, size_t bits, D deleter, detail::process_shared_tag tag) :
            _M_memory(ptr, _M_bits_to_bytes(bits), deleter, tag),
            _M_base(ptr),
            _M_offset(0),
            _M_size(bits)
        {
        }

        ~shared_bitbuffer();

        shared_bitbuffer& operator=(const shared_bitbuffer& other);

        /**
         * Returns the number of bits.
         */
        size_t size() const;

        /**
         * Returns true if the %shared_bitbuffer is empty.
         */
        bool empty() const;

        /**
         * Returns a read-only pointer to the backing array.
         */
        const data_type* data() const;

        /**
         * @brief  Returns the index of the first bit in the backing array.
         *
         * The offset is always in the range [0, 8). Bits are numbered starting
         * at the most significant bit.
         */
        size_t offset() const;

        /**
         * @brief  Subscript bit access.
         * @param index  The index of the desired bit.
         * @return  The value of the bit (0 or 1).
         */
        int operator[] (size_t pos) const;

        /**
         * @brief  Extracts an integer value.
         * @param pos    Index of first bit.
         * @param bits   Number of bits to extract (max 64).
         * @returns  Integer value.
         * @throw std::out_of_range  If @p pos > size(), or there are fewer than
         *                           @p bits at @p pos.
         * @throw std::length_error  If @p bits is greater than 64.
         * @see redhawk::bitops::getint
         */
        uint64_t getint(size_t pos, size_t bits) const;

        /**
         * @brief  Returns a %shared_bitbuffer containing a subset of bits.
         * @param start  Index of first bit.
         * @param end  Index of last bit, exclusive (default end).
         * @throw std::out_of_range  If @p start > size().
         * @return  The new %shared_bitbuffer.
         *
         * If @a end is past the end of the bit buffer, the slice extends to
         * the end of this bit buffer.
         */
        shared_bitbuffer slice(size_t start, size_t end=npos) const;

        /**
         * @brief  Adjusts the start and end bits of this %shared_bitbuffer.
         * @param start  Index of first bit.
         * @param end  Index of last bit, exclusive (default end).
         * @throw std::out_of_range  If @p start > size().
         *
         * If @a end is past the end of the bit buffer, the end index is
         * unchanged.
         */
        void trim(size_t start, size_t end=npos);

        /**
         * @brief  Copies this bit buffer.
         * @returns  A new bit buffer with its own copy of the backing array.
         */
        bitbuffer copy() const;

        /**
         * @brief  Copies this bit buffer.
         * @param allocator  STL-compliant allocator.
         * @returns  A new bit buffer with its own copy of the backing array.
         *
         * The new bit buffer's backing array is allocated with a copy of
         * @a allocator.
         */
        template <class Alloc>
        bitbuffer copy(const Alloc& allocator) const;

        /**
         * @brief  Swap contents with another bit buffer.
         * @param other  Bit buffer to swap with.
         */
        void swap(shared_bitbuffer& other);

        /**
         * Returns the population count (number of 1's in the bit buffer).
         */
        int popcount() const;

        /**
         * @brief  Finds a pattern in this bit buffer within a maximum Hamming
         *         distance.
         * @param pattern  Bit pattern to search for.
         * @param maxDistance  Maximum allowable Hamming distance.
         * @returns  Bit index of first occurence of @p pattern.
         *
         * Searches forward for a position at which the Hamming distance
         * between this bit buffer and @a pattern is less than or equal to
         * @a maxDistance. If found, returns the bit index at which the match
         * occurs. If not found, returns npos.
         */
        size_t find(const shared_bitbuffer& pattern, int maxDistance) const
        {
            return find(0, pattern, maxDistance);
        }

        /**
         * @brief  Finds a pattern in this bit buffer within a maximum Hamming
         *         distance.
         * @param start  Starting bit index.
         * @param pattern  Bit pattern to search for.
         * @param maxDistance  Maximum allowable Hamming distance.
         * @returns  Bit index of first occurence of @p pattern.
         *
         * Starting from @a start, searches forward for a position at which the
         * Hamming distance between this bit buffer and @a pattern is less than
         * or equal to @a maxDistance. If found, returns the bit index at which
         * the match occurs. If not found, returns npos.
         */
        size_t find(size_t start, const shared_bitbuffer& pattern, int maxDistance) const;

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
         * @brief  Returns a transient %shared_bitbuffer.
         * @param data  Pointer to first byte.
         * @param size  Number of bits.
         * @see make_transient(const data_type*,size_t)
         */
        static inline shared_bitbuffer make_transient(const data_type* data, size_t bits)
        {
            return make_transient(data, 0, bits);
        }

        /**
         * @brief  Returns a transient %shared_bitbuffer.
         * @param data   Pointer to first byte.
         * @param start  Index of first bit.
         * @param size   Number of bits.
         *
         * Adapts externally managed memory to work with the %shared_bitbuffer
         * API; however, additional care must be taken to ensure that the data
         * is copied if it needs to be held past the lifetime of the transient
         * %shared_bitbuffer.
         */
        static shared_bitbuffer make_transient(const data_type* data, size_t start, size_t bits);

        /**
         * @brief  Returns true if the bit array's lifetime is not managed.
         *
         * Transient shared_bitbuffers do not own the underlying data. If the
         * receiver of a transient bit buffer needs to hold on to it past the
         * lifetime of the call, they must make a copy.
         */
        bool transient() const
        {
            return !(this->_M_memory);
        }

    protected:
        template <class Alloc>
        shared_bitbuffer(size_t bits, const Alloc& allocator) :
            _M_memory(_M_bits_to_bytes(bits), allocator),
            _M_base((data_type*) _M_memory.address()),
            _M_offset(0),
            _M_size(bits)
        {
        }

        static inline size_t _M_bits_to_bytes(size_t bits)
        {
            return (bits + 7) / 8;
        }

        void _M_copy(bitbuffer& dest) const;

        void _M_normalize();
        void _M_swap(shared_bitbuffer& other);

        void _M_check_index(size_t pos, const char* name) const;

        refcount_memory _M_memory;
        data_type* _M_base;

    private:
        // Disallow swap with bitbuffer.
        void swap(bitbuffer& other);

        size_t _M_offset;
        size_t _M_size;
    };

    class bitbuffer : public shared_bitbuffer
    {
    private:
        class reference {
        public:
            reference(data_type* data, size_t pos);
            operator int () const;
            reference& operator= (bool);
            reference& operator= (const reference& other);
        private:
            data_type* _M_data;
            size_t _M_pos;
        };

    public:
        typedef std::allocator<data_type> default_allocator;

        bitbuffer();

        static inline bitbuffer from_int(uint64_t value, size_t bits)
        {
            bitbuffer result(bits);
            result.setint(0, value, bits);
            return result;
        }

        static inline bitbuffer from_array(const data_type* data, size_t bits)
        {
            return from_array(data, 0, bits);
        }

        static inline bitbuffer from_array(const data_type* data, size_t start, size_t bits)
        {
            return shared_bitbuffer::make_transient(data, start, bits).copy();
        }

        explicit bitbuffer(size_t bits) :
            shared_bitbuffer(bits, default_allocator())
        {
        }

        template <class Alloc>
        bitbuffer(size_t bits, const Alloc& allocator) :
            shared_bitbuffer(bits, allocator)
        {
        }

        using shared_bitbuffer::data;

        /**
         * Returns a read/write pointer to the backing array.
         */
        data_type* data();

        using shared_bitbuffer::operator[];
        reference operator[] (size_t pos);

        /**
         * @brief  Inserts an integer value.
         * @param pos    Index of first bit.
         * @param value  Integer value to set.
         * @param bits   Number of bits in @p value (max 64).
         * @throw std::out_of_range  If @p pos > size(), or there are fewer than
         *                           @p bits at @p pos.
         * @throw std::length_error  If @p bits is greater than 64.
         * @see redhawk::bitops::setint
         */
        void setint(size_t pos, uint64_t value, size_t bits);

        using shared_bitbuffer::slice;

        /**
         * @brief  Returns a %bitbuffer containing a subset of bits.
         * @param start  Index of first bit.
         * @param end  Index of last bit, exclusive (default end).
         * @return  The new %bitbuffer.
         */
        bitbuffer slice(size_t start, size_t end=npos);

        /**
         * @brief  Fills the bit buffer with a value.
         * @param value  Bit value to set.
         *
         * Sets all bits to @a value.
         */
        void fill(bool value)
        {
            fill(0, size(), value);
        }

        /**
         * @brief  Fills a range of bits with a value.
         * @param start  Index of first bit.
         * @param end    Index of last bit, exclusive.
         * @param value  Bit value to set.
         *
         * Sets the bits in the range [@a start, @a end) to @a value.
         */
        void fill(size_t start, size_t end, bool value);

        void resize(size_t bits)
        {
            bitbuffer temp(bits);
            _M_resize(temp);
        }

        template <class Alloc>
        void resize(size_t bits, const Alloc& allocator)
        {
            bitbuffer temp(bits, allocator);
            _M_resize(temp);
        }

        /**
         * @brief  Replaces bits.
         * @param pos   Index of first bit to replace.
         * @param bits  Number of bits to replace.
         * @param src   The bit buffer to insert.
         * @throw std::out_of_range  If @p pos > size(), or there are fewer
         *                           than @p bits at @p pos.
         *
         * Starting at @a pos, replaces existing bit values with the bit values
         * in @a src.
         */
        inline void replace(size_t pos, size_t bits, const shared_bitbuffer& src)
        {
            replace(pos, bits, src, 0);
        }

        /**
         * @brief  Replaces bits.
         * @param pos     Index of first bit to replace.
         * @param bits    Number of bits to replace.
         * @param src     The bit buffer to insert.
         * @param srcpos  Index of first bit in @p src.
         * @throw std::out_of_range  If @p pos > size(), there are fewer than
         *                           @p bits at @p pos, or
         *                           @p srcpos > @p src.size().
         *
         * Starting at @a pos, replaces existing bit values with the bit values
         * starting at @a srcpos in @a src.
         */
        void replace(size_t pos, size_t bits, const shared_bitbuffer& src, size_t srcpos);

        /**
         * @brief  Swap contents with another bit buffer.
         * @param other  Bit buffer to swap with.
         */
        void swap(bitbuffer& other);

    private:
        void _M_resize(bitbuffer& dest);
    };

    template <class Alloc>
    bitbuffer shared_bitbuffer::copy(const Alloc& allocator) const
    {
        bitbuffer result(size(), allocator);
        this->_M_copy(result);
        return result;
    }

    /**
     * @brief  Bit buffer equality comparison.
     * @param  lhs  First bit buffer.
     * @param  rhs  Second bit buffer.
     * @return  True iff the size and bits of the bit buffers are equal.
     */
    bool operator==(const shared_bitbuffer& lhs, const shared_bitbuffer& rhs);

    /**
     * @brief  Bit buffer inequality comparison.
     * @param  lhs  First bit buffer.
     * @param  rhs  Second bit buffer.
     * @return  True iff the size or bits of the bit buffers are not equal.
     */
    bool operator!=(const shared_bitbuffer& lhs, const shared_bitbuffer& rhs);
}

#endif // REDHAWK_BITBUFFER_H
