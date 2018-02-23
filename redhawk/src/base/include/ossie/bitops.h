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

#ifndef REDHAWK_BITOPS_H
#define REDHAWK_BITOPS_H

#include <cstddef>
#include <stdint.h>

namespace redhawk {

    namespace bitops {

        // @brief  Bits are stored MSB first in bytes
        typedef unsigned char byte;

        /**
         * @brief  Gets a single bit from a bit string.
         * @param str  Bit string.
         * @param pos  Index of bit.
         * @returns  Value of bit number @p pos in @p str.
         */
        bool getbit(const byte* str, size_t pos);

        /**
         * @brief  Sets a single bit in a bit string.
         * @param str    Bit string.
         * @param pos    Index of bit.
         * @param value  Bit value to set.
         */
        void setbit(byte* str, size_t pos, bool value);

        /**
         * @brief  Extracts an integer value from a bit string.
         * @param str    Bit string.
         * @param start  Starting bit index.
         * @param bits   Number of bits to extract (max 64).
         * @returns  Integer value.
         * @throw std::length_error  If @p bits is greater than 64.
         *
         * The value is extracted in big-endian order, and returned right-
         * justified (i.e., the least-significant N bits contain the value).
         * If fewer than 64 bits are extracted, the most signifiant bits are
         * zeroed.
         */
        uint64_t getint(const byte* str, size_t start, size_t bits);

        /**
         * @brief  Inserts an integer value into a bit string.
         * @param str    Bit string.
         * @param start  Starting bit index.
         * @param value  Value to insert.
         * @param bits   Number of bits in @p value (max 64).
         * @throw std::length_error  If @p bits is greater than 64.
         *
         * @a value is inserted in big-endian order, and must be right-
         * justified (i.e., the least-significant N bits are inserted). If
         * fewer than 64 bits are requested, the most significant bits are
         * ignored.
         */
        void setint(byte* str, size_t start, uint64_t value, size_t bits);

        /**
         * @brief  Sets all of the bits in a bit string to a given value.
         * @param str     Bit string.
         * @param start   Indef of first bit in @p str.
         * @param length  Number of bits to set.
         * @param value   Bit value to set.
         */
        void fill(byte* str, size_t start, size_t length, bool value);

        /**
         * @brief  Packs a byte array into a bit string.
         * @param dest    Destination bit string.
         * @param dstart  Index of first bit in @p dest.
         * @param src     Source byte array.
         * @param length  Number of elements to pack.
         *
         * Each byte of @a src is turned into a bit in @a dest. If the byte
         * value is 0, the corresponding bit is 0; if the byte value is non-
         * zero, the corresponding bit is 1.
         */
        void pack(byte* dest, size_t dstart, const byte* src, size_t length);

        /**
         * @brief  Unpacks a bit string into a byte array.
         * @param dest    Destination byte array.
         * @param src     Source bit string.
         * @param sstart  Index of first bit in @p src.
         * @param length  Number of elements to unpack.
         *
         * Each bit of @a src is turned into a byte in @a dest. If the bit
         * value is 0, the corresponding byte is 0; if the value is 1, the
         * corresponding byte is 1.
         */
        void unpack(byte* dest, const byte* src, size_t sstart, size_t length);

        /**
         * @brief  Converts a bit string into a character string.
         * @param str     Destination character array.
         * @param src     Source bit string.
         * @param sstart  Index of first bit in @p src.
         * @param length  Number of bits in @p src.
         * @pre  Enough space for @a length characters must have been allocated
         *       at @a str.
         *
         * Expands each bit in @a src into a character in @a str, where each
         * bit is one of '0' or '1'. No null character terminator is added at
         * the end of @a str.
         */
        void toString(char* str, const byte* src, size_t sstart, size_t length);

        /**
         * @brief  Parses a character string into a bit string.
         * @param dest    Destination bit string.
         * @param dstart  Index of first bit in @p dest.
         * @param src     Source character array.
         * @param length  Number of characters in @p src.
         * @returns  Number of characters parsed.
         * @pre  Enough space for @a length bits must have been allocated at
         *       @a dest, taking @a dstart into account.
         *
         * Converts the characters in @a str into bits, where each character
         * must be one of '0' or '1'. On success, returns @a length. If an
         * invalid character is encountered, parsing stops and the number of
         * valid characters is returned.
         */
        int parseString(byte* dest, size_t dstart, const char* str, size_t length);

        /**
         * @brief  Copies bits from one bit string to another.
         * @param dest    Destination bit string.
         * @param dstart  Index of first bit in @p dest.
         * @param src     Source bit string.
         * @param sstart  Index of first bit in @p src.
         * @param length  Number of bits to copy.
         */
        void copy(byte* dest, size_t dstart, const byte* src, size_t sstart, size_t length);

        /**
         * @brief  Compares two bit strings.
         * @param s1      First bit string.
         * @param start1  Index of first bit in @p s1.
         * @param s2      Second bit string.
         * @param start2  Index of first bit in @p s2.
         * @param length  Number of bits to compare.
         * @returns  Positive integer, 0, or negative integer.
         *
         * Returns a positive integer if @a s1 is ordered before @a s2, 0 if
         * both bit strings are equivalent, or a negative integer if @a s1 is
         * ordered after @a s2.
         */
        int compare(const byte* s1, size_t start1, const byte* s2, size_t start2, size_t length);

        /**
         * @brief  Calculates the population count of a bit string.
         * @param str     Bit string.
         * @param start   Index of first bit in @p str.
         * @param length  Length of @p str.
         * @returns  Number of 1's in @p str.
         */
        int popcount(const byte* str, size_t start, size_t length);

        /**
         * @brief  Calculates the Hamming distance between two bit strings.
         * @param s1      First bit string.
         * @param start1  Index of first bit in @p s1.
         * @param s2      Second bit string.
         * @param start2  Index of first bit in @p s2.
         * @param length  Number of bits to compare.
         * @returns  Number of bits that are different between @p s1 and @p s2.
         */
        int hammingDistance(const byte* s1, size_t start1, const byte* s2, size_t start2, size_t length);

        /**
         * @brief  Finds a pattern in a bit string within a maximum Hamming
         *         distance.
         * @param str      Bit string in which to search.
         * @param sstart   Index of first bit in @p str.
         * @param slen     Length of @p str.
         * @param patt     Bit pattern to search for.
         * @param pstart   Index of first bit in @p patt.
         * @param plen     Length of @p patt.
         * @param maxdist  Maximum allowable Hamming distance.
         * @returns  Bit index in @p str where @p patt was found, or -1 if
         *           @p patt was not found within @p maxdist.
         *
         * Searches @a str for a position at which the Hamming distance between
         * @a str and @a patt is less than or equal to @a maxdist.
         */
        int find(const byte* str, size_t sstart, size_t slen,
                 const byte* patt, size_t pstart, size_t plen,
                 int maxdist);

        /**
         * @brief  Performs a take/skip operation.
         * @param dest    Destination bit string.
         * @param dstart  Index of first bit in @p dest.
         * @param src     Source bit string.
         * @param sstart  Index of first bit in @p src.
         * @param slen    Length of @p src.
         * @param take    Number of bits to copy per iteration.
         * @param skip    Number of bits to skip per iteration.
         * @returns  Number of bits copied.
         *
         * Alternately copies @a take bits and skips @a skip bits from @a src
         * into @a dest.
         */
        size_t takeskip(byte* dest, size_t dstart,
                        const byte* src, size_t sstart, size_t slen,
                        size_t take, size_t skip);
    }

}

#endif // REDHAWK_BITOPS_H
