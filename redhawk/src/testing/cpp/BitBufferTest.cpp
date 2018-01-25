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
    // Bit buffer should be left-aligned
    CPPUNIT_ASSERT_EQUAL((size_t) 0, buffer.offset());
    CPPUNIT_ASSERT_ARRAYS_EQUAL(array, buffer.data(), sizeof(array));

    // Test with a non-zero offset and non-integral number of bytes
    buffer = redhawk::bitbuffer::from_array(array, 4, 18);
    // Bit buffer should be left-aligned
    CPPUNIT_ASSERT_EQUAL((size_t) 0, buffer.offset());
    const data_type* data = buffer.data();
    CPPUNIT_ASSERT_EQUAL((data_type) 0x12, data[0]);
    CPPUNIT_ASSERT_EQUAL((data_type) 0x34, data[1]);
    CPPUNIT_ASSERT_EQUAL((data_type) 0x40, (data_type) (data[2] & 0xC0));
}

void BitBufferTest::testSharing()
{
    redhawk::bitbuffer buffer(16);
    *(buffer.data()) = 0xE5;
    *(buffer.data() + 1) = 0x81;

    redhawk::shared_bitbuffer shared = buffer;
    CPPUNIT_ASSERT(shared == buffer);

    *(buffer.data()) = ~(*buffer.data());
    CPPUNIT_ASSERT(shared == buffer);
}

void BitBufferTest::testSlicing()
{
    redhawk::bitbuffer buffer(12);
    buffer.fill(0, 12, 0);
    for (size_t index = 1; index < buffer.size(); index += 2) {
        buffer[index] = 1;
    }

    const redhawk::shared_bitbuffer middle = buffer.slice(4, 8);
    CPPUNIT_ASSERT_EQUAL((size_t) 4, middle.size());
    CPPUNIT_ASSERT_EQUAL((size_t) 4, middle.offset());

    const redhawk::shared_bitbuffer end = buffer.slice(6);
    for (size_t index = 0; index < end.size(); ++index) {
        CPPUNIT_ASSERT_EQUAL((int) buffer[index + 6], end[index]);
    }

    CPPUNIT_ASSERT(middle.slice(2) == end.slice(0, 2));
}
