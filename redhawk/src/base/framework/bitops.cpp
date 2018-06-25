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

#include <ossie/bitops.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <stdexcept>

namespace redhawk {
namespace bitops {

    namespace {
        // Normalizes a byte pointer and bit offset so that the offset is
        // always in the range [0,8). The data type is templatized to support
        // const and non-const pointers.
        template <typename T>
        static inline size_t adjust_buffer(T*& buffer, size_t offset)
        {
            buffer += (offset / 8);
            return offset & 0x7;
        }

        // Generates a bitmask with the least-significant N bits set
        static inline byte bitmask(size_t nbits)
        {
            return (1 << nbits) - 1;
        }

        // Lookup table of Hamming weights by byte value
        static const int hammingWeights[] = {
            0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
            1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
            1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
            2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
            1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
            2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
            2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
            3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
            1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
            2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
            2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
            3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
            2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
            3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
            3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
            4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
        };

        // Tags for describing how a unary or binary operation accesses the bit
        // arrays, allowing the apply functions to support get/set/modify with
        // the same code

        // Only read from the operand (e.g., population count)
        struct read_tag {};

        // Only write to the operand (e.g., fill)
        struct write_tag {};

        // Read and modify the operation (e.g., negate)
        struct readwrite_tag {};

        // Tags for bit operation function bodies with exact alignment for all
        // bit arrays (e.g., in a binary operation, both bit arrays have the
        // same alignment)

        // Use the normal operator() to process element-by-element
        struct element_tag {};

        // Use array-based operator() to process all at once (e.g., in copy,
        // using memcpy to transfer)
        struct array_tag {};

        // Handle reading of partial and split bytes
        struct bit_reader {
            static inline byte read(const byte* src, size_t offset, size_t bits)
            {
                const size_t shift = 8 - (offset + bits);
                return ((*src) >> shift) & bitmask(bits);
            }

            static inline byte read(const byte* src, size_t bits)
            {
                // Get the leftmost bits from src; masking is unnecessary, as
                // unsigned values are zero-filled on right shift
                const size_t shift = 8 - bits;
                return ((*src) >> shift);
            }

            static inline byte read_split(const byte* src, size_t offset)
            {
                // The byte value is formed by combining two adjacent bytes
                // from src and then shifting down. This is a pretty efficient
                // way to handle unaligned values. Although doing this in a
                // loop requires accessing each byte twice, the CPU cache
                // should mitigate that cost.
                size_t shift = 8 - offset;
                return ((src[0] << 8) | src[1]) >> shift;
            }

            static inline byte read_split(const byte* src, size_t offset, size_t bits)
            {
                // Like full-byte read_split, but it is not known whether the
                // second byte at src is required (or even accessible). The
                // first byte is loaded into the high 8 bits of at 16-bit
                // value, and the low 8 bits are set from the byte that
                // contains the last bit, which may still be the first byte.
                // Doing it this way avoids a conditional, the performance of
                // which is less predictable.
                uint16_t value = src[0] << 8;
                size_t last = offset + bits - 1; // index of last bit
                value |= src[last/8];
                // Shift to put the last bit at the LSB, and mask to get only
                // the requested number of bits
                size_t shift = 15 - last;
                return (value >> shift) & bitmask(bits);
            }
        };

        // Handle writing of partial and split bytes
        struct bit_writer {
            static inline void write(byte value, byte* dest, size_t offset, size_t bits)
            {
                // Preserve the leftmost bits, and if necessary, the rightmost
                // bits as well
                const size_t shift = 8 - (offset + bits);
                const byte mask = bitmask(bits) << shift;
                *dest = ((*dest) & ~mask) | ((value << shift) & mask);
            }

            static inline void write(byte value, byte* dest, size_t bits)
            {
                // Sets leftmost bits in dest from value
                const size_t shift = 8 - bits;
                const byte mask = bitmask(shift);
                *dest = ((*dest) & mask) | (value << shift);
            }

            static inline void write_split(byte value, byte* dest, size_t offset)
            {
                // 8-bit split write is equivalent to two partial writes, one
                // to each byte
                write(value >> offset, dest, offset, 8 - offset);
                write(value, dest + 1, offset);
            }

