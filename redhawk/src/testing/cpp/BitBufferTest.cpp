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

#include "BitBufferTest.h"

#include <cstring>
#include <stdexcept>

#include <ossie/bitbuffer.h>

CPPUNIT_TEST_SUITE_REGISTRATION(BitBufferTest);

void BitBufferTest::testDefaultConstructor()
{
    // Empty const shared bitbuffer
    const redhawk::shared_bitbuffer shared;
    CPPUNIT_ASSERT(shared.size() == 0);
    CPPUNIT_ASSERT(shared.empty());

    // Empty regular bitbuffer
    redhawk::bitbuffer buffer;
    CPPUNIT_ASSERT(buffer.size() == 0);
    CPPUNIT_ASSERT(buffer.empty());
}

void BitBufferTest::testConstructor()
{
    // Test allocating constructor
    const size_t NUM_BITS = 16;
    redhawk::bitbuffer buffer(NUM_BITS);
    CPPUNIT_ASSERT(!buffer.empty());
    CPPUNIT_ASSERT_EQUAL(NUM_BITS, buffer.size());

    // Test construction of shared buffer from mutable buffer
    redhawk::shared_bitbuffer shared(buffer);
    CPPUNIT_ASSERT(!shared.empty());
    CPPUNIT_ASSERT_EQUAL(buffer.size(), shared.size());
    CPPUNIT_ASSERT(shared.data() == buffer.data());
}

void BitBufferTest::testFromInt()
{
    // Input value is right-aligned (i.e., take lowest 28 bits)
    redhawk::bitbuffer buffer = redhawk::bitbuffer::from_int(0xBADC0DE, 28);
    // Bit buffer should be left-aligned
    CPPUNIT_ASSERT_EQUAL((size_t) 0, buffer.offset());
    const data_type* data = buffer.data();
    CPPUNIT_ASSERT_EQUAL((data_type) 0xBA, data[0]);
    CPPUNIT_ASSERT_EQUAL((data_type) 0xDC, data[1]);
    CPPUNIT_ASSERT_EQUAL((data_type) 0x0D, data[2]);
    CPPUNIT_ASSERT_EQUAL((data_type) 0xE0, (data_type) (data[3] & 0xF0));
}

void BitBufferTest::testFromArray()
{
    // Test with a large array and offset of 0
    const data_type array[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x11, 0x22 };
    const size_t bits = sizeof(array) * 8;
    redhawk::bitbuffer buffer = redhawk::bitbuffer::from_array(array, bits);
    // Bit buffer should be left-aligned, new memory, and equivalent
    CPPUNIT_ASSERT_EQUAL((size_t) 0, buffer.offset());
    CPPUNIT_ASSERT(buffer.data() != array);
    CPPUNIT_ASSERT_ARRAYS_EQUAL(array, buffer.data(), sizeof(array));

    // Test with a non-zero offset and non-integral number of bytes
    buffer = redhawk::bitbuffer::from_array(array, 4, 18);
    // Bit buffer should be left-aligned, new memory, and equivalent
    CPPUNIT_ASSERT_EQUAL((size_t) 0, buffer.offset());
    CPPUNIT_ASSERT(buffer.data() != array);
    int status = redhawk::bitops::compare(buffer.data(), buffer.offset(), array, 4, buffer.size());
    CPPUNIT_ASSERT_MESSAGE("Offset array is not equal", status == 0);
}

void BitBufferTest::testFromString()
{
    const std::string literal = "0101110101101011010101";
    redhawk::bitbuffer buffer = redhawk::bitbuffer::from_string(literal);
    CPPUNIT_ASSERT_EQUAL(literal.size(), buffer.size());
    const data_type* data = buffer.data();
    CPPUNIT_ASSERT_EQUAL((data_type) 0x5D, data[0]);
    CPPUNIT_ASSERT_EQUAL((data_type) 0x6B, data[1]);
    CPPUNIT_ASSERT_EQUAL((data_type) 0x54, (data_type) (data[2] & 0xFC));

    // Exceptions
    CPPUNIT_ASSERT_THROW(redhawk::bitbuffer::from_string("0101101q"), std::invalid_argument);
}

