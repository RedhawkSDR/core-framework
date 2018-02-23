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

#include "BitopsTest.h"

#include <iostream>
#include <iomanip>
#include <cstring>
#include <vector>
#include <stdexcept>

#include <stdint.h>

#include <ossie/bitops.h>

void BitopsTest::setUp()
{
}

void BitopsTest::tearDown()
{
}

void BitopsTest::testGetBit()
{
    const unsigned char data[] = { 0x5a, 0x18, 0xe7 };

    int expected[] = {
        0, 1, 0, 1, 1, 0, 1, 0, // 0x5a
        0, 0, 0, 1, 1, 0, 0, 0, // 0x18
        1, 1, 1, 0, 0, 1, 1, 1, // 0xe7
    };
    const size_t bits = sizeof(data) * 8;

    for (size_t pos = 0; pos < bits; ++pos) {
        if (expected[pos] !=  redhawk::bitops::getbit(data, pos)) {
            std::ostringstream message;
            message << "bit position " << pos << " differs";
            CPPUNIT_FAIL(message.str());
        }
    }
}

void BitopsTest::testSetBit()
{
    unsigned char data[] = { 0x00, 0xff, 0x3a };

    redhawk::bitops::setbit(data, 1, 1);
    redhawk::bitops::setbit(data, 6, 1);
    redhawk::bitops::setbit(data, 7, 1);
    CPPUNIT_ASSERT_EQUAL(0x43, (int) data[0]);
    CPPUNIT_ASSERT_EQUAL(0xff, (int) data[1]);
    CPPUNIT_ASSERT_EQUAL(0x3a, (int) data[2]);

    redhawk::bitops::setbit(data, 8, 0);
    redhawk::bitops::setbit(data, 11, 0);
    redhawk::bitops::setbit(data, 15, 0);
    CPPUNIT_ASSERT_EQUAL(0x43, (int) data[0]);
    CPPUNIT_ASSERT_EQUAL(0x6e, (int) data[1]);
    CPPUNIT_ASSERT_EQUAL(0x3a, (int) data[2]);

    redhawk::bitops::setbit(data, 16, 1);
    redhawk::bitops::setbit(data, 18, 0);
    redhawk::bitops::setbit(data, 20, 0);
    redhawk::bitops::setbit(data, 21, 1);
    CPPUNIT_ASSERT_EQUAL(0x43, (int) data[0]);
    CPPUNIT_ASSERT_EQUAL(0x6e, (int) data[1]);
    CPPUNIT_ASSERT_EQUAL(0x96, (int) data[2]);
}

void BitopsTest::testGetInt()
{
    const unsigned char packed[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF
    };

    // Read increasing sizes up to full 64-bit value
    uint64_t value = 0x0123456789ABCDEF;
    for (size_t bits = 64; bits > 0; bits -= 4) {
        CPPUNIT_ASSERT_EQUAL(value, redhawk::bitops::getint(packed, 0, bits));
        value >>= 4;
    }

    // 4-bit (nibble) aligned reads
    value = 0;
    for (size_t pos = 0; pos < 16; ++pos) {
        CPPUNIT_ASSERT_EQUAL(value, redhawk::bitops::getint(packed, pos*4, 4));
        value += 1;
    }

    // Byte-aligned 8-bit reads
    value = 0x01;
    for (size_t pos = 0; pos < 8; ++pos) {
        CPPUNIT_ASSERT_EQUAL(value, redhawk::bitops::getint(packed, pos*8, 8));
        value += 0x22;
    }

    // Byte-aligned 16-bit reads
    value = 0x0123;
    for (size_t pos = 0; pos < 7; ++pos) {
        CPPUNIT_ASSERT_EQUAL(value, redhawk::bitops::getint(packed, pos*8, 16));
        value += 0x2222;
    }

    // Byte-aligned 32-bit reads
    value = 0x01234567;
    for (size_t pos = 0; pos < 5; ++pos) {
        CPPUNIT_ASSERT_EQUAL(value, redhawk::bitops::getint(packed, pos*8, 32));
        value += 0x22222222;
    }

    // More than 64 bits is an error
    CPPUNIT_ASSERT_THROW(redhawk::bitops::getint(packed, 0, 65), std::length_error);
}

void BitopsTest::testGetIntUnaligned()
{
    const unsigned char packed[] = {
        0xE4, 0xBC, 0x4F // 11100100|10111100|01001111
    };

    // 111(00100|1)0111100 = 001001
    CPPUNIT_ASSERT_EQUAL((uint64_t) 0x09, redhawk::bitops::getint(packed, 3, 6));

    // 11(100100|10111100) = 100100|10111100 = 0x24BC
    CPPUNIT_ASSERT_EQUAL((uint64_t) 0x24BC, redhawk::bitops::getint(packed, 2, 14));

    // 11100(100|10111100|010011)11 = 1|00101111|00010011 = 0x12F13
    CPPUNIT_ASSERT_EQUAL((uint64_t) 0x12F13, redhawk::bitops::getint(packed, 5, 17));
}

void BitopsTest::testGetIntUnalignedSmall()
{
    const unsigned char packed = 0xA7;
    CPPUNIT_ASSERT_EQUAL((uint64_t) 0x53, redhawk::bitops::getint(&packed, 0, 7));
    CPPUNIT_ASSERT_EQUAL((uint64_t) 0x13, redhawk::bitops::getint(&packed, 1, 6));
    CPPUNIT_ASSERT_EQUAL((uint64_t) 0x13, redhawk::bitops::getint(&packed, 2, 5));
    CPPUNIT_ASSERT_EQUAL((uint64_t) 0x01, redhawk::bitops::getint(&packed, 3, 3));
    CPPUNIT_ASSERT_EQUAL((uint64_t) 0x03, redhawk::bitops::getint(&packed, 4, 3));
    CPPUNIT_ASSERT_EQUAL((uint64_t) 0x03, redhawk::bitops::getint(&packed, 5, 2));
    CPPUNIT_ASSERT_EQUAL((uint64_t) 0x03, redhawk::bitops::getint(&packed, 6, 2));
    CPPUNIT_ASSERT_EQUAL((uint64_t) 0x01, redhawk::bitops::getint(&packed, 7, 1));
}

