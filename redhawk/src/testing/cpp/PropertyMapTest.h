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

#ifndef PROPERTYMAPTEST_H
#define PROPERTYMAPTEST_H

#include "CFTest.h"

class PropertyMapTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(PropertyMapTest);
    CPPUNIT_TEST(testDefaultConstructor);
    CPPUNIT_TEST(testPropertiesConstructor);
    CPPUNIT_TEST(testPropertyTypeFromAny);
    CPPUNIT_TEST(testConstCast);
    CPPUNIT_TEST(testCast);
    CPPUNIT_TEST(testPushBack);
    CPPUNIT_TEST(testConstIndexing);
    CPPUNIT_TEST(testMutableIndexing);
    CPPUNIT_TEST(testConstMapping);
    CPPUNIT_TEST(testMutableMapping);
    CPPUNIT_TEST(testConstFind);
    CPPUNIT_TEST(testMutableFind);
    CPPUNIT_TEST(testConstIteration);
    CPPUNIT_TEST(testMutableIteration);
    CPPUNIT_TEST(testUpdate);
    CPPUNIT_TEST(testErase);
    CPPUNIT_TEST(testGet);
    CPPUNIT_TEST(testToString);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testDefaultConstructor();
    void testPropertiesConstructor();
    void testPropertyTypeFromAny();

    void testConstCast();
    void testCast();

    void testPushBack();

    void testContains();

    void testConstIndexing();
    void testMutableIndexing();

    void testConstMapping();
    void testMutableMapping();

    void testConstFind();
    void testMutableFind();

    void testConstIteration();
    void testMutableIteration();

    void testUpdate();
    void testErase();

    void testGet();

    void testToString();
};

#endif // PROPERTYMAPTEST_H