void BitBufferTest::testFromUnpacked()
{
    const redhawk::bitops::byte unpacked[] = { 0, 1, 0, 1, 0, 0, 0, 1 , 0, 2, 0xFF, 0x80 };
    const size_t bits = sizeof(unpacked);
    redhawk::bitbuffer buffer = redhawk::bitbuffer::from_unpacked(unpacked, bits);
    CPPUNIT_ASSERT_EQUAL(bits, buffer.size());
    const data_type* data = buffer.data();
    CPPUNIT_ASSERT_EQUAL((data_type) 0x51, data[0]);
    CPPUNIT_ASSERT_EQUAL((data_type) 0x70, (data_type) (data[1] & 0xF0));
}

void BitBufferTest::testEquals()
{
    // Fill a bit buffer with a known pattern
    const int64_t pattern = 0x194AB70D385;
    redhawk::bitbuffer first = redhawk::bitbuffer::from_int(pattern, 41);
    CPPUNIT_ASSERT(first == first);

    // Shared buffer aliasing the first should always be equal
    const redhawk::shared_bitbuffer shared = first;
    CPPUNIT_ASSERT(first == shared);

    // Another buffer with different backing memory should still compare
    // equal
    redhawk::bitbuffer second = redhawk::bitbuffer::from_int(pattern, 41);
    CPPUNIT_ASSERT(second.data() != first.data());
    CPPUNIT_ASSERT(first == second);

    // Flip a bit, the comparison should now fail
    second[17] = !second[17];
    CPPUNIT_ASSERT(first != second);

    // Create a new buffer with a different size, but the same data (just
    // offset by few bits). It should compare unequal as-is; however, it should
    // compare equal if taking a slice of the original buffer to re-align them.
    redhawk::bitbuffer third = redhawk::bitbuffer::from_int(pattern, 38);
    CPPUNIT_ASSERT(third != first);
    CPPUNIT_ASSERT(third == shared.slice(3));
}

void BitBufferTest::testCopy()
{
    // Create a bit buffer with known data
    redhawk::bitbuffer original(127);
    for (size_t index = 0; index < original.size(); ++index) {
        // Value is true if index is odd
        original[index] = index & 1;
    }

    // Make a copy, and verify that it's a new underlying buffer
    redhawk::bitbuffer copy = original.copy();
    CPPUNIT_ASSERT(copy == original);
    CPPUNIT_ASSERT(copy.data() != original.data());

    // Set an even index to 1; the copy should be unaffected
    original[2] = 1;
    CPPUNIT_ASSERT_EQUAL(0, (int) copy[2]);
}

void BitBufferTest::testSwap()
{
    // Create two mutable bit buffers with different contents
    redhawk::bitbuffer first(31);
    first.fill(1);
    redhawk::bitbuffer second(24);
    second.fill(0);

    // Swap them and check that the swap worked as expected
    first.swap(second);
    CPPUNIT_ASSERT_EQUAL((size_t) 24, first.size());
    CPPUNIT_ASSERT_EQUAL(0, (int) first[0]);
    CPPUNIT_ASSERT_EQUAL((size_t) 31, second.size());
    CPPUNIT_ASSERT_EQUAL(1, (int) second[0]);

    // Create shared bit buffer aliases for each buffer
    redhawk::shared_bitbuffer shared_first = first;
    redhawk::shared_bitbuffer shared_second = second;

    // Swap the shared buffers and make sure that the underlying data pointers
    // are correct
    shared_first.swap(shared_second);
    CPPUNIT_ASSERT(shared_first.data() == second.data());
    CPPUNIT_ASSERT(shared_second.data() == first.data());
}