void BitopsTest::testSetInt()
{
    unsigned char packed[8];
    std::fill(packed, packed + sizeof(packed), 0);

    const unsigned char expected[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF
    };

    // 4-bit (nibble) writes
    std::fill(packed, packed + sizeof(packed), 0xFF);
    redhawk::bitops::setint(packed, 0, 0x0, 4);
    CPPUNIT_ASSERT_EQUAL(0x0F, (int) packed[0]);
    CPPUNIT_ASSERT_EQUAL(0xFF, (int) packed[1]);
    redhawk::bitops::setint(packed, 4, 0x1, 4);
    CPPUNIT_ASSERT_EQUAL(0x01, (int) packed[0]);
    CPPUNIT_ASSERT_EQUAL(0xFF, (int) packed[1]);

    // 8-bit write
    redhawk::bitops::setint(packed, 8, 0x23, 8);
    CPPUNIT_ASSERT_ARRAYS_EQUAL(expected, packed, 2);
    CPPUNIT_ASSERT_EQUAL(0xFF, (int) packed[2]);

    // 16-bit write
    redhawk::bitops::setint(packed, 16, 0x4567, 16);
    CPPUNIT_ASSERT_ARRAYS_EQUAL(expected, packed, 4);
    CPPUNIT_ASSERT_EQUAL(0xFF, (int) packed[4]);

    // 32-bit write
    redhawk::bitops::setint(packed, 32, 0x89ABCDEF, 32);
    CPPUNIT_ASSERT_ARRAYS_EQUAL(expected, packed, 8);

    // 64-bit write
    std::fill(packed, packed + sizeof(packed), 0xFF);
    redhawk::bitops::setint(packed, 0, 0x0123456789ABCDEF, 64);
    CPPUNIT_ASSERT_ARRAYS_EQUAL(expected, packed, 8);

    // More than 64 bits is an error
    CPPUNIT_ASSERT_THROW(redhawk::bitops::setint(packed, 0, 0, 65), std::length_error);
}

void BitopsTest::testSetIntUnaligned()
{
    unsigned char packed[] = {
        0xE4, 0xBC, 0x4F // 11100100|10111100|01001111
    };

    // 111(11100|0)0111100 = 0xFC3C
    // 111000 = 0x38
    redhawk::bitops::setint(packed, 3, 0x38, 6);
    CPPUNIT_ASSERT_EQUAL(0xFC, (int) packed[0]);
    CPPUNIT_ASSERT_EQUAL(0x3C, (int) packed[1]);
    CPPUNIT_ASSERT_EQUAL(0x4F, (int) packed[2]);

    // 11(000010|11010011) = 0xC2D3
    // 000010|11010011 = 0x02D3
    redhawk::bitops::setint(packed, 2, 0x02D3, 14);
    CPPUNIT_ASSERT_EQUAL(0xC2, (int) packed[0]);
    CPPUNIT_ASSERT_EQUAL(0xD3, (int) packed[1]);
    CPPUNIT_ASSERT_EQUAL(0x4F, (int) packed[2]);

    // 11000(100|01111001|100000)11 = 0xC47983
    // 1|00011110|01100000 = 0x11E60
    redhawk::bitops::setint(packed, 5, 0x11E60, 17);
    CPPUNIT_ASSERT_EQUAL(0xC4, (int) packed[0]);
    CPPUNIT_ASSERT_EQUAL(0x79, (int) packed[1]);
    CPPUNIT_ASSERT_EQUAL(0x83, (int) packed[2]);
}

void BitopsTest::testSetIntUnalignedSmall()
{
    // First byte is initial state, second byte is guard byte
    unsigned char packed[] = { 0x11, 0xBB };

    redhawk::bitops::setint(packed, 0, 0x41, 7);
    CPPUNIT_ASSERT_EQUAL(0x83, (int) packed[0]);
    CPPUNIT_ASSERT_EQUAL(0xBB, (int) packed[1]);

    redhawk::bitops::setint(packed, 1, 0x1B, 5);
    CPPUNIT_ASSERT_EQUAL(0xEF, (int) packed[0]);
    CPPUNIT_ASSERT_EQUAL(0xBB, (int) packed[1]);

    redhawk::bitops::setint(packed, 2, 0x02, 3);
    CPPUNIT_ASSERT_EQUAL(0xD7, (int) packed[0]);
    CPPUNIT_ASSERT_EQUAL(0xBB, (int) packed[1]);

    redhawk::bitops::setint(packed, 3, 0x04, 4);
    CPPUNIT_ASSERT_EQUAL(0xC9, (int) packed[0]);
    CPPUNIT_ASSERT_EQUAL(0xBB, (int) packed[1]);

    redhawk::bitops::setint(packed, 4, 0x03, 3);
    CPPUNIT_ASSERT_EQUAL(0xC7, (int) packed[0]);
    CPPUNIT_ASSERT_EQUAL(0xBB, (int) packed[1]);

    redhawk::bitops::setint(packed, 5, 0x00, 2);
    CPPUNIT_ASSERT_EQUAL(0xC1, (int) packed[0]);
    CPPUNIT_ASSERT_EQUAL(0xBB, (int) packed[1]);

    redhawk::bitops::setint(packed, 6, 0x02, 2);
    CPPUNIT_ASSERT_EQUAL(0xC2, (int) packed[0]);
    CPPUNIT_ASSERT_EQUAL(0xBB, (int) packed[1]);

    redhawk::bitops::setint(packed, 7, 0x01, 1);
    CPPUNIT_ASSERT_EQUAL(0xC3, (int) packed[0]);
    CPPUNIT_ASSERT_EQUAL(0xBB, (int) packed[1]);
}

void BitopsTest::testFill()
{
    // Basic byte-aligned fill
    // pre:  00000000|00000000|00000000|00000000|00000000
    //       ^^^^^^^^ ^^^^^^^^ ^^^^^^^^ ^^^^^^^^
    // post: 11111111|11111111|11111111|11111111|00000000
    unsigned char expected[] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0x00
    };
    unsigned char buffer[sizeof(expected)];
    memset(buffer, 0, sizeof(buffer));
    redhawk::bitops::fill(buffer, 0, 32, 1);
    CPPUNIT_ASSERT_ARRAYS_EQUAL(expected, buffer, sizeof(expected));

    // Clear first 13 bits (partial byte on end)
    // pre:  11111111|11111111|11111111|11111111|00000000
    //       ^^^^^^^^ ^^^^^
    // post: 00000000|00000111|11111111|11111111|00000000
    expected[0] = 0x00;
    expected[1] = 0x07;
    redhawk::bitops::fill(buffer, 0, 13, 0);
    CPPUNIT_ASSERT_ARRAYS_EQUAL(expected, buffer, sizeof(expected));

    // Middle 5 bits in first byte
    // pre:  00000000|00000111|11111111|11111111|00000000
    //         ^^^^^
    // post: 00111110|00000111|11111111|11111111|00000000
    expected[0] = 0x3E;
    redhawk::bitops::fill(buffer, 2, 5, 1);
    CPPUNIT_ASSERT_ARRAYS_EQUAL(expected, buffer, sizeof(expected));

    // Clear range of bits with partial bytes at beginning and end
    // pre:  00111110|00000111|11111111|11111111|00000000
    //                      ^^ ^^^^^^^^ ^^^
    // post: 00111110|00000100|00000000|00011111|00000000
    expected[1] = 0x04;
    expected[2] = 0x00;
    expected[3] = 0x1F;
    redhawk::bitops::fill(buffer, 14, 13, 0);
    CPPUNIT_ASSERT_ARRAYS_EQUAL(expected, buffer, sizeof(expected));

    // Set right-aligned partial bits in a single byte
    // pre:  00111110|00000100|00000000|00011111|00000000
    //                          ^^^^^^^
    // post: 00111110|00000100|01111111|00011111|00000000
    expected[2] = 0x7F;
    redhawk::bitops::fill(buffer, 17, 7, 1);
    CPPUNIT_ASSERT_ARRAYS_EQUAL(expected, buffer, sizeof(expected));

    // Set left-aligned partial bits in a single byte
    // pre:  00111110|00000100|01111111|00011111|00000000
    //                ^^^^
    // post: 00111110|11110100|01111111|00011111|00000000
    expected[1] = 0xF4;
    redhawk::bitops::fill(buffer, 8, 4, 1);
    CPPUNIT_ASSERT_ARRAYS_EQUAL(expected, buffer, sizeof(expected));
}