            static inline void write_split(byte value, byte* dest, size_t offset, size_t bits)
            {
                // Fewer than 8 bits may or may not be split into two bytes;
                // write as many bits as possible to the first byte, and then
                // if necessary, write the remainder to the second byte
                size_t left = std::min(8 - offset, bits);
                write(value >> (bits - left), dest, offset, left);
                if (bits > left) {
                    write(value, dest + 1, bits - left);
                }
            }
        };

        // Traits-like class to take a read/write tag and provide an interface
        // to read and write bits to the operands as necessary. For example, if
        // an operation reads from but does not write to a bit array, the read
        // functions return meaningful values but the write functions are no-
        // ops, which can be eliminated by the compiler.
        template <typename Mode>
        struct bit_handler;

        // Read-only: add no-op write interface to bit reader
        template <>
        struct bit_handler<read_tag> : public bit_reader
        {
            static inline void write(byte, const byte*, size_t, size_t)
            {
            }

            static inline void write(byte, const byte*, size_t)
            {
            }

            static inline void write_split(byte, const byte*, size_t)
            {
            }

            static inline void write_split(byte, const byte*, size_t, size_t)
            {
            }
        };

        // Write-only: add no-op read interface to bit writer
        template <>
        struct bit_handler<write_tag> : public bit_writer
        {
            static inline byte read(byte*, size_t, size_t)
            {
                return byte();
            }

            static inline byte read(byte*, size_t)
            {
                return byte();
            }

            static inline byte read_split(byte*, size_t)
            {
                return byte();
            }

            static inline byte read_split(byte*, size_t, size_t)
            {
                return byte();
            }
        };

        // Read/write: compose bit reader and bit writer
        template <>
        struct bit_handler<readwrite_tag> : public bit_reader, public bit_writer
        {
        };

        // Unary function body for aligned, full-byte operations, performed
        // element-by-element
        template <typename T, class Func>
        inline void unary_body(T*& data, size_t bytes, Func& func, element_tag)
        {
            for (size_t ii = 0; ii < bytes; ++ii) {
                func(*data++, 8);
            }
        }

        // Unary function body for aligned, full-byte operations, where the
        // function supports array-based operation
        template <typename T, class Func>
        inline void unary_body(T*& data, size_t bytes, Func& func, array_tag)
        {
            func(data, bytes);
            data += bytes;
        }

        // Applies a unary function across an array of bits.
        // The data type, T, is templatized to support const/non-const byte
        // data (inner functions eventually specify "const byte*" or "byte*",
        // so the set of supported types is bounded).
        // The functor type, Func, must be a unary operator, preferably a
        // subclass of UnaryGetter/UnarySetter. It must have nested typedefs
        // for dispatching to the correct functions for reading/writing and the
        // main function body:
        //    mode_tag - one of read_tag, write_tag or readwrite_tag
        //    body_tag - element_tag for element-by-element processing of
        //               aligned, full-byte data, or array_tag if Func has an
        //               array-based operator() that can be applied to the
        //               whole data set
        //
        // This template function is designed to support all types of read/
        // write/update methods across a bit array as efficiently as possible,
        // while ensuring correctness irrespective of alignment or bit count.
        // As much as possible, conditionals are avoided, and the compiler will
        // eliminate code that has no effect, such as no-op reads and writes,
        // or checking for function completion when the functor's complete()
        // method always returns false.
        template <class T, class Func>
        inline typename Func::return_type apply_unary(T* data, size_t offset, size_t count, Func func)
        {
            typedef bit_handler<typename Func::mode_tag> handler;

            // Adjust packed bit pointer and offset so offset is less then 8
            offset = adjust_buffer(data, offset);

            // First, account for an unaligned starting bit
            if (offset) {
                size_t nbits = std::min(8 - offset, count);
                // Fetch a subset of the first byte (a no-op if the array is
                // only written)
                byte value = handler::read(data, offset, nbits);

                // Apply the function to the value, which may be modified if
                // func takes it by reference
                func(value, nbits);

                // Write out the modified value (a no-op if the array is only
                // read)
                handler::write(value, data, offset, nbits);
                if (func.complete()) {
                    return func.returns();
                }
                ++data;
                count -= nbits;
            }

            // Apply function to aligned, full-byte values
            const size_t bytes = count / 8;
            unary_body(data, bytes, func, typename Func::body_tag());
            if (func.complete()) {
                return func.returns();
            }

            // If less than a full byte remains, process it
            size_t remain = count & 7;
            if (remain) {
                // Fetch, apply and write (as needed) from the leftmost N bits
                byte value = handler::read(data, remain);
                func(value, remain);
                handler::write(value, data, remain);
            }

            return func.returns();
        }