void BitBufferTest::testResize()
{
    // Fill a bit buffer with known byte data
    const data_type expected[] = { 0xB3, 0x47, 0xC0 };
    redhawk::bitbuffer buffer = redhawk::bitbuffer::from_array(expected, 18);

    // Resize the bit buffer, then check that the memory is new and values are
    // preserved
    const data_type* data = buffer.data();
    buffer.resize(31);
    CPPUNIT_ASSERT_EQUAL((size_t) 31, buffer.size());
    CPPUNIT_ASSERT(buffer.data() != data);
    data = buffer.data();
    CPPUNIT_ASSERT_EQUAL(expected[0], data[0]);
    CPPUNIT_ASSERT_EQUAL(expected[1], data[1]);
    data_type mask = 0xC0;
    CPPUNIT_ASSERT_EQUAL((data_type) (expected[2] & mask), (data_type) (data[2] & mask));

    // Resize down (which can be done better with trim, but is still legal) and
    // check values
    buffer.resize(13);
    CPPUNIT_ASSERT_EQUAL((size_t) 13, buffer.size());
    data = buffer.data();
    CPPUNIT_ASSERT_EQUAL(expected[0], data[0]);
    mask = 0xF8;
    CPPUNIT_ASSERT_EQUAL((data_type) (expected[1] & mask), (data_type) (data[1] & mask));
}

void BitBufferTest::testFill()
{
    // Create a new buffer and set the underlying memory to all ones
    redhawk::bitbuffer buffer(64);
    std::memset(buffer.data(), 0xFF, 8);

    // Fill the entire buffer with all zeros
    buffer.fill(0);

    data_type expected[8];
    std::memset(expected, 0, sizeof(expected));
    CPPUNIT_ASSERT_ARRAYS_EQUAL(expected, buffer.data(), sizeof(expected));

    // Fill a subset of the buffer with ones
    buffer.fill(9, 33, 1);
    expected[1] = 0x7F;
    expected[2] = expected[3] = 0xFF;
    expected[4] = 0x80;
    CPPUNIT_ASSERT_ARRAYS_EQUAL(expected, buffer.data(), sizeof(expected));

    // Implicit offset and non-byte-aligned end
    buffer.trim(42, 47);
    buffer.fill(1);
    CPPUNIT_ASSERT_EQUAL((data_type) 0x3E, *buffer.data());
}

void BitBufferTest::testIndexAccess()
{
    // Bit pattern: 001 0101 0011 1100
    const redhawk::bitbuffer buffer = redhawk::bitbuffer::from_int(0x153C, 15);
    CPPUNIT_ASSERT_EQUAL(0, buffer[0]);
    CPPUNIT_ASSERT_EQUAL(0, buffer[1]);
    CPPUNIT_ASSERT_EQUAL(1, buffer[2]);
    CPPUNIT_ASSERT_EQUAL(0, buffer[3]);
    CPPUNIT_ASSERT_EQUAL(1, buffer[4]);

    // Create a shared alias and continue checking (these should be the same
    // code path, so it's really just checking that syntactically both forms
    // are valid)
    redhawk::shared_bitbuffer shared = buffer;
    CPPUNIT_ASSERT_EQUAL(0, shared[5]);
    CPPUNIT_ASSERT_EQUAL(1, shared[6]);
    CPPUNIT_ASSERT_EQUAL(0, shared[7]);
    CPPUNIT_ASSERT_EQUAL(0, shared[8]);
    CPPUNIT_ASSERT_EQUAL(1, shared[9]);
    CPPUNIT_ASSERT_EQUAL(1, shared[10]);

    // Use slice to create a new bit buffer with a non-zero offset to test that
    // the offset is taken into account
    redhawk::shared_bitbuffer slice = shared.slice(11, 15);
    CPPUNIT_ASSERT(slice.offset() != 0);
    CPPUNIT_ASSERT_EQUAL(1, slice[0]);
    CPPUNIT_ASSERT_EQUAL(1, slice[1]);
    CPPUNIT_ASSERT_EQUAL(0, slice[2]);
    CPPUNIT_ASSERT_EQUAL(0, slice[3]);
}

