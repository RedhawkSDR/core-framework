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

#ifndef VALUETEST_H
#define VALUETEST_H

#include "CFTest.h"

class ValueTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(ValueTest);
    CPPUNIT_TEST(testConstructor);
    CPPUNIT_TEST(testCopyConstructor);
    CPPUNIT_TEST(testType);
    CPPUNIT_TEST(testNumericConversion);
    CPPUNIT_TEST(testStringConversion);
    CPPUNIT_TEST(testConstCast);
    CPPUNIT_TEST(testCast);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testConstructor();
    void testCopyConstructor();

    void testType();

    void testNumericConversion();
    void testStringConversion();

    void testConstCast();
    void testCast();
};

#endif // VALUETEST_H
