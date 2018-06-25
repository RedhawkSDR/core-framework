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

    // Forward declaration of read/write bitbuffer class.
    class bitbuffer;

    /**
     * @brief  An immutable, shared container for working with bit data.
     *
     * The %shared_bitbuffer class provides read-only bit-level access to a
     * backing array of byte data that can be shared between many bitbuffer
     * instances. This enables the transfer of ownership of data without
     * explicit management of references.
     *
     * shared_bitbuffers have reference semantics. Assignment and copy
     * construction do not copy any bits, only the data pointer. A
     * %shared_bitbuffer never peforms any memory allocation of its own, but
     * can take ownership of an existing array. When the last reference to the
     * backing array goes away, the backing array is freed.
     *
     * For write access and memory allocation, see bitbuffer.
     */
    class shared_bitbuffer
    {
    public:
        /// @brief  The backing memory data type.
        typedef unsigned char data_type;
        /// @brief  Value used to represent invalid bit indices.
        static const size_t npos = static_cast<size_t>(-1);

        /**
         * Construct an empty %shared_bitbuffer.
         */
        shared_bitbuffer();

        /**
         * @brief  Construct a %shared_bitbuffer with an existing pointer.
         * @param ptr   Pointer to first byte of bit data.
         * @param size  Number of bits.
         *
         * The newly-created %sharedbit_buffer takes ownership of @a ptr. When
         * the last %shared_buffer pointing to @a ptr is destroyed, @a ptr will
         * be deleted with delete[].
         */
        shared_bitbuffer(data_type* ptr, size_t bits);

        /**
         * @brief  Construct a %shared_bitbuffer with an existing pointer and a
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
         * @brief  Construct a %shared_bitbuffer with an existing pointer known
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
         * @brief  Subscript read access to a bit.
         * @param index  The index of the desired bit.
         * @return  The value of the bit (0 or 1).
         */
        int operator[] (size_t pos) const;

        /**
         * @brief  Extracts an integer value.
         * @param pos    Index of first bit.
         * @param bits   Number of bits to extract (max 64).
         * @return  Integer value.
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
         * @return  The new %shared_bitbuffer.
         * @throw std::out_of_range  If @p start > size().
         * @throw std::invalid_argument  If @p end < @p start.
         *
         * The new %shared_bitbuffer shares the same backing array. If @a end
         * is past the last bit, the slice extends to the last bit.
         */
        shared_bitbuffer slice(size_t start, size_t end=npos) const;

        /**
         * @brief  Adjusts the start and end bits.
         * @param start  Index of first bit.
         * @param end  Index of last bit, exclusive (default end).
         * @throw std::out_of_range      If @p start > size().
         * @throw std::invalid_argument  If @p end < @p start.
         *
         * If @a end is past the last bit, the end index is unchanged.
         */
        void trim(size_t start, size_t end=npos);

        /**
         * @brief  Copies this bit buffer.
         * @return  A new %bitbuffer with its own copy of the backing array.
         */
        bitbuffer copy() const;

        /**
         * @brief  Copies this bit buffer using a provided allocator.
         * @param allocator  STL-compliant allocator.
         * @return  A new bit buffer with its own copy of the backing array.
         *
         * The new %bitbuffer's backing array is allocated with a copy of
         * @a allocator.
         */
        template <class Alloc>
        bitbuffer copy(const Alloc& allocator) const;

        /**
         * @brief  Performs a take/skip operation into a new bitbuffer.
         * @param take   Number of bits to copy per iteration.
         * @param skip   Number of bits to skip per iteration.
         * @param start  Index of first bit (default 0).
         * @param end    Index of last bit, exclusive (default end).
         * @return  New bitbuffer with requested bits.
         * @throw std::out_of_range      If @p start > size().
         * @throw std::invalid_argument  If @p end < @p start.
         * @see bitbuffer::takeskip
         *
         * Alternately copies @a take bits and skips @a skip bits from the
         * range [@a start, @a end) into a new bitbuffer.
         */
        bitbuffer takeskip(size_t take, size_t skip, size_t start=0, size_t end=npos) const;

        /**
         * @brief  Swap contents with another bit buffer.
         * @param other  The %shared_bitbuffer to swap with.
         */
        void swap(shared_bitbuffer& other);

        /**
         * Returns the population count (number of 1's).
         */
        int popcount() const;

        /**
         * @brief  Determines the Hamming distance from another bit buffer.
         * @param other  The %shared_bitbuffer to compare with.
         * @return  Number of bits that are different.
         */
        int distance(const shared_bitbuffer& other) const;

        /**
         * @brief  Finds a pattern in this bit buffer within a maximum Hamming
         *         distance.
         * @param pattern  Bit pattern to search for.
         * @param maxDistance  Maximum allowable Hamming distance.
         * @return  Bit index of first occurence of @p pattern.
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
         * @return  Bit index of first occurrence of @p pattern.
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
         * @brief  Creates a transient %shared_bitbuffer.
         * @param data  Pointer to first byte.
         * @param size  Number of bits.
         * @see make_transient(const data_type*,size_t,size_t)
         */
        static inline shared_bitbuffer make_transient(const data_type* data, size_t bits)
        {
            return make_transient(data, 0, bits);
        }

        /**
         * @brief  Creates a transient %shared_bitbuffer.
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
         * @brief  Returns true if the backing array's lifetime is not managed.
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
        /// @cond IMPL

        // Internal constructor for use by bitbuffer. The implementation is in
        // the header so that it can be inlined, which in premise avoids the
        // need to add/remove extra references on the refcount_memory.
        shared_bitbuffer(refcount_memory memory, size_t bits) :
            _M_memory(memory),
            _M_base((data_type*) _M_memory.address()),
            _M_offset(0),
            _M_size(bits)
        {
        }

        static inline size_t _M_bits_to_bytes(size_t bits)
        {
            return (bits + 7) / 8;
        }

        /**
         * @brief  Checks index for validity.
         * @param pos   Index to check.
         * @param size  Size of container.
         * @param name   Name of calling method for exception message.
         * @throw std::out_of_range  If @p start > @p size.
         */
        static void _M_check_pos(size_t pos, size_t size, const char* name);

        /**
         * @brief  Checks start and end indices for validity.
         * @param start  Start index.
         * @param end    End index (in/out).
         * @param size   Size of container.
         * @param name   Name of calling method for exception message.
         * @throw std::out_of_range      If @p start > @p size.
         * @throw std::invalid_argument  If @p end < @p start.
         * @post  @p end <= @p size
         * 
         * Checks the indices @a start and @a end for validity against @a size
         * and each other, and clamps @a end to be no larger than @a size.
         */
        static void _M_check_range(size_t start, size_t& end, size_t size, const char* name);

        static size_t _M_takeskip_size(size_t size, size_t take, size_t skip);

    private:
        // Prevent user code from calling swap with a bitbuffer.
        void swap(bitbuffer& other);

        refcount_memory _M_memory;
        data_type* _M_base;
        size_t _M_offset;
        size_t _M_size;
        /// @endcond
    };

    /**
     * @brief  A shared container for working with bit data.
     *
     * The %bitbuffer class extends shared_bitbuffer to provides bit-level
     * write access. Multiple bitbuffers and shared_bitbuffers may point to the
     * same backing array.
     *
     * bitbuffers have reference semantics. Assignment and copy construction do
     * not copy any bits, only the data pointer.
     *
     * Unlike %shared_bitbuffer, %bitbuffer has allocating constructors. When
     * the last reference to the backing array goes away, the backing array is
     * freed.
     */ 
    class bitbuffer : public shared_bitbuffer
    {
    private:
        /**
         * @brief  Proxy bit reference class.
         *
         * This class adapts bit indices and data pointers to behave like
         * both rvalues and lvalues.
         *
         * This class is not intended for direct public usage; operator[] can
         * return an instance, and syntactically it behaves like a primitive
         * reference (mostly), but user code cannot declare one.
         */
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

        /**
         * Construct an empty %bitbuffer.
         */
        bitbuffer();

        /**
         * @brief  Creates a %bitbuffer and allocates a backing array.
         * @param bits  Number of bits.
         *
         * Allocates a backing array large enough to hold @a bits bits using
         * the default allocator. The memory is not initialized.
         */
        explicit bitbuffer(size_t bits) :
            shared_bitbuffer(_M_allocate(bits, default_allocator()), bits)
        {
        }

        /**
         * @brief  Creates a %bitbuffer and allocates a backing array.
         * @param bits       Number of bits.
         * @param allocator  STL-compliant allocator.
         *
         * Allocates a backing array large enough to hold @a bits bits using
         * a copy of @a allocator. The memory is not initialized.
         */
        template <class Alloc>
        bitbuffer(size_t bits, const Alloc& allocator) :
            shared_bitbuffer(_M_allocate(bits, allocator), bits)
        {
        }

        /**
         * @brief  Convenience function to create a %bitbuffer from an integer.
         * @param value  Integer value.
         * @param bits   Number of bits in @p value (max 64).
         * @return  A new %bitbuffer.
         * @throw std::length_error  If @p bits is greater than 64.
         *
         * Allocates and initializes a new %bitbuffer from the least
         * significant @a bits bits of @a value.
         */
        static inline bitbuffer from_int(uint64_t value, size_t bits)
        {
            bitbuffer result(bits);
            result.setint(0, value, bits);
            return result;
        }

        /**
         * @brief Convenience function to create a new %bitbuffer from a string
         *        of '0' and '1' characters.
         * @param str  String to be parsed.
         * @return  A new %bitbuffer.
         * @throw std::invalid_argument  If @p str contains any characters
         *                               other than '0' or '1'.
         */
        static inline bitbuffer from_string(const std::string& str)
        {
            bitbuffer result(str.size());
            result._M_parse(str);
            return result;
        }

        /**
         * @brief  Convenience function to create a %bitbuffer from an unpacked
         *         byte array.
         * @param unpacked  Pointer to first element.
         * @param count     Number of values to pack.
         * @return  A new %bitbuffer.
         *
         * Allocates and initializes a new %bitbuffer by packing the values in
         * @a unpacked. Each element of @a unpacked is converted to a bit
         * value, where zero becomes 0 bit and any non-zero value is a 1 bit.
         */
        static inline bitbuffer from_unpacked(const bitops::byte* unpacked, size_t count)
        {
            bitbuffer result(count);
            bitops::pack(result.data(), result.offset(), unpacked, count);
            return result;
        }

        /**
         * @brief  Convenience function to create a %bitbuffer from a byte
         *         array.
         * @param ptr   Pointer to first byte.
         * @param bits  Number of bits.
         * @return  A new %bitbuffer.
         *
         * Allocates and initializes a new %bitbuffer using @a bits bits from
         * the byte array @a ptr. The new %bitbuffer does not take ownership of
         * @a ptr.
         */
        static inline bitbuffer from_array(const data_type* ptr, size_t bits)
        {
            return from_array(ptr, 0, bits);
        }

        /**
         * @brief  Convenience function to create a %bitbuffer from a byte
         *         array.
         * @param ptr    Pointer to first byte.
         * @param start  Index of first bit.
         * @param bits   Number of bits.
         *
         * Allocates and initializes a new %bitbuffer using @a bits bits
         * starting at bit @a start in the byte array @a ptr. The new
         * %bitbuffer does not take ownership of @a ptr.
         */
        static inline bitbuffer from_array(const data_type* ptr, size_t start, size_t bits)
        {
            return shared_bitbuffer::make_transient(ptr, start, bits).copy();
        }

        using shared_bitbuffer::data;

        /**
         * Returns a read/write pointer to the backing array.
         */
        data_type* data();

        using shared_bitbuffer::operator[];

        /**
         * @brief  Subscript read/write access to a bit.
         * @param index  The index of the desired bit.
         * @return  A reference to the bit.
         *
         * Because bits are not directly accessible, a proxy object is returned
         * instead of the more typical reference-to-element. This proxy can be
         * used as both an rvalue and an lvalue; however, it is not exactly
         * equivalent to a primitive reference type.
         */
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

        /**
         * @brief  Resizes the %bitbuffer to the specified number of bits.
         * @param bits  Number of bits.
         * @see trim
         *
         * Allocates a new backing buffer large enough to hold @a bits bits
         * using the default allocator, preserving existing bit values. If
         * @a bits is larger than the current size, new bit values are
         * uninitialized.
         *
         * If @a bits is smaller than the current size, unless a new copy is
         * desired, trim is more efficient because it does not perform any
         * allocation or copy.
         */
        void resize(size_t bits)
        {
            resize(bits, default_allocator());
        }

        /**
         * @brief  Resizes the %bitbuffer to the specified number of bits using
         *         a provided allocator.
         * @param bits       Number of bits.
         * @param allocator  STL-compliant allocator.
         *
         * Allocates a new backing buffer large enough to hold @a bits bits
         * using a copy of @a allocator, preserving existing bit values. If
         * @a bits is larger than the current size, new bit values are
         * uninitialized.
         *
         * If @a bits is smaller than the current size, unless a new copy is
         * desired, trim is more efficient because it does not perform any
         * allocation or copy.
         */
        template <class Alloc>
        void resize(size_t bits, const Alloc& allocator)
        {
            bitbuffer temp(bits, allocator);
            _M_resize(temp);
        }

        /**
         * @brief  Replaces bit values.
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
         * @brief  Replaces bit values.
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

        using shared_bitbuffer::takeskip;

        /**
         * @brief  Performs a take/skip operation into this bitbuffer.
         * @param src    Source bit buffer.
         * @param take   Number of bits to copy per iteration.
         * @param skip   Number of bits to skip per iteration.
         * @param start  Index of first bit in @p src (default 0).
         * @param end    Index of last bit in @p src, exclusive (default
         *               @p src.size()).
         * @return  Number of bits copied.
         * @throw std::out_of_range      If @p start > @p src.size().
         * @throw std::invalid_argument  If @p end < @p start.
         * @throw std::length_error      If this bitbuffer is not large enough.
         *
         * Alternately copies @a take bits and skips @a skip bits into this bit
         * buffer from @a src.
         */
        size_t takeskip(const shared_bitbuffer& src, size_t take, size_t skip, size_t start=0, size_t end=npos)
        {
            return takeskip(0, src, take, skip, start, end);
        }

        /**
         * @brief  Performs a take/skip operation into this bitbuffer.
         * @param pos    Index of first bit to write to.
         * @param src    Source bit buffer.
         * @param take   Number of bits to copy per iteration.
         * @param skip   Number of bits to skip per iteration.
         * @param start  Index of first bit in @p src (default 0).
         * @param end    Index of last bit in @p src, exclusive (default
         *               @p src.size()).
         * @return  Number of bits copied.
         * @throw std::out_of_range      If @p start > @p src.size().
         * @throw std::invalid_argument  If @p end < @p start.
         * @throw std::length_error      If this bitbuffer is not large enough.
         *
         * Alternately copies @a take bits and skips @a skip bits into this bit
         * buffer from @a src.
         */
        size_t takeskip(size_t pos, const shared_bitbuffer& src, size_t take, size_t skip, size_t start=0, size_t end=npos);

        /**
         * @brief  Swap contents with another bitbuffer.
         * @param other  The bitbuffer to swap with.
         */
        void swap(bitbuffer& other);

    private:
        /// @cond IMPL
        // Helper to handle the allocation of reference counted memory to pass
        // to the shared_bitbuffer constructor. Inlining it allows the compiler
        // to elide extra reference counts, as this is being used to initialize
        // the _M_memory member of shared_bitbuffer.
        template <class Alloc>
        static inline refcount_memory _M_allocate(size_t bits, const Alloc& allocator)
        {
            return refcount_memory(_M_bits_to_bytes(bits), allocator);
        }

        // Helper to parse a string into a newly-created bitbuffer.
        void _M_parse(const std::string& str);

        void _M_resize(bitbuffer& dest);
        /// @endcond
    };

    inline bitbuffer shared_bitbuffer::copy() const
    {
        // NB: Implementation cannot be done in-line because the buffer class
        //     is incomplete at that point
        return this->copy(bitbuffer::default_allocator());
    }

    template <class Alloc>
    inline bitbuffer shared_bitbuffer::copy(const Alloc& allocator) const
    {
        // NB: Implementation cannot be done in-line because the buffer class
        //     is incomplete at that point
        bitbuffer result(size(), allocator);
        result.replace(0, result.size(), *this);
        return result;
    }

    inline bitbuffer shared_bitbuffer::takeskip(size_t take, size_t skip, size_t start, size_t end) const
    {
        // NB: Implementation cannot be done in-line because the buffer class
        //     is incomplete at that point
        _M_check_range(start, end, size(), "redhawk::shared_bitbuffer::takeskip");
        size_t bits = _M_takeskip_size(end-start, take, skip);
        bitbuffer result(bits);
        result.takeskip(*this, take, skip, start, end);
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