        // Binary function body for exact alignment between left and right
        // sides, performing the operation element-by-element
        template <typename T1, typename T2, class Func>
        void binop_body_aligned(T1*& lhs, T2*& rhs, size_t bytes, Func& func, element_tag)
        {
            for (size_t ii = 0; ii < bytes; ++ii) {
                func(*lhs, *rhs, 8);
                if (func.complete()) {
                    return;
                }
                ++lhs;
                ++rhs;
            }
        }

        // Binary function body for exact alignment between left and right
        // sides, where the functor supports array-based operation
        template <typename T1, typename T2, class Func>
        void binop_body_aligned(T1*& lhs, T2*& rhs, size_t bytes, Func& func, array_tag)
        {
            func(lhs, rhs, bytes);
            lhs += bytes;
            rhs += bytes;
        }

        // Applies a binary function across two equal-length arrays of bits.
        // The data types, T1 and T2, are templatized to support const/non-
        // const  byte data (inner functions eventually specify "const byte*"
        // or "byte*", so the set of supported types is bounded).
        // The functor type, Func, must be a binary operator, preferably a
        // subclass of BinaryGetter/BinarySetter. It must have nested typedefs
        // for dispatching to the correct functions for reading/writing from/to
        // the bit arrays, and the main function body:
        //    left_mode_tag  - how lhs is accessed (one of read_tag, write_tag
        //                     or readwrite_tag)
        //    right_mode_tag - how rhs is accessed (one of read_tag, write_tag
        //                     or readwrite_tag)
        //    body_tag       - element_tag for element-by-element processing of
        //                     exact aligned, full-byte data, or array_tag if
        //                     Func has an array-based operator() that can be
        //                     applied to a whole data set
        //
        // This template function is designed to support all types of read/
        // write/update methods across two bit arrays as efficiently as
        // possible, while ensuring correctness irrespective of alignment or
        // bit count. As much as possible, conditionals are avoided, and the
        // compiler will eliminate code that has no effect, such as no-op reads
        // and writes, or checking for function completion when the functor's
        // complete() method always returns false.
        template <typename T1, typename T2, class Func>
        typename Func::return_type apply_binop(T1* lhs, size_t lhs_offset,
                                               T2* rhs, size_t rhs_offset,
                                               size_t count, Func func)
        {
            typedef bit_handler<typename Func::left_mode_tag> lhs_handler;
            typedef bit_handler<typename Func::right_mode_tag> rhs_handler;

            // Adjust pointers and offsets so offsets are less then 8
            lhs_offset = adjust_buffer(lhs, lhs_offset);
            rhs_offset = adjust_buffer(rhs, rhs_offset);

            // If the left hand side is not byte-aligned, apply the operation
            // to a sub-byte number of bits so that remaining iterations are
            // aligned
            if (lhs_offset) {
                const size_t nbits = std::min(8 - lhs_offset, count);

                // Get the left and right hand values for the operand (which
                // may be no-ops in the case of an array that is only written,
                // not read); the right hand side may be split between two
                // bytes
                byte lhs_value = lhs_handler::read(lhs, lhs_offset, nbits);
                byte rhs_value = rhs_handler::read_split(rhs, rhs_offset, nbits);

                // Apply the function to the values, which may be modified if
                // func takes one or both by reference
                func(lhs_value, rhs_value, nbits);

                // Write back the updated values (again, no-ops if the arrays
                // are only read)
                lhs_handler::write(lhs_value, lhs, lhs_offset, nbits);
                rhs_handler::write_split(rhs_value, rhs, rhs_offset, nbits);
                if (func.complete()) {
                    return func.returns();
                }

                // Advance to the next byte for the left hand side, and adjust
                // the offset for the right hand side (which may advance to the
                // next byte as well)
                ++lhs;
                rhs_offset = adjust_buffer(rhs, rhs_offset + nbits);
                count -= nbits;
            }

            // Left offset is now guaranteed to be 0; if the right offset is
            // also 0, then both sides are exactly byte-aligned
            const size_t bytes = count / 8;
            if (rhs_offset == 0) {
                binop_body_aligned(lhs, rhs, bytes, func, typename Func::body_tag());
                if (func.complete()) {
                    return func.returns();
                }
            } else {
                // The two bit arrays are not exactly aligned; iterate through
                // each byte from the left-hand side
                for (size_t ii = 0; ii < bytes; ++ii) {
                    // Fetch the right-hand value from two adjacent bytes if
                    // needed
                    byte rhs_value = rhs_handler::read_split(rhs, rhs_offset);
                    // Apply the function; it is not necessary to read or write
                    // via a bit handler because we are using the real byte
                    // address
                    func(*lhs, rhs_value, 8);
                    // Write back the right hand side (if needed)
                    rhs_handler::write_split(rhs_value, rhs, rhs_offset);
                    if (func.complete()) {
                        return func.returns();
                    }
                    ++lhs;
                    ++rhs;
                }
            }

            // If less than a full byte remains, process it
            const size_t remain = count & 7;
            if (remain) {
                // The left hand side is byte aligned, read the left-justified
                // bits (if needed); the right hand side may or may not be split
                // across two bytes, depending on the alignment and number of
                // bits
                byte lhs_value = lhs_handler::read(lhs, remain);
                byte rhs_value = rhs_handler::read_split(rhs, rhs_offset, remain);
                func(lhs_value, rhs_value, remain);
                // Write results if needed (see above)
                lhs_handler::write(lhs_value, lhs, remain);
                rhs_handler::write_split(rhs_value, rhs, rhs_offset, remain);
            }
            return func.returns();
        }
    } // end anonymous namespace

