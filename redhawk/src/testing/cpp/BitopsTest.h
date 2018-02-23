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

#ifndef BITOPSTEST_H
#define BITOPSTEST_H

#include "CFTest.h"

class BitopsTest : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE(BitopsTest);
    CPPUNIT_TEST(testGetBit);
    CPPUNIT_TEST(testSetBit);
    CPPUNIT_TEST(testGetInt);
    CPPUNIT_TEST(testGetIntUnaligned);
    CPPUNIT_TEST(testGetIntUnalignedSmall);
    CPPUNIT_TEST(testSetInt);
    CPPUNIT_TEST(testSetIntUnaligned);
    CPPUNIT_TEST(testSetIntUnalignedSmall);
    CPPUNIT_TEST(testFill);
    CPPUNIT_TEST(testPack);
    CPPUNIT_TEST(testPackSmall);
    CPPUNIT_TEST(testPackUnaligned);
    CPPUNIT_TEST(testPackUnalignedSmall);
    CPPUNIT_TEST(testUnpack);
    CPPUNIT_TEST(testUnpackSmall);
    CPPUNIT_TEST(testUnpackUnaligned);
    CPPUNIT_TEST(testUnpackUnalignedSmall);
    CPPUNIT_TEST(testPopcount);
    CPPUNIT_TEST(testPopcountUnaligned);
    CPPUNIT_TEST(testToString);
    CPPUNIT_TEST(testParseString);
    CPPUNIT_TEST(testParseStringError);
    CPPUNIT_TEST(testCompare);
    CPPUNIT_TEST(testCompareUnaligned);
    CPPUNIT_TEST(testHammingDistance);
    CPPUNIT_TEST(testHammingDistanceUnaligned);
    CPPUNIT_TEST(testCopyAligned);
    CPPUNIT_TEST(testCopyUnaligned);
    CPPUNIT_TEST(testCopyLarge);
    CPPUNIT_TEST(testFind);
    CPPUNIT_TEST(testTakeSkip);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testGetBit();
    void testSetBit();

    void testGetInt();
    void testGetIntUnaligned();
    void testGetIntUnalignedSmall();

    void testSetInt();
    void testSetIntUnaligned();
    void testSetIntUnalignedSmall();

    void testFill();

    void testPack();
    void testPackSmall();
    void testPackUnaligned();
    void testPackUnalignedSmall();

    void testUnpack();
    void testUnpackSmall();
    void testUnpackUnaligned();
    void testUnpackUnalignedSmall();

    void testPopcount();
    void testPopcountUnaligned();

    void testToString();
    void testParseString();
    void testParseStringError();

    void testCompare();
    void testCompareUnaligned();

    void testHammingDistance();
    void testHammingDistanceUnaligned();

    void testCopyAligned();
    void testCopyUnaligned();
    void testCopyLarge();

    void testFind();
    void testTakeSkip();

private:
    void _flipBit(unsigned char* buffer, size_t offset);
};

#endif // BITOPSTEST_H