void BitopsTest::testPack()
{
    const unsigned char implode[] = {
        1, 0, 1, 0, 0, 1, 1, 0, // 0xa6
        1, 0, 0, 1, 0, 0, 1, 1, // 0x93
        0, 1, 0, 1, 1, 0, 1, 1, // 0x5b
    };
    const size_t bits = sizeof(implode);

    std::vector<unsigned char> packed;
    packed.resize((bits + 7) / 8);
    std::fill(packed.begin(), packed.end(), 0);

    redhawk::bitops::pack(&packed[0], 0, &implode[0], bits);

    CPPUNIT_ASSERT_EQUAL(0xa6, (int) packed[0]);
    CPPUNIT_ASSERT_EQUAL(0x93, (int) packed[1]);
    CPPUNIT_ASSERT_EQUAL(0x5b, (int) packed[2]);
}

void BitopsTest::testPackSmall()
{
    const unsigned char implode[] = {
        1, 0, 1, 1, 0, 1, 1, 1, // 0xb7
    };

    unsigned char packed = 0;

    redhawk::bitops::pack(&packed, 0, &implode[0], 6);
    CPPUNIT_ASSERT_EQUAL(0xb4, (int) packed);
}

void BitopsTest::testPackUnaligned()
{
    const unsigned char implode[] = {
        0, 0, 0, 0, 0, 1, 0, 1, // 0x05
        1, 0, 0, 1, 0, 0, 1, 1, // 0x93
        1, 1, 1, 1, 1, 0, 0, 0, // 0xf8
    };
    const size_t offset = 5;
    const size_t bits = sizeof(implode) - offset - 3;

    std::vector<unsigned char> packed;
    packed.resize(3);
    packed[0] = 0x9a; // 10011010
    packed[1] = 0x00; // 00000000
    packed[2] = 0x33; // 00110011

    redhawk::bitops::pack(&packed[0], offset, &implode[offset], bits);

    // First byte should mix 5 bits from existing value with first 3 bits
    // from unpacked value: 10011(010) | (00000)101 = 10011101
    CPPUNIT_ASSERT_EQUAL(0x9d, (int) packed[0]);

    // Second byte should match the middle 8 bits of unpacked value
    CPPUNIT_ASSERT_EQUAL(0x93, (int) packed[1]);

    // Last byte should mix 3 trailing bits from existing value with last
    // 5 bits of unpacked value: (00110)011 | 11111(000) = 11111011
    CPPUNIT_ASSERT_EQUAL(0xfb, (int) packed[2]);
}

void BitopsTest::testPackUnalignedSmall()
{
    const unsigned char implode[] = {
        1, 0, 0, 1, // 0x9
    };

    unsigned char packed = 0;

    redhawk::bitops::pack(&packed, 3, &implode[0], 4);

    CPPUNIT_ASSERT_EQUAL(0x12, (int) packed);
}

void BitopsTest::testUnpack()
{
    const unsigned char packed[] = { 0xa6, 0x93, 0x5b };
    const size_t bits = sizeof(packed) * 8;

    const unsigned char expected[] = {
        1, 0, 1, 0, 0, 1, 1, 0, // 0xa6
        1, 0, 0, 1, 0, 0, 1, 1, // 0x93
        0, 1, 0, 1, 1, 0, 1, 1, // 0x5b
    };

    std::vector<unsigned char> explode;
    explode.resize(bits);
    std::fill(explode.begin(), explode.end(), 0);

    redhawk::bitops::unpack(&explode[0], &packed[0], 0, bits);

    CPPUNIT_ASSERT_ARRAYS_EQUAL(expected, &explode[0], explode.size());
}

void BitopsTest::testUnpackSmall()
{
    const unsigned char packed = 0x9f; // 10011111
    const unsigned char expected[] = {
        1, 0, 0, 1, 1, 1, 1, 1
    };
    const size_t bits = 6;
    std::vector<unsigned char> explode;
    explode.resize(bits + 1);
    std::fill(explode.begin(), explode.end(), 0);

    redhawk::bitops::unpack(&explode[0], &packed, 0, bits);

    CPPUNIT_ASSERT_ARRAYS_EQUAL(expected, &explode[0], bits);

    // Make sure the last value is untouched
    CPPUNIT_ASSERT_EQUAL(0, (int) explode[bits]);
}

void BitopsTest::testUnpackUnaligned()
{
    const size_t offset = 1;
    const size_t remain = 2;
    const unsigned char packed[] = {
        0xa6, // 1010 0110
        0x93, // 1001 0011
        0x5b, // 0101 1011
    };
    const unsigned char expected[] = {
        1, 0, 1, 0, 0, 1, 1, 0,
        1, 0, 0, 1, 0, 0, 1, 1,
        0, 1, 0, 1, 1, 0, 1, 1
    };
    const size_t bits = sizeof(expected) - (offset + remain);

    std::vector<unsigned char> explode;
    explode.resize(bits + 1);
    std::fill(explode.begin(), explode.end(), 0);

    redhawk::bitops::unpack(&explode[0], &packed[0], offset, bits);

    // Check unpacked against the expected bits
    CPPUNIT_ASSERT_ARRAYS_EQUAL(&expected[offset], &explode[0], bits);

    // Make sure the last value is untouched
    CPPUNIT_ASSERT_EQUAL(0, (int) explode[bits]);
}