    // Base class for operations, supporting a return value and completion
    // status. By default, an operation is never "complete" in the sense that
    // it does not need to return early, but subclasses may override complete()
    // in order to stop processing and return immediately (e.g., compare).
    template <typename R>
    class Operation {
    public:
        typedef R return_type;

        Operation(return_type rv=0) :
            result(rv)
        {
        }

        return_type returns()
        {
            return result;
        }

        bool complete ()
        {
            return false;
        }

        R result;
    };

    // Template specialization for void return, which cannot have a result
    // member variable
    template <>
    class Operation<void> {
    public:
        typedef void return_type;

        void returns()
        {
        }

        bool complete ()
        {
            return false;
        }
    };

    // Base class for unary operations to ensure they define the required tags.
    // In most cases, new operations should extend UnarySetter or UnaryGetter.
    template <class R, class Mode, class Body>
    class UnaryOp : public Operation<R> {
    public:
        typedef Mode mode_tag;
        typedef Body body_tag;
    };

    // Base class for unary getters; that is, functions that read from a bit
    // array. Inheriting from this class defines the required function dispatch
    // tags to ensure that apply_unary only reads from the input array.
    template <class R, class Body=element_tag>
    class UnaryGetter : public UnaryOp<R,read_tag,Body>
    {
    public:
        // void operator() (byte value, size_t bits);
    };

    // Base class for unary setters; that is, functions that write to a bit
    // array. Inheriting from this class defines the required function dispatch
    // tags to ensure that apply_unary only writes to input array.
    template <class R, class Body=element_tag>
    class UnarySetter : public UnaryOp<R,write_tag,Body>
    {
    public:
        // void operator() (byte& value, size_t bits);
    };