void BitBufferTest::testIndexAssignment()
{
    // Start with a zero-filled buffer
    redhawk::bitbuffer buffer(48);
    buffer.fill(0);

    // Basic bit setting
    buffer[3] = 1;
    const data_type* data = buffer.data();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Set bit", (data_type) 0x10, data[0]);

    // Two bits in the same byte
    buffer[8] = 1;
    buffer[13] = 1;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Set two bits in same byte", (data_type) 0x84, data[1]);

    // Any non-zero integer should be interpreted as a 1
    buffer[18] = 2;
    buffer[22] = -5289;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Set non-zero integer", (data_type) 0x22, data[2]);

    // 0 should clear an existing bit
    buffer[8] = 0;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Clear bit", (data_type) 0x04, data[1]);

    // Transitive assignment
    buffer[24] = buffer[27] = 1;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Transitive assignment", (data_type) 0x90, data[3]);

    // Use a slice to test that offsets are accounted for
    redhawk::bitbuffer slice = buffer.slice(35, 47);
    CPPUNIT_ASSERT(slice.offset() != 0);
    slice[1] = 1;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Slice with offset", 1, (int) buffer[36]);
}

void BitBufferTest::testSharing()
{
    // Create a bitbuffer with a known pattern
    redhawk::bitbuffer buffer = redhawk::bitbuffer::from_int(0xE581, 16);

    // Create a const shared bit buffer aliasing the original
    const redhawk::shared_bitbuffer shared = buffer;
    CPPUNIT_ASSERT(shared == buffer);

    // Invert some bits and ensure that the bit buffers are still equal
    buffer[2] = !buffer[2];
    buffer[5] = !buffer[5];
    buffer[11] = !buffer[11];
    CPPUNIT_ASSERT(shared == buffer);
}

void BitBufferTest::testSlicing()
{
    // Fill a new bit buffer with alternating 0's and 1's
    redhawk::bitbuffer buffer(12);
    for (size_t index = 0; index < buffer.size(); ++index) {
        buffer[index] = index & 1;
    }

    // Take a 4-bit slice from the middle and check that it points to the
    // same data (offset by the start index)
    const redhawk::shared_bitbuffer middle = buffer.slice(4, 8);
    CPPUNIT_ASSERT_EQUAL((size_t) 4, middle.size());
    CPPUNIT_ASSERT_EQUAL((size_t) 4, middle.offset());

    // Take a slice from the midpoint to the end, and check that the bits match
    const redhawk::shared_bitbuffer end = buffer.slice(6);
    for (size_t index = 0; index < end.size(); ++index) {
        CPPUNIT_ASSERT_EQUAL((int) buffer[index + 6], end[index]);
    }

    // Compare the overlap between the two slices by taking sub-slices
    CPPUNIT_ASSERT(middle.slice(2) == end.slice(0, 2));

    // Starting at the end index should return an empty bit buffer
    redhawk::shared_bitbuffer empty = buffer.slice(buffer.size());
    CPPUNIT_ASSERT(empty.empty());

    // Exceptions
    CPPUNIT_ASSERT_THROW(buffer.slice(buffer.size() + 1), std::out_of_range);
    CPPUNIT_ASSERT_THROW(buffer.slice(1, 0), std::invalid_argument);
}

void BitBufferTest::testTrim()
{
    // Tests bit buffer trimming. Only shared_bitbuffer needs to be tested,
    // because bitbuffer inherits and does not override this function.

    // Start with known bit pattern: 1000 1010 1100 1101 1100
    redhawk::shared_bitbuffer buffer = redhawk::bitbuffer::from_int(0x8ACDC, 20);
    const data_type* data = buffer.data();

    // Use 1-argument trim to remove the first 4 bits. The data pointer should
    // remain the same.
    buffer.trim(4);
    CPPUNIT_ASSERT_EQUAL(data, buffer.data());
    CPPUNIT_ASSERT_EQUAL((size_t) 4, buffer.offset());
    CPPUNIT_ASSERT_EQUAL((size_t) 16, buffer.size());

    // 1000 (1010 1100 1101 1100)
    redhawk::shared_bitbuffer expected = redhawk::bitbuffer::from_int(0xACDC, 16);
    CPPUNIT_ASSERT(expected == buffer);

    // Use 2-argument trim to 9-bit range. The data pointer should advance
    // and the offset should wrap around.
    buffer.trim(5, 14);
    CPPUNIT_ASSERT(buffer.data() != data);
    CPPUNIT_ASSERT_EQUAL((size_t) 1, buffer.offset());
    CPPUNIT_ASSERT_EQUAL((size_t) 9, buffer.size());

    // 1010 1(100 1101 11)00
    expected = redhawk::bitbuffer::from_int(0x137, 9);
    CPPUNIT_ASSERT(expected == buffer);

    // Trim starting at the end index should result in empty buffer
    redhawk::shared_bitbuffer empty = buffer;
    empty.trim(empty.size());
    CPPUNIT_ASSERT(empty.empty());

    // Exceptions
    CPPUNIT_ASSERT_THROW(buffer.trim(buffer.size() + 1), std::out_of_range);
    CPPUNIT_ASSERT_THROW(buffer.trim(1, 0), std::invalid_argument);
}