void BitopsTest::testUnpackUnalignedSmall()
{
    const unsigned char packed = 0x18; // 00011000
    const unsigned char expected[] = {
        0, 0, 0, 1, 1, 0, 0, 0
    };
    const size_t offset = 2;
    const size_t bits = 4;

    std::vector<unsigned char> explode;
    explode.resize(bits + 1);
    std::fill(explode.begin(), explode.end(), 1);

    redhawk::bitops::unpack(&explode[0], &packed, offset, bits);

    CPPUNIT_ASSERT_ARRAYS_EQUAL(&expected[offset], &explode[0], bits);

    // Make sure the last value is untouched
    CPPUNIT_ASSERT_EQUAL(1, (int) explode[bits]);
}

void BitopsTest::testPopcount()
{
    // Nibble:   0 1 2 3 4 5 6 7 8 9 A B C D E F
    // Popcount: 0 1 1 2 1 2 2 3 1 2 2 3 2 3 3 4
    const unsigned char packed[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0x1F, 0x21, 0x87, 0x68, 0xBA, 0xDD, 0xC0, 0xDE
    };

    CPPUNIT_ASSERT_EQUAL(1, redhawk::bitops::popcount(packed, 0, 8));
    CPPUNIT_ASSERT_EQUAL(3, redhawk::bitops::popcount(packed, 8, 8));
    CPPUNIT_ASSERT_EQUAL(3, redhawk::bitops::popcount(packed, 16, 8));
    CPPUNIT_ASSERT_EQUAL(5, redhawk::bitops::popcount(packed, 24, 8));
    CPPUNIT_ASSERT_EQUAL(3, redhawk::bitops::popcount(packed, 32, 8));
    CPPUNIT_ASSERT_EQUAL(5, redhawk::bitops::popcount(packed, 40, 8));
    CPPUNIT_ASSERT_EQUAL(5, redhawk::bitops::popcount(packed, 48, 8));
    CPPUNIT_ASSERT_EQUAL(7, redhawk::bitops::popcount(packed, 56, 8));

    CPPUNIT_ASSERT_EQUAL(4, redhawk::bitops::popcount(packed, 0, 16));
    CPPUNIT_ASSERT_EQUAL(8, redhawk::bitops::popcount(packed, 16, 16));
    CPPUNIT_ASSERT_EQUAL(8, redhawk::bitops::popcount(packed, 32, 16));
    CPPUNIT_ASSERT_EQUAL(12, redhawk::bitops::popcount(packed, 48, 16));

    CPPUNIT_ASSERT_EQUAL(12, redhawk::bitops::popcount(packed, 0, 32));
    CPPUNIT_ASSERT_EQUAL(20, redhawk::bitops::popcount(packed, 32, 32));

    CPPUNIT_ASSERT_EQUAL(32, redhawk::bitops::popcount(packed, 0, 64));

    CPPUNIT_ASSERT_EQUAL(65, redhawk::bitops::popcount(packed, 0, 128));
}

void BitopsTest::testPopcountUnaligned()
{
    unsigned char packed[] = {
        0xB3, 0x9E, 0x6F // 10110011|10011110|01101111
    };

    // 10(11001)1
    CPPUNIT_ASSERT_EQUAL(3, redhawk::bitops::popcount(packed, 2, 5));

    // 10110(011|1001)1110
    CPPUNIT_ASSERT_EQUAL(4, redhawk::bitops::popcount(packed, 5, 7));

    // 101100(11|10011110)|01101111
    CPPUNIT_ASSERT_EQUAL(7, redhawk::bitops::popcount(packed, 6, 10));

    // 101(10011|10011110|01101)111
    CPPUNIT_ASSERT_EQUAL(11, redhawk::bitops::popcount(packed, 3, 18));
}

void BitopsTest::testToString()
{
    // 10100111|10111001|01000110|11100101
    const unsigned char packed[] = { 0xA7, 0xB9, 0x46, 0xE5 };
    const size_t bits = sizeof(packed) * 8;

    // Test with the full string
    std::string dest;
    dest.resize(bits);
    redhawk::bitops::toString(&dest[0], packed, 0, bits);
    CPPUNIT_ASSERT_EQUAL(std::string("10100111101110010100011011100101"), dest);

    // Test with an offset and a non-byte aligned end
    // 101(00111|10111001|01000110|1110)0101
    dest.resize(bits - 7);
    redhawk::bitops::toString(&dest[0], packed, 3, dest.size());
    CPPUNIT_ASSERT_EQUAL(std::string("0011110111001010001101110"), dest);

    // Test with a small unaligned value
    // 10(10011)1
    dest.resize(5);
    redhawk::bitops::toString(&dest[0], packed, 2, dest.size());
    CPPUNIT_ASSERT_EQUAL(std::string("10011"), dest);
}

void BitopsTest::testParseString()
{
    const std::string str("10011010011110001111001000111110");

    // Test with the full string
    unsigned char packed[4];
    int count = redhawk::bitops::parseString(packed, 0, &str[0], str.size());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not all characters parsed", (int) str.size(), count);

    // 10011010|01111000|11110010|00111110
    const unsigned char expected1[] = { 0x9A, 0x78, 0xF2, 0x3E };
    CPPUNIT_ASSERT_ARRAYS_EQUAL(expected1, packed, 4);

    // Test with an offset and a non-byte aligned end
    // src:     (01111 000|11110 010|001)11 1110
    // dest: 100(11010|011 11000|111 100)10|00111110
    // =     100(01111|000 11110|010 001)10|00111110
    const unsigned char expected2[] = { 0x8F, 0x1E, 0x46, 0x3E };
    count = redhawk::bitops::parseString(packed, 3, &str[8], 19);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not all characters parsed", 19, count);
    CPPUNIT_ASSERT_ARRAYS_EQUAL(expected2, packed, 4);

    // Test with a small unaligned value
    // src:  100(11010)0
    // dest:  10(00111)1
    // =      10(11010)1
    count = redhawk::bitops::parseString(packed, 2, &str[3], 5);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not all characters parsed", 5, count);
    CPPUNIT_ASSERT_EQUAL((unsigned char) 0xB5, packed[0]);
}

void BitopsTest::testParseStringError()
{
    // Invalid string with a letter instead of a number at position 11
    const std::string invalid("01101101001x010101101100");

    // Parsing should stop when it hits the invalid character (tests that the
    // full byte case returns early correctly)
    unsigned char packed[] = { 0x00, 0x0A };
    int count = redhawk::bitops::parseString(packed, 0, &invalid[0], invalid.size());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Returned count does not match invalid character", 11, count);

    // The values up to that point should still be updated
    // src:  (01101101|001)x0101
    // dest: (00000000|000)01010
    // =     (01101101|001)01010
    const unsigned char expected[] = { 0x6D, 0x2A };
    CPPUNIT_ASSERT_ARRAYS_EQUAL(expected, packed, sizeof(expected));

    // Repeat with a partial byte, no offset
    // src:  0110110(1001)x0101
    //               ^     ^
    // dest:        (0110)1101
    // =            (1001)1101
    count = redhawk::bitops::parseString(packed, 0, &invalid[7], 6);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Returned count does not match invalid character", 4, count);
    CPPUNIT_ASSERT_EQUAL((unsigned char) 0x9D, packed[0]);

    // Sub-byte with offset
    // src:  0110110100(1)x0101
    //                  ^    ^
    // dest:         10(0)11101
    // -             10(1)11101
    count = redhawk::bitops::parseString(packed, 2, &invalid[10], 5);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Returned count does not match invalid character", 1, count);
    CPPUNIT_ASSERT_EQUAL((unsigned char) 0xBD, packed[0]);
}

