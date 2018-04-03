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

#ifndef BITBUFFERTEST_H
#define BITBUFFERTEST_H

#include "CFTest.h"

#include <ossie/bitbuffer.h>

class BitBufferTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(BitBufferTest);
    CPPUNIT_TEST(testDefaultConstructor);
    CPPUNIT_TEST(testConstructor);
    CPPUNIT_TEST(testFromInt);
    CPPUNIT_TEST(testFromArray);
    CPPUNIT_TEST(testFromString);
    CPPUNIT_TEST(testFromUnpacked);
    CPPUNIT_TEST(testEquals);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST(testSwap);
    CPPUNIT_TEST(testResize);
    CPPUNIT_TEST(testFill);
    CPPUNIT_TEST(testIndexAccess);
    CPPUNIT_TEST(testIndexAssignment);
    CPPUNIT_TEST(testSharing);
    CPPUNIT_TEST(testSlicing);
    CPPUNIT_TEST(testTrim);
    CPPUNIT_TEST(testReplace);
    CPPUNIT_TEST(testGetInt);
    CPPUNIT_TEST(testSetInt);
    CPPUNIT_TEST(testPopcount);
    CPPUNIT_TEST(testDistance);
    CPPUNIT_TEST(testFind);
    CPPUNIT_TEST(testTakeSkip);
    CPPUNIT_TEST(testTakeSkipIntoBuffer);
    CPPUNIT_TEST_SUITE_END();

    typedef redhawk::shared_bitbuffer::data_type data_type;

public:
    void testDefaultConstructor();
    void testConstructor();
    void testFromInt();
    void testFromArray();
    void testFromString();
    void testFromUnpacked();

    void testEquals();
    void testCopy();
    void testSwap();
    void testResize();
    void testFill();

    void testIndexAccess();
    void testIndexAssignment();

    void testSharing();
    void testSlicing();
    void testTrim();
    void testReplace();

    // Bit-specific operations
    void testGetInt();
    void testSetInt();
    void testPopcount();
    void testDistance();
    void testFind();
    void testTakeSkip();
    void testTakeSkipIntoBuffer();
};

#endif // BITBUFFER_TEST_H