void BitBufferTest::testReplace()
{
    // Destination is all 0's
    redhawk::bitbuffer dest(36);
    dest.fill(0);

    // Set known pattern in source
    // 10001100|11000110|1101xxxx
    redhawk::shared_bitbuffer src = redhawk::bitbuffer::from_int(0x8CC6D, 20);

    // 3-argument version: replace 9 bits at offset 1
    //  (1000110 0|1)100
    // 0(1000110|0 1)000000 = 0x4640
    dest.replace(1, 9, (const redhawk::shared_bitbuffer&) src);
    const data_type* data = dest.data();
    CPPUNIT_ASSERT_EQUAL((data_type) 0x46, data[0]);
    CPPUNIT_ASSERT_EQUAL((data_type) 0x40, data[1]);

    // 4-argument version: replace 13 bits at offset 22, starting with the 4th
    // bit of the source
    //   1000(11 00|110001 10|1)101
    // 000000(11|00 110001|10 1)0xxxx = 0x0331A
    dest.replace(22, 13, (const redhawk::shared_bitbuffer&) src, 4);
    CPPUNIT_ASSERT_EQUAL((data_type) 0x03, data[2]);
    CPPUNIT_ASSERT_EQUAL((data_type) 0x31, data[3]);
    CPPUNIT_ASSERT_EQUAL((data_type) 0xA0, (data_type) (data[4] & 0xF0));
}

void BitBufferTest::testGetInt()
{
    const data_type pattern[] = {
        0x35, 0x45, 0xE6, 0xA9, 0xA1, 0x1B, 0xAA, 0xE4, 0x9A, 0x3F, 0x3B, 0x38
    };
    const redhawk::shared_bitbuffer buffer = redhawk::bitbuffer::from_array(pattern, sizeof(pattern)*8);

    // Small value
    CPPUNIT_ASSERT_EQUAL(3, (int) buffer.getint(0, 4));

    // Multi-byte with offset
    // 0x3545E6A9 = 001(10101|01000101|11100110|10101001)
    CPPUNIT_ASSERT_EQUAL((uint64_t) 0x1545E6A9, buffer.getint(3, 29));

    // Implicit offset (slice)
    redhawk::shared_bitbuffer slice = buffer.slice(2, 32);
    CPPUNIT_ASSERT_EQUAL((uint64_t) 0x1545E6A9, slice.getint(1, 29));

    // Maximum size (64 bits)
    CPPUNIT_ASSERT_EQUAL((uint64_t) 0xA11BAAE49A3F3B38, buffer.getint(32, 64));

    // Exceptions
    CPPUNIT_ASSERT_THROW(buffer.getint(buffer.size(), 1), std::out_of_range);
    CPPUNIT_ASSERT_THROW(buffer.getint(0, 65), std::length_error);
}