void BitopsTest::testCompare()
{
    const unsigned char first[] = {
        // 10100001|10000011|11110000|10110101
        0xA1, 0x83, 0xF0, 0xB5
    };
    unsigned char second[sizeof(first)];
    std::memcpy(second, first, sizeof(first));

    // 1: 10100001|10000011|11110000|10110101
    // 2: 10100001|10000011|11110000|10110101
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 0, second, 0, 32) == 0);

    // 1: 10100001|10000011|11110000|10110101
    // 2: 10100001|10000011|11110000|10110100
    //                                      ^
    redhawk::bitops::setbit(second, 31, 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 0, second, 0, 32) > 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(second, 0, first, 0, 32) < 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 0, second, 0, 31) == 0);

    // 1: 10100001|10000011|11110000|10110101
    // 2: 10100001|10000011|10110000|10110100
    //                       ^              ^
    redhawk::bitops::setbit(second, 17, 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 0, second, 0, 31) > 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(second, 0, first, 0, 31) < 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 0, second, 0, 17) == 0);

    // 1: 10100001|10000011|11110000|10110101
    // 2: 00100001|10000011|10110000|10110100
    //    ^                  ^              ^
    redhawk::bitops::setbit(second, 0, 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 0, second, 0, 17) > 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(second, 0, first, 0, 17) < 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 1, second, 1, 16) == 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 18, second, 18, 13) == 0);

    // 1: 10100001|10000011|11110000|10110101
    // 2: 00100001|10100011|10110000|10110100
    //    ^          ^       ^              ^
    redhawk::bitops::setbit(second, 10, 1);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 1, second, 1, 10) < 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(second, 1, first, 1, 10) > 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 1, second, 1, 9) == 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 11, second, 11, 4) == 0);

    // 1: 10100001|10000011|11110000|10110101
    // 2: 00100011|10100011|10110000|10110100
    //    ^     ^    ^       ^              ^
    redhawk::bitops::setbit(second, 6, 1);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 1, second, 1, 6) < 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(second, 1, first, 1, 6) > 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 1, second, 1, 5) == 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 7, second, 7, 3) == 0);
}

void BitopsTest::testCompareUnaligned()
{
    const unsigned char first[] = {
        // 01011101|10100011|11101001|01111011
        0x5D, 0xA3, 0xE9, 0x7B
    };
    unsigned char second[] = {
        // rotate right 3
        // 01101011|10110100|01111101|00101111
        0x6B, 0xB4, 0x7D, 0x2F
    };

    // 01011101|10100011|11101001|01111011
    // 01101011|10110100|01111101|00101111
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 0, second, 0, 32) < 0);

    // Offset by 3 to re-align
    //      01011 101|10100 011|11101 001|01111(011)
    // (011)01011|101 10100|011 11101|001 01111
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 0, second, 3, 29) == 0);

    //      01011 101|10100 011|11101 001|01111(011)
    // (011)01011|101 10100|011 11101|001 01110
    //                                        ^
    redhawk::bitops::setbit(second, 31, 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 0, second, 3, 29) > 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(second, 3, first, 0, 29) < 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 0, second, 3, 28) == 0);

    //      01011 101|10100 011|11101 001|01111(011)
    // (011)01011|101 10100|011 10101|001 01110
    //                           ^            ^
    redhawk::bitops::setbit(second, 20, 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 0, second, 3, 28) > 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(second, 3, first, 0, 28) < 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 0, second, 3, 17) == 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 18, second, 21, 10) == 0);

    //      01011 101|10100 011|11101 001|01111(011)
    // (011)11011|101 10100|011 10101|001 01110
    //      ^                    ^            ^
    redhawk::bitops::setbit(second, 3, 1);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 0, second, 3, 17) < 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(second, 3, first, 0, 17) > 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 1, second, 4, 17) != 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 1, second, 4, 16) == 0);

    //      01011 101|10100 011|11101 001|01111(011)
    // (011)11011|101 10000|011 10101|001 01110
    //      ^           ^        ^            ^
    redhawk::bitops::setbit(second, 13, 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 1, second, 4, 16) > 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(second, 4, first, 1, 16) < 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 1, second, 4, 10) != 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 1, second, 4, 9) == 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 11, second, 14, 6) == 0);

    //      01011 101|10100 011|11101 001|01111(011)
    // (011)11001|101 10000|011 10101|001 01110
    //      ^  ^        ^        ^            ^
    redhawk::bitops::setbit(second, 6, 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 1, second, 4, 9) > 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(second, 4, first, 1, 9) < 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 1, second, 4, 3) != 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 1, second, 4, 2) == 0);
    CPPUNIT_ASSERT(redhawk::bitops::compare(first, 4, second, 7, 6) == 0);
}

