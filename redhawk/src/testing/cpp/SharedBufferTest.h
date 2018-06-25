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

#ifndef SHAREDBUFFERTEST_H
#define SHAREDBUFFERTEST_H

#include "CFTest.h"

class SharedBufferTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(SharedBufferTest);
    CPPUNIT_TEST(testDefaultConstructor);
    CPPUNIT_TEST(testConstructor);
    CPPUNIT_TEST(testMakeBuffer);
    CPPUNIT_TEST(testEquals);
    CPPUNIT_TEST(testIteration);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST(testSwap);
    CPPUNIT_TEST(testResize);
    CPPUNIT_TEST(testSharing);
    CPPUNIT_TEST(testSlicing);
    CPPUNIT_TEST(testTrim);
    CPPUNIT_TEST(testTrimShared);
    CPPUNIT_TEST(testReplace);
    CPPUNIT_TEST(testRecast);
    CPPUNIT_TEST(testAllocator);
    CPPUNIT_TEST(testAllocatorCopy);
    CPPUNIT_TEST(testCustomDeleter);
    CPPUNIT_TEST(testTransient);
    CPPUNIT_TEST(testGetMemory);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    // Constructors
    void testDefaultConstructor();
    void testConstructor();
    void testMakeBuffer();

    // Basic container behavior
    void testEquals();
    void testIteration();
    void testCopy();
    void testSwap();
    void testResize();

    // Extended container behavior
    void testSharing();
    void testSlicing();
    void testTrim();
    void testTrimShared();
    void testReplace();
    void testRecast();

    // Advanced features
    void testAllocator();
    void testAllocatorCopy();
    void testCustomDeleter();
    void testTransient();
    void testGetMemory();

private:
    template <class Buffer>
    void testTrimImpl();
};

#endif // SHARED_BUFFER_TEST_H