    // Base class for binary operations to ensure they define the required
    // tags. In most cases, new operations should extend BinarySetter or
    // BinaryGetter.
    template <class R, class LeftMode=read_tag, class RightMode=read_tag, class Body=element_tag>
    class BinaryOp : public Operation<R> {
    public:
        typedef LeftMode left_mode_tag;
        typedef RightMode right_mode_tag;
        typedef Body body_tag;
    };

    // Base class for binary getters; that is, functions that read values from
    // two bit arrays. Inheriting from this class defines the required function
    // dispatch tags to ensure that apply_binop only reads from both arrays.
    template <class R, class Body=element_tag>
    class BinaryGetter : public BinaryOp<R,read_tag,read_tag,Body>
    {
        // void operator() (byte lhs, byte rhs, size_t bits);
    };

    // Base class for binary setters; that is, functions that read values from
    // one bit array and write them to another. Inheriting from this class
    // defines the required function dispatch tags to ensure that apply_binop
    // only reads from the right array and writes to the left array.
    template <class R, class Body=element_tag>
    class BinarySetter : public BinaryOp<R,write_tag,read_tag,Body>
    {
        // void operator() (byte& lhs, byte rhs, size_t bits);
    };


    //
    // Public function implementations
    //
    bool getbit(const byte* str, size_t pos)
    {
        const size_t bit_offset = adjust_buffer(str, pos);
        return ((*str) >> (7 - bit_offset)) & 1;
    }

    void setbit(byte* str, size_t pos, bool value)
    {
        const size_t bit_offset = adjust_buffer(str, pos);
        const size_t shift = (7 - bit_offset);
        const byte mask = ~(1 << shift);
        *str = ((*str) & mask) | (value << shift);
    }

    // Unary getter functor that accumulates an integer value of up to 64 bits
    // by shifting and or-ing successive bits from an array of bits. The first
    // bit is always in the MSB.
    class GetInteger : public UnaryGetter<uint64_t> {
    public:
        inline void operator() (byte value, size_t bits)
        {
            // Shift the existing value over to accomodate the new bits,
            // which maintains the first bit in the MSB
            result = (result << bits) | value;
        }
    };

    uint64_t getint(const byte* str, size_t start, size_t bits)
    {
        if (bits > 64) {
            throw std::length_error("redhawk::bitops::getint()");
        }
        return apply_unary(str, start, bits, GetInteger());
    }

    // Unary setter functor that takes an integer value of up to 64 bits and
    // returns successive bits from that value starting with the MSB.
    class SetInteger : public UnarySetter<void> {
    public:
        SetInteger(uint64_t value, int bits) :
            // Shift the value up to the MSB of the 64-bit integer
            value(value << (64 - bits))
        {
        }

        inline void operator() (byte& dest, size_t bits)
        {
            // The current value is in the MSB, shift the requested number of
            // bits down to the LSB
            dest = (value >> (64-bits));
            // Shift the next bits up to the MSB
            value <<= bits;
        }

        uint64_t value;
    };

    void setint(byte* str, size_t start, uint64_t value, size_t bits)
    {
        if (bits > 64) {
            throw std::length_error("redhawk::bitops::setint()");
        }
        apply_unary(str, start, bits, SetInteger(value, bits));
    }

    // Unary setter functor that fills a bit array with either 0's or 1's. For
    // the aligned full-byte case, a second operator() is defined that uses
    // memset, which is faster than setting each byte individually.
    class Fill : public UnarySetter<void,array_tag> {
    public:
        Fill(bool value) :
            value(-1 * value) // all bits 1 if true, 0 if false
        {
        }

        inline void operator() (byte& dest, size_t /*unused*/)
        {
            dest = value;
        }

        inline void operator() (byte* dest, size_t bytes)
        {
            memset(dest, value, bytes);
        }

        const byte value;
    };

    void fill(byte* str, size_t start, size_t length, bool value)
    {
        apply_unary(str, start, length, Fill(value));
    }

    // Unary setter functor that takes a source array of byte values and packs
    // each byte value into a bit, where a zero byte results in a 0 bit and any
    // non-zero value results in a 1 bit.
    class Pack : public UnarySetter<void> {
    public:
        Pack(const byte* src) :
            src(src)
        {
        }