void BitopsTest::testHammingDistance()
{
    const unsigned char first[] = {
        0x71, 0x0A, 0x68, 0xF2, // 01110001|00001010|01101000|11110010
    };
    unsigned char second[sizeof(first)];
    std::memcpy(second, first, sizeof(first));

    // 01110001|00001010|01101000|11110010
    // 01110001|00001010|01101000|11110010
    CPPUNIT_ASSERT_EQUAL(0, redhawk::bitops::hammingDistance(first, 0, second, 0, 32));

    //   01110001|00001010|01101000|11110010
    //   11110001|00001010|01101000|11110010
    //   ^
    // 1 |---------------------------------|
    // 2  |--------------------------------|
    redhawk::bitops::setbit(second, 0, 1);
    CPPUNIT_ASSERT_EQUAL(1, redhawk::bitops::hammingDistance(first, 0, second, 0, 32));
    CPPUNIT_ASSERT_EQUAL(0, redhawk::bitops::hammingDistance(first, 1, second, 1, 31));

    //   01110001|00001010|01101000|11110010
    //   11110001|01001110|01101000|11110010
    //   ^         ^   ^
    // 1 |---------------------------------|
    // 2  |--------------------------------|
    // 3  |-------|
    redhawk::bitops::setbit(second, 9, 1);
    redhawk::bitops::setbit(second, 13, 1);
    CPPUNIT_ASSERT_EQUAL(3, redhawk::bitops::hammingDistance(first, 0, second, 0, 32));
    CPPUNIT_ASSERT_EQUAL(2, redhawk::bitops::hammingDistance(first, 1, second, 1, 31));
    CPPUNIT_ASSERT_EQUAL(0, redhawk::bitops::hammingDistance(first, 1, second, 1, 8));

    //   01110001|00011010|01101000|11110010
    //   11110001|01011110|01001001|11110010
    //   ^         ^ ^ ^     ^    ^
    // 1 |---------------------------------|
    // 2    |--------------------|
    redhawk::bitops::setbit(second, 11, 1);
    redhawk::bitops::setbit(second, 18, 0);
    redhawk::bitops::setbit(second, 23, 1);
    CPPUNIT_ASSERT_EQUAL(6, redhawk::bitops::hammingDistance(first, 0, second, 0, 32));
    CPPUNIT_ASSERT_EQUAL(4, redhawk::bitops::hammingDistance(first, 3, second, 3, 20));

    //   01110001|00011010|01101000|11110010
    //   11110011|01011110|01001001|00010010
    //   ^     ^   ^ ^ ^     ^    ^ ^^^
    // 1 |---------------------------------|
    // 2        |--------------------|
    // 2                          |---|
    redhawk::bitops::setbit(second, 6, 1);
    redhawk::bitops::setint(second, 24, 0, 3);
    CPPUNIT_ASSERT_EQUAL(10, redhawk::bitops::hammingDistance(first, 0, second, 0, 32));
    CPPUNIT_ASSERT_EQUAL(7, redhawk::bitops::hammingDistance(first, 7, second, 7, 19));
    CPPUNIT_ASSERT_EQUAL(4, redhawk::bitops::hammingDistance(first, 23, second, 23, 4));

    //   01110001|00011010|01101000|11110010
    //   11110011|11100101|01001001|00010010
    //   ^     ^  ^^^^^^^^   ^    ^ ^^^
    // 1 |---------------------------------|
    // 2       |-------------|
    // 3          |------|
    second[1] = ~first[1];
    CPPUNIT_ASSERT_EQUAL(15, redhawk::bitops::hammingDistance(first, 0, second, 0, 32));
    CPPUNIT_ASSERT_EQUAL(10, redhawk::bitops::hammingDistance(first, 6, second, 6, 13));
    CPPUNIT_ASSERT_EQUAL(8, redhawk::bitops::hammingDistance(first, 8, second, 8, 8));

    // Sub-byte tests
    //   01110001
    //   11100010
    //   ^  ^  ^^
    // 1 |-----|
    // 2  |----|
    // 3  |-----|
    redhawk::bitops::setbit(second, 3, 0);
    redhawk::bitops::setbit(second, 7, 0);
    CPPUNIT_ASSERT_EQUAL(3, redhawk::bitops::hammingDistance(first, 0, second, 0, 7));
    CPPUNIT_ASSERT_EQUAL(2, redhawk::bitops::hammingDistance(first, 1, second, 1, 6));
    CPPUNIT_ASSERT_EQUAL(3, redhawk::bitops::hammingDistance(first, 1, second, 1, 7));
}

void BitopsTest::testHammingDistanceUnaligned()
{
    const unsigned char first[] = {
        // 01011101|10100011|11101001|01111011
        0x5D, 0xA3, 0xE9, 0x7B
    };
    unsigned char second[] = {
        // rotate right 5
        // 11011010|11101101|00011111|01001011
        0xDA, 0xED, 0x1F, 0x4B
    };

    //        010 11101|101 00011|111 01001|011(11011)
    // (11011)010|11101 101|00011 111|01001 011
    CPPUNIT_ASSERT_EQUAL(0, redhawk::bitops::hammingDistance(first, 0, second, 5, 27));

    //        010 11101|101 00011|111 01001|011(11011)
    // (11011)010|11101 101|00011 111|01001 010
    //                                        ^
    // 1      |-------------------------------|
    // 2      |------------------------------|
    redhawk::bitops::setbit(second, 31, 0);
    CPPUNIT_ASSERT_EQUAL(1, redhawk::bitops::hammingDistance(first, 0, second, 5, 27));
    CPPUNIT_ASSERT_EQUAL(0, redhawk::bitops::hammingDistance(first, 0, second, 5, 26));

    //        010 11101|101 00011|111 01001|011(11011)
    // (11011)110|11101 101|00011 111|01001 010
    //        ^                               ^
    // 1      |-------------------------------|
    // 2       |-----------------------------|
    redhawk::bitops::setbit(second, 5, 1);
    CPPUNIT_ASSERT_EQUAL(2, redhawk::bitops::hammingDistance(first, 0, second, 5, 27));
    CPPUNIT_ASSERT_EQUAL(0, redhawk::bitops::hammingDistance(first, 1, second, 6, 25));

    //        010 11101|101 00011|111 01001|011(11011)
    // (11011)110|11101 111|01011 111|01001 010
    //        ^          ^   ^                ^
    // 1      |-------------------------------|
    // 2       |-----------------------------|
    // 3      |-------------|
    // 4                      |--------------|
    redhawk::bitops::setbit(second, 14, 1);
    redhawk::bitops::setbit(second, 17, 1);
    CPPUNIT_ASSERT_EQUAL(4, redhawk::bitops::hammingDistance(first, 0, second, 5, 27));
    CPPUNIT_ASSERT_EQUAL(2, redhawk::bitops::hammingDistance(first, 1, second, 6, 25));
    CPPUNIT_ASSERT_EQUAL(2, redhawk::bitops::hammingDistance(first, 0, second, 5, 12));
    CPPUNIT_ASSERT_EQUAL(0, redhawk::bitops::hammingDistance(first, 13, second, 18, 13));

    //        010 11101|101 00011|111 01001|011(11011)
    // (11011)110|00001 111|01011 111|01001 010
    //        ^   ^^^    ^   ^                ^
    // 1      |-------------------------------|
    // 2        |----|
    // 3           |-----|
    // 4      |-|
    redhawk::bitops::setint(second, 8, 0, 3);
    // Full string
    CPPUNIT_ASSERT_EQUAL(7, redhawk::bitops::hammingDistance(first, 0, second, 5, 27));
    // Middle bits from 1st byte of first, split 1st/2nd byte of second
    CPPUNIT_ASSERT_EQUAL(3, redhawk::bitops::hammingDistance(first, 2, second, 7, 5));
    // Split 1st/2nd byte of first, middle bits from 2nd byte of second
    CPPUNIT_ASSERT_EQUAL(3, redhawk::bitops::hammingDistance(first, 4, second, 9, 6));
    // Left bits from 1st byte of first, right bits from 1st byte of second
    CPPUNIT_ASSERT_EQUAL(1, redhawk::bitops::hammingDistance(first, 0, second, 5, 3));
}