void BitBufferTest::testSetInt()
{
    redhawk::bitbuffer buffer(96);
    buffer.fill(0);
    const data_type* data = buffer.data();

    // Small value
    buffer.setint(0, 0x03, 4);
    CPPUNIT_ASSERT_EQUAL((data_type) 0x30, data[0]);

    // Multi-byte with offset
    buffer.setint(4, 0x6A8BCD, 24);
    CPPUNIT_ASSERT_EQUAL((data_type) 0x36, data[0]);
    CPPUNIT_ASSERT_EQUAL((data_type) 0xA8, data[1]);
    CPPUNIT_ASSERT_EQUAL((data_type) 0xBC, data[2]);
    CPPUNIT_ASSERT_EQUAL((data_type) 0xD0, data[3]);

    // Implicit offset (slice)
    //        00110110|10101000
    // slice    110110|101
    // set       01001|0
    // =      00101001|00101000
    redhawk::bitbuffer slice = buffer.slice(2, 11);
    slice.setint(1, 0x12, 6);
    CPPUNIT_ASSERT_EQUAL((data_type) 0x29, data[0]);
    CPPUNIT_ASSERT_EQUAL((data_type) 0x28, data[1]);

    // Maximum size (64 bits)
    const data_type expected[] = {
        0xA1, 0x1B, 0xAA, 0xE4, 0x9A, 0x3F, 0x3B, 0x38
    };
    buffer.setint(32, 0xA11BAAE49A3F3B38, 64);
    CPPUNIT_ASSERT_ARRAYS_EQUAL(expected, data + 4, sizeof(expected));

    // Exceptions
    CPPUNIT_ASSERT_THROW(buffer.setint(buffer.size(), 0, 1), std::out_of_range);
    CPPUNIT_ASSERT_THROW(buffer.setint(0, 0, 65), std::length_error);
}

void BitBufferTest::testPopcount()
{
    // 10111001000100011101000110111100001
    redhawk::shared_bitbuffer buffer = redhawk::bitbuffer::from_int(0x5C88E8DE1, 35);
    CPPUNIT_ASSERT_EQUAL(17, buffer.popcount());

    // 100011101000110111100
    buffer.trim(11, buffer.size() - 3);
    CPPUNIT_ASSERT_EQUAL(11, buffer.popcount());

    // 1010001
    buffer.trim(6, buffer.size() - 8);
    CPPUNIT_ASSERT_EQUAL(3, buffer.popcount());
}

void BitBufferTest::testDistance()
{
    // 110100010011110100011001111100111
    redhawk::shared_bitbuffer first = redhawk::bitbuffer::from_int(0x1A27A33E7, 33);

    // 000101011111111010011110100000011
    redhawk::shared_bitbuffer second = redhawk::bitbuffer::from_int(0x02BFD3D03, 33);

    // Distance from self should always be 0
    CPPUNIT_ASSERT_EQUAL(0, first.distance(first));

    // a         110100010011110100011001111100111
    // b         000101011111111010011110100000011
    // a XOR b = 110001001100001110000111011100100
    CPPUNIT_ASSERT_EQUAL(15, first.distance(second));
}

void BitBufferTest::testFind()
{
    // Pick a oddly-sized pattern
    redhawk::shared_bitbuffer pattern = redhawk::bitbuffer::from_int(0x2C07BE, 22);

    // Fill a bit buffer with 1's, then copy the pattern into it in a couple of
    // places
    redhawk::bitbuffer buffer(300);
    buffer.fill(1);
    buffer.replace(37, pattern.size(), pattern);
    buffer.replace(200, pattern.size(), pattern);

    // 2-argument find searches from the beginning
    CPPUNIT_ASSERT_EQUAL((size_t) 37, buffer.find(pattern, 0));

    // Use the optional 3rd argument to start after the first occurrence
    CPPUNIT_ASSERT_EQUAL((size_t) 200, buffer.find(59, pattern, 0));

    // Finally, the search should fail when started after both occurrences
    CPPUNIT_ASSERT_EQUAL(redhawk::bitbuffer::npos, buffer.find(222, pattern, 0));

    // Introduce some bit errors
    buffer[38] = !buffer[38];
    buffer[48] = !buffer[48];
    buffer[220] = !buffer[220];

    // Try decreasing tolerances
    CPPUNIT_ASSERT_EQUAL((size_t) 37, buffer.find(pattern, 2));
    CPPUNIT_ASSERT_EQUAL((size_t) 200, buffer.find(pattern, 1));
    CPPUNIT_ASSERT_EQUAL(redhawk::bitbuffer::npos, buffer.find(pattern, 0));

    // Starting the search past the end of the bit buffer should always fail,
    // but without an exception (or crash)
    CPPUNIT_ASSERT_EQUAL(redhawk::bitbuffer::npos, buffer.find(buffer.size(), pattern, 0));
}


