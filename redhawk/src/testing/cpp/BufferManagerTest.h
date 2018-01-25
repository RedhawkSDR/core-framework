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

#ifndef BUFFERMANAGERTEST_H
#define BUFFERMANAGERTEST_H

#include <set>

#include "CFTest.h"

#include <ossie/BufferManager.h>

class BufferManagerTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(BufferManagerTest);
    CPPUNIT_TEST(testBasicAllocate);
    CPPUNIT_TEST(testAllocator);
    CPPUNIT_TEST(testEnable);
    CPPUNIT_TEST(testStatistics);
    CPPUNIT_TEST(testThreading);
    CPPUNIT_TEST(testPolicyBytes);
    CPPUNIT_TEST(testPolicyBlocks);
    CPPUNIT_TEST(testPolicyAge);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testBasicAllocate();
    void testAllocator();

    void testEnable();

    void testStatistics();

    void testThreading();

    void testPolicyBytes();
    void testPolicyBlocks();
    void testPolicyAge();

private:
    typedef std::set<void*> BufferList;

    void* _allocate(size_t bytes);
    void _deallocate(void* ptr);

    void _fillCache(size_t count, size_t bufferSize);

    redhawk::BufferManager* _manager;

    BufferList _allocations;
};

#endif // BUFFERMANAGER_TEST_H