void BitopsTest::testCopyAligned()
{
    const unsigned char src[] = {
        // 00101001|10110111|10010110
        0x29, 0xB7, 0x96, 0xA8
    };
    unsigned char dest[sizeof(src)];

    // Single byte copy
    dest[0] = 0;
    redhawk::bitops::copy(dest, 0, src, 0, 8);
    CPPUNIT_ASSERT_EQUAL((int) src[0], (int) dest[0]);

    // Left bits in single byte
    // src:  (00101)001
    // dest: (11100)110
    // =     (00101)110
    dest[0] = 0xE6;
    redhawk::bitops::copy(dest, 0, src, 0, 5);
    CPPUNIT_ASSERT_EQUAL(0x2E,(int) dest[0]);

    // Right bits in single byte
    // src:  0010(1001)
    // dest: 1110(0110)
    // =     1110(1001)
    dest[0] = 0xE6;
    redhawk::bitops::copy(dest, 4, src, 4, 4);
    CPPUNIT_ASSERT_EQUAL(0xE9, (int) dest[0]);

    // Middle bits in single byte
    // src:  001(010)01
    // dest: 101(111)10
    // =     101(010)10
    dest[0] = 0xBE;
    redhawk::bitops::copy(dest, 3, src, 3, 3);
    CPPUNIT_ASSERT_EQUAL(0xAA, (int) dest[0]);

    // Split across 2 bytes: right, full byte
    // src:  0010100(1|10110111)
    // dest: 1001100(0|10011000)
    // =     1001100(1|10110111)
    dest[0] = dest[1] = 0x98;
    redhawk::bitops::copy(dest, 7, src, 7, 9);
    CPPUNIT_ASSERT_EQUAL(0x99, (int) dest[0]);
    CPPUNIT_ASSERT_EQUAL((int) src[1], (int) dest[1]);

    // Split across 2 bytes: full byte, left
    // src:  (00101001|101101)11
    // dest: (10011000|100110)00
    // =     (00101001|101101)00
    dest[0] = dest[1] = 0x98;
    redhawk::bitops::copy(dest, 0, src, 0, 14);
    CPPUNIT_ASSERT_EQUAL((int) src[0], (int) dest[0]);
    CPPUNIT_ASSERT_EQUAL(0xB4, (int) dest[1]);

    // Split across 2 bytes: right, left
    // src:  00101(001|1)0110111
    // dest: 01011(100|0)1011100
    // =     01011(001|1)1011100
    dest[0] = dest[1] = 0x5C;
    redhawk::bitops::copy(dest, 5, src, 5, 4);
    CPPUNIT_ASSERT_EQUAL(0x59, (int) dest[0]);
    CPPUNIT_ASSERT_EQUAL(0xDC, (int) dest[1]);

    // Split across 3 bytes: right, full byte, left
    // src:  00(101001|10110111|100)10110
    // dest: 11(111111|11111111|111)11111
    // =     11(101001|10110111|100)11111
    memset(dest, 0xFF, sizeof(dest));
    redhawk::bitops::copy(dest, 2, src, 2, 17);
    CPPUNIT_ASSERT_EQUAL(0xE9, (int) dest[0]);
    CPPUNIT_ASSERT_EQUAL((int) src[1], (int) dest[1]);
    CPPUNIT_ASSERT_EQUAL(0x9F, (int) dest[2]);
}

void BitopsTest::testCopyUnaligned()
{
    const unsigned char src[] = {
        // 00101001|10110111|00001110
        0x29, 0xB7, 0x0E
    };
    unsigned char dest[] = {
        // 10001111|01100100|10100101
        0x8F, 0x64, 0xA5
    };

    // Middle bits in both
    // src:  00(1010)01
    // dest:  1(0001)111
    // =      1(1010)111
    redhawk::bitops::copy(dest, 1, src, 2, 4);
    CPPUNIT_ASSERT_EQUAL(0xD7, (int) dest[0]);
    CPPUNIT_ASSERT_EQUAL(0x64, (int) dest[1]);
    CPPUNIT_ASSERT_EQUAL(0xA5, (int) dest[2]);

    // Middle bits in src, split in dest
    // src:      0(010 100)1
    // dest: 11010(111|011)00100
    // =     11010(010|100)00100
    redhawk::bitops::copy(dest, 5, src, 1, 6);
    CPPUNIT_ASSERT_EQUAL(0xD2, (int) dest[0]);
    CPPUNIT_ASSERT_EQUAL(0x84, (int) dest[1]);
    CPPUNIT_ASSERT_EQUAL(0xA5, (int) dest[2]);

    // Split in src, middle bits in dest
    // src:  0010(1001|1)0110111
    // dest:   11(0100 1)0
    // =       11(1001 1)0
    redhawk::bitops::copy(dest, 2, src, 4, 5);
    CPPUNIT_ASSERT_EQUAL(0xE6, (int) dest[0]);
    CPPUNIT_ASSERT_EQUAL(0x84, (int) dest[1]);
    CPPUNIT_ASSERT_EQUAL(0xA5, (int) dest[2]);

    // src:    00(1010 01|101101)11
    // dest: 1110(0110|10 000100)
    // =     1110(1010|01 101101)
    redhawk::bitops::copy(dest, 4, src, 2, 12);
    CPPUNIT_ASSERT_EQUAL(0xEA, (int) dest[0]);
    CPPUNIT_ASSERT_EQUAL(0x6D, (int) dest[1]);
    CPPUNIT_ASSERT_EQUAL(0xA5, (int) dest[2]);

    // src:     001(01 001|10110 111|0)0001110
    // dest: 110110(10|011 01101|101 0)0101
    // =     110110(01|001 10110|111 0)0101
    redhawk::bitops::copy(dest, 6, src, 3, 14);
    CPPUNIT_ASSERT_EQUAL(0xE9, (int) dest[0]);
    CPPUNIT_ASSERT_EQUAL(0x36, (int) dest[1]);
    CPPUNIT_ASSERT_EQUAL(0xE5, (int) dest[2]);
}