static std::string ascii7toString(const redhawk::shared_bitbuffer& ascii)
{
    std::string result;
    result.resize(ascii.size() / 7);
    for (size_t bit = 0; bit < ascii.size(); bit += 7) {
        result[bit/7] = (char) ascii.getint(bit, 7);
    }
    return result;
}

void BitBufferTest::testTakeSkip()
{
    // Use ASCII text, where the high bit is always zero; a start offset is
    // required
    const std::string msg = "Here is some text";
    redhawk::shared_bitbuffer buffer = redhawk::bitbuffer::from_array((unsigned char*) msg.c_str(), msg.size() * 8);
    // Take 7, skip 1, start at 1
    redhawk::bitbuffer ascii = buffer.takeskip(7, 1, 1);
    CPPUNIT_ASSERT_EQUAL(msg.size() * 7, ascii.size());

    // Reconstruct the input text by taking 7 bits at a time
    std::string result = ascii7toString(ascii);
    CPPUNIT_ASSERT_EQUAL(msg, result);

    // Repeat with a starting and ending offset
    // char 5 = bit 40 (+1 to skip high bit)
    // char 12 = bit 96 (+1 to skip high bit)
    ascii = buffer.takeskip(7, 1, 41, 97);
    result = ascii7toString(ascii);
    CPPUNIT_ASSERT_EQUAL(msg.substr(5, 7), result);

    // Exceptions
    // Start index past end of source
    CPPUNIT_ASSERT_THROW(buffer.takeskip(7, 1, buffer.size() + 1), std::out_of_range);
    // End less than start
    CPPUNIT_ASSERT_THROW(buffer.takeskip(7, 1, 19, 18), std::invalid_argument);
}

void BitBufferTest::testTakeSkipIntoBuffer()
{
    // Use a 28-bit marker and an 8-bit counter
    const uint32_t marker = 0x7C3ABA9;
    uint8_t counter = 0;

    // Source buffer holds 8 "frames"
    redhawk::bitbuffer src(288);
    src.fill(0);
    for (size_t pos = 0; pos < src.size(); pos += 36) {
        src.setint(pos, marker, 28);
        src.setint(pos+28, counter, 8);
        ++counter;
    }

    // Do a take/skip to copy just the markers out of the source
    redhawk::bitbuffer dest(28*8);
    dest.fill(0);
    size_t bits = dest.takeskip(src, 28, 8);
    CPPUNIT_ASSERT_EQUAL(dest.size(), bits);
    for (size_t pos = 0; pos < dest.size(); pos += 28) {
        std::ostringstream oss;
        oss << "position " << pos;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(oss.str(), (uint64_t) marker, dest.getint(pos, 28));
    }

    // Use a start index to copy the counters, and an end index to limit the
    // bits copied
    dest = redhawk::bitbuffer(6*8);
    dest.fill(0);
    bits = dest.takeskip(src, 8, 28, 28, 6*36);
    CPPUNIT_ASSERT_EQUAL(dest.size(), bits);
    counter = 0;
    for (size_t pos = 0; pos < dest.size(); pos += 8) {
        std::ostringstream oss;
        oss << "position " << pos;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(oss.str(), (uint64_t) counter, dest.getint(pos, 8));
        ++counter;
    }

    // Exceptions
    // Start index past end of source
    CPPUNIT_ASSERT_THROW(dest.takeskip(src, 28, 8, src.size() + 1), std::out_of_range);
    // End less than start
    CPPUNIT_ASSERT_THROW(dest.takeskip(src, 28, 8, 1, 0), std::invalid_argument);
    // Destination too small
    CPPUNIT_ASSERT_THROW(dest.takeskip(src, 40, 1), std::length_error);
}
