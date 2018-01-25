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

#include <ossie/bitbuffer.h>

CPPUNIT_TEST_SUITE_REGISTRATION(BitBufferTest);

void BitBufferTest::setUp()
{
}

void BitBufferTest::tearDown()
{
}

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
}

void BitBufferTest::testReplace()
{
    // Destination is all 0's
    redhawk::bitbuffer dest(36);
    dest.fill(0, dest.size(), 0);

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

    // 4-argument version: replace
    //   1000(11 00|110001 10|1)101
    // 000000(11|00 110001|10 1)00000 = 0x0331A0
    dest.replace(22, 13, (const redhawk::shared_bitbuffer&) src, 4);
    CPPUNIT_ASSERT_EQUAL((data_type) 0x03, data[2]);
    CPPUNIT_ASSERT_EQUAL((data_type) 0x31, data[3]);
    CPPUNIT_ASSERT_EQUAL((data_type) 0xA0, data[4]);
}