void BitopsTest::testCopyLarge()
{
    // Use a source vector with a predictable bit pattern (01010101...)
    std::vector<unsigned char> src;
    src.resize(253);
    std::fill(src.begin(), src.end(), 0x55);
    std::vector<unsigned char> dest;
    dest.resize(src.size());
    const size_t bits = src.size() * 8;

    // Aligned bytewise copy
    std::fill(dest.begin(), dest.end(), 0);
    redhawk::bitops::copy(&dest[0], 0, &src[0], 0, bits);
    CPPUNIT_ASSERT_ARRAYS_EQUAL(&src[0], &dest[0], src.size());

    // Aligned copy, 4-bit offset
    std::fill(dest.begin(), dest.end(), 0xAA);
    redhawk::bitops::copy(&dest[0], 4, &src[0], 4, bits-8);
    CPPUNIT_ASSERT_EQUAL(0xA5, (int) dest[0]);
    CPPUNIT_ASSERT_ARRAYS_EQUAL(&src[1], &dest[1], src.size() - 2);
    CPPUNIT_ASSERT_EQUAL(0x5A, (int) dest[dest.size()-1]);

    // Unaligned copy, shifting by 3 bits turns 01010101 into 10101010
    // dest:  01000001
    // first: 01(101010)
    // last:  (10101)001
    std::fill(dest.begin(), dest.end(), 0x41);
    redhawk::bitops::copy(&dest[0], 2, &src[0], 5, bits-5);
    CPPUNIT_ASSERT_EQUAL(0x6A, (int) dest[0]);
    for (size_t pos = 1; pos < (dest.size() - 1); ++pos) {
        CPPUNIT_ASSERT_EQUAL(0xAA, (int) dest[pos]);
    }
    CPPUNIT_ASSERT_EQUAL(0xA9, (int) dest[dest.size()-1]);
}

void BitopsTest::testFind()
{
    // String: 10101010...
    const size_t string_bits = 253;
    std::vector<unsigned char> buf;
    buf.resize((string_bits + 7) / 8);
    std::fill(buf.begin(), buf.end(), 0xAA);
    unsigned char* string = &buf[0];

    // Pattern: 10110011|0000111x
    const size_t pattern_bits = 15;
    const unsigned char pattern[] = { 0xB3, 0x0E };

    // Pre-condition: the pattern should not match the string as-is
    CPPUNIT_ASSERT_EQUAL(-1, redhawk::bitops::find(string, 0, string_bits, pattern, 0, pattern_bits, 0));

    // Copy the pattern into the string four times, with different numbers
    // of errors; the higher error counts go first, so that stricter max
    // distance values will ignore them.
    const int three_errors = 16;
    redhawk::bitops::copy(string, three_errors, pattern, 0, pattern_bits);
    _flipBit(string, three_errors + 2);
    _flipBit(string, three_errors + 8);
    _flipBit(string, three_errors + 11);

    const int two_errors = 59;
    redhawk::bitops::copy(string, two_errors, pattern, 0, pattern_bits);
    _flipBit(string, two_errors + 4);
    _flipBit(string, two_errors + 5);

    const int one_error = 126;
    redhawk::bitops::copy(string, one_error, pattern, 0, pattern_bits);
    _flipBit(string, one_error + 14);

    const int exact = 200;
    redhawk::bitops::copy(string, exact, pattern, 0, pattern_bits);

    // Increasing tolerances should find the earlier occurrences
    CPPUNIT_ASSERT_EQUAL(exact, redhawk::bitops::find(string, 0, string_bits, pattern, 0, pattern_bits, 0));
    CPPUNIT_ASSERT_EQUAL(one_error, redhawk::bitops::find(string, 0, string_bits, pattern, 0, pattern_bits, 1));
    CPPUNIT_ASSERT_EQUAL(two_errors, redhawk::bitops::find(string, 0, string_bits, pattern, 0, pattern_bits, 2));
    CPPUNIT_ASSERT_EQUAL(three_errors, redhawk::bitops::find(string, 0, string_bits, pattern, 0, pattern_bits, 3));

    // Exclude the exact match at the end
    CPPUNIT_ASSERT_EQUAL(-1, redhawk::bitops::find(string, 0, exact, pattern, 0, pattern_bits, 0));

    // Exclude the three-error match at the beginning
    CPPUNIT_ASSERT_EQUAL(two_errors, redhawk::bitops::find(string, three_errors + pattern_bits, string_bits,
                                                  pattern, 0, pattern_bits, 3));
}

void BitopsTest::testTakeSkip()
{
    // Use a non byte-aligned starting offset and a repeating pattern of an
    // irregular length, where the discarded part is disjoint
    // 10000100001/1001 = 0x4219
    size_t patt_len = 15;
    const size_t src_start = 5;
    size_t src_bits = 5 * patt_len;
    std::vector<unsigned char> src((src_start + src_bits + 7) / 8);
    for (size_t pos = src_start; pos < (src_start+src_bits); pos += patt_len) {
        redhawk::bitops::setint(&src[0], pos, 0x4219, patt_len);
    }

    // Take 11 and skip 4 for the extent of the input string, using a different
    // non byte-aligned starting offset
    const size_t take_len = 11;
    const size_t dest_start = 3;
    const size_t dest_bits = 5 * take_len;

    // Allocate memory and intialize with alternating 0/1 bit pattern
    std::vector<unsigned char> dest((dest_start + dest_bits +7) / 8, 0x55);

    // Ensure that the take/skip copies the correct number of bits, and that
    // the destination now contains the first 11 bits of the pattern repeated
    size_t count = redhawk::bitops::takeskip(&dest[0], dest_start, &src[0], src_start, src_bits, take_len, 4);
    CPPUNIT_ASSERT_EQUAL(dest_bits, count);
    for (size_t pos = dest_start; pos < (dest_start+dest_bits); pos += take_len) {
        CPPUNIT_ASSERT_EQUAL((uint64_t) 0x421, redhawk::bitops::getint(&dest[0], pos, take_len));
    }

    // Make sure it didn't disturb any existing values
    // front: 010xxxxx
    // back:  xx010101
    CPPUNIT_ASSERT_EQUAL((uint64_t) 2, redhawk::bitops::getint(&dest[0], 0, dest_start));
    CPPUNIT_ASSERT_EQUAL((uint64_t) 0x15, redhawk::bitops::getint(&dest[0], dest_start+dest_bits, 6));

    // Use a source length that truncates the last take and verify that only
    // bits up to that point are taken
    // src  = (10000100001100110000)10000
    //        (10000xxx00110xxx0000) = 10000001100000
    // dest = (01010101010101)01
    //        (10000001100000)xx
    //         10000001100000 01 = 0x8181
    std::fill(dest.begin(), dest.end(), 0x55);
    count = redhawk::bitops::takeskip(&dest[0], 0, &src[0], src_start, 20, 5, 3);
    CPPUNIT_ASSERT_EQUAL((size_t) 14, count);
    CPPUNIT_ASSERT_EQUAL((uint64_t) 0x8181, redhawk::bitops::getint(&dest[0], 0, 16));
}

void BitopsTest::_flipBit(unsigned char* buffer, size_t offset)
{
    redhawk::bitops::setbit(buffer, offset, !(redhawk::bitops::getbit(buffer, offset)));
}

CPPUNIT_TEST_SUITE_REGISTRATION(BitopsTest);