        inline void operator() (byte& dest, size_t bits)
        {
            // NB: Accumulate the packed value in a temporary variable so the
            // compiler knows it doesn't have to write back to dest on each
            // iteration
            byte value = 0;
            for (size_t ii = 0; ii < bits; ++ii) {
                value = (value << 1) | ((*src++)?1:0);
            }
            dest = value;
        }

        const byte* src;
    };

    void pack(byte* dest, size_t dstart, const byte* src, size_t length)
    {
        apply_unary(dest, dstart, length, Pack(src));
    }

    // Unary getter functor that takes a destination array of byte values and
    // unpacks each bit value into a byte, where a bit value of 0 expands to a
    // byte value of 0, and a bit value of 1 expands to a byte value of 1.
    class Unpack : public UnaryGetter<void> {
    public:
        Unpack(byte* dest) :
            dest(dest)
        {
        }

        inline void operator() (byte value, size_t bits)
        {
            // When bits is known at compile time (i.e., in the aligned full-
            // byte case), the compiler will usually unroll this loop to remove
            // the conditional check
            for (int pos = (bits-1); pos >= 0; --pos) {
                *dest++ = (value >> pos) & 1;
            }
        }

        byte* dest;
    };

    void unpack(byte* dest, const byte* src, size_t sstart, size_t length)
    {
        apply_unary(src, sstart, length, Unpack(dest));
    }

    // Binary setter to copy bits, using memcpy for accelerated copies when
    // both arrays are exactly aligned.
    class Copy : public BinarySetter<void,array_tag>
    {
    public:
        inline void operator() (byte& dest, byte src, size_t /*unused*/)
        {
            dest = src;
        }

        inline void operator() (byte* dest, const byte* src, size_t bytes)
        {
            // Copy aligned bytes directly
            std::memcpy(dest, src, bytes);
        }
    };

    void copy(byte* dest, size_t dstart, const byte* src, size_t sstart, size_t length)
    {
        apply_binop(dest, dstart, src, sstart, length, Copy());
    }

    // Comparison functor, returning:
    // * A positive integer if the left operand is greater than the right
    // * Zero if the left operand is equal to the right
    // * A negative integer if the left operand is less than the right
    // The complete() method is overridden to return early once a difference is
    // found.
    class Compare : public BinaryGetter<int,array_tag> {
    public:
        inline void operator() (byte lhs, byte rhs, size_t /*unused*/) {
            if (lhs == rhs) {
                result = 0;
            } else if (lhs > rhs) {
                result = 1;
            } else {
                result = -1;
            }
        }

        inline void operator() (const byte* lhs, const byte* rhs, size_t bytes)
        {
            result = memcmp(lhs, rhs, bytes);
        }

        bool complete()
        {
            return (result != 0);
        }
    };

    int compare(const byte* s1, size_t start1, const byte* s2, size_t start2, size_t length)
    {
        return apply_binop(s1, start1, s2, start2, length, Compare());
    }

    // Unary getter functor that returns the population count (Hamming weight)
    // of the input bit array.
    class Popcount : public UnaryGetter<int> {
    public:
        inline void operator() (byte value, size_t /*unused*/)
        {
            result += hammingWeights[value];
        }
    };

    int popcount(const byte* str, size_t offset, size_t count)
    {
        return apply_unary(str, offset, count, Popcount());
    }

    // Unary getter functor that turns a bit string into a character string.
    class ToString : public UnaryGetter<void> {
    public:
        ToString(char* dest) :
            dest(dest)
        {
        }

        inline void operator() (byte value, size_t bits)
        {
            // When bits is known at compile time (i.e., in the aligned full-
            // byte case), the compiler will usually unroll this loop to remove
            // the conditional check
            for (int pos = (bits-1); pos >= 0; --pos) {
                // Simple optimization: the value has to be 0 or 1, and the
                // ASCII characters for 0 and 1 are adjacent, so adding the bit
                // value to '0' gives the right value, as long as you cast back
                // to a char (addition promotes to int here).
                *dest++ = '0' + ((value >> pos) & 1);
            }
        }

        char* dest;
    };

    void toString(char* str, const byte* ptr, size_t start, size_t length)
    {
        apply_unary(ptr, start, length, ToString(str));
    }

    // Unary functor that takes a character string and parses into a sequence
    // of bits, returning the number of characters parsed.
    // The complete() method is overridden to return early if an invalid
    // character (not '0' or '1') is encountered, and the return value will be
    // less than the requested number of characters.
    // In order to handle the possibility of a partial write, this functor does
    // not inherit from the expected UnarySetter, but instead uses read/write
    // functionality to avoid overwriting exisiting bits.
    class FromString : public UnaryOp<int,readwrite_tag,element_tag> {
    public:
        FromString(const char* src) :
            src(src),
            valid(true)
        {
        }

        inline void operator() (byte& dest, size_t bits)
        {
            byte value = 0;
            for (size_t ii = 0; ii < bits; ++ii) {
                byte bit = 0;
                switch (*src) {
                case '0': bit = 0; break;
                case '1': bit = 1; break;
                default:
                    // Invalid character: stop parsing, write the valid bits
                    // (which are in the least-significant bits, and the count
                    // is equal to the loop index) and return.
                    valid = false;
                    const size_t offset = 8 - bits;
                    bit_writer::write(value, &dest, offset, ii);
                    return;
                }
                value = (value << 1) | bit;
                ++src;
                ++result;
            }
            // Everything worked as planned, assign the value
            dest = value;
        }

        bool complete()
        {
            return !valid;
        }

        inline byte parse(size_t bits)
        {
            byte value = 0;
            for (size_t ii = 0; ii < bits; ++ii) {
                byte bit = 0;
                switch (*src) {
                case '0': bit = 0; break;
                case '1': bit = 1; break;
                default:
                    // Stop parsing and return immediately
                    valid = false;
                    return value;
                }
                value = (value << 1) | bit;
                ++src;
                ++result;
            }
            return value;
        }

        const char* src;
        bool valid;
    };

    int parseString(byte* dest, size_t dstart, const char* str, size_t length)
    {
        return apply_unary(dest, dstart, length, FromString(str));
    }

    // Hamming distance functor that accumulates the number of bit positions
    // that differ between two bit arrays.
    class HammingDist : public BinaryGetter<int> {
    public:
        inline void operator() (byte lhs, byte rhs, size_t /*unused*/) {
            result += hammingWeights[lhs ^ rhs];
        }
    };

    int hammingDistance(const byte* s1, size_t start1, const byte* s2, size_t start2, size_t length)
    {
        return apply_binop(s1, start1, s2, start2, length, HammingDist());
    }


    // Hamming distance-based comparsion, for inexact search up to a maximum
    // number of bit differences. The complete() method is overridden to return
    // early once a the Hamming distance exceeds the threshold.
    class HammingCompare : public HammingDist {
    public:
        HammingCompare(int maxDistance) :
            _maxDistance(maxDistance)
        {
        }

        bool complete()
        {
            return result > _maxDistance;
        }

    private:
        int _maxDistance;
    };

    int find(const byte* str, size_t sstart, size_t slen,
             const byte* patt, size_t pstart, size_t plen,
             int maxdist)
    {
        // Basic validity checks
        if (slen < plen) {
            throw std::logic_error("pattern is longer than string");
        }

        const size_t end = slen - plen;
        for (size_t index = sstart; index < end; ++index) {
            // Use a Hamming calculation that short-circuits if the maximum
            // distance is exceeded
            int dist = apply_binop(str, index, patt, pstart, plen, HammingCompare(maxdist));
            if (dist <= maxdist) {
                return index;
            }
        }
        return -1;
    }

    size_t takeskip(byte* dest, size_t dstart,
                  const byte* src, size_t sstart, size_t slen,
                  size_t take, size_t skip)
    {
        size_t dest_pos = dstart;
        size_t end = sstart + slen;
        for (; sstart < end; sstart += (take+skip)) {
            size_t pass = std::min(take, end-sstart);
            copy(dest, dest_pos, src, sstart, pass);
            dest_pos += pass;
        }
        return dest_pos - dstart;
    }

} // namespace bitops
} // namespace redhawk
